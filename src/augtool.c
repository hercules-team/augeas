/*
 * augtool.c:
 *
 * Copyright (C) 2007-2015 David Lutterkort
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
#include "auglua.h"
#include "internal.h"
#include "safe-alloc.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <argz.h>
#include <getopt.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdarg.h>
#include <sys/time.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* Global variables */

static augeas *aug = NULL;
static const char *const progname = "augtool";
static unsigned int flags = AUG_NONE;
const char *root = NULL;
char *loadpath = NULL;
char *transforms = NULL;
size_t transformslen = 0;
const char *inputfile = NULL;
int echo_commands = 0;         /* Gets also changed in main_loop */
bool use_lua = false;
static lua_State *LS = NULL;
bool print_version = false;
bool auto_save = false;
bool interactive = false;
bool timing = false;
/* History file is ~/.augeas/history */
char *history_file = NULL;

#define AUGTOOL_PROMPT "augtool> "
#define AUGTOOL_LUA_PROMPT "augtool|lua> "
#define AUGTOOL_LUA_CONT_PROMPT "augtool|lua?>   "

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
                if (c == NULL) {
                    free(child);
                    return NULL;
                }
                child = c;
                strcat(child, "/");
            }

            /* strip off context if the user didn't give it */
            if (ctx != NULL) {
                char *c = realloc(child, strlen(child)-strlen(ctx)+1);
                if (c == NULL) {
                    free(child);
                    return NULL;
                }
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
        "get", "label", "ins", "load", "ls", "match",
        "mv", "cp", "rename", "print", "dump-xml", "rm", "save", "set", "setm",
        "clearm", "span", "store", "retrieve", "transform",
        "help", "touch", "insert", "move", "copy", "errors", NULL };

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

#ifndef HAVE_RL_CRLF
static int rl_crlf(void) {
    if (rl_outstream != NULL)
        putc('\n', rl_outstream);
    return 0;
}
#endif

#ifndef HAVE_RL_REPLACE_LINE
static void rl_replace_line(ATTRIBUTE_UNUSED const char *text,
                              ATTRIBUTE_UNUSED int clear_undo) {
    return;
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
static void help(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] [COMMAND]\n", progname);
    fprintf(stderr, "Load the Augeas tree and modify it. If no COMMAND is given, run interactively\n");
    fprintf(stderr, "Run '%s help' to get a list of possible commands.\n",
            progname);
    fprintf(stderr, "\nOptions:\n\n");
    fprintf(stderr, "  -c, --typecheck      typecheck lenses\n");
    fprintf(stderr, "  -b, --backup         preserve originals of modified files with\n"
                    "                       extension '.augsave'\n");
    fprintf(stderr, "  -n, --new            save changes in files with extension '.augnew',\n"
                    "                       leave original unchanged\n");
    fprintf(stderr, "  -r, --root ROOT      use ROOT as the root of the filesystem\n");
    fprintf(stderr, "  -I, --include DIR    search DIR for modules; can be given multiple times\n");
    fprintf(stderr, "  -t, --transform XFM  add a file transform; uses the 'transform' command\n"
                    "                       syntax, e.g. -t 'Fstab incl /etc/fstab.bak'\n");
    fprintf(stderr, "  -e, --echo           echo commands when reading from a file\n");
    fprintf(stderr, "  -f, --file FILE      read commands from FILE\n");
    fprintf(stderr, "  -l, --lua            use Lua interpreter instead of native Augeas\n");
    fprintf(stderr, "  -s, --autosave       automatically save at the end of instructions\n");
    fprintf(stderr, "  -i, --interactive    run an interactive shell after evaluating\n"
                    "                       the commands in STDIN and FILE\n");
    fprintf(stderr, "  -S, --nostdinc       do not search the builtin default directories\n"
                    "                       for modules\n");
    fprintf(stderr, "  -L, --noload         do not load any files into the tree on startup\n");
    fprintf(stderr, "  -A, --noautoload     do not autoload modules from the search path\n");
    fprintf(stderr, "  --span               load span positions for nodes related to a file\n");
    fprintf(stderr, "  --timing             after executing each command, show how long it took\n");
    fprintf(stderr, "  --version            print version information and exit.\n");

    exit(EXIT_FAILURE);
}

static void parse_opts(int argc, char **argv) {
    int opt;
    size_t loadpathlen = 0;
    enum {
        VAL_VERSION = CHAR_MAX + 1,
        VAL_SPAN = VAL_VERSION + 1,
        VAL_TIMING = VAL_SPAN + 1
    };
    struct option options[] = {
        { "help",        0, 0, 'h' },
        { "typecheck",   0, 0, 'c' },
        { "backup",      0, 0, 'b' },
        { "new",         0, 0, 'n' },
        { "root",        1, 0, 'r' },
        { "include",     1, 0, 'I' },
        { "transform",   1, 0, 't' },
        { "echo",        0, 0, 'e' },
        { "file",        1, 0, 'f' },
        { "lua",         0, 0, 'l' },
        { "autosave",    0, 0, 's' },
        { "interactive", 0, 0, 'i' },
        { "nostdinc",    0, 0, 'S' },
        { "noload",      0, 0, 'L' },
        { "noautoload",  0, 0, 'A' },
        { "span",        0, 0, VAL_SPAN },
        { "timing",      0, 0, VAL_TIMING },
        { "version",     0, 0, VAL_VERSION },
        { 0, 0, 0, 0}
    };
    int idx;

    while ((opt = getopt_long(argc, argv, "hnbcr:I:t:ef:lsiSLA", options, &idx)) != -1) {
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
            help();
            break;
        case 'r':
            root = optarg;
            break;
        case 'I':
            argz_add(&loadpath, &loadpathlen, optarg);
            break;
        case 't':
            argz_add(&transforms, &transformslen, optarg);
            break;
        case 'e':
            echo_commands = 1;
            break;
        case 'f':
            inputfile = optarg;
            break;
        case 'l':
            use_lua = true;
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
        case VAL_TIMING:
            timing = true;
            break;
        default:
            fprintf(stderr, "Try '%s --help' for more information.\n",
                    progname);
            exit(EXIT_FAILURE);
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
    fprintf(stderr, "Copyright (C) 2007-2015 David Lutterkort\n");
    fprintf(stderr, "License LGPLv2+: GNU LGPL version 2.1 or later\n");
    fprintf(stderr, "                 <http://www.gnu.org/licenses/lgpl-2.1.html>\n");
    fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stderr, "Written by David Lutterkort\n");
    return;
 error:
    fprintf(stderr, "Something went terribly wrong internally - please file a bug\n");
}

static void print_time_taken(const struct timeval *start,
                             const struct timeval *stop) {
    time_t elapsed = (stop->tv_sec - start->tv_sec)*1000
                   + (stop->tv_usec - start->tv_usec)/1000;
    printf("Time: %ld ms\n", elapsed);
}

static int run_command(const char *line, bool with_timing) {
    int result;
    struct timeval stop, start;

    gettimeofday(&start, NULL);
    result = aug_srun(aug, stdout, line);
    gettimeofday(&stop, NULL);
    if (with_timing && result >= 0) {
        print_time_taken(&start, &stop);
    }

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

static void sigint_handler(ATTRIBUTE_UNUSED int signum) {
    // Cancel the current line of input, along with undo info for that line.
    rl_replace_line("", 1);

    // Move the cursor to the next screen line, then force a re-display.
    rl_crlf();
    rl_forced_update_display();
}

static void install_signal_handlers(void) {
    // On Ctrl-C, cancel the current line (rather than exit the program).
    struct sigaction sigint_action;
    MEMZERO(&sigint_action, 1);
    sigint_action.sa_handler = sigint_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;
    sigaction(SIGINT, &sigint_action, NULL);
}


static int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

static int main_loop(int argc, char **argv) {
    char *line = NULL, *cur_line = NULL;
    int ret = 0;
    char inputline [128];
    int code;
    bool end_reached = false;
    bool get_line = true;
    bool in_interactive = false;

    if (inputfile) {
        if (use_lua) {
            if (luaL_loadfile(LS, inputfile)) {
                printf("luaL_loadfile() failed: %s\n", lua_tostring(LS, -1));
                lua_close(LS);
                return -1;
            }
            for (int i=0; i < argc; i++)
                lua_pushstring(LS, argv[i]);
            if (lua_pcall(LS, argc, 0, 0)) {
                printf("lua_pcall() failed: %s\n", lua_tostring(LS, -1));
                lua_close(LS);
                return -1;
            }
            if (auto_save) {
                strncpy(inputline, "save()", sizeof(inputline));
                line = inputline;
                code = luaL_loadbuffer(LS, line, strlen(line), "line") || lua_pcall(LS, 0, 0, 0);

                if (code) {
                    fprintf(stderr, "%s\n", lua_tostring(LS, -1));
                    lua_pop(LS, 1); /* pop error message from the stack */
                    ret = -1;
                }
            }
            lua_close(LS);
            return 0;
        } else {
            if (freopen(inputfile, "r", stdin) == NULL) {
                char *msg = NULL;
                if (asprintf(&msg, "Failed to open %s", inputfile) < 0)
                    perror("Failed to open input file");
                else
                    perror(msg);
                return -1;
            }
        }
    }

    install_signal_handlers();

    // make readline silent by default
    echo_commands = echo_commands || isatty(fileno(stdin));
    if (echo_commands)
        rl_outstream = NULL;
    else
        rl_outstream = fopen("/dev/null", "w");

    while(1) {
        if (get_line) {
            if (use_lua) {
                if (cur_line == NULL)
                    line = readline(AUGTOOL_LUA_PROMPT);
                else
                    line = readline(AUGTOOL_LUA_CONT_PROMPT);
            } else {
                line = readline(AUGTOOL_PROMPT);
            }
        } else {
            line = NULL;
        }

        if (line == NULL) {
            if (!isatty(fileno(stdin)) && interactive && !in_interactive) {
                in_interactive = true;
                if (echo_commands)
                    printf("\n");
                echo_commands = true;

                // reopen in stream
                if (freopen("/dev/tty", "r", stdin) == NULL) {
                    perror("Failed to open terminal for reading");
                    return -1;
                }
                rl_instream = stdin;

                // reopen stdout and stream to a tty if originally silenced or
                // not connected to a tty, for full interactive mode
                if (rl_outstream == NULL || !isatty(fileno(rl_outstream))) {
                    if (rl_outstream != NULL) {
                        fclose(rl_outstream);
                        rl_outstream = NULL;
                    }
                    if (freopen("/dev/tty", "w", stdout) == NULL) {
                        perror("Failed to reopen stdout");
                        return -1;
                    }
                    rl_outstream = stdout;
                }
                continue;
            }

            if (auto_save) {
                strncpy(inputline, "save", sizeof(inputline));
                line = inputline;
                if (echo_commands)
                    printf("%s\n", line);
                auto_save = false;
            } else {
                end_reached = true;
            }
            get_line = false;
        }

        if (end_reached) {
            if (use_lua)
                lua_close(LS);
            if (echo_commands)
                printf("\n");
            return ret;
        }

        if (*line == '\0' || *line == '#') {
            free(line);
            continue;
        }

        if (use_lua) {
            char *buf;
            if (cur_line == NULL) {
                buf = malloc(sizeof(char) * strlen(line));
                strcpy(buf, line);
            } else {
                sprintf(buf, "%s\n%s", cur_line, line);
            }

            code = aug_lua(LS, buf);
            if (isatty(fileno(stdin)))
                add_history(line);

            if (code) {
                const char *err = lua_tostring(LS, -1);
                if (ends_with(err, " near <eof>")) {
                    cur_line = malloc(sizeof(char) * strlen(buf));
                    strcpy(cur_line, buf);
                } else {
                  fprintf(stderr, "%s\n", err);
                  lua_pop(LS, 1); /* pop error message from the stack */
                  ret = -1;
                }
            } else {
                cur_line = NULL;
            }
        } else {
            code = run_command(line, timing);
            if (code == -2) {
                free(line);
                return ret;
            }

            if (code < 0) {
                ret = -1;
                print_aug_error();
            }
        }

        if (line != inputline)
            free(line);
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
    if (echo_commands) {
        if (use_lua)
            printf("%s%s\n", AUGTOOL_LUA_PROMPT, line);
        else
            printf("%s%s\n", AUGTOOL_PROMPT, line);
    }
    if (use_lua)
        code = luaL_loadbuffer(LS, line, strlen(line), "line") || lua_pcall(LS, 0, 0, 0);
    else
        code = run_command(line, timing);
    free(line);
    if (code >= 0 && auto_save)
        if (echo_commands)
            printf("%ssave\n", AUGTOOL_PROMPT);
    code = run_command("save", false);

    if (code < 0) {
        code = -1;
        print_aug_error();
    }
    return (code >= 0 || code == -2) ? 0 : -1;
}

static void add_transforms(char *ts, size_t tslen) {
    char *command;
    int r;
    char *t = NULL;
    bool added_transform = false;

    while ((t = argz_next(ts, tslen, t))) {
        r = xasprintf(&command, "transform %s", t);
        if (r < 0)
            fprintf(stderr, "error: Failed to add transform %s: could not allocate memory\n", t);

        r = aug_srun(aug, stdout, command);
        if (r < 0)
            fprintf(stderr, "error: Failed to add transform %s: %s\n", t, aug_error_message(aug));

        free(command);
        added_transform = true;
    }

    if (added_transform) {
        r = aug_load(aug);
        if (r < 0)
            fprintf(stderr, "error: Failed to load with new transforms: %s\n", aug_error_message(aug));
    }
}

int main(int argc, char **argv) {
    int r;
    struct timeval start, stop;

    setlocale(LC_ALL, "");

    parse_opts(argc, argv);

    if (timing) {
        printf("Initializing augeas ... ");
        fflush(stdout);
    }
    gettimeofday(&start, NULL);

    aug = aug_init(root, loadpath, flags|AUG_NO_ERR_CLOSE);

    gettimeofday(&stop, NULL);
    if (timing) {
        printf("done\n");
        print_time_taken(&start, &stop);
    }

    if (aug == NULL || aug_error(aug) != AUG_NOERROR) {
        fprintf(stderr, "Failed to initialize Augeas\n");
        if (aug != NULL)
            print_aug_error();
        exit(EXIT_FAILURE);
    }

    if (use_lua)
        LS = setup_lua(aug);

    add_transforms(transforms, transformslen);
    if (print_version) {
        print_version_info();
        return EXIT_SUCCESS;
    }
    readline_init();
    if (optind < argc && !use_lua) {
        // Accept one command from the command line
        r = run_args(argc - optind, argv+optind);
    } else {
        r = main_loop(argc - optind, argv+optind);
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
