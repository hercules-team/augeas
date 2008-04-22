/*
 * augtool.c: 
 *
 * Copyright (C) 2007, 2008 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: David Lutterkort <dlutter@redhat.com>
 */

#include "augeas.h"
#include "internal.h"
#include "config.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <argz.h>

struct command {
    const char *name;
    int nargs;
    void(*handler) (char *args[]);
    const char *synopsis;
    const char *help;
};

static struct command commands[];

static augeas *aug = NULL;
static const char *const progname = "augtool";
static unsigned int flags = AUG_NONE;
const char *root = NULL;
char *loadpath = NULL;

static char *cleanpath(char *path) {
    char *e = path + strlen(path) - 1;
    while (e != path && (*e == SEP || isspace(*e)))
        *e-- = '\0';
    return path;
}

/*
 * Dup PATH and split it into a directory and basename. The returned value
 * points to the copy of PATH. Adding strlen(PATH)+1 to it gives the
 * basename.
 *
 * If PATH can not be split, returns NULL
 */
ATTRIBUTE_UNUSED
static char *pathsplit(const char *path) {
    char *ppath = strdup(path);
    char *pend = strrchr(ppath, SEP);

    if (pend == NULL || pend == ppath) {
        free(ppath);
        return NULL;
    }
    *pend = '\0';
    return ppath;
}

static char *ls_pattern(const char *path) {
    char *q;
    int r;

    if (path[strlen(path)-1] == SEP)
        r = asprintf(&q, "%s*", path);
    else
        r = asprintf(&q, "%s/*", path);
    if (r == -1)
        return NULL;
    return q;
}

static int child_count(const char *path) {
    char *q = ls_pattern(path);
    int cnt;

    if (q == NULL)
        return 0;
    cnt = aug_match(aug, q, NULL);
    free(q);
    return cnt;
}

static void cmd_ls(char *args[]) {
    int cnt;
    char *path = cleanpath(args[0]);
    char **paths;

    path = ls_pattern(path);
    if (path == NULL)
        return;
    cnt = aug_match(aug, path, &paths);
    for (int i=0; i < cnt; i++) {
        const char *val;
        const char *basnam = strrchr(paths[i], SEP);
        int dir = child_count(paths[i]);
        aug_get(aug, paths[i], &val);
        basnam = (basnam == NULL) ? paths[i] : basnam + 1;
        if (val == NULL)
            val = "(none)";
        printf("%s%s= %s\n", basnam, dir ? "/ " : " ", val);
        free(paths[i]);
    }
    if (cnt > 0)
        free(paths);
}

static void cmd_match(char *args[]) {
    int cnt;
    const char *pattern = cleanpath(args[0]);
    char **matches;
    int filter = (args[1] != NULL) && (strlen(args[1]) > 0);

    cnt = aug_match(aug, pattern, &matches);
    if (cnt == -1) {
        printf("  (error matching %s)\n", pattern);
        return;
    }
    if (cnt == 0) {
        printf("  (no matches)\n");
        return;
    }

    for (int i=0; i < cnt; i++) {
        const char *val;
        aug_get(aug, matches[i], &val);
        if (val == NULL)
            val = "(none)";
        if (filter) {
            if (STREQ(args[1], val))
                printf("%s\n", matches[i]);
        } else {
            printf("%s = %s\n", matches[i], val);
        }
        free((void *) matches[i]);
    }
    free(matches);
}

static void cmd_rm(char *args[]) {
    int cnt;
    const char *path = cleanpath(args[0]);
    printf("rm : %s", path);
    cnt = aug_rm(aug, path);
    printf(" %d\n", cnt);
}

static void cmd_set(char *args[]) {
    const char *path = cleanpath(args[0]);
    const char *val = args[1];
    int r;

    r = aug_set(aug, path, val);
    if (r == -1)
        printf ("Failed\n");
}

static void cmd_clear(char *args[]) {
    const char *path = cleanpath(args[0]);
    int r;

    r = aug_set(aug, path, NULL);
    if (r == -1)
        printf ("Failed\n");
}

static void cmd_get(char *args[]) {
    const char *path = cleanpath(args[0]);
    const char *val;

    printf("%s", path);
    if (aug_get(aug, path, &val) != 1) {
        printf(" (o)\n");
        return;
    } else if (val == NULL) {
        val = "(none)";
    } else {
        printf(" = %s\n", val);
    }
}

static void cmd_print(char *args[]) {
    aug_print(aug, stdout, cleanpath(args[0]));
}

static void cmd_save(ATTRIBUTE_UNUSED char *args[]) {
    int r;
    r = aug_save(aug);
    if (r == -1) {
        printf("Saving failed\n");
    }
}

static void cmd_ins(char *args[]) {
    const char *label = args[0];
    const char *where = args[1];
    const char *path = cleanpath(args[2]);
    int before;
    int r;

    if (STREQ(where, "after"))
        before = 0;
    else if (STREQ(where, "before"))
        before = 1;
    else {
        printf("The <WHERE> argument must be either 'before' or 'after'.");
        return;
    }

    r = aug_insert(aug, path, label, before);
    if (r == -1)
        printf ("Failed\n");
}

static void cmd_help(ATTRIBUTE_UNUSED char *args[]) {
    struct command *c;

    printf("Commands:\n\n");
    printf("    exit, quit\n        Exit the program\n\n");
    for (c=commands; c->name != NULL; c++) {
        printf("    %s\n        %s\n\n", c->synopsis, c->help);
    }
    printf("\nEnvironment:\n\n");
    printf("    AUGEAS_ROOT\n        the file system root, defaults to '/'\n\n");
    printf("    AUGEAS_LENS_LIB\n        colon separated list of directories with lenses,\n\
        defaults to " AUGEAS_LENS_DIR "\n\n");
}

static int chk_args(const char *cmd, int n, char *args[]) {
    for (int i=0; i<n; i++) {
        if (strlen(args[i]) == 0) {
            fprintf(stderr, "Not enough arguments for %s\n", cmd);
            return -1;
        }
    }
    return 0;
}

static char *nexttoken(char **line) {
    char *r, *s;

    s = *line;

    while (*s && isblank(*s)) s+= 1;
    r = s;
    while (*s && !isblank(*s)) s+= 1;
    if (*s)
        *s++ = '\0';
    *line = s;
    return r;
}

static char *parseline(char *line, char *args[], int argc) {
    char *cmd;

    line = strdup(line);

    cmd = nexttoken(&line);

    for (int i=0; i < argc; i++) {
        args[i] = nexttoken(&line);
    }

    return cmd;
}

static struct command commands[] = {
    { "ls",  1, cmd_ls, "ls <PATH>",
      "List the direct children of PATH"
    },
    { "match",  1, cmd_match, "match <PATTERN> [<VALUE>]",
      "Find all paths that match PATTERN (according to fnmatch(3)). If\n"
      "        VALUE is given, only the matching paths whose value equals VALUE\n"
      "        are printed"
    },
    { "rm",  1, cmd_rm, "rm <PATH>",
      "Delete PATH and all its children from the tree"
    },
    { "set", 2, cmd_set, "set <PATH> <VALUE>",
      "Associate VALUE with PATH. If PATH is not in the tree yet,\n"
      "        it and all its ancestors will be created. These new tree entries\n"
      "        will appear last amongst their siblings"
    },
    { "clear", 1, cmd_clear, "clear <PATH>",
      "Set the value for PATH to NULL. If PATH is not in the tree yet,\n"
      "        it and all its ancestors will be created. These new tree entries\n"
      "        will appear last amongst their siblings"
    },
    { "get", 1, cmd_get, "get <PATH>",
      "Print the value associated with PATH"
    },
    { "print", 0, cmd_print, "print [<PATH>]",
      "Print entries in the tree. If PATH is given, printing starts there,\n"
      "        otherwise the whole tree is printed"
    },
    { "ins", 3, cmd_ins, "ins <LABEL> <WHERE> <PATH>",
      "Insert a new node with label LABEL right before or after PATH into\n"
     "        the tree. WHERE must be either 'before' or 'after'."
    },
    { "save", 0, cmd_save, "save",
      "Save all pending changes to disk. For now, files are not overwritten.\n"
      "        Instead, new files with extension .augnew are created"
    },
    { "help", 0, cmd_help, "help",
      "Print this help text"
    },
    { NULL, -1, NULL, NULL, NULL }
};

static int run_command(char *cmd, char **args) {
    int r = 0;
    struct command *c;

    if (STREQ("exit", cmd) || STREQ("quit", cmd)) {
        exit(0);
    }
    for (c = commands; c->name; c++) {
        if (STREQ(cmd, c->name))
            break;
    }
    if (c->name) {
        r = chk_args(cmd, c->nargs, args);
        if (r == 0) {
            (*c->handler)(args);
        }
    } else {
        fprintf(stderr, "Unknown command '%s'\n", cmd);
        r = -1;
    }

    return r;
}

static char *readline_path_generator(const char *text, int state) {
    static int current = 0;
    static char **children = NULL;
    static int nchildren = 0;

    if (state == 0) {
        char *end = strrchr(text, SEP);
        char *path;
        if (end == NULL)
            path = strdup("/*");
        else {
            end += 1;
            CALLOC(path, end - text + 2);
            strncpy(path, text, end - text);
            strcat(path, "*");
        }

        for (;current < nchildren; current++)
            free((void *) children[current]);
        free((void *) children);
        nchildren = aug_match(aug, path, &children);
        current = 0;
        free(path);
    }

    while (current < nchildren) {
        char *child = children[current];
        current += 1;
        if (STREQLEN(child, text, strlen(text))) {
            if (child_count(child) > 0) {
                child = realloc(child, strlen(child)+2);
                strcat(child, "/");
            }
            rl_filename_completion_desired = 1;
            rl_completion_append_character = '\0';
            return child;
        } else {
            free(child);
        }
    }
    return NULL;
}

static char *readline_command_generator(const char *text, int state) {
    static int current = 0;
    const char *name;

    if (state == 0)
        current = 0;

    rl_completion_append_character = ' ';
    while ((name = commands[current].name) != NULL) {
        current += 1;
        if (STREQLEN(text, name, strlen(text)))
            return strdup(name);
    }
    return NULL;
}

static char **readline_completion(const char *text, int start,
                                  ATTRIBUTE_UNUSED int end) {
    if (start == 0)
        return rl_completion_matches(text, readline_command_generator);
    else
        return rl_completion_matches(text, readline_path_generator);

    return NULL;
}

static void readline_init(void) {
    rl_readline_name = "augtool";
    rl_attempted_completion_function = readline_completion;
    rl_completion_entry_function = readline_path_generator;
}

__attribute__((noreturn))
static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] [COMMAND]\n", progname);
    fprintf(stderr, "Load the Augeas tree and modify it. If no COMMAND is given, run interactively\n");
    fprintf(stderr, "Run '%s help' to get a list of possible commands.\n",
            progname);
    fprintf(stderr, "\nOptions:\n\n");
    fprintf(stderr, "  -c            Typecheck lenses. This can be very slow, and is therefore not\n"
                    "                done by default, but is highly recommended during development.\n");
    fprintf(stderr, "  -b            When files are changed, preserve the originals in a file\n"
                    "                with extension '.augsave'\n");
    fprintf(stderr, "  -n            Save changes in files with extension '.augnew', do not modify\n"
                    "                the original files\n");
    fprintf(stderr, "  -r ROOT       Use directory ROOT as the root of the filesystem\n");
    fprintf(stderr, "  -I DIR        Add DIR to the module loadpath. Can be given multiple times.\n");
    exit(EXIT_FAILURE);
}

static void parse_opts(int argc, char **argv) {
    int opt;
    size_t loadpathlen = 0;

    while ((opt = getopt(argc, argv, "hnbcr:I:")) != -1) {
        switch(opt) {
        case 'c':
            flags |= AUG_TYPE_CHECK;
            break;
        case 'b':
            flags |= AUG_SAVE_BACKUP;
            break;
        case 'n':
            flags |= AUG_SAVE_NEWFILE;
            break;
        case 'h':
            usage();
            break;
        case 'r':
            root = optarg;
            break;
        case 'I':
            argz_add(&loadpath, &loadpathlen, optarg);
            break;
        default:
            usage();
            break;
        }
    }
    argz_stringify(loadpath, loadpathlen, PATH_SEP_CHAR);
}

int main(int argc, char **argv) {
    char *line;
    char *cmd, *args[3];
    int r;

    parse_opts(argc, argv);

    aug = aug_init(root, loadpath, flags);
    if (aug == NULL) {
        fprintf(stderr, "Failed to initialize Augeas\n");
        exit(EXIT_FAILURE);
    }
    readline_init();
    if (optind < argc) {
        // Accept one command from the command line
        r = run_command(argv[optind], argv+optind+1);
        return r;
    }

    while (1) {
        line = readline("augtool> ");
        if (line == NULL) {
            printf("\n");
            return 0;
        }

        cmd = parseline(line, args, 3);
        if (strlen(cmd) > 0) {
            run_command(cmd, args);
            add_history(line);
        }
    }
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
