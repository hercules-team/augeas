/*
 * syntax.h: Data types to represent language syntax
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

#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <regex.h>
#include <limits.h>
#include "internal.h"
#include "lens.h"

/* Reference counting for pointers to structs with a REF field
 *
 * When a pointer to such a struct is passed into a function that stores
 * it, the function can either "receive ownership", meaning it does not
 * increment the reference count, or it can "take ownership", meaning it
 * increments the reference count. In the first case, the reference is now
 * owned by wherever the function stored it, and not the caller anymore; in
 * the second case, the caller and whereever the reference was stored both
 * own the reference.
 */
// FIXME: This is not threadsafe; incr/decr ref needs to be protected

#define REF_MAX UINT_MAX

#define make_ref(var)                                                   \
    do {                                                                \
        CALLOC(var, 1);                                                 \
        var->ref = 1;                                                   \
    } while(0)
#define ref(s) (((s) == NULL || (s)->ref == REF_MAX) ? (s) : ((s)->ref++, (s)))
#define unref(s, t)                                                     \
    do {                                                                \
        if ((s) != NULL && (s)->ref != REF_MAX) {                      \
            assert((s)->ref > 0);                                       \
            if (--(s)->ref == 0) {                                      \
                /*memset(s, 255, sizeof(*s));*/                         \
                free_##t(s);                                            \
                (s) = NULL;                                             \
            }                                                           \
        }                                                               \
    } while(0)

/* Make VAR uncollectable and pin it in memory for eternity */
#define ref_pin(var)   (var)->ref = REF_MAX

struct info {
    unsigned int ref;
    struct string *filename;
    unsigned int first_line;
    unsigned int first_column;
    unsigned int last_line;
    unsigned int last_column;
};

/* syntax.c */
char *format_info(struct info *info);
void print_info(FILE *out, struct info *info);

void syntax_error(struct info *info, const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 2, 3);

__attribute__((noreturn))
void fatal_error(struct info *info, const char *format, ...);

void assert_error_at(const char *srcfile, int srclineno, struct info *info,
                     const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 4, 5);

enum term_tag {
    A_MODULE,
    A_BIND,              /* Module scope binding of a name */
    A_LET,               /* local LET .. IN binding */
    A_COMPOSE,
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
            const char    *mname;
            const char    *autoload;
            struct term   *decls;
        };
        struct {                       /* A_BIND */
            const char    *bname;
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

/* Defined in regexp.c */

void print_regexp(FILE *out, struct regexp *regexp);

/* Make a regexp with pattern PAT, which is not copied. Ownership
 * of INFO is taken.
 */
struct regexp *make_regexp(struct info *info, const char *pat);

/* Make a regexp that matches TEXT literally; the string TEXT
 * is not used by the returned rgexp and must be freed by the caller
 */
struct regexp *make_regexp_literal(struct info *info, const char *text);

/* Do not call directly, use UNREF instead */
void free_regexp(struct regexp *regexp);

/* Compile R->PATTERN into R->RE; return -1 and print an error
 * if compilation fails. Return 0 otherwise
 */
int regexp_compile(struct regexp *r);

/* Call RE_MATCH on R->RE and return its result; if R hasn't been compiled
 * yet, compile it. Return -3 if compilation fails
 */
int regexp_match(struct regexp *r, const char *string, const int size,
                 const int start, struct re_registers *regs);

/* Return the number of subexpressions (parentheses) inside R. May cause
 * compilation of R; return -1 if compilation fails.
 */
int regexp_nsub(struct regexp *r);

struct regexp *
regexp_union(struct info *, struct regexp *r1, struct regexp *r2);

struct regexp *
regexp_concat(struct info *, struct regexp *r1, struct regexp *r2);

struct regexp *
regexp_iter(struct info *info, struct regexp *r, int min, int max);

struct regexp *
regexp_maybe(struct info *info, struct regexp *r);

struct regexp *regexp_make_empty(struct info *);

struct native {
    unsigned int argc;
    struct type *type;
    struct value *(*impl)(void);
};

/*
 * Transformers for going from file globs to path names in the tree
 * functions are in transform.c
 */

/* Filters for globbing files */
struct filter {
    unsigned int   ref;
    struct filter *next;
    struct string *glob;
    unsigned int   include : 1;
};

struct filter *make_filter(struct string *glb, unsigned int include);
void free_filter(struct filter *filter);

/* Transformers that actually run lenses on contents of files */
struct transform {
    unsigned int      ref;
    struct lens      *lens;
    struct filter    *filter;
};

struct transform *make_transform(struct lens *lens, struct filter *filter);
void free_transform(struct transform *xform);

/* Load all files matching the TRANSFORM's filter into the tree in AUG by
 * applying the TRANSFORM's lens to their contents and putting the
 * resulting tree under "/files" + filename. Also stores some information
 * about filename underneath "/augeas/files" + filename
 */
int transform_load(struct augeas *aug, struct transform *transform);

/* Return 1 if TRANSFORM applies to PATH, 0 otherwise. The TRANSFORM
 * applies to PATH if (1) PATH starts with "/files/" and (2) the rest of
 * PATH matches the transform's filter
*/
int transform_applies(struct transform *transform, const char *path);

/* Save TREE into the file corresponding to PATH. It is assumed that the
 * TRANSFORM applies to that PATH
 */
int transform_save(struct augeas *aug, struct transform *transform,
                   const char *path, struct tree *tree);

/* An exception in the interpreter */
struct exn {
    struct info *info;
    unsigned int seen : 1;      /* Whether the user has seen this EXN */
    const char  *message;
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
    V_CLOS
};

#define EXN(v) ((v)->tag == V_EXN)

struct value {
    unsigned int   ref;
    struct info   *info;
    enum value_tag tag;
    union {
        struct string  *string;  /* V_STRING */
        struct regexp  *regexp;  /* V_REGEXP */
        struct lens    *lens;    /* V_LENS */
        struct native  *native;  /* V_NATIVE */
        struct tree    *tree;    /* V_TREE */
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

/* A module maps names to TYPE * VALUE. */
struct module {
    unsigned int       ref;
    struct module     *next;     /* Only used for the global list of modules */
    struct transform  *autoload;
    const char        *name;
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
struct term *make_param(const char *name, struct type *type,
                        struct info *info);
struct value *make_value(enum value_tag tag, struct info *info);
struct string *make_string(const char *str);
struct term *make_app_term(struct term *func, struct term *arg,
                           struct info *info);
struct term *make_app_ident(const char *id, struct term *func,
                            struct info *info);

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
void free_info(struct info *info);
void free_string(struct string *string);
void free_value(struct value *v);
void free_module(struct module *module);

/* Turn a list of PARAMS (represented as terms tagged as A_FUNC with the
 * param in PARAM) into nested A_FUNC terms
 */
struct term *build_func(struct term *params, struct term *exp);

struct module *module_create(const char *name);

#define define_native(module, name, argc, impl, types ...)       \
    define_native_intl(__FILE__, __LINE__, module, name, argc, impl, ## types)

void define_native_intl(const char *fname, int line,
                        struct module *module, const char *name,
                        int argc, void *impl, ...);

struct module *builtin_init(void);

/* Used by augparse for some testing */
int __aug_load_module_file(struct augeas *aug, const char *filename);

int interpreter_init(struct augeas *aug);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
