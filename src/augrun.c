/*
 * augrun.c: command interpreter for augeas
 *
 * Copyright (C) 2011 David Lutterkort
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
 * Author: David Lutterkort <lutter@redhat.com>
 */

#include <config.h>
#include "augeas.h"
#include "internal.h"
#include "memory.h"
#include "errcode.h"

#include <ctype.h>
#include <libxml/tree.h>

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

struct command {
    const struct command_def *def;
    struct command_opt       *opt;
    struct augeas            *aug;
    struct error             *error; /* Same as aug->error */
    FILE                     *out;
    bool                      quit;
};

typedef void (*cmd_handler)(struct command*);

struct command_def {
    const char                   *name;
    const char                   *category;
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

struct command_grp_def {
    const char                     *name;
    const struct command_def const *commands[];
};

static const struct command_grp_def cmd_grp_def_last =
    { .name = NULL, .commands = { } };

static const struct command_def *lookup_cmd_def(const char *name);

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

static char *nexttoken(struct command *cmd, char **line, bool path) {
    char *r, *s, *w;
    char quot = '\0';
    int nbracket = 0;
    int nescaped = 0;
    bool copy;

    s = *line;

    while (*s && isblank(*s)) s+= 1;
    r = s;
    w = s;
    while (*s) {
        copy = true;
        if (*s == '\\') {
            switch (*(s+1)) {
                case '[':
                case ']':  /* pass both literally */
                    nescaped = 2;
                    break;
                case 't':  /* insert tab */
                    *(s+1) = '\t';
                    nescaped = 1;
                    s += 1;
                    break;
                case 'n':  /* insert newline */
                    *(s+1) = '\n';
                    nescaped = 1;
                    s += 1;
                    break;
                case ' ':
                case '\t': /* pass both through if quoted, else fall */
                    if (quot) break;
                case '\'':
                case '"':  /* pass both through if opposite quote, else fall */
                    if (quot && quot != *(s+1)) break;
                case '\\': /* pass next character through */
                    nescaped = 1;
                    s += 1;
                    break;
                default:
                    ERR_REPORT(cmd, AUG_ECMDRUN, "unknown escape sequence");
                    return NULL;
            }
        }

        if (nescaped == 0) {
            if (*s == '[') nbracket += 1;
            if (*s == ']') nbracket -= 1;
            if (nbracket < 0) {
                ERR_REPORT(cmd, AUG_ECMDRUN, "unmatched [");
                return NULL;
            }

            if (!path || nbracket == 0) {
                if (!quot && (*s == '\'' || *s == '"')) {
                    quot = *s;
                    copy = false;
                } else if (quot && *s == quot) {
                    quot = '\0';
                    copy = false;
                }

                if (!quot && isblank(*s))
                    break;
            }
        } else {
            nescaped -= 1;
        }

        if (copy) {
            *w = *s;
            w += 1;
        }
        s += 1;
    }
    if (*s == '\0' && path && nbracket > 0) {
        ERR_REPORT(cmd, AUG_ECMDRUN, "unmatched [");
        return NULL;
    }
    if (*s == '\0' && quot) {
        ERR_REPORT(cmd, AUG_ECMDRUN, "unmatched %c", quot);
        return NULL;
    }
    while (*w && w <= s)
        *w++ = '\0';
    *line = w;
    return r;
}

static struct command_opt *
make_command_opt(struct command *cmd, const struct command_opt_def *def) {
    struct command_opt *copt = NULL;
    int r;

    r = ALLOC(copt);
    ERR_NOMEM(r < 0, cmd->aug);
    copt->def = def;
    list_append(cmd->opt, copt);
 error:
    return copt;
}

static void free_command_opts(struct command *cmd) {
    struct command_opt *next;

    next = cmd->opt;
    while (next != NULL) {
        struct command_opt *del = next;
        next = del->next;
        free(del);
    }
    cmd->opt = NULL;
}

static int parseline(struct command *cmd, char *line) {
    char *tok;
    int narg = 0, nopt = 0;
    const struct command_opt_def *def;

    tok = nexttoken(cmd, &line, false);
    if (tok == NULL)
        return -1;
    cmd->def = lookup_cmd_def(tok);
    if (cmd->def == NULL) {
        ERR_REPORT(cmd, AUG_ECMDRUN, "Unknown command '%s'", tok);
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
            ERR_REPORT(cmd, AUG_ECMDRUN,
                 "Too many arguments. Command %s takes only %d arguments",
                  cmd->def->name, narg);
                return -1;
        }

        struct command_opt *opt = make_command_opt(cmd, def);
        if (opt == NULL)
            return -1;

        if (def->type == CMD_PATH) {
            tok = nexttoken(cmd, &line, true);
            cleanpath(tok);
        } else {
            tok = nexttoken(cmd, &line, false);
        }
        if (tok == NULL)
            return -1;
        opt->value = tok;
        curarg += 1;
        def += 1;
        while (*line && isblank(*line)) line += 1;
    }

    if (curarg < narg - nopt) {
        ERR_REPORT(cmd, AUG_ECMDRUN, "Not enough arguments for %s", cmd->def->name);
        return -1;
    }

    return 0;
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

static void cmd_help(struct command *cmd);

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
    cmd->quit = true;
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
    char *q = NULL;
    int r;

    if (path[strlen(path)-1] == SEP)
        r = xasprintf(&q, "%s*", path);
    else
        r = xasprintf(&q, "%s/*", path);
    ERR_NOMEM(r < 0, cmd->aug);
 error:
    return q;
}

static int child_count(struct command *cmd, const char *path) {
    char *q = ls_pattern(cmd, path);
    int cnt;

    if (q == NULL)
        return 0;
    cnt = aug_match(cmd->aug, q, NULL);
    if (HAS_ERR(cmd))
        cnt = -1;
    free(q);
    return cnt;
}

static void cmd_ls(struct command *cmd) {
    int cnt = 0;
    char *path = NULL;
    char **paths = NULL;

    path = ls_pattern(cmd, arg_value(cmd, "path"));
    ERR_BAIL(cmd);

    cnt = aug_match(cmd->aug, path, &paths);
    ERR_BAIL(cmd);
    for (int i=0; i < cnt; i++) {
        const char *val;
        const char *basnam = strrchr(paths[i], SEP);
        int dir = child_count(cmd, paths[i]);
        aug_get(cmd->aug, paths[i], &val);
        ERR_BAIL(cmd);
        basnam = (basnam == NULL) ? paths[i] : basnam + 1;
        if (val == NULL)
            val = "(none)";
        fprintf(cmd->out, "%s%s= %s\n", basnam, dir ? "/ " : " ", val);
        FREE(paths[i]);
    }
 error:
    free(path);
    for (int i=0; i < cnt; i++)
        FREE(paths[i]);
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
    int cnt = 0;
    const char *pattern = arg_value(cmd, "path");
    const char *value = arg_value(cmd, "value");
    char **matches = NULL;
    bool filter = (value != NULL) && (strlen(value) > 0);

    cnt = aug_match(cmd->aug, pattern, &matches);
    ERR_BAIL(cmd);
    ERR_THROW(cnt < 0, cmd->aug, AUG_ECMDRUN,
              "  (error matching %s)\n", pattern);
    if (cnt == 0) {
        fprintf(cmd->out, "  (no matches)\n");
        goto done;
    }

    for (int i=0; i < cnt; i++) {
        const char *val;
        aug_get(cmd->aug, matches[i], &val);
        ERR_BAIL(cmd);
        if (val == NULL)
            val = "(none)";
        if (filter) {
            if (STREQ(value, val))
                fprintf(cmd->out, "%s\n", matches[i]);
        } else {
            fprintf(cmd->out, "%s = %s\n", matches[i], val);
        }
    }
 error:
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
    cnt = aug_rm(cmd->aug, path);
    if (! HAS_ERR(cmd))
        fprintf(cmd->out, "rm : %s %d\n", path, cnt);
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

    r = aug_mv(cmd->aug, src, dst);
    if (r < 0)
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "Moving %s to %s failed", src, dst);
}

static const struct command_opt_def cmd_mv_opts[] = {
    { .type = CMD_PATH, .name = "src", .optional = false,
      .help = "the tree to move" },
    { .type = CMD_PATH, .name = "dst", .optional = false,
      .help = "where to put the source tree" },
    CMD_OPT_DEF_LAST
};

static const char const cmd_mv_help[] =
    "Move node  SRC to DST.  SRC must match  exactly one node in  "
    "the tree.\n DST  must either  match  exactly one  node  in the  tree,  "
    "or may  not\n exist  yet. If  DST exists  already, it  and all  its  "
    "descendants are\n deleted.  If  DST  does  not   exist  yet,  it  and  "
    "all  its  missing\n ancestors are created.";

static const struct command_def cmd_mv_def = {
    .name = "mv",
    .opts = cmd_mv_opts,
    .handler = cmd_mv,
    .synopsis = "move a subtree",
    .help = cmd_mv_help
};

static const struct command_def cmd_move_def = {
    .name = "move",
    .opts = cmd_mv_opts,
    .handler = cmd_mv,
    .synopsis = "move a subtree (alias of 'mv')",
    .help = cmd_mv_help
};

static void cmd_rename(struct command *cmd) {
    const char *src = arg_value(cmd, "src");
    const char *lbl = arg_value(cmd, "lbl");
    int cnt;

    cnt = aug_rename(cmd->aug, src, lbl);
    if (cnt < 0)
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "Renaming %s to %s failed", src, lbl);
    if (! HAS_ERR(cmd))
        fprintf(cmd->out, "rename : %s to %s %d\n", src, lbl, cnt);
}

static const struct command_opt_def cmd_rename_opts[] = {
    { .type = CMD_PATH, .name = "src", .optional = false,
      .help = "the tree to rename" },
    { .type = CMD_STR, .name = "lbl", .optional = false,
      .help = "the new label" },
    CMD_OPT_DEF_LAST
};

static const char const cmd_rename_help[] =
    "Rename the label of all nodes matching SRC to LBL.";

static const struct command_def cmd_rename_def = {
    .name = "rename",
    .opts = cmd_rename_opts,
    .handler = cmd_rename,
    .synopsis = "rename a subtree label",
    .help = cmd_rename_help
};

static void cmd_set(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    const char *val = arg_value(cmd, "value");
    int r;

    r = aug_set(cmd->aug, path, val);
    if (r < 0)
        ERR_REPORT(cmd, AUG_ECMDRUN, "Setting %s failed", path);
}

static const struct command_opt_def cmd_set_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "set the value of this node" },
    { .type = CMD_STR, .name = "value", .optional = true,
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

    aug_setm(cmd->aug, base, sub, val);
}

static const struct command_opt_def cmd_setm_opts[] = {
    { .type = CMD_PATH, .name = "base", .optional = false,
      .help = "the base node" },
    { .type = CMD_PATH, .name = "sub", .optional = false,
      .help = "the subtree relative to the base" },
    { .type = CMD_STR, .name = "value", .optional = true,
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

    aug_setm(cmd->aug, base, sub, NULL);
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

static void cmd_span(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    int r;
    uint label_start, label_end, value_start, value_end, span_start, span_end;
    char *filename = NULL;
    const char *option = NULL;

    if (aug_get(cmd->aug, AUGEAS_SPAN_OPTION, &option) != 1) {
        printf("Error: option " AUGEAS_SPAN_OPTION " not found\n");
        return;
    }
    if (streqv(AUG_DISABLE, option)) {
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "Span is not enabled. To enable, run the commands:\n"
                   "    set %s %s\n    rm %s\n    load\n",
                   AUGEAS_SPAN_OPTION, AUG_ENABLE, AUGEAS_FILES_TREE);
        return;
    } else if (! streqv(AUG_ENABLE, option)) {
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "option %s must be %s or %s\n", AUGEAS_SPAN_OPTION,
                   AUG_ENABLE, AUG_DISABLE);
        return;
    }
    r = aug_span(cmd->aug, path, &filename, &label_start, &label_end,
                 &value_start, &value_end, &span_start, &span_end);
    ERR_THROW(r == -1, cmd, AUG_ECMDRUN, "failed to retrieve span");

    fprintf(cmd->out, "%s label=(%i:%i) value=(%i:%i) span=(%i,%i)\n",
            filename, label_start, label_end,
            value_start, value_end, span_start, span_end);
 error:
    free(filename);
}

static const struct command_opt_def cmd_span_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "path matching exactly one node" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_span_def = {
    .name = "span",
    .opts = cmd_span_opts,
    .handler = cmd_span,
    .synopsis = "print position in input file corresponding to tree",
    .help = "Print the name of the file from which the node PATH was generated, as\n well as information about the positions in the file  corresponding to\n the label, the value, and the  entire  node. PATH must match  exactly\n one node.\n\n You need to run 'set /augeas/span enable' prior to  loading files to\n enable recording of span information. It is disabled by default."
};

static void cmd_defvar(struct command *cmd) {
    const char *name = arg_value(cmd, "name");
    const char *path = arg_value(cmd, "expr");

    aug_defvar(cmd->aug, name, path);
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

    /* Make 'defnode foo ""' mean the same as 'defnode foo' */
    if (value != NULL && strlen(value) == 0)
        value = NULL;
    aug_defnode(cmd->aug, name, path, value, NULL);
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

    r = aug_set(cmd->aug, path, NULL);
    if (r < 0)
        ERR_REPORT(cmd, AUG_ECMDRUN, "Clearing %s failed", path);
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

static void cmd_touch(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    int r;

    r = aug_match(cmd->aug, path, NULL);
    if (r == 0) {
        r = aug_set(cmd->aug, path, NULL);
        if (r < 0)
            ERR_REPORT(cmd, AUG_ECMDRUN, "Touching %s failed", path);
    }
}

static const struct command_opt_def cmd_touch_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "touch this node" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_touch_def = {
    .name = "touch",
    .opts = cmd_touch_opts,
    .handler = cmd_touch,
    .synopsis = "create a new node",
    .help = "Create PATH with the value NULL if it is not in the tree yet.  "
    "All its\n ancestors will also be created.  These new tree entries will "
    "appear\n last amongst their siblings."
};

static void cmd_get(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    const char *val;
    int r;

    r = aug_get(cmd->aug, path, &val);
    ERR_RET(cmd);
    fprintf(cmd->out, "%s", path);
    if (r == 0) {
        fprintf(cmd->out, " (o)\n");
    } else if (val == NULL) {
        fprintf(cmd->out, " (none)\n");
    } else {
        fprintf(cmd->out, " = %s\n", val);
    }
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

static void cmd_label(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    const char *lbl;
    int r;

    r = aug_label(cmd->aug, path, &lbl);
    ERR_RET(cmd);
    fprintf(cmd->out, "%s", path);
    if (r == 0) {
        fprintf(cmd->out, " (o)\n");
    } else if (lbl == NULL) {
        fprintf(cmd->out, " (none)\n");
    } else {
        fprintf(cmd->out, " = %s\n", lbl);
    }
}

static const struct command_opt_def cmd_label_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "get the label of this node" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_label_def = {
    .name = "label",
    .opts = cmd_label_opts,
    .handler = cmd_label,
    .synopsis = "get the label of a node",
    .help = "Get and print the label associated with PATH"
};

static void cmd_print(struct command *cmd) {
    const char *path = arg_value(cmd, "path");

    aug_print(cmd->aug, cmd->out, path);
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

static void cmd_dump_xml(struct command *cmd) {
    const char *path = arg_value(cmd, "path");
    const char *filename = arg_value(cmd, "filename");
    xmlNodePtr xmldoc;
    int r;

    r = aug_to_xml(cmd->aug, path, &xmldoc, 0);
    if (r < 0)
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "XML export of path %s to file %s failed", path, filename);

    xmlElemDump(stdout, NULL, xmldoc);
    printf("\n");

    if (filename != NULL) {
        printf("Saving to %s\n", filename);
    }

    xmlFreeNode(xmldoc);
}

static const struct command_opt_def cmd_dump_xml_opts[] = {
    { .type = CMD_PATH, .name = "path", .optional = true,
      .help = "print this subtree" },
    { .type = CMD_NONE, .name = "filename", .optional = true,
      .help = "save to this file" },
    CMD_OPT_DEF_LAST
};

static const struct command_def cmd_dump_xml_def = {
    .name = "dump-xml",
    .opts = cmd_dump_xml_opts,
    .handler = cmd_dump_xml,
    .synopsis = "print a subtree as XML",
    .help = "Export entries in the tree as XML. If PATH is given, printing starts there,\n otherwise the whole tree is printed. If FILENAME is given, the XML is saved\n to the given file."
};

static void cmd_transform(struct command *cmd) {
    const char *lens = arg_value(cmd, "lens");
    const char *filter = arg_value(cmd, "filter");
    const char *file = arg_value(cmd, "file");
    int r, excl = 0;

    if (STREQ("excl", filter))
        excl = 1;
    else if (STREQ("incl", filter))
        excl = 0;
    else
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "FILTER must be \"incl\" or \"excl\"");

    r = aug_transform(cmd->aug, lens, file, excl);
    if (r < 0)
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "Adding transform for %s on lens %s failed", lens, file);
}

static const struct command_opt_def cmd_transform_opts[] = {
    { .type = CMD_PATH, .name = "lens", .optional = false,
      .help = "the lens to use" },
    { .type = CMD_PATH, .name = "filter", .optional = false,
      .help = "the type of filter, either \"incl\" or \"excl\"" },
    { .type = CMD_PATH, .name = "file", .optional = false,
      .help = "the file to associate to the lens" },
    CMD_OPT_DEF_LAST
};

static const char const cmd_transform_help[] =
    "Add a transform for FILE using LENS. The LENS may be a module name or a\n"
    " full lens name.  If a module name is given, then \"lns\" will be the lens\n"
    " assumed.  The FILTER must be either \"incl\" or \"excl\".  If the filter is\n"
    " \"incl\",  the FILE will be parsed by the LENS.  If the filter is \"excl\",\n"
    " the FILE will be excluded from the LENS. FILE may contain wildcards." ;

static const struct command_def cmd_transform_def = {
    .name = "transform",
    .opts = cmd_transform_opts,
    .handler = cmd_transform,
    .synopsis = "add a file transform",
    .help = cmd_transform_help
};

static void cmd_save(struct command *cmd) {
    int r;
    r = aug_save(cmd->aug);
    if (r == -1) {
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "saving failed (run 'print /augeas//error' for details)");
    } else {
        r = aug_match(cmd->aug, "/augeas/events/saved", NULL);
        if (r > 0) {
            fprintf(cmd->out, "Saved %d file(s)\n", r);
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
    r = aug_load(cmd->aug);
    if (r == -1) {
        ERR_REPORT(cmd, AUG_ECMDRUN,
                   "loading failed (run 'print /augeas//error' for details)");
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
        ERR_REPORT(cmd, AUG_ECMDRUN,
          "the <WHERE> argument for ins must be either 'before' or 'after'.");
        return;
    }

    aug_insert(cmd->aug, path, label, before);
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

static const char const cmd_ins_help[] =
    "Insert a new node with label LABEL right before or after "
    "PATH into the\n tree. WHERE must be either 'before' or 'after'.";

static const struct command_def cmd_ins_def = {
    .name = "ins",
    .opts = cmd_ins_opts,
    .handler = cmd_ins,
    .synopsis = "insert new node",
    .help = cmd_ins_help
};

static const struct command_def cmd_insert_def = {
    .name = "insert",
    .opts = cmd_ins_opts,
    .handler = cmd_ins,
    .synopsis = "insert new node (alias of 'ins')",
    .help = cmd_ins_help
};

static void cmd_store(struct command *cmd) {
    const char *lens = arg_value(cmd, "lens");
    const char *path = arg_value(cmd, "path");
    const char *node = arg_value(cmd, "node");

    aug_text_store(cmd->aug, lens, node, path);
}

static const struct command_opt_def cmd_store_opts[] = {
    { .type = CMD_STR, .name = "lens", .optional = false,
      .help = "the name of the lens" },
    { .type = CMD_PATH, .name = "node", .optional = false,
      .help = "where to find the input text" },
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "where to store parsed text" },
    CMD_OPT_DEF_LAST
};

static const char const cmd_store_help[] =
    "Parse NODE using LENS and store the resulting tree at PATH.";

static const struct command_def cmd_store_def = {
    .name = "store",
    .opts = cmd_store_opts,
    .handler = cmd_store,
    .synopsis = "parse text into tree",
    .help = cmd_store_help
};

static void cmd_retrieve(struct command *cmd) {
    const char *lens = arg_value(cmd, "lens");
    const char *node_in = arg_value(cmd, "node_in");
    const char *path = arg_value(cmd, "path");
    const char *node_out = arg_value(cmd, "node_out");

    aug_text_retrieve(cmd->aug, lens, node_in, path, node_out);
}

static const struct command_opt_def cmd_retrieve_opts[] = {
    { .type = CMD_STR, .name = "lens", .optional = false,
      .help = "the name of the lens" },
    { .type = CMD_PATH, .name = "node_in", .optional = false,
      .help = "the node containing the initial text (path expression)" },
    { .type = CMD_PATH, .name = "path", .optional = false,
      .help = "the tree to transform (path expression)" },
    { .type = CMD_PATH, .name = "node_out", .optional = false,
      .help = "where to store the resulting text (path expression)" },
    CMD_OPT_DEF_LAST
};

static const char const cmd_retrieve_help[] =
    "Transform tree at PATH back into text using lens LENS and store the\n"
    " resulting string at NODE_OUT. Assume that the tree was initially read in\n"
    " with the same lens and the string stored at NODE_IN as input.";

static const struct command_def cmd_retrieve_def = {
    .name = "retrieve",
    .opts = cmd_retrieve_opts,
    .handler = cmd_retrieve,
    .synopsis = "transform tree into text",
    .help = cmd_retrieve_help
};

/* Groups of commands */
static const struct command_grp_def cmd_grp_admin_def = {
    .name = "Admin",
    .commands = {
        &cmd_help_def,
        &cmd_load_def,
        &cmd_quit_def,
        &cmd_retrieve_def,
        &cmd_save_def,
        &cmd_store_def,
        &cmd_transform_def,
        &cmd_def_last
    }
};

static const struct command_grp_def cmd_grp_read_def = {
    .name = "Read",
    .commands = {
        &cmd_dump_xml_def,
        &cmd_get_def,
        &cmd_label_def,
        &cmd_ls_def,
        &cmd_match_def,
        &cmd_print_def,
        &cmd_span_def,
        &cmd_def_last
    }
};

static const struct command_grp_def cmd_grp_write_def = {
    .name = "Write",
    .commands = {
        &cmd_clear_def,
        &cmd_clearm_def,
        &cmd_ins_def,
        &cmd_insert_def,
        &cmd_mv_def,
        &cmd_move_def,
        &cmd_rename_def,
        &cmd_rm_def,
        &cmd_set_def,
        &cmd_setm_def,
        &cmd_touch_def,
        &cmd_def_last
    }
};

static const struct command_grp_def cmd_grp_pathx_def = {
    .name = "Path expression",
    .commands = {
        &cmd_defnode_def,
        &cmd_defvar_def,
        &cmd_def_last
    }
};

static const struct command_grp_def const *cmd_groups[] = {
    &cmd_grp_admin_def,
    &cmd_grp_read_def,
    &cmd_grp_write_def,
    &cmd_grp_pathx_def,
    &cmd_grp_def_last
};

static const struct command_def *lookup_cmd_def(const char *name) {
    for (int i = 0; cmd_groups[i]->name != NULL; i++) {
        for (int j = 0; cmd_groups[i]->commands[j]->name != NULL; j++) {
            if (STREQ(name, cmd_groups[i]->commands[j]->name))
                return cmd_groups[i]->commands[j];
        }
    }
    return NULL;
}

static void cmd_help(struct command *cmd) {
    const char *name = arg_value(cmd, "command");
    char buf[100];

    if (name == NULL) {
        //fprintf(cmd->out, "Commands:\n\n");
        fprintf(cmd->out, "\n");
        for (int i=0; cmd_groups[i]->name != NULL; i++) {
            fprintf(cmd->out, "%s commands:\n", cmd_groups[i]->name);
            for (int j=0; cmd_groups[i]->commands[j]->name != NULL; j++) {
                const struct command_def *def = cmd_groups[i]->commands[j];
                fprintf(cmd->out, "  %-10s - %s\n", def->name, def->synopsis);
            }
            fprintf(cmd->out, "\n");
        }
        fprintf(cmd->out,
           "Type 'help <command>' for more information on a command\n\n");
    } else {
        const struct command_def *def = lookup_cmd_def(name);
        const struct command_opt_def *odef = NULL;

        ERR_THROW(def == NULL, cmd->aug, AUG_ECMDRUN,
                  "unknown command %s\n", name);
        fprintf(cmd->out, "  COMMAND\n");
        fprintf(cmd->out, "    %s - %s\n\n", name, def->synopsis);
        fprintf(cmd->out, "  SYNOPSIS\n");
        fprintf(cmd->out, "    %s", name);

        for (odef = def->opts; odef->name != NULL; odef++) {
            format_defname(buf, odef, true);
            fprintf(cmd->out, "%s", buf);
        }
        fprintf(cmd->out, "\n\n");
        fprintf(cmd->out, "  DESCRIPTION\n");
        format_desc(def->help);
        if (def->opts->name != NULL) {
            fprintf(cmd->out, "  OPTIONS\n");
            for (odef = def->opts; odef->name != NULL; odef++) {
                const char *help = odef->help;
                if (help == NULL)
                    help = "";
                format_defname(buf, odef, false);
                fprintf(cmd->out, "    %-10s %s\n", buf, help);
            }
        }
        fprintf(cmd->out, "\n");
    }
 error:
    return;
}

int aug_srun(augeas *aug, FILE *out, const char *text) {
    char *line = NULL;
    const char *eol;
    struct command cmd;
    int result = 0;

    api_entry(aug);

    if (text == NULL)
        goto done;

    MEMZERO(&cmd, 1);
    cmd.aug = aug;
    cmd.error = aug->error;
    cmd.out = out;

    while (*text != '\0' && result >= 0) {
        eol = strchrnul(text, '\n');
        while (isspace(*text) && text < eol) text++;
        if (*text == '\0')
            break;
        if (*text == '#' || text == eol) {
            text = (*eol == '\0') ? eol : eol + 1;
            continue;
        }

        line = strndup(text, eol - text);
        ERR_NOMEM(line == NULL, aug);

        if (parseline(&cmd, line) == 0) {
            cmd.def->handler(&cmd);
            result += 1;
        } else {
            result = -1;
        }

        ERR_BAIL(aug);
        if (result >= 0 && cmd.quit) {
            result = -2;
            goto done;
        }

        free_command_opts(&cmd);
        FREE(line);
        text = (*eol == '\0') ? eol : eol + 1;
    }
 done:
    FREE(line);

    api_exit(aug);
    return result;
 error:
    result = -1;
    goto done;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
