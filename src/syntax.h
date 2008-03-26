/*
 * syntax.h: Data types to represent language syntax
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

#ifndef __SYNTAX_H
#define __SYNTAX_H

#include <regex.h>
#include <limits.h>
#include "internal.h"
#include "lens.h"

/* Reference counting for pointers to structs with a REF field */
// FIXME: This is not threadsafe; incr/decr ref needs to be protected
#define make_ref(var)                                                   \
    do {                                                                \
        CALLOC(var, 1);                                                 \
        var->ref = 1;                                                   \
    } while(0)
#define ref(s) (((s) == NULL || (s)->ref == UINT_MAX) ? (s) : ((s)->ref++, (s)))
#define unref(s, t)                                                     \
    do {                                                                \
        if ((s) != NULL && (s)->ref != UINT_MAX) {                      \
            assert((s)->ref > 0);                                       \
            if (--(s)->ref == 0) {                                      \
                /*memset(s, 255, sizeof(*s));*/                         \
                free_##t(s);                                            \
                (s) = NULL;                                             \
            }                                                           \
        }                                                               \
    } while(0)

#define list_unref(list, type)                                          \
    do {                                                                \
        while ((list) != NULL && (list)->ref == 1) {                    \
            typeof(list) _del = (list);                                 \
            (list) = (list)->next;                                      \
            unref(_del, type);                                          \
        }                                                               \
    } while(0)

struct info {
    unsigned int ref;
    struct string *filename;
    unsigned int first_line;
    unsigned int first_column;
    unsigned int last_line;
    unsigned int last_column;
};

/* syntax.c */
void print_info(FILE *out, struct info *info);

void syntax_error(struct info *info, const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 2, 3);

__attribute__((noreturn))
void fatal_error(struct info *info, const char *format, ...);

enum term_tag {
    A_MODULE,
    A_BIND,
    A_UNION,
    A_CONCAT,
    A_APP,
    A_VALUE,
    A_IDENT,
    A_BRACKET,
    A_FUNC,
    A_REP,
    A_TEST
};

enum quant_tag {
    Q_STAR,
    Q_PLUS,
    Q_MAYBE
};

struct term {
    struct term *next;
    unsigned int ref;
    struct info *info;
    struct type *type;                /* Filled in by the typechecker */
    enum term_tag tag;
    union {
        struct {                       /* A_MODULE */
            const char    *mname;
            struct term   *decls;
        };
        struct {                       /* A_BIND */
            const char    *bname;
            struct term   *exp;
        };
        struct {
            struct term *left;          /* A_UNION, A_CONCAT, A_APP */
            struct term *right;
        };
        struct value    *value;         /* A_VALUE */
        struct term     *brexp;         /* A_BRACKET */
        struct string   *ident;         /* A_IDENT */
        struct {                        /* A_REP */
            enum quant_tag quant;
            struct term  *rexp;
        };
        struct {                       /* A_FUNC */
            struct param *param;
            struct term   *body;
        };
        struct {                       /* A_TEST */
            struct term *test;
            struct term *result;
        };
    };
};

struct param {
    struct info   *info;
    unsigned int   ref;
    struct string *name;
    struct type   *type;
};

struct string {
    unsigned int   ref;
    const char    *str;
};

struct regexp {
    unsigned int              ref;
    struct info              *info;
    struct string            *pattern;
    struct re_pattern_buffer *re;
};

struct native {
    unsigned int argc;
    struct type *type;
    struct value *(*impl)(void);
};

enum value_tag {
    V_STRING,
    V_REGEXP,
    V_LENS,
    V_TREE,
    V_NATIVE,
    V_CLOS
};

struct value {
    unsigned int   ref;
    struct info   *info;
    enum value_tag tag;
    union {
        struct string  *string;  /* V_STRING */
        struct regexp  *regexp;  /* V_REGEXP */
        struct lens    *lens;    /* V_LENS */
        struct native  *native;  /* V_NATIVE */
        struct tree    *tree;    /* V_TREE, FIXME: really needed ? */
        struct {                 /* V_CLOS */
            struct term     *func;
            struct binding  *bindings;
        };
    };
};

enum type_tag {
    T_STRING,
    T_REGEXP,
    T_LENS,
    T_TREE,
    T_ARROW
};

struct type {
    unsigned int   ref;
    enum type_tag  tag;
    struct type   *dom;    /* T_ARROW */
    struct type   *img;    /* T_ARROW */
};

struct binding {
    unsigned int    ref;
    struct binding *next;
    struct string  *ident;
    struct type    *type;
    struct value   *value;
};

/* An environment maps names to TYPE * VALUE. Each module has its own
   environment */
struct env {
    unsigned int    ref;
    struct env     *next;     /* Only used for the global list of modules */
    const char     *name;
    struct binding *bindings;
};

struct ctx {
    struct env     *global;
    struct binding *local;
};

struct type *make_arrow_type(struct type *dom, struct type *img);
struct type *make_base_type(enum type_tag tag);
/* Do not call this directly. Use unref(t, type) instead */
void free_type(struct type *type);

/* Constructors for some terms in syntax.c Constructor assumes ownership of
 * arguments without incrementing. Caller owns returned objects.
 */
struct term *make_term(enum term_tag tag, struct info *info);
struct term *make_param(const char *name, struct type *type,
                        struct info *info);
struct value *make_value(enum value_tag tag, struct info *info);
struct string *make_string(const char *str);
struct term *make_app_term(struct term *func, struct term *arg,
                           struct info *info);
struct term *make_app_ident(const char *id, struct term *func,
                            struct info *info);

/* Turn a list of PARAMS (represented as terms tagged as A_FUNC with the
 * param in PARAM) into nested A_FUNC terms
 */
struct term *build_func(struct term *params, struct term *exp);

struct env *env_create(const char *name);

#define define_native(env, name, argc, impl, types ...)       \
    define_native_intl(__FILE__, __LINE__, env, name, argc, impl, ## types)

void define_native_intl(const char *fname, int line,
                        struct env *env, const char *name,
                        int argc, void *impl, ...);

int typecheck(struct term *module, struct env *env);
struct env *compile(struct term *term, struct env *global);
struct env *builtin_init(void);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
