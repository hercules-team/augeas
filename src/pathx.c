/*
 * pathx.c: handling path expressions
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
#include <internal.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <ctype.h>

#include "ref.h"
#include "regexp.h"
#include "errcode.h"

static const char *const errcodes[] = {
    "no error",
    "empty name",
    "illegal string literal",
    "illegal number",
    "string missing ending ' or \"",
    "expected '='",
    "allocation failed",
    "unmatched '['",
    "unmatched '('",
    "expected a '/'",
    "internal error",                             /* PATHX_EINTERNAL */
    "type error",                                 /* PATHX_ETYPE */
    "undefined variable",                         /* PATHX_ENOVAR */
    "garbage at end of path expression",          /* PATHX_EEND */
    "no match for path expression",               /* PATHX_ENOMATCH */
    "wrong number of arguments in function call", /* PATHX_EARITY */
    "invalid regular expression",                 /* PATHX_EREGEXP */
    "too many matches"                            /* PATHX_EMMATCH */
};

/*
 * Path expressions are strings that use a notation modelled on XPath.
 */

enum type {
    T_NONE = 0,     /* Not a type */
    T_NODESET,
    T_BOOLEAN,
    T_NUMBER,
    T_STRING,
    T_REGEXP
};

enum expr_tag {
    E_FILTER,
    E_BINARY,
    E_VALUE,
    E_VAR,
    E_APP
};

enum binary_op {
    OP_EQ,         /* '='  */
    OP_NEQ,        /* '!=' */
    OP_LT,         /* '<'  */
    OP_LE,         /* '<=' */
    OP_GT,         /* '>'  */
    OP_GE,         /* '>=' */
    OP_PLUS,       /* '+'  */
    OP_MINUS,      /* '-'  */
    OP_STAR,       /* '*'  */
    OP_AND,        /* 'and' */
    OP_OR,         /* 'or' */
    OP_RE_MATCH,   /* '=~' */
    OP_RE_NOMATCH, /* '!~' */
    OP_UNION       /* '|' */
};

struct pred {
    int               nexpr;
    struct expr     **exprs;
};

enum axis {
    SELF,
    CHILD,
    DESCENDANT,
    DESCENDANT_OR_SELF,
    PARENT,
    ANCESTOR,
    ROOT,
    PRECEDING_SIBLING,
    FOLLOWING_SIBLING
};

/* This array is indexed by enum axis */
static const char *const axis_names[] = {
    "self",
    "child",
    "descendant",
    "descendant-or-self",
    "parent",
    "ancestor",
    "root",
    "preceding-sibling",
    "following-sibling"
};

static const char *const axis_sep = "::";

/* Doubly linked list of location steps. Besides the information from the
 * path expression, also contains information to iterate over a node set,
 * in particular, the context node CTX for the step, and the current node
 * CUR within that context.
 */
struct step {
    struct step *next;
    enum axis    axis;
    char        *name;              /* NULL to match any name */
    struct pred *predicates;
};

/* Initialise the root nodeset with the first step */
static struct tree *step_root(struct step *step, struct tree *ctx,
                              struct tree *root_ctx);
/* Iteration over the nodes on a step, ignoring the predicates */
static struct tree *step_first(struct step *step, struct tree *ctx);
static struct tree *step_next(struct step *step, struct tree *ctx,
                              struct tree *node);

struct pathx_symtab {
    struct pathx_symtab *next;
    char                *name;
    struct value        *value;
};

struct pathx {
    struct state   *state;
    struct nodeset *nodeset;
    int             node;
    struct tree    *origin;
};

#define L_BRACK '['
#define R_BRACK ']'

struct locpath {
    struct step *steps;
};

struct nodeset {
    struct tree **nodes;
    size_t        used;
    size_t        size;
};

typedef uint32_t value_ind_t;

struct value {
    enum type tag;
    union {
        struct nodeset  *nodeset;     /* T_NODESET */
        int              number;      /* T_NUMBER  */
        char            *string;      /* T_STRING  */
        bool             boolval;     /* T_BOOLEAN */
        struct regexp   *regexp;      /* T_REGEXP  */
    };
};

struct expr {
    enum expr_tag tag;
    enum type     type;
    union {
        struct {                       /* E_FILTER */
            struct expr     *primary;
            struct pred     *predicates;
            struct locpath  *locpath;
        };
        struct {                       /* E_BINARY */
            enum binary_op op;
            struct expr *left;
            struct expr *right;
        };
        value_ind_t      value_ind;    /* E_VALUE */
        char            *ident;        /* E_VAR */
        struct {                       /* E_APP */
            const struct func *func;
            struct expr       **args;
        };
    };
};

struct locpath_trace {
    unsigned int      maxns;
    struct nodeset  **ns;
    struct locpath   *lp;
};

/* Internal state of the evaluator/parser */
struct state {
    pathx_errcode_t errcode;
    const char     *file;
    int             line;
    char           *errmsg;

    const char     *txt;  /* Entire expression */
    const char     *pos;  /* Current position within TXT during parsing */

    struct tree *ctx; /* The current node */
    uint            ctx_pos;
    uint            ctx_len;

    struct tree *root_ctx; /* Root context for relative paths */

    /* A table of all values. The table is dynamically reallocated, i.e.
     * pointers to struct value should not be used across calls that
     * might allocate new values
     *
     * value_pool[0] is always the boolean false, and value_pool[1]
     * always the boolean true
     */
    struct value  *value_pool;
    value_ind_t    value_pool_used;
    value_ind_t    value_pool_size;
    /* Stack of values (as indices into value_pool), with bottom of
       stack in values[0] */
    value_ind_t   *values;
    size_t         values_used;
    size_t         values_size;
    /* Stack of expressions, with bottom of stack in exprs[0] */
    struct expr  **exprs;
    size_t         exprs_used;
    size_t         exprs_size;
    /* Trace of a locpath evaluation, needed by pathx_expand_tree.
       Generally NULL, unless a trace is needed.
     */
    struct locpath_trace *locpath_trace;
    /* Symbol table for variable lookups */
    struct pathx_symtab *symtab;
    /* Error structure, used to communicate errors to struct augeas;
     * we never own this structure, and therefore never free it */
    struct error        *error;
};

/* We consider NULL and the empty string to be equal */
ATTRIBUTE_PURE
static inline int streqx(const char *s1, const char *s2) {
    if (s1 == NULL)
        return (s2 == NULL || strlen(s2) == 0);
    if (s2 == NULL)
        return strlen(s1) == 0;
    return STREQ(s1, s2);
}

/* Functions */

typedef void (*func_impl_t)(struct state *state);

struct func {
    const char      *name;
    unsigned int     arity;
    enum type        type;
    const enum type *arg_types;
    func_impl_t      impl;
};

static void func_last(struct state *state);
static void func_position(struct state *state);
static void func_count(struct state *state);
static void func_label(struct state *state);
static void func_regexp(struct state *state);
static void func_glob(struct state *state);
static void func_int(struct state *state);

static const enum type const arg_types_nodeset[] = { T_NODESET };
static const enum type const arg_types_string[] = { T_STRING };
static const enum type const arg_types_bool[] = { T_BOOLEAN };

static const struct func builtin_funcs[] = {
    { .name = "last", .arity = 0, .type = T_NUMBER, .arg_types = NULL,
      .impl = func_last },
    { .name = "position", .arity = 0, .type = T_NUMBER, .arg_types = NULL,
      .impl = func_position },
    { .name = "label", .arity = 0, .type = T_STRING, .arg_types = NULL,
      .impl = func_label },
    { .name = "count", .arity = 1, .type = T_NUMBER,
      .arg_types = arg_types_nodeset,
      .impl = func_count },
    { .name = "regexp", .arity = 1, .type = T_REGEXP,
      .arg_types = arg_types_string,
      .impl = func_regexp },
    { .name = "regexp", .arity = 1, .type = T_REGEXP,
      .arg_types = arg_types_nodeset,
      .impl = func_regexp },
    { .name = "glob", .arity = 1, .type = T_REGEXP,
      .arg_types = arg_types_string,
      .impl = func_glob },
    { .name = "glob", .arity = 1, .type = T_REGEXP,
      .arg_types = arg_types_nodeset,
      .impl = func_glob },
    { .name = "int", .arity = 1, .type = T_NUMBER,
      .arg_types = arg_types_string, .impl = func_int },
    { .name = "int", .arity = 1, .type = T_NUMBER,
      .arg_types = arg_types_nodeset, .impl = func_int },
    { .name = "int", .arity = 1, .type = T_NUMBER,
      .arg_types = arg_types_bool, .impl = func_int }
};

#define RET_ON_ERROR                                                    \
    if (state->errcode != PATHX_NOERROR) return

#define RET0_ON_ERROR                                                   \
    if (state->errcode != PATHX_NOERROR) return 0

#define STATE_ERROR(state, err)                                         \
    do {                                                                \
        state->errcode = err;                                           \
        state->file = __FILE__;                                         \
        state->line = __LINE__;                                         \
    } while (0)

#define HAS_ERROR(state) (state->errcode != PATHX_NOERROR)

#define STATE_ENOMEM STATE_ERROR(state, PATHX_ENOMEM)

#define ENOMEM_ON_NULL(state, v)                                        \
    do {                                                                \
        if (v == NULL) {                                                \
            STATE_ERROR(state, PATHX_ENOMEM);                           \
            return NULL;                                                \
        }                                                               \
    } while (0);

/*
 * Free the various data structures
 */

static void free_expr(struct expr *expr);

static void free_pred(struct pred *pred) {
    if (pred == NULL)
        return;

    for (int i=0; i < pred->nexpr; i++) {
        free_expr(pred->exprs[i]);
    }
    free(pred->exprs);
    free(pred);
}

static void free_step(struct step *step) {
    while (step != NULL) {
        struct step *del = step;
        step = del->next;
        free(del->name);
        free_pred(del->predicates);
        free(del);
    }
}

static void free_locpath(struct locpath *locpath) {
    if (locpath == NULL)
        return;
    while (locpath->steps != NULL) {
        struct step *step = locpath->steps;
        locpath->steps = step->next;
        free(step->name);
        free_pred(step->predicates);
        free(step);
    }
    free(locpath);
}

static void free_expr(struct expr *expr) {
    if (expr == NULL)
        return;
    switch (expr->tag) {
    case E_FILTER:
        free_expr(expr->primary);
        free_pred(expr->predicates);
        free_locpath(expr->locpath);
        break;
    case E_BINARY:
        free_expr(expr->left);
        free_expr(expr->right);
        break;
    case E_VALUE:
        break;
    case E_VAR:
        free(expr->ident);
        break;
    case E_APP:
        for (int i=0; i < expr->func->arity; i++)
            free_expr(expr->args[i]);
        free(expr->args);
        break;
    default:
        assert(0);
    }
    free(expr);
}

static void free_nodeset(struct nodeset *ns) {
    if (ns != NULL) {
        free(ns->nodes);
        free(ns);
    }
}

/* Free all objects used by VALUE, but not VALUE itself */
static void release_value(struct value *v) {
    if (v == NULL)
        return;

    switch (v->tag) {
    case T_NODESET:
        free_nodeset(v->nodeset);
        break;
    case T_STRING:
        free(v->string);
        break;
    case T_BOOLEAN:
    case T_NUMBER:
        break;
    case T_REGEXP:
        unref(v->regexp, regexp);
        break;
    default:
        assert(0);
    }
}

static void free_state(struct state *state) {
    if (state == NULL)
        return;

    for(int i=0; i < state->exprs_used; i++)
        free_expr(state->exprs[i]);
    free(state->exprs);

    for(int i=0; i < state->value_pool_used; i++)
        release_value(state->value_pool + i);
    free(state->value_pool);
    free(state->values);
    free(state);
}

void free_pathx(struct pathx *pathx) {
    if (pathx == NULL)
        return;
    free_state(pathx->state);
    free(pathx);
}

/*
 * Nodeset helpers
 */
static struct nodeset *make_nodeset(struct state *state) {
    struct nodeset *result;
    if (ALLOC(result) < 0)
        STATE_ENOMEM;
    return result;
}

static void ns_add(struct nodeset *ns, struct tree *node,
                   struct state *state) {
    for (int i=0; i < ns->used; i++)
        if (ns->nodes[i] == node)
            return;
    if (ns->used >= ns->size) {
        size_t size = 2 * ns->size;
        if (size < 10) size = 10;
        if (REALLOC_N(ns->nodes, size) < 0)
            STATE_ENOMEM;
        ns->size = size;
    }
    ns->nodes[ns->used] = node;
    ns->used += 1;
}

static struct nodeset *
clone_nodeset(struct nodeset *ns, struct state *state)
{
    struct nodeset *clone;
    if (ALLOC(clone) < 0) {
        STATE_ENOMEM;
        return NULL;
    }
    if (ALLOC_N(clone->nodes, ns->used) < 0) {
        free(clone);
        STATE_ENOMEM;
        return NULL;
    }
    clone->used = ns->used;
    clone->size = ns->used;
    for (int i=0; i < ns->used; i++)
        clone->nodes[i] = ns->nodes[i];
    return clone;
}

/*
 * Handling values
 */
static value_ind_t make_value(enum type tag, struct state *state) {
    assert(tag != T_BOOLEAN);

    if (state->value_pool_used >= state->value_pool_size) {
        value_ind_t new_size = 2*state->value_pool_size;
        if (new_size <= state->value_pool_size) {
            STATE_ENOMEM;
            return 0;
        }
        if (REALLOC_N(state->value_pool, new_size) < 0) {
            STATE_ENOMEM;
            return 0;
        }
        state->value_pool_size = new_size;
    }
    state->value_pool[state->value_pool_used].tag = tag;
    state->value_pool[state->value_pool_used].nodeset = NULL;
    return state->value_pool_used++;
}

ATTRIBUTE_UNUSED
static value_ind_t clone_value(struct value *v, struct state *state) {
    value_ind_t vind = make_value(v->tag, state);
    RET0_ON_ERROR;
    struct value *clone = state->value_pool + vind;

    switch (v->tag) {
    case T_NODESET:
        clone->nodeset = clone_nodeset(v->nodeset, state);
        break;
    case T_NUMBER:
        clone->number = v->number;
        break;
    case T_STRING:
        clone->string = strdup(v->string);
        if (clone->string == NULL) {
            FREE(clone);
            STATE_ENOMEM;
        }
        break;
    case T_BOOLEAN:
        clone->boolval = v->boolval;
        break;
    case T_REGEXP:
        clone->regexp = ref(v->regexp);
        break;
    default:
        assert(0);
    }
    return vind;
}

static value_ind_t pop_value_ind(struct state *state) {
    if (state->values_used > 0) {
        state->values_used -= 1;
        return state->values[state->values_used];
    } else {
        STATE_ERROR(state, PATHX_EINTERNAL);
        assert(0);
        return 0;
    }
}

static struct value *pop_value(struct state *state) {
    value_ind_t vind = pop_value_ind(state);
    if (HAS_ERROR(state))
        return NULL;
    return state->value_pool + vind;
}

static void push_value(value_ind_t vind, struct state *state) {
    if (state->values_used >= state->values_size) {
        size_t new_size = 2*state->values_size;
        if (new_size == 0) new_size = 8;
        if (REALLOC_N(state->values, new_size) < 0) {
            STATE_ENOMEM;
            return;
        }
        state->values_size = new_size;
    }
    state->values[state->values_used++] = vind;
}

static void push_boolean_value(int b, struct state *state) {
    push_value(b != 0, state);
}

ATTRIBUTE_PURE
static struct value *expr_value(struct expr *expr, struct state *state) {
    return state->value_pool + expr->value_ind;
}

/*************************************************************************
 * Evaluation
 ************************************************************************/
static void eval_expr(struct expr *expr, struct state *state);

static void func_last(struct state *state) {
    value_ind_t vind = make_value(T_NUMBER, state);
    RET_ON_ERROR;

    state->value_pool[vind].number = state->ctx_len;
    push_value(vind, state);
}

static void func_position(struct state *state) {
    value_ind_t vind = make_value(T_NUMBER, state);
    RET_ON_ERROR;

    state->value_pool[vind].number = state->ctx_pos;
    push_value(vind, state);
}

static void func_count(struct state *state) {
    value_ind_t vind = make_value(T_NUMBER, state);
    RET_ON_ERROR;

    struct value *ns = pop_value(state);
    state->value_pool[vind].number = ns->nodeset->used;
    push_value(vind, state);
}

static void func_label(struct state *state) {
    value_ind_t vind = make_value(T_STRING, state);
    char *s;

    RET_ON_ERROR;

    if (state->ctx->label)
        s = strdup(state->ctx->label);
    else
        s = strdup("");
    if (s == NULL) {
        STATE_ENOMEM;
        return;
    }
    state->value_pool[vind].string = s;
    push_value(vind, state);
}

static void func_int(struct state *state) {
    value_ind_t vind = make_value(T_NUMBER, state);
    int64_t i = -1;
    RET_ON_ERROR;

    struct value *v = pop_value(state);
    if (v->tag == T_BOOLEAN) {
        i = v->boolval;
    } else {
        const char *s = NULL;
        if (v->tag == T_STRING) {
            s = v->string;
        } else {
            /* T_NODESET */
            if (v->nodeset->used != 1) {
                STATE_ERROR(state, PATHX_EMMATCH);
                return;
            }
            s = v->nodeset->nodes[0]->value;
        }
        if (s != NULL) {
            int r;
            r = xstrtoint64(s, 10, &i);
            if (r < 0) {
                STATE_ERROR(state, PATHX_ENUMBER);
                return;
            }
        }
    }
    state->value_pool[vind].number = i;
    push_value(vind, state);
}

static struct regexp *
nodeset_as_regexp(struct info *info, struct nodeset *ns, int glob) {
    struct regexp *result = NULL;
    struct regexp **rx = NULL;
    int used = 0;

    for (int i = 0; i < ns->used; i++) {
        if (ns->nodes[i]->value != NULL)
            used += 1;
    }

    if (used == 0) {
        /* If the nodeset is empty, make sure we produce a regexp
         * that never matches anything */
        result = make_regexp_unescape(info, "[^\001-\7ff]", 0);
    } else {
        if (ALLOC_N(rx, ns->used) < 0)
            goto error;
        for (int i=0; i < ns->used; i++) {
            if (ns->nodes[i]->value == NULL)
                continue;

            if (glob)
                rx[i] = make_regexp_from_glob(info, ns->nodes[i]->value);
            else
                rx[i] = make_regexp_unescape(info, ns->nodes[i]->value, 0);
            if (rx[i] == NULL)
                goto error;
        }
        result = regexp_union_n(info, ns->used, rx);
    }

 error:
    if (rx != NULL) {
        for (int i=0; i < ns->used; i++)
            unref(rx[i], regexp);
        free(rx);
    }
    return result;
}

static void func_regexp_or_glob(struct state *state, int glob) {
    value_ind_t vind = make_value(T_REGEXP, state);
    int r;

    RET_ON_ERROR;

    struct value *v = pop_value(state);
    struct regexp *rx = NULL;

    if (v->tag == T_STRING) {
        if (glob)
            rx = make_regexp_from_glob(state->error->info, v->string);
        else
            rx = make_regexp_unescape(state->error->info, v->string, 0);
    } else if (v->tag == T_NODESET) {
        rx = nodeset_as_regexp(state->error->info, v->nodeset, glob);
    } else {
        assert(0);
    }

    if (rx == NULL) {
        STATE_ENOMEM;
        return;
    }

    state->value_pool[vind].regexp = rx;
    r = regexp_compile(rx);
    if (r < 0) {
        const char *msg;
        regexp_check(rx, &msg);
        state->errmsg = strdup(msg);
        STATE_ERROR(state, PATHX_EREGEXP);
        return;
    }
    push_value(vind, state);
}

static void func_regexp(struct state *state) {
    func_regexp_or_glob(state, 0);
}

static void func_glob(struct state *state) {
    func_regexp_or_glob(state, 1);
}

static bool coerce_to_bool(struct value *v) {
    switch (v->tag) {
    case T_NODESET:
        return v->nodeset->used > 0;
        break;
    case T_BOOLEAN:
        return v->boolval;
        break;
    case T_NUMBER:
        return v->number > 0;
        break;
    case T_STRING:
        return strlen(v->string) > 0;
        break;
    case T_REGEXP:
        return true;
    default:
        assert(0);
        return false;
    }
}

static int calc_eq_nodeset_nodeset(struct nodeset *ns1, struct nodeset *ns2,
                                   int neq) {
    for (int i1=0; i1 < ns1->used; i1++) {
        struct tree *t1 = ns1->nodes[i1];
        for (int i2=0; i2 < ns2->used; i2++) {
            struct tree *t2 = ns2->nodes[i2];
            if (neq) {
                if (!streqx(t1->value, t2->value))
                    return 1;
            } else {
                if (streqx(t1->value, t2->value))
                    return 1;
            }
        }
    }
    return 0;
}

static int calc_eq_nodeset_string(struct nodeset *ns, const char *s,
                                  int neq) {
    for (int i=0; i < ns->used; i++) {
        struct tree *t = ns->nodes[i];
        if (neq) {
            if (!streqx(t->value, s))
                return 1;
        } else {
            if (streqx(t->value, s))
                return 1;
        }
    }
    return 0;
}

static void eval_eq(struct state *state, int neq) {
    struct value *r = pop_value(state);
    struct value *l = pop_value(state);
    int res;

    if (l->tag == T_NODESET && r->tag == T_NODESET) {
        res = calc_eq_nodeset_nodeset(l->nodeset, r->nodeset, neq);
    } else if (l->tag == T_NODESET) {
        res = calc_eq_nodeset_string(l->nodeset, r->string, neq);
    } else if (r->tag == T_NODESET) {
        res = calc_eq_nodeset_string(r->nodeset, l->string, neq);
    } else if (l->tag == T_NUMBER && r->tag == T_NUMBER) {
        if (neq)
            res = (l->number != r->number);
        else
            res = (l->number == r->number);
    } else {
        assert(l->tag == T_STRING);
        assert(r->tag == T_STRING);
        res = streqx(l->string, r->string);
        if (neq)
            res = !res;
    }
    RET_ON_ERROR;

    push_boolean_value(res, state);
}

static void eval_arith(struct state *state, enum binary_op op) {
    value_ind_t vind = make_value(T_NUMBER, state);
    struct value *r = pop_value(state);
    struct value *l = pop_value(state);
    int res;

    assert(l->tag == T_NUMBER);
    assert(r->tag == T_NUMBER);

    RET_ON_ERROR;

    if (op == OP_PLUS)
        res = l->number + r->number;
    else if (op == OP_MINUS)
        res = l->number - r->number;
    else if (op == OP_STAR)
        res = l->number * r->number;
    else
        assert(0);

    state->value_pool[vind].number = res;
    push_value(vind, state);
}

static void eval_rel(struct state *state, bool greater, bool equal) {
    struct value *r, *l;
    int res;

    /* We always check l < r or l <= r */
    if (greater) {
        l = pop_value(state);
        r = pop_value(state);
    } else {
        r = pop_value(state);
        l = pop_value(state);
    }
    if (l->tag == T_NUMBER) {
        if (equal)
            res = (l->number <= r->number);
        else
            res = (l->number < r->number);
    } else if (l->tag == T_STRING) {
        int cmp = strcmp(l->string, r->string);
        if (equal)
            res = cmp <= 0;
        else
            res = cmp < 0;
    } else {
        assert(0);
    }

    push_boolean_value(res, state);
}

static void eval_and_or(struct state *state, enum binary_op op) {
    struct value *r = pop_value(state);
    struct value *l = pop_value(state);
    bool left = coerce_to_bool(l);
    bool right = coerce_to_bool(r);


    if (op == OP_AND)
        push_boolean_value(left && right, state);
    else
        push_boolean_value(left || right, state);
}

static bool eval_re_match_str(struct state *state, struct regexp *rx,
                              const char *str) {
    int r;

    if (str == NULL)
        str = "";

    r = regexp_match(rx, str, strlen(str), 0, NULL);
    if (r == -2) {
        STATE_ERROR(state, PATHX_EINTERNAL);
    } else if (r == -3) {
        /* We should never get this far; func_regexp should catch
         * invalid regexps */
        assert(false);
    }
    return r == strlen(str);
}

static void eval_union(struct state *state) {
    value_ind_t vind = make_value(T_NODESET, state);
    struct value *r = pop_value(state);
    struct value *l = pop_value(state);
    struct nodeset *res = NULL;

    assert(l->tag == T_NODESET);
    assert(r->tag == T_NODESET);

    RET_ON_ERROR;

    res = clone_nodeset(l->nodeset, state);
    RET_ON_ERROR;
    for (int i=0; i < r->nodeset->used; i++) {
        ns_add(res, r->nodeset->nodes[i], state);
        RET_ON_ERROR;
    }
    state->value_pool[vind].nodeset = res;
    push_value(vind, state);
}

static void eval_concat_string(struct state *state) {
    value_ind_t vind = make_value(T_STRING, state);
    struct value *r = pop_value(state);
    struct value *l = pop_value(state);
    char *res = NULL;

    RET_ON_ERROR;

    if (ALLOC_N(res, strlen(l->string) + strlen(r->string) + 1) < 0) {
        STATE_ENOMEM;
        return;
    }
    strcpy(res, l->string);
    strcat(res, r->string);
    state->value_pool[vind].string = res;
    push_value(vind, state);
}

static void eval_concat_regexp(struct state *state) {
    value_ind_t vind = make_value(T_REGEXP, state);
    struct value *r = pop_value(state);
    struct value *l = pop_value(state);
    struct regexp *rx = NULL;

    RET_ON_ERROR;

    rx = regexp_concat(state->error->info, l->regexp, r->regexp);
    if (rx == NULL) {
        STATE_ENOMEM;
        return;
    }

    state->value_pool[vind].regexp = rx;
    push_value(vind, state);
}

static void eval_re_match(struct state *state, enum binary_op op) {
    struct value *rx  = pop_value(state);
    struct value *v = pop_value(state);

    bool result = false;

    if (v->tag == T_STRING) {
        result = eval_re_match_str(state, rx->regexp, v->string);
        RET_ON_ERROR;
    } else if (v->tag == T_NODESET) {
        for (int i=0; i < v->nodeset->used && result == false; i++) {
            struct tree *t = v->nodeset->nodes[i];
            result = eval_re_match_str(state, rx->regexp, t->value);
            RET_ON_ERROR;
        }
    }
    if (op == OP_RE_NOMATCH)
        result = !result;
    push_boolean_value(result, state);
}

static void eval_binary(struct expr *expr, struct state *state) {
    eval_expr(expr->left, state);
    eval_expr(expr->right, state);
    RET_ON_ERROR;

    switch (expr->op) {
    case OP_EQ:
        eval_eq(state, 0);
        break;
    case OP_NEQ:
        eval_eq(state, 1);
        break;
    case OP_LT:
        eval_rel(state, false, false);
        break;
    case OP_LE:
        eval_rel(state, false, true);
        break;
    case OP_GT:
        eval_rel(state, true, false);
        break;
    case OP_GE:
        eval_rel(state, true, true);
        break;
    case OP_PLUS:
        if (expr->type == T_NUMBER)
            eval_arith(state, expr->op);
        else if (expr->type == T_STRING)
            eval_concat_string(state);
        else if (expr->type == T_REGEXP)
            eval_concat_regexp(state);
        break;
    case OP_MINUS:
    case OP_STAR:
        eval_arith(state, expr->op);
        break;
    case OP_AND:
    case OP_OR:
        eval_and_or(state, expr->op);
        break;
    case OP_UNION:
        eval_union(state);
        break;
    case OP_RE_MATCH:
    case OP_RE_NOMATCH:
        eval_re_match(state, expr->op);
        break;
    default:
        assert(0);
    }
}

static void eval_app(struct expr *expr, struct state *state) {
    assert(expr->tag == E_APP);

    for (int i=0; i < expr->func->arity; i++) {
        eval_expr(expr->args[i], state);
        RET_ON_ERROR;
    }
    expr->func->impl(state);
}

static bool eval_pred(struct expr *expr, struct state *state) {
    eval_expr(expr, state);
    RET0_ON_ERROR;

    struct value *v = pop_value(state);
    switch(v->tag) {
    case T_BOOLEAN:
        return v->boolval;
    case T_NUMBER:
        return (state->ctx_pos == v->number);
    case T_NODESET:
        return v->nodeset->used > 0;
    default:
        assert(0);
        return false;
    }
}

static void ns_remove(struct nodeset *ns, int ind) {
    memmove(ns->nodes + ind, ns->nodes + ind+1,
            sizeof(ns->nodes[0]) * (ns->used - (ind+1)));
    ns->used -= 1;
}

/*
 * Remove all nodes from NS for which one of PRED is false
 */
static void ns_filter(struct nodeset *ns, struct pred *predicates,
                      struct state *state) {
    if (predicates == NULL)
        return;

    struct tree *old_ctx = state->ctx;
    uint old_ctx_len = state->ctx_len;
    uint old_ctx_pos = state->ctx_pos;

    for (int p=0; p < predicates->nexpr; p++) {
        state->ctx_len = ns->used;
        state->ctx_pos = 1;
        for (int i=0; i < ns->used; state->ctx_pos++) {
            state->ctx = ns->nodes[i];
            bool match = eval_pred(predicates->exprs[p], state);
            RET_ON_ERROR;
            if (match) {
                i+=1;
            } else {
                ns_remove(ns, i);
            }
        }
    }

    state->ctx = old_ctx;
    state->ctx_pos = old_ctx_pos;
    state->ctx_len = old_ctx_len;
}

/* Return an array of nodesets, one for each step in the locpath.
 *
 * On return, (*NS)[0] will contain state->ctx, and (*NS)[*MAXNS] will
 * contain the nodes that matched the entire locpath
 */
static void ns_from_locpath(struct locpath *lp, uint *maxns,
                            struct nodeset ***ns,
                            const struct nodeset *root,
                            struct state *state) {
    struct tree *old_ctx = state->ctx;

    *maxns = 0;
    *ns = NULL;
    list_for_each(step, lp->steps)
        *maxns += 1;
    if (ALLOC_N(*ns, *maxns+1) < 0) {
        STATE_ERROR(state, PATHX_ENOMEM);
        goto error;
    }
    for (int i=0; i <= *maxns; i++) {
        (*ns)[i] = make_nodeset(state);
        if (HAS_ERROR(state))
            goto error;
    }

    if (root == NULL) {
        struct step *first_step = NULL;
        if (lp != NULL)
            first_step = lp->steps;

        struct tree *root_tree;
        root_tree = step_root(first_step, state->ctx, state->root_ctx);
        ns_add((*ns)[0], root_tree, state);
    } else {
        for (int i=0; i < root->used; i++)
            ns_add((*ns)[0], root->nodes[i], state);
    }

    if (HAS_ERROR(state))
        goto error;

    uint cur_ns = 0;
    list_for_each(step, lp->steps) {
        struct nodeset *work = (*ns)[cur_ns];
        struct nodeset *next = (*ns)[cur_ns + 1];
        for (int i=0; i < work->used; i++) {
            for (struct tree *node = step_first(step, work->nodes[i]);
                 node != NULL;
                 node = step_next(step, work->nodes[i], node))
                ns_add(next, node, state);
        }
        ns_filter(next, step->predicates, state);
        if (HAS_ERROR(state))
            goto error;
        cur_ns += 1;
    }

    state->ctx = old_ctx;
    return;
 error:
    if (*ns != NULL) {
        for (int i=0; i <= *maxns; i++)
            free_nodeset((*ns)[i]);
        FREE(*ns);
    }
    state->ctx = old_ctx;
    return;
}

static void eval_filter(struct expr *expr, struct state *state) {
    struct locpath *lp = expr->locpath;
    struct nodeset **ns = NULL;
    struct locpath_trace *lpt = state->locpath_trace;
    uint maxns;

    state->locpath_trace = NULL;
    if (expr->primary == NULL) {
        ns_from_locpath(lp, &maxns, &ns, NULL, state);
    } else {
        eval_expr(expr->primary, state);
        RET_ON_ERROR;
        value_ind_t primary_ind = pop_value_ind(state);
        struct value *primary = state->value_pool + primary_ind;
        assert(primary->tag == T_NODESET);
        ns_filter(primary->nodeset, expr->predicates, state);
        /* Evaluating predicates might have reallocated the value_pool */
        primary = state->value_pool + primary_ind;
        ns_from_locpath(lp, &maxns, &ns, primary->nodeset, state);
    }
    RET_ON_ERROR;

    value_ind_t vind = make_value(T_NODESET, state);
    RET_ON_ERROR;
    state->value_pool[vind].nodeset = ns[maxns];
    push_value(vind, state);

    if (lpt != NULL) {
        assert(lpt->ns == NULL);
        assert(lpt->lp == NULL);
        lpt->maxns = maxns;
        lpt->ns = ns;
        lpt->lp = lp;
        state->locpath_trace = lpt;
    } else {
        for (int i=0; i < maxns; i++)
            free_nodeset(ns[i]);
        FREE(ns);
    }
}

static struct value *lookup_var(const char *ident, struct state *state) {
    list_for_each(tab, state->symtab) {
        if (STREQ(ident, tab->name))
            return tab->value;
    }
    return NULL;
}

static void eval_var(struct expr *expr, struct state *state) {
    struct value *v = lookup_var(expr->ident, state);
    value_ind_t vind = clone_value(v, state);
    RET_ON_ERROR;
    push_value(vind, state);
}

static void eval_expr(struct expr *expr, struct state *state) {
    RET_ON_ERROR;
    switch (expr->tag) {
    case E_FILTER:
        eval_filter(expr, state);
        break;
    case E_BINARY:
        eval_binary(expr, state);
        break;
    case E_VALUE:
        push_value(expr->value_ind, state);
        break;
    case E_VAR:
        eval_var(expr, state);
        break;
    case E_APP:
        eval_app(expr, state);
        break;
    default:
        assert(0);
    }
}

/*************************************************************************
 * Typechecker
 *************************************************************************/

static void check_expr(struct expr *expr, struct state *state);

/* Typecheck a list of predicates. A predicate is a function of
 * one of the following types:
 *
 * T_NODESET -> T_BOOLEAN
 * T_NUMBER  -> T_BOOLEAN  (position test)
 * T_BOOLEAN -> T_BOOLEAN
 */
static void check_preds(struct pred *pred, struct state *state) {
    if (pred == NULL)
        return;
    for (int i=0; i < pred->nexpr; i++) {
        struct expr *e = pred->exprs[i];
        check_expr(e, state);
        RET_ON_ERROR;
        if (e->type != T_NODESET && e->type != T_NUMBER &&
            e->type != T_BOOLEAN) {
            STATE_ERROR(state, PATHX_ETYPE);
            return;
        }
    }
}

static void check_filter(struct expr *expr, struct state *state) {
    assert(expr->tag == E_FILTER);
    struct locpath *locpath = expr->locpath;

    if (expr->primary != NULL) {
        check_expr(expr->primary, state);
        if (expr->primary->type != T_NODESET) {
            STATE_ERROR(state, PATHX_ETYPE);
            return;
        }
        check_preds(expr->predicates, state);
        RET_ON_ERROR;
    }
    list_for_each(s, locpath->steps) {
        check_preds(s->predicates, state);
        RET_ON_ERROR;
    }
    expr->type = T_NODESET;
}

static void check_app(struct expr *expr, struct state *state) {
    assert(expr->tag == E_APP);

    for (int i=0; i < expr->func->arity; i++) {
        check_expr(expr->args[i], state);
        RET_ON_ERROR;
    }

    int f;
    for (f=0; f < ARRAY_CARDINALITY(builtin_funcs); f++) {
        const struct func *fn = builtin_funcs + f;
        if (STRNEQ(expr->func->name, fn->name))
            continue;

        int match = 1;
        for (int i=0; i < expr->func->arity; i++) {
            if (expr->args[i]->type != fn->arg_types[i]) {
                match = 0;
                break;
            }
        }
        if (match)
            break;
    }

    if (f < ARRAY_CARDINALITY(builtin_funcs)) {
        expr->func = builtin_funcs + f;
        expr->type = expr->func->type;
    } else {
        STATE_ERROR(state, PATHX_ETYPE);
    }
}

/* Check the binary operators. Type rules:
 *
 * '=', '!='  : T_NODESET -> T_NODESET -> T_BOOLEAN
 *              T_STRING  -> T_NODESET -> T_BOOLEAN
 *              T_NODESET -> T_STRING  -> T_BOOLEAN
 *              T_NUMBER  -> T_NUMBER  -> T_BOOLEAN
 *
 * '>', '>=',
 * '<', '<='  : T_NUMBER -> T_NUMBER -> T_BOOLEAN
 *              T_STRING -> T_STRING -> T_BOOLEAN
 * '+'        : T_NUMBER -> T_NUMBER -> T_NUMBER
 *              T_STRING -> T_STRING -> T_STRING
 *              T_REGEXP -> T_REGEXP -> T_REGEXP
 * '+', '-', '*': T_NUMBER -> T_NUMBER -> T_NUMBER
 *
 * 'and', 'or': T_BOOLEAN -> T_BOOLEAN -> T_BOOLEAN
 * '=~', '!~' : T_STRING  -> T_REGEXP  -> T_BOOLEAN
 *              T_NODESET -> T_REGEXP  -> T_BOOLEAN
 *
 * '|'        : T_NODESET -> T_NODESET -> T_NODESET
 *
 * Any type can be coerced to T_BOOLEAN (see coerce_to_bool)
 */
static void check_binary(struct expr *expr, struct state *state) {
    check_expr(expr->left, state);
    check_expr(expr->right, state);
    RET_ON_ERROR;

    enum type l = expr->left->type;
    enum type r = expr->right->type;
    int  ok = 1;
    enum type res;

    switch(expr->op) {
    case OP_EQ:
    case OP_NEQ:
        ok = ((l == T_NODESET || l == T_STRING)
              && (r == T_NODESET || r == T_STRING))
            || (l == T_NUMBER && r == T_NUMBER);;
        res = T_BOOLEAN;
        break;
    case OP_LT:
    case OP_LE:
    case OP_GT:
    case OP_GE:
        ok = (l == T_NUMBER && r == T_NUMBER)
            || (l == T_STRING && r == T_STRING);
        res = T_BOOLEAN;
        break;
    case OP_PLUS:
        ok = (l == r && (l == T_NUMBER || l == T_STRING || l == T_REGEXP));
        res = l;
        break;
    case OP_MINUS:
    case OP_STAR:
        ok =  (l == T_NUMBER && r == T_NUMBER);
        res = T_NUMBER;
        break;
    case OP_UNION:
        ok = (l == T_NODESET && r == T_NODESET);
        res = T_NODESET;
        break;
    case OP_AND:
    case OP_OR:
        ok = 1;
        res = T_BOOLEAN;
        break;
    case OP_RE_MATCH:
    case OP_RE_NOMATCH:
        ok = ((l == T_STRING || l == T_NODESET) && r == T_REGEXP);
        res = T_BOOLEAN;
        break;
    default:
        assert(0);
    }
    if (! ok) {
        STATE_ERROR(state, PATHX_ETYPE);
    } else {
        expr->type = res;
    }
}

static void check_var(struct expr *expr, struct state *state) {
    struct value *v = lookup_var(expr->ident, state);
    if (v == NULL) {
        STATE_ERROR(state, PATHX_ENOVAR);
        return;
    }
    expr->type = v->tag;
}

/* Typecheck an expression */
static void check_expr(struct expr *expr, struct state *state) {
    RET_ON_ERROR;
    switch(expr->tag) {
    case E_FILTER:
        check_filter(expr, state);
        break;
    case E_BINARY:
        check_binary(expr, state);
        break;
    case E_VALUE:
        expr->type = expr_value(expr, state)->tag;
        break;
    case E_VAR:
        check_var(expr, state);
        break;
    case E_APP:
        check_app(expr, state);
        break;
    default:
        assert(0);
    }
}

/*
 * Utility functions for the parser
 */

static void skipws(struct state *state) {
    while (isspace(*state->pos)) state->pos += 1;
}

static int match(struct state *state, char m) {
    skipws(state);

    if (*state->pos == '\0')
        return 0;
    if (*state->pos == m) {
        state->pos += 1;
        return 1;
    }
    return 0;
}

static int peek(struct state *state, const char *chars) {
    return strchr(chars, *state->pos) != NULL;
}

/* Return 1 if STATE->POS starts with TOKEN, followed by optional
 * whitespace, followed by FOLLOW. In that case, STATE->POS is set to the
 * first character after FOLLOW. Return 0 otherwise and leave STATE->POS
 * unchanged.
 */
static int looking_at(struct state *state, const char *token,
                      const char *follow) {
    if (STREQLEN(state->pos, token, strlen(token))) {
        const char *p = state->pos + strlen(token);
        while (isspace(*p)) p++;
        if (STREQLEN(p, follow, strlen(follow))) {
            state->pos = p + strlen(follow);
            return 1;
        }
    }
    return 0;
}

/*************************************************************************
 * The parser
 *************************************************************************/

static void parse_expr(struct state *state);

static struct expr* pop_expr(struct state *state) {
    if (state->exprs_used > 0) {
        state->exprs_used -= 1;
        return state->exprs[state->exprs_used];
    } else {
        STATE_ERROR(state, PATHX_EINTERNAL);
        assert(0);
        return NULL;
    }
}

static void push_expr(struct expr *expr, struct state *state) {
    if (state->exprs_used >= state->exprs_size) {
        size_t new_size = 2*state->exprs_size;
        if (new_size == 0) new_size = 8;
        if (REALLOC_N(state->exprs, new_size) < 0) {
            STATE_ENOMEM;
            return;
        }
        state->exprs_size = new_size;
    }
    state->exprs[state->exprs_used++] = expr;
}

static void push_new_binary_op(enum binary_op op, struct state *state) {
    struct expr *expr = NULL;
    if (ALLOC(expr) < 0) {
        STATE_ENOMEM;
        return;
    }

    expr->tag = E_BINARY;
    expr->op  = op;
    expr->right = pop_expr(state);
    expr->left = pop_expr(state);
    push_expr(expr, state);
}

/*
 * NameNoWS ::= [^][|/\= \t\n] | \\.
 * NameWS   ::= [^][|/\=] | \\.
 * Name ::= NameNoWS NameWS* NameNoWS | NameNoWS
 */
static char *parse_name(struct state *state) {
    static const char const follow[] = "][|/=()!";
    const char *s = state->pos;
    char *result;

    while (*state->pos != '\0' && strchr(follow, *state->pos) == NULL) {
        /* This is a hack: since we allow spaces in names, we need to avoid
         * gobbling up stuff that is in follow(Name), e.g. 'or' so that
         * things like [name1 or name2] still work.
         */
        if (STREQLEN(state->pos, " or ", strlen(" or ")) ||
            STREQLEN(state->pos, " and ", strlen(" and ")))
            break;

        if (*state->pos == '\\') {
            state->pos += 1;
            if (*state->pos == '\0') {
                STATE_ERROR(state, PATHX_ENAME);
                return NULL;
            }
        }
        state->pos += 1;
    }

    /* Strip trailing white space */
    if (state->pos > s) {
        state->pos -= 1;
        while (isspace(*state->pos) && state->pos >= s)
            state->pos -= 1;
        state->pos += 1;
    }

    if (state->pos == s) {
        STATE_ERROR(state, PATHX_ENAME);
        return NULL;
    }

    result = strndup(s, state->pos - s);
    if (result == NULL) {
        STATE_ENOMEM;
        return NULL;
    }

    char *p = result;
    for (char *t = result; *t != '\0'; t++, p++) {
        if (*t == '\\')
            t += 1;
        *p = *t;
    }
    *p = '\0';

    return result;
}

/*
 * Predicate    ::= "[" Expr "]" *
 */
static struct pred *parse_predicates(struct state *state) {
    struct pred *pred = NULL;
    int nexpr = 0;

    while (match(state, L_BRACK)) {
        parse_expr(state);
        nexpr += 1;
        RET0_ON_ERROR;

        if (! match(state, R_BRACK)) {
            STATE_ERROR(state, PATHX_EPRED);
            return NULL;
        }
        skipws(state);
    }

    if (nexpr == 0)
        return NULL;

    if (ALLOC(pred) < 0) {
        STATE_ENOMEM;
        return NULL;
    }
    pred->nexpr = nexpr;

    if (ALLOC_N(pred->exprs, nexpr) < 0) {
        free_pred(pred);
        STATE_ENOMEM;
        return NULL;
    }

    for (int i = nexpr - 1; i >= 0; i--)
        pred->exprs[i] = pop_expr(state);

    return pred;
}

/*
 * Step ::= AxisSpecifier NameTest Predicate* | '.' | '..'
 * AxisSpecifier ::= AxisName '::' | <epsilon>
 * AxisName ::= 'ancestor'
 *            | 'ancestor-or-self'
 *            | 'child'
 *            | 'descendant'
 *            | 'descendant-or-self'
 *            | 'parent'
 *            | 'self'
 *            | 'root'
 */
static struct step *parse_step(struct state *state) {
    struct step *step;
    int explicit_axis = 0, allow_predicates = 1;

    if (ALLOC(step) < 0) {
        STATE_ENOMEM;
        return NULL;
    }

    step->axis = CHILD;
    for (int i = 0; i < ARRAY_CARDINALITY(axis_names); i++) {
        if (looking_at(state, axis_names[i], "::")) {
            step->axis = i;
            explicit_axis = 1;
            break;
        }
    }

    if (! match(state, '*')) {
        step->name = parse_name(state);
        if (HAS_ERROR(state))
            goto error;
        if (! explicit_axis) {
            if (STREQ(step->name, ".") || STREQ(step->name, "..")) {
                step->axis = STREQ(step->name, ".") ? SELF : PARENT;
                FREE(step->name);
                allow_predicates = 0;
            }
        }
    }

    if (allow_predicates) {
        step->predicates = parse_predicates(state);
        if (HAS_ERROR(state))
            goto error;
    }

    return step;

 error:
    free_step(step);
    return NULL;
}

static struct step *make_step(enum axis axis, struct state *state) {
    struct step *result = NULL;

    if (ALLOC(result) < 0) {
        STATE_ENOMEM;
        return NULL;
    }
    result->axis = axis;
    return result;
}

/*
 * RelativeLocationPath ::= Step
 *                        | RelativeLocationPath '/' Step
 *                        | AbbreviatedRelativeLocationPath
 * AbbreviatedRelativeLocationPath ::= RelativeLocationPath '//' Step
 *
 * The above is the same as
 * RelativeLocationPath ::= Step ('/' Step | '//' Step)*
 */
static struct locpath *
parse_relative_location_path(struct state *state) {
    struct step *step = NULL;
    struct locpath *locpath = NULL;

    step = parse_step(state);
    if (HAS_ERROR(state))
        goto error;

    if (ALLOC(locpath) < 0) {
        STATE_ENOMEM;
        goto error;
    }
    list_append(locpath->steps, step);
    step = NULL;

    while (match(state, '/')) {
        if (*state->pos == '/') {
            state->pos += 1;
            step = make_step(DESCENDANT_OR_SELF, state);
            ENOMEM_ON_NULL(state, step);
            list_append(locpath->steps, step);
        }
        step = parse_step(state);
        if (HAS_ERROR(state))
            goto error;
        list_append(locpath->steps, step);
        step = NULL;
    }
    return locpath;

 error:
    free_step(step);
    free_locpath(locpath);
    return NULL;
}

/*
 * LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * AbsoluteLocationPath ::= '/' RelativeLocationPath?
 *                        | AbbreviatedAbsoluteLocationPath
 * AbbreviatedAbsoluteLocationPath ::= '//' RelativeLocationPath
 *
 */
static void parse_location_path(struct state *state) {
    struct expr *expr = NULL;
    struct locpath *locpath = NULL;

    if (match(state, '/')) {
        if (*state->pos == '/') {
            state->pos += 1;
            locpath = parse_relative_location_path(state);
            if (HAS_ERROR(state))
                return;
            struct step *step = make_step(DESCENDANT_OR_SELF, state);
            if (HAS_ERROR(state))
                goto error;
            list_cons(locpath->steps, step);
        } else {
            if (*state->pos != '\0') {
                locpath = parse_relative_location_path(state);
            } else {
                if (ALLOC(locpath) < 0)
                    goto err_nomem;
            }
            struct step *step = make_step(ROOT, state);
            if (HAS_ERROR(state))
                goto error;
            list_cons(locpath->steps, step);
        }
    } else {
        locpath = parse_relative_location_path(state);
    }

    if (ALLOC(expr) < 0)
        goto err_nomem;
    expr->tag = E_FILTER;
    expr->locpath = locpath;
    push_expr(expr, state);
    return;

 err_nomem:
    STATE_ENOMEM;
 error:
    free_expr(expr);
    free_locpath(locpath);
    return;
}

/*
 * Number       ::= /[0-9]+/
 */
static void parse_number(struct state *state) {
    struct expr *expr = NULL;
    unsigned long val;
    char *end;

    errno = 0;
    val = strtoul(state->pos, &end, 10);
    if (errno || end == state->pos || (int) val != val) {
        STATE_ERROR(state, PATHX_ENUMBER);
        return;
    }

    state->pos = end;

    if (ALLOC(expr) < 0)
        goto err_nomem;
    expr->tag = E_VALUE;
    expr->value_ind = make_value(T_NUMBER, state);
    if (HAS_ERROR(state))
        goto error;
    expr_value(expr, state)->number = val;

    push_expr(expr, state);
    return;

 err_nomem:
    STATE_ENOMEM;
 error:
    free_expr(expr);
    return;
}

/*
 * Literal ::= '"' /[^"]* / '"' | "'" /[^']* / "'"
 */
static void parse_literal(struct state *state) {
    char delim;
    const char *s;
    struct expr *expr = NULL;

    if (*state->pos == '"')
        delim = '"';
    else if (*state->pos == '\'')
        delim = '\'';
    else {
        STATE_ERROR(state, PATHX_ESTRING);
        return;
    }
    state->pos += 1;

    s = state->pos;
    while (*state->pos != '\0' && *state->pos != delim) state->pos += 1;

    if (*state->pos != delim) {
        STATE_ERROR(state, PATHX_EDELIM);
        return;
    }
    state->pos += 1;

    if (ALLOC(expr) < 0)
        goto err_nomem;
    expr->tag = E_VALUE;
    expr->value_ind = make_value(T_STRING, state);
    if (HAS_ERROR(state))
        goto error;
    expr_value(expr, state)->string = strndup(s, state->pos - s - 1);
    if (expr_value(expr, state)->string == NULL)
        goto err_nomem;

    push_expr(expr, state);
    return;

 err_nomem:
    STATE_ENOMEM;
 error:
    free_expr(expr);
    return;
}

/*
 * FunctionCall ::= Name '(' ( Expr ( ',' Expr )* )? ')'
 */
static void parse_function_call(struct state *state) {
    const struct func *func = NULL;
    struct expr *expr = NULL;
    int nargs = 0;

    for (int i=0; i < ARRAY_CARDINALITY(builtin_funcs); i++) {
        if (looking_at(state, builtin_funcs[i].name, "("))
            func = builtin_funcs + i;
    }
    if (func == NULL) {
        STATE_ERROR(state, PATHX_ENAME);
        return;
    }

    if (! match(state, ')')) {
        do {
            nargs += 1;
            parse_expr(state);
            RET_ON_ERROR;
        } while (match(state, ','));

        if (! match(state, ')')) {
            STATE_ERROR(state, PATHX_EPAREN);
            return;
        }
    }

    if (nargs != func->arity) {
        STATE_ERROR(state, PATHX_EARITY);
        return;
    }

    if (ALLOC(expr) < 0) {
        STATE_ENOMEM;
        return;
    }
    expr->tag = E_APP;
    if (ALLOC_N(expr->args, nargs) < 0) {
        free_expr(expr);
        STATE_ENOMEM;
        return;
    }
    expr->func = func;
    for (int i = nargs - 1; i >= 0; i--)
        expr->args[i] = pop_expr(state);

    push_expr(expr, state);
}

/*
 * VariableReference ::= '$' /[a-zA-Z_][a-zA-Z0-9_]* /
 *
 * The '$' is consumed by parse_primary_expr
 */
static void parse_var(struct state *state) {
    const char *id = state->pos;
    struct expr *expr = NULL;

    if (!isalpha(*id) && *id != '_') {
        STATE_ERROR(state, PATHX_ENAME);
        return;
    }
    id++;
    while (isalpha(*id) || isdigit(*id) || *id == '_')
        id += 1;

    if (ALLOC(expr) < 0)
        goto err_nomem;
    expr->tag = E_VAR;
    expr->ident = strndup(state->pos, id - state->pos);
    if (expr->ident == NULL)
        goto err_nomem;

    push_expr(expr, state);
    state->pos = id;
    return;
 err_nomem:
    STATE_ENOMEM;
    free_expr(expr);
    return;
}

/*
 * PrimaryExpr ::= Literal
 *               | Number
 *               | FunctionCall
 *               | VariableReference
 *               | '(' Expr ')'
 *
 */
static void parse_primary_expr(struct state *state) {
    if (peek(state, "'\"")) {
        parse_literal(state);
    } else if (peek(state, "0123456789")) {
        parse_number(state);
    } else if (match(state, '(')) {
        parse_expr(state);
        RET_ON_ERROR;
        if (! match(state, ')')) {
            STATE_ERROR(state, PATHX_EPAREN);
            return;
        }
    } else if (match(state, '$')) {
        parse_var(state);
    } else {
        parse_function_call(state);
    }
}

static int looking_at_primary_expr(struct state *state) {
    const char *s = state->pos;
    /* Is it a Number, Literal or VariableReference ? */
    if (peek(state, "$'\"0123456789"))
        return 1;

    /* Or maybe a function call, i.e. a word followed by a '(' ?
     * Note that our function names are only [a-zA-Z]+
     */
    while (*s != '\0' && isalpha(*s)) s++;
    while (*s != '\0' && isspace(*s)) s++;
    return *s == '(';
}

/*
 * PathExpr ::= LocationPath
 *            | FilterExpr
 *            | FilterExpr '/' RelativeLocationPath
 *            | FilterExpr '//' RelativeLocationPath
 *
 * FilterExpr ::= PrimaryExpr Predicate
 *
 * The grammar is ambiguous here: the expression '42' can either be the
 * number 42 (a PrimaryExpr) or the RelativeLocationPath 'child::42'. The
 * reason for this ambiguity is that we allow node names like '42' in the
 * tree; rather than forbid them, we resolve the ambiguity by always
 * parsing '42' as a number, and requiring that the user write the
 * RelativeLocationPath in a different form, e.g. 'child::42' or './42'.
 */
static void parse_path_expr(struct state *state) {
    struct expr *expr = NULL;
    struct pred *predicates = NULL;
    struct locpath *locpath = NULL;

    if (looking_at_primary_expr(state)) {
        parse_primary_expr(state);
        RET_ON_ERROR;
        predicates = parse_predicates(state);
        RET_ON_ERROR;
        if (match(state, '/')) {
            if (match(state, '/')) {
                locpath = parse_relative_location_path(state);
                if (HAS_ERROR(state))
                    goto error;

                struct step *step = make_step(DESCENDANT_OR_SELF, state);
                if (HAS_ERROR(state))
                    return;
                list_cons(locpath->steps, step);
            } else {
                if (*state->pos == '\0') {
                    STATE_ERROR(state, PATHX_EEND);
                    goto error;
                }
                locpath = parse_relative_location_path(state);
            }
        }
        /* A PathExpr without predicates and locpath is
         * just a PrimaryExpr
         */
        if (predicates == NULL && locpath == NULL)
            return;
        /* To make evaluation easier, we parse something like
         *   $var[pred] as $var[pred]/.
         */
        if (locpath == NULL) {
            if (ALLOC(locpath) < 0)
                goto error;
            if (ALLOC(locpath->steps) < 0)
                goto error;
            locpath->steps->axis = SELF;
        }
        if (ALLOC(expr) < 0)
            goto error;
        expr->tag = E_FILTER;
        expr->predicates = predicates;
        expr->primary    = pop_expr(state);
        expr->locpath    = locpath;
        push_expr(expr, state);
    } else {
        parse_location_path(state);
    }
    return;
 error:
    free_expr(expr);
    free_pred(predicates);
    free_locpath(locpath);
    return;
}

/*
 * UnionExpr ::= PathExpr ('|' PathExpr)*
 */
static void parse_union_expr(struct state *state) {
    parse_path_expr(state);
    RET_ON_ERROR;
    while (match(state, '|')) {
        parse_path_expr(state);
        RET_ON_ERROR;
        push_new_binary_op(OP_UNION, state);
    }
}

/*
 * MultiplicativeExpr ::= UnionExpr ('*' UnionExpr)*
 */
static void parse_multiplicative_expr(struct state *state) {
    parse_union_expr(state);
    RET_ON_ERROR;
    while (match(state, '*')) {
        parse_union_expr(state);
        RET_ON_ERROR;
        push_new_binary_op(OP_STAR, state);
    }
}

/*
 * AdditiveExpr ::= MultiplicativeExpr (AdditiveOp MultiplicativeExpr)*
 * AdditiveOp   ::= '+' | '-'
 */
static void parse_additive_expr(struct state *state) {
    parse_multiplicative_expr(state);
    RET_ON_ERROR;
    while (*state->pos == '+' || *state->pos == '-') {
        enum binary_op op = (*state->pos == '+') ? OP_PLUS : OP_MINUS;
        state->pos += 1;
        skipws(state);
        parse_multiplicative_expr(state);
        RET_ON_ERROR;
        push_new_binary_op(op, state);
    }
}

/*
 * RelationalExpr ::= AdditiveExpr (RelationalOp AdditiveExpr)?
 * RelationalOp ::= ">" | "<" | ">=" | "<="
 */
static void parse_relational_expr(struct state *state) {
    parse_additive_expr(state);
    RET_ON_ERROR;
    if (*state->pos == '<' || *state->pos == '>') {
        enum binary_op op = (*state->pos == '<') ? OP_LT : OP_GT;
        state->pos += 1;
        if (*state->pos == '=') {
            op = (op == OP_LT) ? OP_LE : OP_GE;
            state->pos += 1;
        }
        skipws(state);
        parse_additive_expr(state);
        RET_ON_ERROR;
        push_new_binary_op(op, state);
    }
}

/*
 * EqualityExpr ::= RelationalExpr (EqualityOp RelationalExpr)? | ReMatchExpr
 * EqualityOp ::= "=" | "!="
 * ReMatchExpr ::= RelationalExpr MatchOp RelationalExpr
 * MatchOp ::= "=~" | "!~"
 */
static void parse_equality_expr(struct state *state) {
    parse_relational_expr(state);
    RET_ON_ERROR;
    if ((*state->pos == '=' || *state->pos == '!') && state->pos[1] == '~') {
        enum binary_op op = (*state->pos == '=') ? OP_RE_MATCH : OP_RE_NOMATCH;
        state->pos += 2;
        skipws(state);
        parse_relational_expr(state);
        RET_ON_ERROR;
        push_new_binary_op(op, state);
    } else if (*state->pos == '=' ||
        (*state->pos == '!' && state->pos[1] == '=')) {
        enum binary_op op = (*state->pos == '=') ? OP_EQ : OP_NEQ;
        state->pos += (op == OP_EQ) ? 1 : 2;
        skipws(state);
        parse_relational_expr(state);
        RET_ON_ERROR;
        push_new_binary_op(op, state);
    }
}

/*
 * AndExpr ::= EqualityExpr ('and' EqualityExpr)*
 */
static void parse_and_expr(struct state *state) {
    parse_equality_expr(state);
    RET_ON_ERROR;
    while (*state->pos == 'a' && state->pos[1] == 'n'
        && state->pos[2] == 'd') {
        state->pos += 3;
        skipws(state);
        parse_equality_expr(state);
        RET_ON_ERROR;
        push_new_binary_op(OP_AND, state);
    }
}

/*
 * OrExpr ::= AndExpr ('or' AndExpr)*
 */
static void parse_or_expr(struct state *state) {
    parse_and_expr(state);
    RET_ON_ERROR;
    while (*state->pos == 'o' && state->pos[1] == 'r') {
        state->pos += 2;
        skipws(state);
        parse_and_expr(state);
        RET_ON_ERROR;
        push_new_binary_op(OP_OR, state);
    }
}

/*
 * Expr ::= OrExpr
 */
static void parse_expr(struct state *state) {
    skipws(state);
    parse_or_expr(state);
}

static void store_error(struct pathx *pathx) {
    const char *pathx_msg = NULL;
    const char *path = pathx->state->txt;
    const pathx_errcode_t errcode = pathx->state->errcode;
    struct error *err = pathx->state->error;

    char *pos_str = pathx->state->errmsg;
    pathx->state->errmsg = NULL;

    if (err == NULL || errcode == PATHX_NOERROR || err->code != AUG_NOERROR)
        return;

    switch (errcode) {
    case PATHX_ENOMEM:
        err->code = AUG_ENOMEM;
        break;
    case PATHX_EMMATCH:
        err->code = AUG_EMMATCH;
        break;
    case PATHX_ENOMATCH:
        err->code = AUG_ENOMATCH;
        break;
    default:
        err->code = AUG_EPATHX;
        break;
    }

    /* We only need details for pathx syntax errors */
    if (err->code != AUG_EPATHX)
        return;

    int pos;
    pathx_msg = pathx_error(pathx, NULL, &pos);

    bool has_msg = pos_str != NULL;
    int pos_str_len = pos_str == NULL ? 0 : strlen(pos_str);
    if (REALLOC_N(pos_str, pos_str_len + strlen(path) + 8) >= 0) {
        if (has_msg) {
            strcat(pos_str, " in ");
            strncat(pos_str, path, pos);
        } else {
            /* initialize pos_str explicitly, path might be "" */
            pos_str[0] = '\0';
            strncat(pos_str, path, pos);
        }
        strcat(pos_str, "|=|");
        strcat(pos_str, path + pos);
    }

    err->minor = errcode;
    err->details = pos_str;
    pos_str = NULL;
    err->minor_details = pathx_msg;
}

int pathx_parse(const struct tree *tree,
                struct error *err,
                const char *txt,
                bool need_nodeset,
                struct pathx_symtab *symtab,
                struct tree *root_ctx,
                struct pathx **pathx) {
    struct state *state = NULL;

    *pathx = NULL;

    if (ALLOC(*pathx) < 0)
        goto oom;

    (*pathx)->origin = (struct tree *) tree;

    /* Set up state */
    if (ALLOC((*pathx)->state) < 0)
        goto oom;
    state = (*pathx)->state;

    state->errcode = PATHX_NOERROR;
    state->errmsg = NULL;
    state->txt = txt;
    state->pos = txt;
    state->symtab = symtab;
    state->root_ctx = root_ctx;
    state->error = err;

    if (ALLOC_N(state->value_pool, 8) < 0) {
        STATE_ENOMEM;
        goto done;
    }
    state->value_pool_size = 8;
    state->value_pool[0].tag = T_BOOLEAN;
    state->value_pool[0].boolval = 0;
    state->value_pool[1].tag = T_BOOLEAN;
    state->value_pool[1].boolval = 1;
    state->value_pool_used = 2;

    /* Parse */
    parse_expr(state);
    if (HAS_ERROR(state))
        goto done;
    if (state->pos != state->txt + strlen(state->txt)) {
        STATE_ERROR(state, PATHX_EEND);
        goto done;
    }

    if (state->exprs_used != 1) {
        STATE_ERROR(state, PATHX_EINTERNAL);
        goto done;
    }

    /* Typecheck */
    check_expr(state->exprs[0], state);
    if (HAS_ERROR(state))
        goto done;

    if (need_nodeset && state->exprs[0]->type != T_NODESET) {
        STATE_ERROR(state, PATHX_ETYPE);
        goto done;
    }

 done:
    store_error(*pathx);
    return state->errcode;
 oom:
    free_pathx(*pathx);
    *pathx = NULL;
    if (err != NULL)
        err->code = AUG_ENOMEM;
    return PATHX_ENOMEM;
}

/*************************************************************************
 * Searching in the tree
 *************************************************************************/

static bool step_matches(struct step *step, struct tree *tree) {
    return (step->name == NULL || streqx(step->name, tree->label));
}

static struct tree *tree_prev(struct tree *pos) {
    struct tree *node = NULL;
    if (pos != pos->parent->children) {
        for (node = pos->parent->children;
             node->next != pos;
             node = node->next);
    }
    return node;
}

/* When the first step doesn't begin with ROOT then use relative root context
 * instead. */
static struct tree *step_root(struct step *step, struct tree *ctx,
                              struct tree *root_ctx) {
    struct tree *node = NULL;
    switch (step->axis) {
    case SELF:
    case CHILD:
    case DESCENDANT:
    case PARENT:
    case ANCESTOR:
    case PRECEDING_SIBLING:
    case FOLLOWING_SIBLING:
        /* only use root_ctx when ctx is the absolute tree root */
        if (ctx == ctx->parent && root_ctx != NULL)
            node = root_ctx;
        else
            node = ctx;
        break;
    case ROOT:
    case DESCENDANT_OR_SELF:
        node = ctx;
        break;
    default:
        assert(0);
    }
    if (node == NULL)
        return NULL;
    return node;
}

static struct tree *step_first(struct step *step, struct tree *ctx) {
    struct tree *node = NULL;
    switch (step->axis) {
    case SELF:
    case DESCENDANT_OR_SELF:
        node = ctx;
        break;
    case CHILD:
    case DESCENDANT:
        node = ctx->children;
        break;
    case PARENT:
    case ANCESTOR:
        node = ctx->parent;
        break;
    case ROOT:
        while (ctx->parent != ctx)
            ctx = ctx->parent;
        node = ctx;
        break;
    case PRECEDING_SIBLING:
        node = tree_prev(ctx);
        break;
    case FOLLOWING_SIBLING:
        node = ctx->next;
        break;
    default:
        assert(0);
    }
    if (node == NULL)
        return NULL;
    if (step_matches(step, node))
        return node;
    return step_next(step, ctx, node);
}

static struct tree *step_next(struct step *step, struct tree *ctx,
                              struct tree *node) {
    while (node != NULL) {
        switch (step->axis) {
        case SELF:
            node = NULL;
            break;
        case CHILD:
            node = node->next;
            break;
        case DESCENDANT:
        case DESCENDANT_OR_SELF:
            if (node->children != NULL) {
                node = node->children;
            } else {
                while (node->next == NULL && node != ctx)
                    node = node->parent;
                if (node == ctx)
                    node = NULL;
                else
                    node = node->next;
            }
            break;
        case PARENT:
        case ROOT:
            node = NULL;
            break;
        case ANCESTOR:
            if (node->parent == node)
                node = NULL;
            else
                node = node->parent;
            break;
        case PRECEDING_SIBLING:
            node = tree_prev(node);
            break;
        case FOLLOWING_SIBLING:
            node = node->next;
            break;
        default:
            assert(0);
        }
        if (node != NULL && step_matches(step, node))
            break;
    }
    return node;
}

static struct value *pathx_eval(struct pathx *pathx) {
    struct state *state = pathx->state;
    state->ctx = pathx->origin;
    state->ctx_pos = 1;
    state->ctx_len = 1;
    eval_expr(state->exprs[0], state);
    if (HAS_ERROR(state))
        return NULL;

    if (state->values_used != 1) {
        STATE_ERROR(state, PATHX_EINTERNAL);
        return NULL;
    }
    return pop_value(state);
}

struct tree *pathx_next(struct pathx *pathx) {
    if (pathx->node + 1 < pathx->nodeset->used)
        return pathx->nodeset->nodes[++pathx->node];
    return NULL;
}

/* Find the first node in TREE matching PATH. */
struct tree *pathx_first(struct pathx *pathx) {
    if (pathx->nodeset == NULL) {
        struct value *v = pathx_eval(pathx);

        if (HAS_ERROR(pathx->state))
            goto error;
        assert(v->tag == T_NODESET);
        pathx->nodeset = v->nodeset;
    }
    pathx->node = 0;
    if (pathx->nodeset->used == 0)
        return NULL;
    else
        return pathx->nodeset->nodes[0];
 error:
    store_error(pathx);
    return NULL;
}

/* Find a node in the tree that matches the longest prefix of PATH.
 *
 * Return 1 if a node was found that exactly matches PATH, 0 if an incomplete
 * prefix matches, and -1 if more than one node in the tree match.
 *
 * TMATCH is set to the tree node that matches, and SMATCH to the next step
 * after the one where TMATCH matched. If no node matches or multiple nodes
 * at the same depth match, TMATCH and SMATCH will be NULL. When exactly
 * one node matches, TMATCH will be that node, and SMATCH will be NULL.
 */
static int locpath_search(struct locpath_trace *lpt,
                          struct tree **tmatch, struct step **smatch) {
    int last;
    int result = -1;

    for (last=lpt->maxns; last >= 0 && lpt->ns[last]->used == 0; last--);
    if (last < 0) {
        *smatch = lpt->lp->steps;
        result = 1;
        goto done;
    }
    if (lpt->ns[last]->used > 1) {
        result = -1;
        goto done;
    }
    result = 0;
    *tmatch = lpt->ns[last]->nodes[0];
    *smatch = lpt->lp->steps;
    for (int i=0; i < last; i++)
        *smatch = (*smatch)->next;
 done:
    for (int i=0; i < lpt->maxns; i++)
        free_nodeset(lpt->ns[i]);
    FREE(lpt->ns);
    return result;
}

/* Expand the tree ROOT so that it contains all components of PATH. PATH
 * must have been initialized against ROOT by a call to PATH_FIND_ONE.
 *
 * Return the first segment that was created by this operation, or NULL on
 * error.
 */
int pathx_expand_tree(struct pathx *path, struct tree **tree) {
    int r;
    struct step *step = NULL;
    struct locpath_trace lpt;
    struct tree *first_child = NULL;
    struct value *v = NULL;

    MEMZERO(&lpt, 1);
    path->state->locpath_trace = &lpt;
    v = pathx_eval(path);
    path->state->locpath_trace = NULL;
    if (HAS_ERROR(path->state))
        goto error;

    if (lpt.maxns == 0) {
        if (v->tag != T_NODESET || v->nodeset->used == 0) {
            STATE_ERROR(path->state, PATHX_ENOMATCH);
            goto error;
        }
        if (v->nodeset->used > 1)
            goto error;
        *tree = v->nodeset->nodes[0];
        return 0;
    }

    *tree = path->origin;
    r = locpath_search(&lpt, tree, &step);
    if (r == -1) {
        STATE_ERROR(path->state, PATHX_EMMATCH);
        goto error;
    }

    if (step == NULL)
        return 0;

    struct tree *parent = *tree;
    if (parent == NULL)
        parent = path->origin;

    list_for_each(s, step) {
        if (s->name == NULL || s->axis != CHILD)
            goto error;
        struct tree *t = make_tree(strdup(s->name), NULL, parent, NULL);
        if (first_child == NULL)
            first_child = t;
        if (t == NULL || t->label == NULL)
            goto error;
        list_append(parent->children, t);
        parent = t;
    }

    while (first_child->children != NULL)
        first_child = first_child->children;

    *tree = first_child;
    return 1;

 error:
    if (first_child != NULL) {
        list_remove(first_child, first_child->parent->children);
        free_tree(first_child);
    }
    *tree = NULL;
    store_error(path);
    return -1;
}

int pathx_find_one(struct pathx *path, struct tree **tree) {
    *tree = pathx_first(path);
    if (HAS_ERROR(path->state))
        return -1;
    return path->nodeset->used;
}

struct error *err_of_pathx(struct pathx *px) {
    return px->state->error;
}

const char *pathx_error(struct pathx *path, const char **txt, int *pos) {
    int errcode = PATHX_ENOMEM;

    if (path != NULL) {
        if (path->state->errcode < ARRAY_CARDINALITY(errcodes))
            errcode = path->state->errcode;
        else
            errcode = PATHX_EINTERNAL;

        if (txt)
            *txt = path->state->txt;

        if (pos)
            *pos = path->state->pos - path->state->txt;
    }
    return errcodes[errcode];
}

/*
 * Symbol tables
 */
static struct pathx_symtab
*make_symtab(struct pathx_symtab *symtab, const char *name,
             struct value *value)
{
    struct pathx_symtab *new;
    char *n = NULL;

    n = strdup(name);
    if (n == NULL)
        return NULL;

    if (ALLOC(new) < 0) {
        free(n);
        return NULL;
    }
    new->name = n;
    new->value = value;
    if (symtab == NULL) {
        return new;
    } else {
        new->next = symtab->next;
        symtab->next = new;
    }
    return symtab;
}

void free_symtab(struct pathx_symtab *symtab) {

    while (symtab != NULL) {
        struct pathx_symtab *del = symtab;
        symtab = del->next;
        free(del->name);
        release_value(del->value);
        free(del->value);
        free(del);
    }
}

struct pathx_symtab *pathx_get_symtab(struct pathx *pathx) {
    return pathx->state->symtab;
}

static int pathx_symtab_set(struct pathx_symtab **symtab,
                            const char *name, struct value *v) {
    int found = 0;

    list_for_each(tab, *symtab) {
        if (STREQ(tab->name, name)) {
            release_value(tab->value);
            free(tab->value);
            tab->value = v;
            found = 1;
            break;
        }
    }

    if (!found) {
        struct pathx_symtab *new = NULL;

        new = make_symtab(*symtab, name, v);
        if (new == NULL)
            goto error;
        *symtab = new;
    }
    return 0;
 error:
    return -1;
}

int pathx_symtab_define(struct pathx_symtab **symtab,
                        const char *name, struct pathx *px) {
    int r;
    struct value *value = NULL, *v = NULL;
    struct state *state = px->state;

    value = pathx_eval(px);
    if (HAS_ERROR(px->state))
        goto error;

    if (ALLOC(v) < 0) {
        STATE_ENOMEM;
        goto error;
    }

    *v = *value;
    value->tag = T_BOOLEAN;

    r = pathx_symtab_set(symtab, name, v);
    if (r < 0) {
        STATE_ENOMEM;
        goto error;
    }

    if (v->tag == T_NODESET)
        return v->nodeset->used;
    else
        return 0;
 error:
    release_value(value);
    free(value);
    release_value(v);
    free(v);
    store_error(px);
    return -1;
}

int pathx_symtab_undefine(struct pathx_symtab **symtab, const char *name) {
    struct pathx_symtab *del = NULL;

    for(del = *symtab;
        del != NULL && !STREQ(del->name, name);
        del = del->next);
    if (del == NULL)
        return 0;
    list_remove(del, *symtab);
    free_symtab(del);
    return 0;
}

int pathx_symtab_assign_tree(struct pathx_symtab **symtab,
                             const char *name, struct tree *tree) {
    struct value *v = NULL;
    int r;

    if (ALLOC(v) < 0)
        goto error;

    v->tag = T_NODESET;
    if (ALLOC(v->nodeset) < 0)
        goto error;
    if (ALLOC_N(v->nodeset->nodes, 1) < 0)
        goto error;
    v->nodeset->used = 1;
    v->nodeset->size = 1;
    v->nodeset->nodes[0] = tree;

    r = pathx_symtab_set(symtab, name, v);
    if (r < 0)
        goto error;
    return 1;
 error:
    release_value(v);
    free(v);
    return -1;
}

void pathx_symtab_remove_descendants(struct pathx_symtab *symtab,
                                     const struct tree *tree) {
    list_for_each(tab, symtab) {
        if (tab->value->tag != T_NODESET)
            continue;
        struct nodeset *ns = tab->value->nodeset;
        for (int i=0; i < ns->used;) {
            struct tree *t = ns->nodes[i];
            while (t != t->parent && t != tree)
                t = t->parent;
            if (t == tree)
                ns_remove(ns, i);
            else
                i += 1;
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
