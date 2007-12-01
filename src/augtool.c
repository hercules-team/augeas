/*
 * augtool.c: 
 *
 * Copyright (C) 2007 Red Hat Inc.
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

#include <readline/readline.h>
#include <readline/history.h>

struct command {
    const char *name;
    int nargs;
    void(*handler) (char *args[]);
    const char *synopsis;
    const char *help;
};

static struct command commands[];

static void cmd_ls(char *args[]) {
    int cnt;
    const char *path = args[0];
    const char **paths;

    cnt = aug_ls(path, &paths);
    for (int i=0; i < cnt; i++) {
        const char *val = aug_get(paths[i]);
        if (val == NULL)
            val = "(none)";
        printf("%s = %s\n", paths[i] + strlen(path), val);
    }
    free(paths);
}

static void cmd_match(char *args[]) {
    int cnt;
    const char *pattern = args[0];
    const char **matches;
    int filter = (strlen(args[1]) > 0);

    cnt = aug_match(pattern, NULL, 0);
    if (cnt == 0) {
        printf("  (no matches)\n");
        return;
    }
        
    matches = calloc(cnt, sizeof(char *));
    cnt = aug_match(pattern, matches, cnt);
    for (int i=0; i < cnt; i++) {
        const char *val = aug_get(matches[i]);
        if (val == NULL)
            val = "(none)";
        if (filter) {
            if (STREQ(args[1], val))
                printf("%s\n", matches[i]);
        } else {
            printf("%s = %s\n", matches[i], val);
        }
    }
    free(matches);
}

static void cmd_rm(char *args[]) {
    int cnt;
    const char *path = args[0];
    printf("rm : %s", path);
    cnt = aug_rm(path);
    printf(" %d\n", cnt);
}

static void cmd_set(char *args[]) {
    const char *path = args[0];
    const char *val = args[1];
    int r;

    r = aug_set(path, val);
    if (r == -1)
        printf ("Failed\n");
}

static void cmd_get(char *args[]) {
    const char *path = args[0];
    const char *val;

    printf("%s", path);
    if (! aug_exists(path)) {
        printf(" (o)\n");
        return;
    }
    val = aug_get(path);
    if (val == NULL)
        val = "(none)";
    printf(" = %s\n", val);
}

static void cmd_print(ATTRIBUTE_UNUSED char *args[]) {
    aug_print(stdout, args[0]);
}

static void cmd_save(ATTRIBUTE_UNUSED char *args[]) {
    int r;
    r = aug_save();
    if (r == -1) {
        printf("Saving failed\n");
    }
}

static void cmd_ins(char *args[]) {
    const char *path = args[0];
    const char *sibling = args[1];
    int r;

    r = aug_insert(path, sibling);
    if (r == -1)
        printf ("Failed\n");
}

static void cmd_help(ATTRIBUTE_UNUSED char *args[]) {
    struct command *c;

    printf ("  augtool [COMMAND OPTIONS]\n\n");
    printf ("      Inspect the configuration tree. Without arguments, "
            "run interactively.\n"
            "      With arguments, execute that command, print its output and exit.\n\n");

    printf("Subcommands:\n\n");
    printf("    exit, quit\n        Exit the program\n\n");
    for (c=commands; c->name != NULL; c++) {
        printf("    %s\n        %s\n\n", c->synopsis, c->help);
    }
    printf("\n    Note that all config files are loaded"
           " from underneath " ROOT_DIR " not '/'\n\n");
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
      "        are printed\n"
    },
    { "rm",  1, cmd_rm, "rm <PATH>",
      "Delete PATH and all its children from the tree"
    },
    { "set", 2, cmd_set, "set <PATH> <VALUE>",
      "Associate VALUE with PATH. If PATH is not in the tree yet,\n"
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
    { "ins", 2, cmd_ins, "ins <PATH> <SIBLING>",
      "Insert PATH right before SIBLING into the tree."
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

int main(int argc, char **argv) {
    char *line;
    char *cmd, *args[3];
    int r;

    aug_init();
    if (argc > 1) {
        // Accept one command from the command line
        r = run_command(argv[1], argv+2);
        return r;
    }

    while (1) {
        line = readline("augtool> ");
        if (line == NULL)
            return 0;

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
