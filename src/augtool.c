/*
 * augtool.c:
 *
 * Copyright (C) 2007-2010 David Lutterkort
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

/* Global variables */

static const struct command_def const *commands[];
static augeas *aug = NULL;
static const char *const progname = "augtool";
static unsigned int flags = AUG_NONE;
const char *root = NULL;
char *loadpath = NULL;
const char *inputfile = NULL;
int echo = 0;
bool print_version = false;

/*
 * General utilities
 */
static char *cleanstr(char *path, const char sep) {
    if (path == NULL || strlen(path) == 0)
        return path;
    char *e = path + strlen(path) - 1;
    while (e >= path && (*e == sep || isspace(*e)))
        *e-- = '\0';
    return path;
}

static char *cleanpath(char *path) {
    return cleanstr(path, SEP);
}

/*
 * Command handling infrastructure
 */
enum command_opt_type {
    CMD_NONE,
    CMD_STR,           /* String argument */
    CMD_PATH           /* Path expression */
};

struct command_opt_def {
    bool                  optional; /* Optional or mandatory */
    enum command_opt_type type;
    const char           *name;
    const char           *help;
};

#define CMD_OPT_DEF_LAST { .type = CMD_NONE, .name = NULL }

/* Handlers return one of these */
enum command_result {
    CMD_RES_OK,
    CMD_RES_ERR,
    CMD_RES_ENOMEM,
    CMD_RES_QUIT
};

struct command {
    const struct command_def *def;
    struct command_opt       *opt;
    enum command_result       result;
};

typedef void (*cmd_handler)(struct command*);

struct command_def {
    const char                   *name;
    const struct command_opt_def *opts;
    cmd_handler                   handler;
    const char                   *synopsis;
    const char                   *help;
};

static const struct command_def cmd_def_last =
    { .name = NULL, .opts = NULL, .handler = NULL,
      .synopsis = NULL, .help = NULL };

struct command_opt {
    struct command_opt           *next;
    const struct command_opt_def *def;
    char                         *value;
};

static const struct command_def *lookup_cmd_def(const char *name) {
    for (int i = 0; commands[i]->name != NULL; i++) {
        if (STREQ(name, commands[i]->name))
            return commands[i];
    }
    return NULL;
}

static const struct command_opt_def *
find_def(const struct command *cmd, const char *name) {
    const struct command_opt_def *def;
    for (def = cmd->def->opts; def->name != NULL; def++) {
        if (STREQ(def->name, name))
            return def;
    }
    return NULL;
}

static struct command_opt *
find_opt(const struct command *cmd, const char *name) {
    const struct command_opt_def *def = find_def(cmd, name);
    assert(def != NULL);

    for (struct command_opt *opt = cmd->opt; opt != NULL; opt = opt->next) {
        if (opt->def == def)
            return opt;
    }
    assert(def->optional);
    return NULL;
}

static const char *arg_value(const struct command *cmd, const char *name) {
    struct command_opt *opt = find_opt(cmd, name);

    return (opt == NULL) ? NULL : opt->value;
}

static char *nexttoken(char **line, bool path) {
    char *r, *s;
    char quot = '\0';
    int nbracket = 0;

    s = *line;

    while (*s && isblank(*s)) s+= 1;
    if (*s == '\'' || *s == '"') {
        quot = *s;
        s += 1;
    }
    r = s;
    while (*s) {
        if (*s == '[') nbracket += 1;
        if (*s == ']') nbracket -= 1;
        if (nbracket < 0) {
            fprintf(stderr, "unmatched [\n");
            return NULL;
        }
        if ((quot && *s == quot)
            || (!quot && isblank(*s) && (!path || nbracket == 0)))
            break;
        s += 1;
    }
    if (*s == '\0' && path && nbracket > 0) {
        fprintf(stderr, "unmatched [\n");
        return NULL;
    }
    if (*s)
        *s++ = '\0';
    *line = s;
    return r;
}

static struct command_opt *
make_command_opt(struct command *cmd, const struct command_opt_def *def) {
    struct command_opt *copt = NULL;
    if (ALLOC(copt) < 0) {
        fprintf(stderr, "Allocation failed\n");
        return NULL;
    }
    copt->def = def;
    list_append(cmd->opt, copt);
    return copt;
}

static int parseline(struct command *cmd, char *line) {
    char *tok;
    int narg = 0, nopt = 0;
    const struct command_opt_def *def;

    MEMZERO(cmd, 1);
    tok = nexttoken(&line, false);
    if (tok == NULL)
        return -1;
    cmd->def = lookup_cmd_def(tok);
    if (cmd->def == NULL) {
        fprintf(stderr, "Unknown command '%s'\n", tok);
        return -1;
    }

    for (def = cmd->def->opts; def->name != NULL; def++) {
        narg += 1;
        if (def->optional)
            nopt += 1;
    }

    int curarg = 0;
    def = cmd->def->opts;
    while (*line != '\0') {
        while (*line && isblank(*line)) line += 1;

        if (curarg >= narg) {
            fprintf(stderr,
                 "Too many arguments. Command %s takes only %d arguments\n",
                  cmd->def->name, narg);
                return -1;
        }

        struct command_opt *opt = make_command_opt(cmd, def);
        if (opt == NULL)
            return -1;

        if (def->type == CMD_PATH) {
            tok = nexttoken(&line, true);
            cleanpath(tok);
        } else {
            tok = nexttoken(&line, false);
        }
        if (tok == NULL)
            return -1;
        opt->value = tok;
        curarg += 1;
        def += 1;
    }

    if (curarg < narg - nopt) {
        fprintf(stderr, "Not enough arguments for %s\n", cmd->def->name);
        return -1;
    }

    return 0;
}

static int err_check(struct command *cmd) {
    if (aug_error(aug) != AUG_NOERROR) {
        const char *minor = aug_error_minor_message(aug);
        const char *details = aug_error_details(aug);

        cmd->result = CMD_RES_ERR;
        fprintf(stderr, "error: %s\n", aug_error_message(aug));
        if (minor != NULL)
            fprintf(stderr, "error: %s\n", minor);
        if (details != NULL)
            fprintf(stderr, "error: %s\n", details);
        return -1;
    }
    return 0;
}

#define ERR_CHECK(cmd) if (err_check(cmd) < 0) return;

#define ERR_RET(cmd) if ((cmd)->result != CMD_RES_OK) return;

#define ERR_EXIT(cond, cmd, code)                \
    if (cond) {                                  \
        (cmd)->result = code;                    \
        return;                                  \
    }

/*
 * Commands
 */
static void format_desc(const char *d) {
    printf("    ");
    for (const char *s = d; *s; s++) {
        if (*s == '\n')
            printf("\n   ");
        else
            putchar(*s);
    }
    printf("\n\n");
}

static void format_defname(char *buf, const struct command_opt_def *def,
                           bool mark_optional) {
    char *p;
    if (mark_optional && def->optional)
        p = stpcpy(buf, " [<");
    else
        p = stpcpy(buf, " <");
    for (int i=0; i < strlen(def->name); i++)
        *p++ = toupper(def->name[i]);
    *p++ = '>';
    if (mark_optional && def->optional)
        *p++ = ']';
    *p = '\0';
}

static void cmd_help(struct command *cmd) {
    const char *name = arg_value(cmd, "command");
    char buf[100];

    if (name == NULL) {
        printf("Commands:\n\n");
        for (int i=0; commands[i]->name != NULL; i++) {
            const struct command_def *def = commands[i];
            printf("    %-10s - %s\n", def->name, def->synopsis);
        }
        printf("\nType 'help <command>' for more information on a command\n\n");
    } else {
        const struct command_def *def = lookup_cmd_def(name);
        const struct command_opt_def *odef = NULL;
        if (def == NULL) {
            fprintf(stderr, "unknown command %s\n", name);
            cmd->result = CMD_RES_ERR;
            return;
        }
        printf("  COMMAND\n");
        printf("    %s - %s\n\n", name, def->synopsis);
        printf("  SYNOPSIS\n");
        printf("    %s", name);

        for (odef = def->opts; odef->name != NULL; odef++) {
            format_defname(buf, odef, true);
            printf("%s", buf);
        }
        printf("\n\n");
        printf("  DESCRIPTION\n");
        format_desc(def->help);
        if (def->opts->name != NULL) {
            printf("  OPTIONS\n");
            for (odef = def->opts; odef->name != NULL; odef++) {
                const char *help = odef->help;
                if (help == NULL)
                    help = "";
                format_defname(buf, odef, false);
                printf("    %-10s %s\n", buf, help);
            }
        }
        printf("\n");
    }
}

static const struct command_opt_def cmd_help_opts[] = {
    { .type = CMD_STR, .name = "command", .optional = true,
      .help = "print help for this command only" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_help_def = {
    .name = "help",
    .opts = cmd_help_opts,
    .handler = cmd_help,
    .synopsis = "print help",
    .help = "list all commands or print details about one command"
};

static void cmd_quit(ATTRIBUTE_UNUSED struct command *cmd) {
    cmd->result = CMD_RES_QUIT;
}

static const struct command_opt_def cmd_quit_opts[] = {
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_quit_def = {
    .name = "quit",
    .opts = cmd_quit_opts,
    .handler = cmd_quit,
    .synopsis = "exit the program",
    .help = "Exit the program"
};

static char *ls_pattern(struct command *cmd, const char *path) {
    char *q;
    int r;

    if (path[strlen(path)-1] == SEP)
        r = asprintf(&q, "%s*", path);
    else
        r = asprintf(&q, "%s/*", path);
    if (r < 0) {
        cmd->result = CMD_RES_ENOMEM;
        return NULL;
    }
    return q;
}

static int child_count(struct command *cmd, const char *path) {
    char *q = ls_pattern(cmd, path);
    int cnt;

    if (q == NULL)
        return 0;
    cnt = aug_match(aug, q, NULL);
    err_check(cmd);
    free(q);
    return cnt;
}

static void cmd_ls(struct command *cmd) {
    int cnt;
    const char *path = arg_value(cmd, "path");
    char **paths;

    path = ls_pattern(cmd, path);
    if (path == NULL)
        ERR_RET(cmd);
    cnt = aug_match(aug, path, &paths);
    ERR_CHECK(cmd);
    for (int i=0; i < cnt; i++) {
        const char *val;
        const char *basnam = strrchr(paths[i], SEP);
        int dir = child_count(cmd, paths[i]);
        aug_get(aug, paths[i], &val);
        err_check(cmd);
        basnam = (basnam == NULL) ? paths[i] : basnam + 1;
        if (val == NULL)
            val = "(none)";
        printf("%s%s= %s\n", basnam, dir ? "/ " : " ", val);
        free(paths[i]);
    }
    if (cnt > 0)
        free(paths);
}

static const struct command_opt_def cmd_ls_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "the node whose children to list" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_ls_def = {
    .name = "ls",
    .opts = cmd_ls_opts,
    .handler = cmd_ls,
    .synopsis = "list children of a node",
    .help = "list the direct children of a node"
};

static void cmd_match(struct command *cmd) {
    int cnt;
    const char *pattern = arg_value(cmd, "path");
    const char *value = arg_value(cmd, "value");
    char **matches;
    bool filter = (value != NULL) && (strlen(value) > 0);

    cnt = aug_match(aug, pattern, &matches);
    err_check(cmd);
    if (cnt < 0) {
        printf("  (error matching %s)\n", pattern);
        cmd->result = CMD_RES_ERR;
        goto done;
    }
    if (cnt == 0) {
        printf("  (no matches)\n");
        goto done;
    }

    for (int i=0; i < cnt; i++) {
        const char *val;
        aug_get(aug, matches[i], &val);
        err_check(cmd);
        if (val == NULL)
            val = "(none)";
        if (filter) {
            if (STREQ(value, val))
                printf("%s\n", matches[i]);
        } else {
            printf("%s = %s\n", matches[i], val);
        }
    }
 done:
    for (int i=0; i < cnt; i++)
        free(matches[i]);
    free(matches);
}

static const struct command_opt_def cmd_match_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "the path expression to match" },
    { .type = CMD_STR, .name = "value", .optional = true,
      .help = "only show matches with this value" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_match_def = {
    .name = "match",
    .opts = cmd_match_opts,
    .handler = cmd_match,
    .synopsis = "print matches for a path expression",
    .help = "Find all paths that match the path expression PATH. "
            "If VALUE is given,\n only the matching paths whose value equals "
            "VALUE are printed"
};

static void cmd_rm(struct command *cmd) {
    int cnt;
    const char *path = arg_value(cmd, "path");
    printf("rm : %s", path);
    cnt = aug_rm(aug, path);
    err_check(cmd);
    printf(" %d\n", cnt);
}

static const struct command_opt_def cmd_rm_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "remove all nodes matching this path expression" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_rm_def = {
    .name = "rm",
    .opts = cmd_rm_opts,
    .handler = cmd_rm,
    .synopsis = "delete nodes and subtrees",
    .help = "Delete PATH and all its children from the tree"
};

static void cmd_mv(struct command *cmd) {
    const char *src = arg_value(cmd, "src");
    const char *dst = arg_value(cmd, "dst");
    int r;

    r = aug_mv(aug, src, dst);
    err_check(cmd);
    if (r < 0)
        printf("Failed\n");
}

static const struct command_opt_def cmd_mv_opts[] = {
    { .type = CMD_PATH, .name = "src", .optional = false,
      .help = "the tree to move" },
    { .type = CMD_PATH, .name = "dst", .optional = false,
      .help = "where to put the source tree" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_mv_def = {
    .name = "mv",
    .opts = cmd_mv_opts,
    .handler = cmd_mv,
    .synopsis = "move a subtree",
    .help = "Move node  SRC to DST.  SRC must match  exactly one node in  "
    "the tree.\n DST  must either  match  exactly one  node  in the  tree,  "
    "or may  not\n exist  yet. If  DST exists  already, it  and all  its  "
    "descendants are\n deleted.  If  DST  does  not   exist  yet,  it  and  "
    "all  its  missing\n ancestors are created."
};

static void cmd_set(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    const char *val = arg_value(cmd, "value");
    int r;

    r = aug_set(aug, path, val);
    err_check(cmd);
    if (r == -1)
        printf ("Failed\n");
}

static const struct command_opt_def cmd_set_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "set the value of this node" },
    { .type = CMD_STR, .name = "value", .optional = false,
      .help = "the new value for the node" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_set_def = {
    .name = "set",
    .opts = cmd_set_opts,
    .handler = cmd_set,
    .synopsis = "set the value of a node",
    .help = "Associate VALUE with PATH.  If PATH is not in the tree yet, "
    "it and all\n its ancestors will be created. These new tree entries "
    "will appear last\n amongst their siblings"
};

static void cmd_setm(struct command *cmd) {
    const char *base = arg_value(cmd, "base");
    const char *sub  = arg_value(cmd, "sub");
    const char *val  = arg_value(cmd, "value");
    int r;

    r = aug_setm(aug, base, sub, val);
    err_check(cmd);
    if (r == -1)
        printf ("Failed\n");
}

static const struct command_opt_def cmd_setm_opts[] = {
    { .type = CMD_PATH, .name = "base", .optional = false,
      .help = "the base node" },
    { .type = CMD_PATH, .name = "sub", .optional = false,
      .help = "the subtree relative to the base" },
    { .type = CMD_STR, .name = "value", .optional = false,
      .help = "the value for the nodes" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_setm_def = {
    .name = "setm",
    .opts = cmd_setm_opts,
    .handler = cmd_setm,
    .synopsis = "set the value of multiple nodes",
    .help = "Set multiple nodes in one operation.  Find or create a node"
    " matching SUB\n by interpreting SUB as a  path expression relative"
    " to each node matching\n BASE. If SUB is '.', the nodes matching "
    "BASE will be modified."
};

static void cmd_clearm(struct command *cmd) {
    const char *base = arg_value(cmd, "base");
    const char *sub  = arg_value(cmd, "sub");
    int r;

    r = aug_setm(aug, base, sub, NULL);
    err_check(cmd);
    if (r == -1)
        printf ("Failed\n");
}

static const struct command_opt_def cmd_clearm_opts[] = {
    { .type = CMD_PATH, .name = "base", .optional = false,
      .help = "the base node" },
    { .type = CMD_PATH, .name = "sub", .optional = false,
      .help = "the subtree relative to the base" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_clearm_def = {
    .name = "clearm",
    .opts = cmd_clearm_opts,
    .handler = cmd_clearm,
    .synopsis = "clear the value of multiple nodes",
    .help = "Clear multiple nodes values in one operation. Find or create a"
    " node matching SUB\n by interpreting SUB as a path expression relative"
    " to each node matching\n BASE. If SUB is '.', the nodes matching "
    "BASE will be modified."
};

static void cmd_defvar(struct command *cmd) {
    const char *name = arg_value(cmd, "name");
    const char *path = arg_value(cmd, "expr");
    int r;

    r = aug_defvar(aug, name, path);
    err_check(cmd);
    if (r == -1)
        printf("Failed\n");
}

static const struct command_opt_def cmd_defvar_opts[] = {
    { .type = CMD_STR, .name = "name", .optional = false,
      .help = "the name of the variable" },
    { .type = CMD_PATH, .name = "expr", .optional = false,
      .help = "the path expression" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_defvar_def = {
    .name = "defvar",
    .opts = cmd_defvar_opts,
    .handler = cmd_defvar,
    .synopsis = "set a variable",
    .help = "Evaluate EXPR and set the variable NAME to the resulting "
    "nodeset. The\n variable can be used in path expressions as $NAME.  "
    "Note that EXPR is\n evaluated when the variable is defined, not when "
    "it is used."
};

static void cmd_defnode(struct command *cmd) {
    const char *name = arg_value(cmd, "name");
    const char *path = arg_value(cmd, "expr");
    const char *value = arg_value(cmd, "value");
    int r;

    /* Our simple minded line parser treats non-existant and empty values
     * the same. We choose to take the empty string to mean NULL */
    if (value != NULL && strlen(value) == 0)
        value = NULL;
    r = aug_defnode(aug, name, path, value, NULL);
    err_check(cmd);
    if (r == -1)
        printf ("Failed\n");
}

static const struct command_opt_def cmd_defnode_opts[] = {
    { .type = CMD_STR, .name = "name", .optional = false,
      .help = "the name of the variable" },
    { .type = CMD_PATH, .name = "expr", .optional = false,
      .help = "the path expression" },
    { .type = CMD_STR, .name = "value", .optional = true,
      .help = "the value for the new node" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_defnode_def = {
    .name = "defnode",
    .opts = cmd_defnode_opts,
    .handler = cmd_defnode,
    .synopsis = "set a variable, possibly creating a new node",
    .help = "Define the variable NAME to the result of evalutating EXPR, "
    " which must\n be a nodeset.  If no node matching EXPR exists yet,  one "
    "is created and\n NAME will refer to it.   When a node is created and "
    "VALUE is given, the\n new node's value is set to VALUE."
};

static void cmd_clear(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    int r;

    r = aug_set(aug, path, NULL);
    err_check(cmd);
    if (r == -1)
        printf ("Failed\n");
}

static const struct command_opt_def cmd_clear_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "clear the value of this node" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_clear_def = {
    .name = "clear",
    .opts = cmd_clear_opts,
    .handler = cmd_clear,
    .synopsis = "clear the value of a node",
    .help = "Set the value for PATH to NULL. If PATH is not in the tree yet, "
    "it and\n all its ancestors will be created.  These new tree entries "
    "will appear\n last amongst their siblings"
};

static void cmd_get(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    const char *val;
    int r;

    r = aug_get(aug, path, &val);
    ERR_CHECK(cmd);
    printf("%s", path);
    if (r == 0) {
        printf(" (o)\n");
    } else if (val == NULL) {
        printf(" (none)\n");
    } else {
        printf(" = %s\n", val);
    }
    err_check(cmd);
}

static const struct command_opt_def cmd_get_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "get the value of this node" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_get_def = {
    .name = "get",
    .opts = cmd_get_opts,
    .handler = cmd_get,
    .synopsis = "get the value of a node",
    .help = "Get and print the value associated with PATH"
};

static void cmd_print(struct command *cmd) {
    const char *path = arg_value(cmd, "path");

    aug_print(aug, stdout, path);
    err_check(cmd);
}

static const struct command_opt_def cmd_print_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = true,
      .help = "print this subtree" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_print_def = {
    .name = "print",
    .opts = cmd_print_opts,
    .handler = cmd_print,
    .synopsis = "print a subtree",
    .help = "Print entries in the tree.  If PATH is given, printing starts there,\n otherwise the whole tree is printed"
};

static void cmd_save(struct command *cmd) {
    int r;
    r = aug_save(aug);
    err_check(cmd);
    if (r == -1) {
        printf("Saving failed\n");
        cmd->result = CMD_RES_ERR;
    } else {
        r = aug_match(aug, "/augeas/events/saved", NULL);
        if (r > 0) {
            printf("Saved %d file(s)\n", r);
        } else if (r < 0) {
            printf("Error during match: %d\n", r);
        }
    }
}

static const struct command_opt_def cmd_save_opts[] = {
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_save_def = {
    .name = "save",
    .opts = cmd_save_opts,
    .handler = cmd_save,
    .synopsis = "save all pending changes",
    .help = "Save all pending changes to disk. How exactly that is done depends on\n the value of the node /augeas/save, which can be changed by the user.\n The possible values for it are\n \n   noop      - do not write files; useful for finding errors that\n               might happen during a save\n   backup    - save the original file in a file by appending the extension\n               '.augsave' and overwrite the original with new content\n   newfile   - leave the original file untouched and write new content to\n               a file with extension '.augnew' next to the original file\n   overwrite - overwrite the original file with new content\n \n Save always tries to save all files for which entries in the tree have\n changed. When saving fails, some files will be written.  Details about\n why a save failed can by found by issuing the command 'print\n /augeas//error' (note the double slash)"
};

static void cmd_load(struct command *cmd) {
    int r;
    r = aug_load(aug);
    err_check(cmd);
    if (r == -1) {
        printf("Loading failed\n");
    }
}

static const struct command_opt_def cmd_load_opts[] = {
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_load_def = {
    .name = "load",
    .opts = cmd_load_opts,
    .handler = cmd_load,
    .synopsis = "(re)load files under /files",
    .help = "Load files  according to the  transforms in /augeas/load.  "
    "A transform\n Foo  is  represented  with  a  subtree  /augeas/load/Foo."
    "   Underneath\n /augeas/load/Foo, one node labelled  'lens' must exist,"
    " whose value is\n the  fully  qualified name  of  a  lens,  for example  "
    "'Foo.lns',  and\n multiple nodes 'incl' and 'excl' whose values are "
    "globs that determine\n which files are  transformed by that lens. It "
    "is an  error if one file\n can be processed by multiple transforms."
};

static void cmd_ins(struct command *cmd) {
    const char *label = arg_value(cmd, "label");
    const char *where = arg_value(cmd, "where");
    const char *path = arg_value(cmd, "path");
    int before;

    if (STREQ(where, "after"))
        before = 0;
    else if (STREQ(where, "before"))
        before = 1;
    else {
        printf("The <WHERE> argument must be either 'before' or 'after'.");
        cmd->result = CMD_RES_ERR;
        return;
    }

    aug_insert(aug, path, label, before);
    err_check(cmd);
}

static const struct command_opt_def cmd_ins_opts[] = {
    { .type = CMD_STR, .name = "label", .optional = false,
      .help = "the label for the new node" },
    { .type = CMD_STR, .name = "where", .optional = false,
      .help = "either 'before' or 'after'" },
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "the node before/after which to insert" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_ins_def = {
    .name = "ins",
    .opts = cmd_ins_opts,
    .handler = cmd_ins,
    .synopsis = "insert new node before/after and existing node",
    .help = "Insert a new node with label LABEL right before or after "
    "PATH into the\n tree. WHERE must be either 'before' or 'after'."
};

static char *readline_path_generator(const char *text, int state) {
    static int current = 0;
    static char **children = NULL;
    static int nchildren = 0;
    struct command fake;  /* Used only for the result field */

    MEMZERO(&fake, 1);
    if (state == 0) {
        char *end = strrchr(text, SEP);
        char *path;
        if (end == NULL) {
            if ((path = strdup("/*")) == NULL)
                return NULL;
        } else {
            end += 1;
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
        free(path);
    }

    while (current < nchildren) {
        char *child = children[current];
        current += 1;
        if (STREQLEN(child, text, strlen(text))) {
            if (child_count(&fake, child) > 0) {
                char *c = realloc(child, strlen(child)+2);
                if (c == NULL)
                    return NULL;
                child = c;
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

static const struct command_def const *commands[] = {
    &cmd_quit_def,
    &cmd_clear_def,
    &cmd_defnode_def,
    &cmd_defvar_def,
    &cmd_get_def,
    &cmd_ins_def,
    &cmd_load_def,
    &cmd_ls_def,
    &cmd_match_def,
    &cmd_mv_def,
    &cmd_print_def,
    &cmd_rm_def,
    &cmd_save_def,
    &cmd_set_def,
    &cmd_setm_def,
    &cmd_clearm_def,
    &cmd_help_def,
    &cmd_def_last
};

static char *readline_command_generator(const char *text, int state) {
    static int current = 0;
    const char *name;

    if (state == 0)
        current = 0;

    rl_completion_append_character = ' ';
    while ((name = commands[current]->name) != NULL) {
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
    fprintf(stderr, "  -c, --typecheck    typecheck lenses\n");
    fprintf(stderr, "  -b, --backup       preserve originals of modified files with\n"
                    "                     extension '.augsave'\n");
    fprintf(stderr, "  -n, --new          save changes in files with extension '.augnew',\n"
                    "                     leave original unchanged\n");
    fprintf(stderr, "  -r, --root ROOT    use ROOT as the root of the filesystem\n");
    fprintf(stderr, "  -I, --include DIR  search DIR for modules; can be given mutiple times\n");
    fprintf(stderr, "  -e, --echo         echo commands when reading from a file\n");
    fprintf(stderr, "  -f, --file FILE    read commands from FILE\n");
    fprintf(stderr, "  --nostdinc         do not search the builtin default directories for modules\n");
    fprintf(stderr, "  --noload           do not load any files into the tree on startup\n");
    fprintf(stderr, "  --noautoload       do not autoload modules from the search path\n");
    fprintf(stderr, "  --version          print version information and exit.\n");

    exit(EXIT_FAILURE);
}

static void parse_opts(int argc, char **argv) {
    int opt;
    size_t loadpathlen = 0;
    enum {
        VAL_NO_STDINC = CHAR_MAX + 1,
        VAL_NO_LOAD = VAL_NO_STDINC + 1,
        VAL_NO_AUTOLOAD = VAL_NO_LOAD + 1,
        VAL_VERSION = VAL_NO_AUTOLOAD + 1
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
        { "nostdinc",  0, 0, VAL_NO_STDINC },
        { "noload",    0, 0, VAL_NO_LOAD },
        { "noautoload", 0, 0, VAL_NO_AUTOLOAD },
        { "version",   0, 0, VAL_VERSION },
        { 0, 0, 0, 0}
    };
    int idx;

    while ((opt = getopt_long(argc, argv, "hnbcr:I:ef:", options, &idx)) != -1) {
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
        case VAL_NO_STDINC:
            flags |= AUG_NO_STDINC;
            break;
        case VAL_NO_LOAD:
            flags |= AUG_NO_LOAD;
            break;
        case VAL_NO_AUTOLOAD:
            flags |= AUG_NO_MODL_AUTOLOAD;
            break;
        case VAL_VERSION:
            flags |= AUG_NO_MODL_AUTOLOAD;
            print_version = true;
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
    fprintf(stderr, "Copyright (C) 2009-2010 David Lutterkort\n");
    fprintf(stderr, "License LGPLv2+: GNU LGPL version 2.1 or later\n");
    fprintf(stderr, "                 <http://www.gnu.org/licenses/lgpl-2.1.html>\n");
    fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stderr, "Written by David Lutterkort\n");
    return;
 error:
    fprintf(stderr, "Something went terribly wrong internally - please file a bug\n");
}

static enum command_result
run_command(const char *line) {
    char *dup_line = strdup(line);
    struct command cmd;

    if (dup_line == NULL) {
        fprintf(stderr, "Out of memory\n");
        return CMD_RES_ENOMEM;
    }

    MEMZERO(&cmd, 1);
    if (parseline(&cmd, dup_line) == 0) {
        cmd.def->handler(&cmd);
        if (isatty(fileno(stdin)))
            add_history(line);
    } else {
        cmd.result = CMD_RES_ERR;
    }
    free(dup_line);
    return cmd.result;
}

static int main_loop(void) {
    char *line = NULL;
    int ret = 0;
    size_t len = 0;
    char inputline [128];
    enum command_result code;

    FILE *fp;
    if (inputfile) {
       fp = fopen(inputfile, "r");

       if (fp == NULL) {
           fprintf(stderr, "Failed to open input file %s.\n", inputfile);
           return -1;
       }
    }

    while(1) {
        if (inputfile) {
            if (fgets(inputline, sizeof(inputline), fp) == NULL) {
                line = NULL;
            } else {
                line = inputline;
            }
            if (echo)
                printf("augtool> %s", line);
        } else if (isatty(fileno(stdin))) {
            line = readline("augtool> ");
        } else {
            if (getline(&line, &len, stdin) == -1)
                return ret;
            if (echo)
                printf("augtool> %s", line);
        }

        cleanstr(line, '\n');
        if (line == NULL) {
            printf("\n");
            return ret;
        }
        if (*line == '\0' || *line == '#')
            continue;

        code = run_command(line);
        if (code == CMD_RES_QUIT)
            return 0;
        if (code == CMD_RES_ERR)
            ret = -1;
        if (code == CMD_RES_ENOMEM) {
            fprintf(stderr, "Out of memory.\n");
            return -1;
        }
        if (isatty(fileno(stdin)))
            add_history(line);
    }
}

static int run_args(int argc, char **argv) {
    size_t len = 0;
    char *line = NULL;
    enum command_result code;

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
    return (code == CMD_RES_OK || code == CMD_RES_QUIT) ? 0 : -1;
}

int main(int argc, char **argv) {
    int r;

    setlocale(LC_ALL, "");

    parse_opts(argc, argv);

    aug = aug_init(root, loadpath, flags);
    if (aug == NULL) {
        fprintf(stderr, "Failed to initialize Augeas\n");
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
