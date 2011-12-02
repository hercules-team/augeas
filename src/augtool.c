/*
 * augtool.c:
 *
 * Copyright (C) 2007-2011 David Lutterkort
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

#include <config.h>
#include "augeas.h"
#include "internal.h"
#include "safe-alloc.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <argz.h>
#include <getopt.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdarg.h>

/* Global variables */

static augeas *aug = NULL;
static const char *const progname = "augtool";
static unsigned int flags = AUG_NONE;
const char *root = NULL;
char *loadpath = NULL;
const char *inputfile = NULL;
int echo = 0;         /* Gets also changed in main_loop */
bool print_version = false;
bool auto_save = false;
bool interactive = false;
/* History file is ~/.augeas/history */
char *history_file = NULL;

#define AUGTOOL_PROMPT "augtool> "

/*
 * General utilities
 */

/* Not static, since prototype is in internal.h */
int xasprintf(char **strp, const char *format, ...) {
  va_list args;
  int result;

  va_start (args, format);
  result = vasprintf (strp, format, args);
  va_end (args);
  if (result < 0)
      *strp = NULL;
  return result;
}

static int child_count(const char *path) {
    char *pat = NULL;
    int r;

    if (path[strlen(path)-1] == SEP)
        r = asprintf(&pat, "%s*", path);
    else
        r = asprintf(&pat, "%s/*", path);
    if (r < 0)
        return -1;
    r = aug_match(aug, pat, NULL);
    free(pat);

    return r;
}

static char *readline_path_generator(const char *text, int state) {
    static int current = 0;
    static char **children = NULL;
    static int nchildren = 0;
    static char *ctx = NULL;

    char *end = strrchr(text, SEP);
    if (end != NULL)
        end += 1;

    if (state == 0) {
        char *path;
        if (end == NULL) {
            if ((path = strdup("*")) == NULL)
                return NULL;
        } else {
            CALLOC(path, end - text + 2);
            if (path == NULL)
                return NULL;
            strncpy(path, text, end - text);
            strcat(path, "*");
        }

        for (;current < nchildren; current++)
            free((void *) children[current]);
        free((void *) children);
        nchildren = aug_match(aug, path, &children);
        current = 0;

        ctx = NULL;
        if (path[0] != SEP)
            aug_get(aug, AUGEAS_CONTEXT, (const char **) &ctx);

        free(path);
    }

    if (end == NULL)
        end = (char *) text;

    while (current < nchildren) {
        char *child = children[current];
        current += 1;

        char *chend = strrchr(child, SEP) + 1;
        if (STREQLEN(chend, end, strlen(end))) {
            if (child_count(child) > 0) {
                char *c = realloc(child, strlen(child)+2);
                if (c == NULL)
                    return NULL;
                child = c;
                strcat(child, "/");
            }

            /* strip off context if the user didn't give it */
            if (ctx != NULL) {
                char *c = realloc(child, strlen(child)-strlen(ctx)+1);
                if (c == NULL)
                    return NULL;
                int ctxidx = strlen(ctx);
                if (child[ctxidx] == SEP)
                    ctxidx++;
                strcpy(c, &child[ctxidx]);
                child = c;
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
    // FIXME: expose somewhere under /augeas
    static const char *const commands[] = {
        "quit", "clear", "defnode", "defvar",
        "get", "ins", "load", "ls", "match",
        "mv", "print", "dump-xml", "rm", "save", "set", "setm",
        "clearm", "span", "help", NULL };

    static int current = 0;
    const char *name;

    if (state == 0)
        current = 0;

    rl_completion_append_character = ' ';
    while ((name = commands[current]) != NULL) {
        current += 1;
        if (STREQLEN(text, name, strlen(text)))
            return strdup(name);
    }
    return NULL;
}

#ifndef HAVE_RL_COMPLETION_MATCHES
typedef char *rl_compentry_func_t(const char *, int);
static char **rl_completion_matches(ATTRIBUTE_UNUSED const char *text,
                           ATTRIBUTE_UNUSED rl_compentry_func_t *func) {
    return NULL;
}
#endif

static char **readline_completion(const char *text, int start,
                                  ATTRIBUTE_UNUSED int end) {
    if (start == 0)
        return rl_completion_matches(text, readline_command_generator);
    else
        return rl_completion_matches(text, readline_path_generator);

    return NULL;
}

static char *get_home_dir(uid_t uid) {
    char *strbuf;
    char *result;
    struct passwd pwbuf;
    struct passwd *pw = NULL;
    long val = sysconf(_SC_GETPW_R_SIZE_MAX);
    size_t strbuflen = val;

    if (val < 0)
        return NULL;

    if (ALLOC_N(strbuf, strbuflen) < 0)
        return NULL;

    if (getpwuid_r(uid, &pwbuf, strbuf, strbuflen, &pw) != 0 || pw == NULL) {
        free(strbuf);
        return NULL;
    }

    result = strdup(pw->pw_dir);

    free(strbuf);

    return result;
}

static void readline_init(void) {
    rl_readline_name = "augtool";
    rl_attempted_completion_function = readline_completion;
    rl_completion_entry_function = readline_path_generator;

    /* Set up persistent history */
    char *home_dir = get_home_dir(getuid());
    char *history_dir = NULL;

    if (home_dir == NULL)
        goto done;

    if (xasprintf(&history_dir, "%s/.augeas", home_dir) < 0)
        goto done;

    if (mkdir(history_dir, 0755) < 0 && errno != EEXIST)
        goto done;

    if (xasprintf(&history_file, "%s/history", history_dir) < 0)
        goto done;

    stifle_history(500);

    read_history(history_file);

 done:
    free(home_dir);
    free(history_dir);
}

__attribute__((noreturn))
static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] [COMMAND]\n", progname);
    fprintf(stderr, "Load the Augeas tree and modify it. If no COMMAND is given, run interactively\n");
    fprintf(stderr, "Run '%s help' to get a list of possible commands.\n",
            progname);
    fprintf(stderr, "\nOptions:\n\n");
    fprintf(stderr, "  -c, --typecheck    typecheck lenses\n");
    fprintf(stderr, "  -b, --backup       preserve originals of modified files with\n"
                    "                     extension '.augsave'\n");
    fprintf(stderr, "  -n, --new          save changes in files with extension '.augnew',\n"
                    "                     leave original unchanged\n");
    fprintf(stderr, "  -r, --root ROOT    use ROOT as the root of the filesystem\n");
    fprintf(stderr, "  -I, --include DIR  search DIR for modules; can be given mutiple times\n");
    fprintf(stderr, "  -e, --echo         echo commands when reading from a file\n");
    fprintf(stderr, "  -f, --file FILE    read commands from FILE\n");
    fprintf(stderr, "  -s, --autosave     automatically save at the end of instructions\n");
    fprintf(stderr, "  -i, --interactive  run an interactive shell after evaluating the commands in STDIN and FILE\n");
    fprintf(stderr, "  -S, --nostdinc     do not search the builtin default directories for modules\n");
    fprintf(stderr, "  -L, --noload       do not load any files into the tree on startup\n");
    fprintf(stderr, "  -A, --noautoload   do not autoload modules from the search path\n");
    fprintf(stderr, "  --span             load span positions for nodes related to a file\n");
    fprintf(stderr, "  --version          print version information and exit.\n");

    exit(EXIT_FAILURE);
}

static void parse_opts(int argc, char **argv) {
    int opt;
    size_t loadpathlen = 0;
    enum {
        VAL_VERSION = CHAR_MAX + 1,
        VAL_SPAN = VAL_VERSION + 1
    };
    struct option options[] = {
        { "help",      0, 0, 'h' },
        { "typecheck", 0, 0, 'c' },
        { "backup",    0, 0, 'b' },
        { "new",       0, 0, 'n' },
        { "root",      1, 0, 'r' },
        { "include",   1, 0, 'I' },
        { "echo",      0, 0, 'e' },
        { "file",      1, 0, 'f' },
        { "autosave",  0, 0, 's' },
        { "interactive",  0, 0, 'i' },
        { "nostdinc",  0, 0, 'S' },
        { "noload",    0, 0, 'L' },
        { "noautoload", 0, 0, 'A' },
        { "span",      0, 0, VAL_SPAN },
        { "version",   0, 0, VAL_VERSION },
        { 0, 0, 0, 0}
    };
    int idx;

    while ((opt = getopt_long(argc, argv, "hnbcr:I:ef:siSLA", options, &idx)) != -1) {
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
        case 'e':
            echo = 1;
            break;
        case 'f':
            inputfile = optarg;
            break;
        case 's':
            auto_save = true;
            break;
        case 'i':
            interactive = true;
            break;
        case 'S':
            flags |= AUG_NO_STDINC;
            break;
        case 'L':
            flags |= AUG_NO_LOAD;
            break;
        case 'A':
            flags |= AUG_NO_MODL_AUTOLOAD;
            break;
        case VAL_VERSION:
            flags |= AUG_NO_MODL_AUTOLOAD;
            print_version = true;
            break;
        case VAL_SPAN:
            flags |= AUG_ENABLE_SPAN;
            break;
        default:
            usage();
            break;
        }
    }
    argz_stringify(loadpath, loadpathlen, PATH_SEP_CHAR);
}

static void print_version_info(void) {
    const char *version;
    int r;

    r = aug_get(aug, "/augeas/version", &version);
    if (r != 1)
        goto error;

    fprintf(stderr, "augtool %s <http://augeas.net/>\n", version);
    fprintf(stderr, "Copyright (C) 2007-2011 David Lutterkort\n");
    fprintf(stderr, "License LGPLv2+: GNU LGPL version 2.1 or later\n");
    fprintf(stderr, "                 <http://www.gnu.org/licenses/lgpl-2.1.html>\n");
    fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stderr, "Written by David Lutterkort\n");
    return;
 error:
    fprintf(stderr, "Something went terribly wrong internally - please file a bug\n");
}

static int run_command(const char *line) {
    int result;

    result = aug_srun(aug, stdout, line);
    if (isatty(fileno(stdin)))
        add_history(line);
    return result;
}

static void print_aug_error(void) {
    if (aug_error(aug) == AUG_ENOMEM) {
        fprintf(stderr, "Out of memory.\n");
        return;
    }
    if (aug_error(aug) != AUG_NOERROR) {
        fprintf(stderr, "error: %s\n", aug_error_message(aug));
        if (aug_error_minor_message(aug) != NULL)
            fprintf(stderr, "error: %s\n",
                    aug_error_minor_message(aug));
        if (aug_error_details(aug) != NULL) {
            fputs(aug_error_details(aug), stderr);
            fprintf(stderr, "\n");
        }
    }
}

static int main_loop(void) {
    char *line = NULL;
    int ret = 0;
    char inputline [128];
    int code;
    bool end_reached = false;
    bool get_line = true;
    bool in_interactive = false;
    // make readline silent by default
    rl_outstream = fopen("/dev/null", "w");

    if (inputfile) {
        if (freopen(inputfile, "r", stdin) == NULL) {
            char *msg = NULL;
            if (asprintf(&msg, "Failed to open %s", inputfile) < 0)
                perror("Failed to open input file");
            else
                perror(msg);
            return -1;
        }
    }

    echo = echo || isatty(fileno(stdin));

    if (echo)
        rl_outstream = NULL;

    while(1) {
        if (get_line) {
            line = readline(AUGTOOL_PROMPT);
        } else {
            line = NULL;
        }

        if (line == NULL) {
            if (!isatty(fileno(stdin)) && interactive && !in_interactive) {
               in_interactive = true;
               echo = true;
               // reopen in and out streams
               if ((rl_instream = fopen("/dev/tty", "r")) == NULL) {
                   perror("Failed to open terminal for reading");
                   return -1;
               }
               if (rl_outstream != NULL) {
                   fclose(rl_outstream);
                   rl_outstream = NULL;
               }
               if ((rl_outstream = fopen("/dev/stdout", "w")) == NULL) {
                   perror("Failed to reopen stdout");
                   return -1;
               }
               continue;
            }

            if (auto_save) {
                strncpy(inputline, "save", sizeof(inputline));
                line = inputline;
                if (echo)
                    printf("%s\n", line);
                auto_save = false;
            } else {
                end_reached = true;
            }
            get_line = false;
        }

        if (end_reached) {
            if (echo)
                printf("\n");
            return ret;
        }

        if (*line == '\0' || *line == '#')
            continue;

        code = run_command(line);
        if (code == -2)
            return ret;
        if (code < 0) {
            ret = -1;
            print_aug_error();
        }
    }
}

static int run_args(int argc, char **argv) {
    size_t len = 0;
    char *line = NULL;
    int   code;

    for (int i=0; i < argc; i++)
        len += strlen(argv[i]) + 1;
    if (ALLOC_N(line, len + 1) < 0)
        return -1;
    for (int i=0; i < argc; i++) {
        strcat(line, argv[i]);
        strcat(line, " ");
    }
    code = run_command(line);
    free(line);
    if (code >= 0 && auto_save)
        code = run_command("save");
    return (code == 0 || code == -2) ? 0 : -1;
}

int main(int argc, char **argv) {
    int r;

    setlocale(LC_ALL, "");

    parse_opts(argc, argv);

    aug = aug_init(root, loadpath, flags|AUG_NO_ERR_CLOSE);
    if (aug == NULL || aug_error(aug) != AUG_NOERROR) {
        fprintf(stderr, "Failed to initialize Augeas\n");
        if (aug != NULL)
            print_aug_error();
        exit(EXIT_FAILURE);
    }
    if (print_version) {
        print_version_info();
        return EXIT_SUCCESS;
    }
    readline_init();
    if (optind < argc) {
        // Accept one command from the command line
        r = run_args(argc - optind, argv+optind);
    } else {
        r = main_loop();
    }
    if (history_file != NULL)
        write_history(history_file);

    return r == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
