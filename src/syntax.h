/*
 * syntax.h: Data types to represent language syntax
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

#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <limits.h>
#include "internal.h"
#include "lens.h"
#include "ref.h"
#include "fa.h"
#include "regexp.h"
#include "info.h"

void syntax_error(struct info *info, const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 2, 3);

void fatal_error(struct info *info, const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 2, 3);

enum term_tag {
    A_MODULE,
    A_BIND,              /* Module scope binding of a name */
    A_LET,               /* local LET .. IN binding */
    A_COMPOSE,
    A_UNION,
    A_MINUS,
    A_CONCAT,
    A_APP,
    A_VALUE,
    A_IDENT,
    A_BRACKET,
    A_FUNC,
    A_REP,
    A_TEST
};

enum test_result_tag {
    TR_CHECK,
    TR_PRINT,
    TR_EXN
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
            char          *mname;
            char          *autoload;
            struct term   *decls;
        };
        struct {                       /* A_BIND */
            char          *bname;
            struct term   *exp;
        };
        struct {              /* A_COMPOSE, A_UNION, A_CONCAT, A_APP, A_LET */
            struct term *left;
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
            enum test_result_tag tr_tag;
            struct term         *test;
            struct term         *result;
        };
    };
};

struct param {
    struct info   *info;
    unsigned int   ref;
    struct string *name;
    struct type   *type;
};

struct native {
    unsigned int argc;
    struct type *type;
    struct value *(*impl)(void);
};

/* An exception in the interpreter. Some exceptions are reported directly
 * into the central struct error; an exception for those is only generated
 * to follow the control flow for exceptions. Such exceptions have both
 * seen and error set to 1. They are the only exceptions with error == 1.
 * When error == 1, none of the other fields in the exn will be usable.
 */
struct exn {
    struct info *info;
    unsigned int seen : 1;      /* Whether the user has seen this EXN */
    unsigned int error : 1;
    char        *message;
    size_t       nlines;
    char       **lines;
};

/*
 * Values in the interpreter
 */
enum value_tag {
    V_STRING,
    V_REGEXP,
    V_LENS,
    V_TREE,
    V_FILTER,
    V_TRANSFORM,
    V_NATIVE,
    V_EXN,
    V_CLOS,
    V_UNIT
};

#define EXN(v) ((v)->tag == V_EXN)

struct value {
    unsigned int   ref;
    struct info   *info;
    enum value_tag tag;
    /* Nothing in this union for V_UNIT */
    union {
        struct string  *string;  /* V_STRING */
        struct regexp  *regexp;  /* V_REGEXP */
        struct lens    *lens;    /* V_LENS */
        struct native  *native;  /* V_NATIVE */
        struct tree    *origin;  /* V_TREE */
        struct filter  *filter;  /* V_FILTER */
        struct transform *transform; /* V_TRANSFORM */
        struct exn     *exn;     /* V_EXN */
        struct {                 /* V_CLOS */
            struct term     *func;
            struct binding  *bindings;
        };
    };
};

/* All types except for T_ARROW (functions) are simple. Subtype relations
 * for the simple types:
 *   T_STRING <: T_REGEXP
 * and the usual subtype relation for functions.
 */
enum type_tag {
    T_STRING,
    T_REGEXP,
    T_LENS,
    T_TREE,
    T_FILTER,
    T_TRANSFORM,
    T_ARROW,
    T_UNIT
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

/* A module maps names to TYPE * VALUE. */
struct module {
    unsigned int       ref;
    struct module     *next;     /* Only used for the global list of modules */
    struct transform  *autoload;
    char              *name;
    struct binding    *bindings;
};

struct type *make_arrow_type(struct type *dom, struct type *img);
struct type *make_base_type(enum type_tag tag);
/* Do not call this directly. Use unref(t, type) instead */
void free_type(struct type *type);

/* Constructors for some terms in syntax.c Constructor assumes ownership of
 * arguments without incrementing. Caller owns returned objects.
 */
struct term *make_term(enum term_tag tag, struct info *info);
void free_term(struct term *term);
struct term *make_param(char *name, struct type *type, struct info *info);
struct value *make_value(enum value_tag tag, struct info *info);
struct value *make_unit(struct info *info);
struct term *make_app_term(struct term *func, struct term *arg,
                           struct info *info);
struct term *make_app_ident(char *id, struct term *func, struct info *info);

/* Print a tree in the braces style used in modules */
void print_tree_braces(FILE *out, int indent, struct tree *tree);

/* Make an EXN value
 * Receive ownership of INFO
 *
 * FORMAT and following arguments are printed to a new string. Caller must
 * clean those up.
 */
struct value *make_exn_value(struct info *info, const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 2, 3);

/* Add NLINES lines (passed as const char *) to EXN, which must be a
 * value with tag V_EXN, created by MAKE_EXN_VALUE.
 *
 * The strings belong to EXN * after the call.
 */
void exn_add_lines(struct value *exn, int nlines, ...);

void exn_printf_line(struct value *exn, const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 2, 3);

/* Do not call these directly, use UNREF instead */
void free_value(struct value *v);
void free_module(struct module *module);

/* Turn a list of PARAMS (represented as terms tagged as A_FUNC with the
 * param in PARAM) into nested A_FUNC terms
 */
struct term *build_func(struct term *params, struct term *exp);

struct module *module_create(const char *name);

#define define_native(error, module, name, argc, impl, types ...)       \
    define_native_intl(__FILE__, __LINE__, error, module, name,         \
                       argc, impl, ## types)

ATTRIBUTE_RETURN_CHECK
int define_native_intl(const char *fname, int line,
                       struct error *error,
                       struct module *module, const char *name,
                       int argc, void *impl, ...);

struct module *builtin_init(struct error *);

int load_module_file(struct augeas *aug, const char *filename);

/* The name of the builtin function that checks recursive lenses */
#define LNS_CHECK_REC_NAME "lns_check_rec"

int interpreter_init(struct augeas *aug);

struct lens *lens_lookup(struct augeas *aug, const char *qname);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
