/*
 * pathx.c: handling path expressions
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

#include <config.h>
#include <internal.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <ctype.h>

static const char *const errcodes[] = {
    "no error",
    "empty name",
    "illegal string literal",
    "illegal number",
    "string missing ending ' or \"",
    "expected '='",
    "allocation failed",
    "unmatched ']'",
    "expected a '/'",
    "internal error",   /* PATHX_EINTERNAL */
    "type error"        /* PATHX_ETYPE */
};

/*
 * Path expressions are strings that use a notation modelled on XPath.
 */

enum type {
    T_NONE = 0,     /* Not a type */
    T_LOCPATH,
    T_BOOLEAN,
    T_NUMBER,
    T_STRING
};

enum expr_tag {
    E_BINARY,
    E_VALUE,
    E_APP
};

enum binary_op {
    OP_EQ,         /* '='  */
    OP_NEQ,        /* '!=' */
    OP_PLUS,       /* '+'  */
    OP_MINUS,      /* '-'  */
    OP_STAR        /* '*'  */
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
    ROOT
};

/* This array is indexed by enum axis */
static const char *const axis_names[] = {
    "self",
    "child",
    "descendant",
    "descendant-or-self",
    "parent",
    "ancestor",
    "root"
};

static const char *const axis_sep = "::";

/* Doubly linked list of location steps. Besides the information from the
 * path expression, also contains information to iterate over a node set,
 * in particular, the context node CTX for the step, and the current node
 * CUR within that context.
 */
struct step {
    struct step *next;
    struct step *prev;
    enum axis    axis;
    char        *name;              /* NULL to match any name */
    struct pred *predicates;
    struct tree *ctx;
    struct tree *cur;
};
/* Iteration over the nodes on a step, ignoring the predicates */
static struct tree *step_first(struct step *step, struct tree *ctx);
static struct tree *step_next(struct step *step);

struct pathx {
    struct state   *state;
    struct locpath *locpath;
    struct tree    *origin;
};

#define L_BRACK '['
#define R_BRACK ']'

/* Locpaths are represented as iterators, consisting of multiple steps */
struct locpath {
    struct step *steps;
    struct step *last;
    struct tree *ctx;
};

static struct tree *locpath_first(struct locpath *lp, struct state *state);
static struct tree *locpath_next(struct locpath *lp, struct state *state);

#define for_each_node(t, locpath, state)                                \
    for (struct tree *t = locpath_first(locpath, state);                \
         t != NULL;                                                     \
         t = locpath_next(locpath, state))

typedef uint32_t value_ind_t;

struct value {
    enum type tag;
    union {
        struct locpath  *locpath;     /* T_LOCPATH */
        int              number;      /* T_NUMBER */
        char            *string;      /* T_STRING  */
        bool             boolval;     /* T_BOOLEAN */
    };
};

struct expr {
    enum expr_tag tag;
    enum type     type;
    union {
        struct {                       /* E_BINARY */
            enum binary_op op;
            struct expr *left;
            struct expr *right;
        };
        value_ind_t      value_ind;    /* E_VALUE */
        struct {                       /* E_APP */
            const struct func *func;
            struct expr       *args[];
        };
    };
};

/* Internal state of the evaluator/parser */
struct state {
    pathx_errcode_t errcode;
    const char     *file;
    int             line;

    const char     *txt;  /* Entire expression */
    const char     *pos;  /* Current position within TXT during parsing */

    struct step    *step; /* Evaluation context */

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
static void func_value(struct state *state);

static const struct func builtin_funcs[] = {
    { .name = "value", .arity = 0, .type = T_STRING, .arg_types = NULL,
      .impl = func_value },
    { .name = "last", .arity = 0, .type = T_NUMBER, .arg_types = NULL,
      .impl = func_last }
};

#define CHECK_ERROR                                                     \
    if (state->errcode != PATHX_NOERROR) return

#define CHECK_ERROR_RET0                                                \
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

static void free_expr(struct expr *expr) {
    if (expr == NULL)
        return;
    switch (expr->tag) {
    case E_BINARY:
        free_expr(expr->left);
        free_expr(expr->right);
        break;
    case E_VALUE:
        break;
    case E_APP:
        for (int i=0; i < expr->func->arity; i++)
            free_expr(expr->args[i]);
        break;
    default:
        assert(0);
    }
    free(expr);
}

static void free_pred(struct pred *pred) {
    if (pred == NULL)
        return;

    for (int i=0; i < pred->nexpr; i++) {
        free_expr(pred->exprs[i]);
    }
    free(pred->exprs);
    free(pred);
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

static void free_step(struct step *step) {
    while (step != NULL) {
        struct step *del = step;
        step = del->next;
        free(del->name);
        free_pred(del->predicates);
        free(del);
    }
}

static void free_state(struct state *state) {
    if (state == NULL)
        return;

    for(int i=0; i < state->exprs_used; i++)
        free_expr(state->exprs[i]);
    free(state->exprs);

    for(int i=0; i < state->value_pool_used; i++) {
        struct value *v = state->value_pool + i;
        switch (v->tag) {
        case T_LOCPATH:
            free_locpath(v->locpath);
            break;
        case T_STRING:
            free(v->string);
            break;
        case T_BOOLEAN:
        case T_NUMBER:
            break;
        default:
            assert(0);
        }
    }
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
    return state->value_pool_used++;
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
    int count = 0;
    value_ind_t vind;
    struct step *step = state->step;
    struct tree *cur = step->cur;

    for (struct tree *t = step_first(step, step->ctx);
         t != NULL;
         t = step_next(step))
        count += 1;
    state->step->cur = cur;

    vind = make_value(T_NUMBER, state);
    CHECK_ERROR;
    state->value_pool[vind].number = count;
    push_value(vind, state);
}

static void func_value(struct state *state) {
    value_ind_t vind = make_value(T_STRING, state);
    const char *s = state->step->cur->value;
    char *v;

    if (s == NULL) s = "";

    v = strdup(s);
    if (v == NULL) {
        STATE_ENOMEM;
        return;
    }
    state->value_pool[vind].string = v;
    push_value(vind, state);
}

static int calc_eq_locpath_locpath(struct locpath *ns1, struct locpath *ns2,
                                   int neq, struct state *state) {
    for_each_node(t1, ns1, state) {
        for_each_node(t2, ns2, state) {
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

static int calc_eq_locpath_string(struct locpath *lp, const char *s,
                                  int neq, struct state *state) {
    lp->ctx = state->step->cur;
    for_each_node(t, lp, state) {
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

    if (l->tag == T_LOCPATH && r->tag == T_LOCPATH) {
        res = calc_eq_locpath_locpath(l->locpath, r->locpath, neq, state);
    } else if (l->tag == T_LOCPATH) {
        res = calc_eq_locpath_string(l->locpath, r->string, neq, state);
    } else if (r->tag == T_LOCPATH) {
        res = calc_eq_locpath_string(r->locpath, l->string, neq, state);
    } else {
        assert(l->tag == T_STRING);
        assert(r->tag == T_STRING);
        res = streqx(l->string, r->string);
    }
    CHECK_ERROR;

    push_boolean_value(res, state);
}

static void eval_arith(struct state *state, enum binary_op op) {
    struct value *r = pop_value(state);
    struct value *l = pop_value(state);
    value_ind_t vind;
    int res;

    assert(l->tag == T_NUMBER);
    assert(r->tag == T_NUMBER);

    vind = make_value(T_NUMBER, state);
    CHECK_ERROR;

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

static void eval_binary(struct expr *expr, struct state *state) {
    eval_expr(expr->left, state);
    eval_expr(expr->right, state);
    CHECK_ERROR;

    switch (expr->op) {
    case OP_EQ:
        eval_eq(state, 0);
        break;
    case OP_NEQ:
        eval_eq(state, 1);
        break;
    case OP_MINUS:
    case OP_PLUS:
    case OP_STAR:
        eval_arith(state, expr->op);
        break;
    default:
        assert(0);
    }
}

static void eval_app(struct expr *expr, struct state *state) {
    assert(expr->tag == E_APP);

    for (int i=0; i < expr->func->arity; i++) {
        eval_expr(expr->args[i], state);
        CHECK_ERROR;
    }
    expr->func->impl(state);
}

static void eval_expr(struct expr *expr, struct state *state) {
    CHECK_ERROR;
    switch (expr->tag) {
    case E_BINARY:
        eval_binary(expr, state);
        break;
    case E_VALUE:
        push_value(expr->value_ind, state);
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
 * T_LOCPATH -> T_BOOLEAN
 * T_NUMBER  -> T_BOOLEAN  (position test)
 * T_BOOLEAN -> T_BOOLEAN
 */
static void check_preds(struct pred *pred, struct state *state) {
    for (int i=0; i < pred->nexpr; i++) {
        struct expr *e = pred->exprs[i];
        check_expr(e, state);
        CHECK_ERROR;
        if (e->type != T_LOCPATH && e->type != T_NUMBER &&
            e->type != T_BOOLEAN) {
            STATE_ERROR(state, PATHX_ETYPE);
            return;
        }
    }
}

/* Typecheck a value; the type of the expression is the type of the value.
 * We also need to make sure that the expressions inside predicates are
 * typechecked for locpaths.
 */
static void check_value(struct expr *expr, struct state *state) {
    assert(expr->tag == E_VALUE);
    struct value *v = expr_value(expr, state);

    if (v->tag == T_LOCPATH) {
        struct locpath *locpath = v->locpath;
        list_for_each(s, locpath->steps) {
            if (s->predicates != NULL) {
                check_preds(s->predicates, state);
                CHECK_ERROR;
            }
        }
    }
    expr->type = v->tag;
}

static void check_app(struct expr *expr, struct state *state) {
    assert(expr->tag == E_APP);
    for (int i=0; i < expr->func->arity; i++) {
        check_expr(expr->args[i], state);
        CHECK_ERROR;
        if (expr->args[i]->type != expr->func->arg_types[i]) {
            STATE_ERROR(state, PATHX_ETYPE);
            return;
        }
    }
    expr->type = expr->func->type;
}

/* Check the binary operators. Type rules:
 *
 * '=', '!='  : T_LOCPATH -> T_LOCPATH -> T_BOOLEAN
 *              T_STRING  -> T_LOCPATH -> T_BOOLEAN
 *              T_LOCPATH -> T_STRING  -> T_BOOLEAN
 *
 * '|' :        T_LOCPATH -> T_LOCPATH -> T_LOCPATH
 *
 * '+', '-', '*': T_NUMBER -> T_NUMBER -> T_NUMBER
 *
 */
static void check_binary(struct expr *expr, struct state *state) {
    check_expr(expr->left, state);
    check_expr(expr->right, state);
    CHECK_ERROR;

    enum type l = expr->left->type;
    enum type r = expr->right->type;
    int  ok = 1;
    enum type res;

    switch(expr->op) {
    case OP_EQ:
    case OP_NEQ:
        ok = ((l == T_LOCPATH || l == T_STRING)
              && (r == T_LOCPATH || r == T_STRING));
        res = T_BOOLEAN;
        break;
    case OP_PLUS:
    case OP_MINUS:
    case OP_STAR:
        ok =  (l == T_NUMBER && r == T_NUMBER);
        res = T_NUMBER;
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

/* Typecheck an expression */
static void check_expr(struct expr *expr, struct state *state) {
    CHECK_ERROR;
    switch(expr->tag) {
    case E_BINARY:
        check_binary(expr, state);
        break;
    case E_VALUE:
        check_value(expr, state);
        break;
    case E_APP:
        check_app(expr, state);
        break;
    default:
        assert(0);
    }
}

static void append_step(struct locpath *locpath, struct step *tail) {
    tail->prev = locpath->last;
    if (locpath->last != NULL)
        locpath->last->next = tail;
    locpath->last = tail;
    if (locpath->steps == NULL)
        locpath->steps = tail;
}

static void prepend_step(struct locpath *locpath, struct step *head) {
    if (locpath->steps != NULL)
        locpath->steps->prev = head;
    head->next = locpath->steps;
    locpath->steps = head;
    if (locpath->last == NULL)
        locpath->last = head;
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
 * Name ::= /[^/\[ \t\n]+/
 */
static char *parse_name(struct state *state) {
    const char *s = state->pos;
    char *result;

    while (*state->pos != '\0' &&
           *state->pos != L_BRACK && *state->pos != SEP &&
           *state->pos != R_BRACK && *state->pos != '=' &&
           !isspace(*state->pos)) {
        if (*state->pos == '\\') {
            state->pos += 1;
            if (*state->pos == '\0') {
                STATE_ERROR(state, PATHX_ENAME);
                return NULL;
            }
        }
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
        CHECK_ERROR_RET0;

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

    if (ALLOC(step) < 0) {
        STATE_ENOMEM;
        return NULL;
    }

    if (*state->pos == '.' && state->pos[1] == '.') {
        state->pos += 2;
        step->axis = PARENT;
    } else if (match(state, '.')) {
        step->axis = SELF;
    } else {
        step->axis = CHILD;
        for (int i = 0; i < ARRAY_CARDINALITY(axis_names); i++) {
            if (looking_at(state, axis_names[i], "::")) {
                step->axis = i;
                break;
            }
        }

        if (! match(state, '*')) {
            step->name = parse_name(state);
            if (HAS_ERROR(state))
                goto error;
        }

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
    CHECK_ERROR_RET0;

    if (ALLOC(locpath) < 0) {
        STATE_ENOMEM;
        goto error;
    }
    append_step(locpath, step);
    step = NULL;

    while (match(state, '/')) {
        if (*state->pos == '/') {
            state->pos += 1;
            step = make_step(DESCENDANT_OR_SELF, state);
            ENOMEM_ON_NULL(state, step);
            append_step(locpath, step);
        }
        step = parse_step(state);
        if (HAS_ERROR(state))
            goto error;
        append_step(locpath, step);
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
            prepend_step(locpath, step);
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
            prepend_step(locpath, step);
        }
    } else {
        locpath = parse_relative_location_path(state);
    }

    if (ALLOC(expr) < 0)
        goto err_nomem;
    expr->tag = E_VALUE;
    expr->value_ind = make_value(T_LOCPATH, state);
    CHECK_ERROR;
    expr_value(expr, state)->locpath = locpath;
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
            CHECK_ERROR;
        } while (match(state, ','));

        if (! match(state, ')')) {
            STATE_ERROR(state, PATHX_EDELIM);
            return;
        }
    }

    if (nargs != func->arity) {
        STATE_ERROR(state, PATHX_EDELIM);
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
 * PrimaryExpr ::= Literal
 *               | Number
 *               | FunctionCall
 *
 */
static void parse_primary_expr(struct state *state) {
    if (peek(state, "'\"")) {
        parse_literal(state);
    } else if (peek(state, "0123456789")) {
        parse_number(state);
    } else {
        parse_function_call(state);
    }
}

static int looking_at_primary_expr(struct state *state) {
    const char *s = state->pos;
    /* Is it a Number or Literal ? */
    if (peek(state, "'\"0123456789"))
        return 1;

    /* Or maybe a function call, i.e. a word followed by a '(' ?
     * Note that our function names are only [a-zA-Z]+
     */
    while (*s != '\0' && isalpha(*s)) s++;
    while (*s != '\0' && isspace(*s)) s++;
    return *s == '(';
}

/*
 * PathExpr ::= LocationPath | PrimaryExpr
 *
 * The grammar is ambiguous here: the expression '42' can either be the
 * number 42 (a PrimaryExpr) or the RelativeLocationPath 'child::42'. The
 * reason for this ambiguity is that we allow node names like '42' in the
 * tree; rather than forbid them, we resolve the ambiguity by always
 * parsing '42' as a number, and requiring that the user write the
 * RelativeLocationPath in a different form, e.g. 'child::42' or './42'.
 */
static void parse_path_expr(struct state *state) {
    if (looking_at_primary_expr(state)) {
        parse_primary_expr(state);
    } else {
        parse_location_path(state);
    }
}

/*
 * MultiplicativeExpr ::= PathExpr ('*' PathExpr)*
 */
static void parse_multiplicative_expr(struct state *state) {
    parse_path_expr(state);
    while (match(state, '*')) {
        parse_path_expr(state);
        push_new_binary_op(OP_STAR, state);
    }
}

/*
 * AdditiveExpr ::= MultiplicativeExpr (AdditiveOp MultiplicativeExpr)*
 * AdditiveOp   ::= '+' | '-'
 */
static void parse_additive_expr(struct state *state) {
    parse_multiplicative_expr(state);
    while (*state->pos == '+' || *state->pos == '-') {
        enum binary_op op = (*state->pos == '+') ? OP_PLUS : OP_MINUS;
        state->pos += 1;
        skipws(state);
        parse_multiplicative_expr(state);
        push_new_binary_op(op, state);
    }
}

/*
 * EqualityExpr ::= AdditiveExpr (EqualityOp AdditiveExpr)?
 * EqualityOp ::= "=" | "!="
 */
static void parse_equality_expr(struct state *state) {
    parse_additive_expr(state);
    if (*state->pos == '=' ||
        (*state->pos == '!' && state->pos[1] == '=')) {
        enum binary_op op = (*state->pos == '=') ? OP_EQ : OP_NEQ;
        state->pos += (op == OP_EQ) ? 1 : 2;
        skipws(state);
        parse_additive_expr(state);
        push_new_binary_op(op, state);
    }
}

/*
 * Expr ::= EqualityExpr
 */
static void parse_expr(struct state *state) {
    skipws(state);
    parse_equality_expr(state);
}

int pathx_parse(const struct tree *tree, const char *txt,
                struct pathx **pathx) {
    struct state *state = NULL;

    *pathx = NULL;

    if (ALLOC(*pathx) < 0)
        return PATHX_ENOMEM;

    (*pathx)->origin = (struct tree *) tree;

    /* Set up state */
    if (ALLOC((*pathx)->state) < 0) {
        free_pathx(*pathx);
        *pathx = NULL;
        return PATHX_ENOMEM;
    }
    state = (*pathx)->state;

    state->errcode = PATHX_NOERROR;
    state->txt = txt;
    state->pos = txt;

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

    if (state->exprs_used != 1) {
        STATE_ERROR(state, PATHX_EINTERNAL);
        goto done;
    }

    /* Typecheck */
    check_expr(state->exprs[0], state);
    if (HAS_ERROR(state))
        goto done;

    if (state->exprs[0]->type != T_LOCPATH) {
        STATE_ERROR(state, PATHX_ETYPE);
        goto done;
    }

    /* Evaluate */
    eval_expr(state->exprs[0], state);
    if (HAS_ERROR(state))
        goto done;

    if (state->values_used != 1) {
        STATE_ERROR(state, PATHX_EINTERNAL);
        goto done;
    }

    (*pathx)->locpath = pop_value(state)->locpath;

 done:
    return state->errcode;
}

/*************************************************************************
 * Searching in the tree
 *************************************************************************/

static bool step_matches(struct step *step, struct tree *tree) {
    return (step->name == NULL || streqx(step->name, tree->label));
}

/* Return the 1-based position of STEP->CUR amongst its siblings */
static int position(struct step *step) {
    int pos = 0;
    struct tree *cur = step->cur;

    for (struct tree *t = step_first(step, step->ctx);
         t != NULL;
         t = step_next(step)) {
        if (step_matches(step, t)) {
            pos += 1;
            if (t == cur) {
                step->cur = cur;
                return pos;
            }
        }
    }
    assert(0);
}

static int pred_matches(struct step *step, struct state *state) {
    struct pred *pred = step->predicates;

    if (step->cur == NULL)
        return 0;

    if (pred == NULL)
        return 1;

    for (int i=0; i < pred->nexpr; i++) {
        state->step = step;
        eval_expr(pred->exprs[i], state);
        struct value *v = pop_value(state);
        switch(v->tag) {
        case T_BOOLEAN:
            if (! v->boolval)
                return 0;
            break;
        case T_NUMBER:
            if (position(state->step) != v->number)
                return 0;
            break;
        case T_LOCPATH:
            v->locpath->ctx = state->step->cur;
            if (locpath_first(v->locpath, state) == NULL)
                return 0;
            break;
        default:
            assert(0);
        }
    }
    return 1;
}

static struct tree *step_first(struct step *step, struct tree *ctx) {
    step->ctx = ctx;
    switch (step->axis) {
    case SELF:
        step->cur = ctx;
        break;
    case CHILD:
        step->cur = ctx->children;
        break;
    case DESCENDANT:
        step->cur = ctx->children;
        break;
    case DESCENDANT_OR_SELF:
        step->cur = ctx;
        break;
    case PARENT:
    case ANCESTOR:
        step->cur = ctx->parent;
        break;
    case ROOT:
        while (ctx->parent != ctx)
            ctx = ctx->parent;
        step->cur = ctx;
        break;
    default:
        assert(0);
    }
    if (step->cur == NULL)
        return NULL;
    if (! step_matches(step, step->cur))
        step_next(step);
    return step->cur;
}

static struct tree *step_next(struct step *step) {
    while (step->cur != NULL) {
        switch (step->axis) {
        case SELF:
            step->cur = NULL;
            break;
        case CHILD:
            step->cur = step->cur->next;
            break;
        case DESCENDANT:
        case DESCENDANT_OR_SELF:
            if (step->cur->children != NULL) {
                step->cur = step->cur->children;
            } else {
                while (step->cur->next == NULL && step->cur != step->ctx)
                    step->cur = step->cur->parent;
                if (step->cur == step->ctx)
                    step->cur = NULL;
                else
                    step->cur = step->cur->next;
            }
            break;
        case PARENT:
            step->cur = NULL;
            break;
        case ANCESTOR:
            if (step->cur->parent == step->cur)
                step->cur = NULL;
            else
                step->cur = step->cur->parent;
            break;
        case ROOT:
            step->cur = NULL;
            break;
        default:
            assert(0);
        }
        if (step->cur != NULL && step_matches(step, step->cur))
            break;
    }
    return step->cur;
}

static struct tree *complete_path(struct step *step, struct state *state) {
    if (step == NULL)
        return NULL;

    while (1) {
        int found = pred_matches(step, state);

        if (found && step->next == NULL) {
            return step->cur;
        } else if (found && step_first(step->next, step->cur) != NULL) {
            step = step->next;
        } else {
            do {
                step_next(step);
                if (step->cur == NULL) {
                    step = step->prev;
                    if (step == NULL)
                        return NULL;
                    step_next(step);
                }
            } while (step->cur == NULL);
        }
    }
}

static struct tree *locpath_next(struct locpath *lp, struct state *state) {
    step_next(lp->last);
    return complete_path(lp->last, state);
}

struct tree *pathx_next(struct pathx *pathx) {
    return locpath_next(pathx->locpath, pathx->state);
}

static struct tree *locpath_first(struct locpath *lp, struct state *state) {
    step_first(lp->steps, lp->ctx);
    return complete_path(lp->steps, state);
}

/* Find the first node in TREE matching PATH. */
struct tree *pathx_first(struct pathx *pathx) {
    pathx->locpath->ctx = pathx->origin;
    return locpath_first(pathx->locpath, pathx->state);
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
static int locpath_search(struct locpath *lp, struct state *state,
                          struct tree **tmatch, struct step **smatch) {
    struct tree **matches = NULL;
    struct step *step;

    int *nmatches = NULL, nsteps = 0;
    int result, level;

    lp->ctx = *tmatch;
    *smatch = lp->steps;

    if (lp->ctx == NULL)
        return -1;

    list_for_each(s, lp->steps) nsteps++;

    if (ALLOC_N(matches, nsteps) < 0)
        return -1;

    if (ALLOC_N(nmatches, nsteps) < 0) {
        free(matches);
        return -1;
    }

    step = lp->steps;
    if (step_first(step, lp->ctx) == NULL) {
        *smatch = lp->steps;
        return 0;
    }

    level = 0;
    while (step != NULL) {
        int found = pred_matches(step, state);

        if (found) {
            if (nmatches[level] == 0)
                matches[level] = step->cur;
            nmatches[level]++;
        }

        if (found && step->next == NULL) {
            break;
        } else if (found && step_first(step->next, step->cur) != NULL) {
            step = step->next;
            level += 1;
        } else {
            do {
                step_next(step);
                if (step->cur == NULL) {
                    step = step->prev;
                    level -= 1;
                    if (step != NULL)
                        step_next(step);
                }
            } while (step != NULL && step->cur == NULL);
        }
    }

    result = nmatches[nsteps - 1] == 1;
    for (level = nsteps-1, step = lp->last;
         level >=0;
         level--, step = step->prev) {
        if (nmatches[level] > 1) {
            result = -1;
            break;
        }
        if (nmatches[level] == 1) {
            *tmatch = matches[level];
            *smatch = step->next;
            break;
        }
    }

    free(matches);
    free(nmatches);
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
    struct step *step;

    *tree = path->origin;
    r = locpath_search(path->locpath, path->state, tree, &step);
    if (r == -1)
        return -1;

    if (step == NULL)
        return 0;

    struct tree *first_child = NULL;
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
    return 0;

 error:
    if (first_child != NULL) {
        list_remove(first_child, first_child->parent->children);
        free_tree(first_child);
    }
    *tree = NULL;
    return -1;
}

int pathx_find_one(struct pathx *path, struct tree **tree) {
    *tree = pathx_first(path);
    if (*tree == NULL)
        return 0;

    if (pathx_next(path) != NULL) {
        *tree = NULL;
        return -1;
    }
    return 1;
}

const char *pathx_error(struct pathx *path, const char **txt, int *pos) {
    int errcode = PATHX_ENOMEM;

    if (path != NULL) {
        if (path->state->errcode < ARRAY_CARDINALITY(errcodes))
            errcode = path->state->errcode;
        else
            errcode = PATHX_EINTERNAL;
    }

    if (txt)
        *txt = path->state->txt;

    if (pos)
        *pos = path->state->pos - path->state->txt;

    return errcodes[errcode];
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
