/*
 * syntax.c: 
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

#include <assert.h>
#include <stdarg.h>
#include <limits.h>

#include "syntax.h"

static const char *const builtin_module = "Builtin";

const struct type string_type = { .ref = UINT_MAX, .tag = T_STRING };
const struct type regexp_type = { .ref = UINT_MAX, .tag = T_REGEXP };
const struct type lens_type =   { .ref = UINT_MAX, .tag = T_LENS };
const struct type tree_type =   { .ref = UINT_MAX, .tag = T_TREE };

const struct type *const t_string = &string_type;
const struct type *const t_regexp = &regexp_type;
const struct type *const t_lens   = &lens_type;
const struct type *const t_tree   = &tree_type;

static const char *const type_names[] = {
    "string", "regexp", "lens", "tree", "function", NULL
};

void print_info(FILE *out, struct info *info) {
    fprintf(out, "%s:",
            info->filename != NULL ? info->filename->str : "(unknown file)");
    if (info->first_line > 0) {
        if (info->first_line == info->last_line) {
            if (info->first_column == info->last_column) {
                fprintf(out, "%d.%d:", info->first_line, info->first_column);
            } else {
                fprintf(out, "%d.%d-.%d:", info->first_line,
                        info->first_column, info->last_column);
            }
        } else {
            fprintf(out, "%d.%d-%d.%d:",
                    info->first_line, info->first_column,
                    info->last_line, info->last_column);
        }
    }
}

void syntax_error(struct info *info, const char *format, ...) {
    va_list ap;

    print_info(stderr, info);
	va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void fatal_error(struct info *info, const char *format, ...) {
    va_list ap;

    fprintf(stderr, "Fatal internal error. Aborting. Details:\n");
    print_info(stderr, info);
	va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    abort();
}

static void free_value(struct value *v);

static void free_string(struct string *string) {
    if (string == NULL)
        return;
    assert(string->ref == 0);
    free((char *) string->str);
    free(string);
}

static void free_info(struct info *info) {
    if (info == NULL)
        return;
    assert(info->ref == 0);
    unref(info->filename, string);
    free(info);
}

static void free_param(struct param *param) {
    if (param == NULL)
        return;
    assert(param->ref == 0);
    unref(param->info, info);
    unref(param->name, string);
    unref(param->type, type);
    free(param);
}

static void free_regexp(struct regexp *regexp) {
    if (regexp == NULL)
        return;
    assert(regexp->ref == 0);
    unref(regexp->info, info);
    free((void *) regexp->pattern);
    if (regexp->re != NULL)
        FIXME("Free regexp->re");
    free(regexp);
}

static void free_lens(struct lens *lens) {
    if (lens == NULL)
        return;
    //assert(lens->ref == 0);

    FIXME("Free lens");
}

static void free_term(struct term *term) {
    if (term == NULL)
        return;
    assert(term->ref == 0);
    switch(term->tag) {
    case A_MODULE:
        free((char *) term->mname);
        list_unref(term->decls, term);
        break;
    case A_BIND:
        free((char *) term->bname);
        unref(term->exp, term);
    case A_UNION:
    case A_CONCAT:
    case A_APP:
        unref(term->left, term);
        unref(term->right, term);
        break;
    case A_VALUE:
        unref(term->value, value);
        break;
    case A_IDENT:
        unref(term->ident, string);
        break;
    case A_BRACKET:
        unref(term->brexp, term);
        break;
    case A_FUNC:
        unref(term->param, param);
        unref(term->body, term);
        break;
    case A_REP:
        unref(term->rexp, term);
        break;
    default:
        assert(0);
        break;
    }
    unref(term->next, term);
    unref(term->info, info);
    unref(term->type, type);
    free(term);
}

static void free_binding(struct binding *binding) {
    if (binding == NULL)
        return;
    assert(binding->ref == 0);
    unref(binding->ident, string);
    unref(binding->type, type);
    unref(binding->value, value);
    free(binding);
}

static void free_env(struct env *env) {
    if (env == NULL)
        return;
    assert(env->ref == 0);
    free((char *) env->name);
    list_unref(env->bindings, binding);
    free(env);
}

void free_type(struct type *type) {
    if (type == NULL)
        return;
    assert(type->ref == 0);

    if (type->tag == T_ARROW) {
        unref(type->dom, type);
        unref(type->img, type);
    }
    free(type);
}

static void free_value(struct value *v) {
    if (v == NULL)
        return;
    assert(v->ref == 0);

    switch(v->tag) {
    case V_STRING:
        free((void *) v->string);
        break;
    case V_REGEXP:
        unref(v->regexp, regexp);
        break;
    case V_LENS:
        unref(v->lens, lens);
        break;
    case V_CLOS:
        unref(v->func, term);
        unref(v->bindings, binding);
        break;
    default:
        assert(0);
    }
    unref(v->info, info);
    free(v);
}

/*
 * Creation of (some) terms. Others are in parser.y
 * Refernce counted arguments are now owned by the returned object, i.e.
 * the make_* functions do not increment the count.
 * Returned objects have a referece count of 1.
 */
struct term *make_term(enum term_tag tag, struct info *info) {
  struct term *term;
  make_ref(term);
  term->tag = tag;
  if (info != NULL)
      term->info = info;
  return term;
}

struct term *make_param(const char *name, struct type *type,
                        struct info *info) {
  struct term *term = make_term(A_FUNC, info);
  make_ref(term->param);
  term->param->info = ref(term->info);
  make_ref(term->param->name);
  term->param->name->str = name;
  term->param->type = type;
  return term;
}

struct value *make_value(enum value_tag tag, struct info *info) {
    struct value *value;
    make_ref(value);
    value->tag = tag;
    value->info = info;
    return value;
}

struct string *make_string(const char *str) {
    struct string *string;
    make_ref(string);
    string->str = str;
    return string;
}

struct term *make_app_term(struct term *lambda, struct term *arg,
                           struct info *info) {
  struct term *app = make_term(A_APP, info);
  app->left = lambda;
  app->right = arg;
  return app;
}

struct term *make_app_ident(const char *id, struct term *arg,
                            struct info *info) {
    struct term *ident = make_term(A_IDENT, ref(info));
    ident->ident = make_string(id);
    return make_app_term(ident, arg, info);
}

struct term *build_func(struct term *params, struct term *exp) {
  assert(params->tag == A_FUNC);
  if (params->next != NULL)
    exp = build_func(params->next, exp);

  params->body = exp;
  params->next = NULL;
  return params;
}

/* Ownership is taken as needed */
static struct value *make_closure(struct term *func, struct binding *bnds) {
    struct value *v;
    make_ref(v);
    v->tag  = V_CLOS;
    v->info = ref(func->info);
    v->func = ref(func);
    v->bindings = ref(bnds);
    return v;
}

/*
 * Environments
 */
struct env *env_create(const char *name) {
    struct env *env;
    make_ref(env);
    env->name = strdup(name);
    return env;
}

static struct env *env_find(struct env *env, const char *name) {
    list_for_each(e, env) {
        if (STREQ(e->name, name))
            return e;
    }
    return NULL;
}

static struct binding *bnd_lookup(struct binding *bindings, const char *name) {
    list_for_each(b, bindings) {
        if (STREQ(b->ident->str, name))
            return b;
    }
    return NULL;
}

static struct binding *env_qual_lookup(struct env *env, const char *name) {
    if (env == NULL)
        return NULL;
    int nlen = strlen(env->name);
    if (STREQLEN(env->name, name, strlen(env->name))
        && name[nlen] == '.')
        return bnd_lookup(env->bindings, name + nlen + 1);
    return NULL;
}

static struct binding *ctx_lookup_bnd(struct ctx *ctx, const char *name) {
    struct binding *b = bnd_lookup(ctx->local, name);
    if (b != NULL)
        return b;

    if (strchr(name, '.') != NULL) {
        list_for_each(env, ctx->global) {
            b = env_qual_lookup(env, name);
            if (b != NULL)
                return b;
        }
    } else {
        struct env *builtin = env_find(ctx->global, builtin_module);
        assert(builtin != NULL);
        return bnd_lookup(builtin->bindings, name);
    }
    return NULL;
}

static struct value *ctx_lookup(struct ctx *ctx, struct string *ident) {
    struct binding *b = ctx_lookup_bnd(ctx, ident->str);
    return b == NULL ? NULL : b->value;
}

static struct type *ctx_lookup_type(struct ctx *ctx, struct string *ident) {
    struct binding *b = ctx_lookup_bnd(ctx, ident->str);
    return b == NULL ? NULL : b->type;
}

/* Takes ownership as needed */
static struct binding *bind_type(struct binding **bnds,
                                 const char *name, struct type *type) {
    struct binding *binding;
    make_ref(binding);
    make_ref(binding->ident);
    binding->ident->str = strdup(name);
    binding->type = ref(type);
    list_cons(*bnds, binding);

    return binding;
}

/* Takes ownership as needed */
static void bind_param(struct binding **bnds, struct param *param,
                       struct value *v) {
    struct binding *b;
    make_ref(b);
    b->ident = ref(param->name);
    b->type  = ref(param->type);
    b->value = ref(v);
    list_cons(*bnds, b);
}

static void unbind_param(struct binding **bnds, struct param *param) {
    struct binding *b = *bnds;
    assert(b->ident == param->name);
    assert(b->next != *bnds);
    *bnds = b->next;
    unref(b, binding);
}

/* Takes ownership as needed */
static void bind(struct binding **bnds,
                 const char *name, struct type *type, struct value *value) {
    struct binding *b = bind_type(bnds, name, type);
    b->value = ref(value);
}

/*
 * Some debug printing
 */

static const char *type_string(struct type *t);

static void dump_bindings(struct binding *bnds) {
    list_for_each(b, bnds) {
        const char *st = type_string(b->type);
        fprintf(stderr, "    %s: %s", b->ident->str, st);
        fprintf(stderr, "%s\n", b->value == NULL ? " n" : " v");
        free((char *) st);
    }
}

static void dump_env(struct env *env) {
    if (env == NULL)
        return;
    fprintf(stderr, "Env %s\n:", env->name);
    dump_bindings(env->bindings);
    dump_env(env->next);
}

ATTRIBUTE_UNUSED
static void dump_ctx(struct ctx *ctx) {
    fprintf(stderr, "Context:\n");
    dump_bindings(ctx->local);
    dump_env(ctx->global);
}

/*
 * Types
 */
struct type *make_arrow_type(struct type *dom, struct type *img) {
  struct type *type;
  make_ref(type);
  type->tag = T_ARROW;
  type->dom = ref(dom);
  type->img = ref(img);
  return type;
}

struct type *make_base_type(enum type_tag tag) {
    if (tag == T_STRING)
        return (struct type *) t_string;
    else if (tag == T_REGEXP)
        return (struct type *) t_regexp;
    else if (tag == T_LENS)
        return (struct type *) t_lens;
    else if (tag == T_TREE)
        return (struct type *) t_tree;
    else
        assert(0);
}

static const char *type_name(struct type *t) {
    for (int i = 0; type_names[i] != NULL; i++)
        if (i == t->tag)
            return type_names[i];
    assert(0);
}

static const char *type_string(struct type *t) {
    if (t->tag == T_ARROW) {
        char *s = NULL;
        const char *sd = type_string(t->dom);
        const char *si = type_string(t->img);
        if (t->dom->tag == T_ARROW)
            asprintf(&s, "(%s) -> %s", sd, si);
        else
            asprintf(&s, "%s -> %s", sd, si);
        free((char *) sd);
        free((char *) si);
        return s;
    } else {
        return strdup(type_name(t));
    }
}

/* Decide whether T1 is a subtype of T2. The only subtype relations are
 * T_STRING <: T_REGEXP and the usual subtyping of functions based on
 * comparing domains/images 
 *
 * Return 1 if T1 is a subtype of T2, 0 otherwise
 */
static int subtype(struct type *t1, struct type *t2) {
    if (t1 == t2)
        return 1;
    /* We only promote T_STRING => T_REGEXP, no automatic conversion
       of strings/regexps to lenses (yet) */
    if (t1->tag == T_STRING)
        return (t2->tag == T_STRING || t2->tag == T_REGEXP);
    if (t1->tag == T_REGEXP)
        return t2->tag == T_REGEXP;
    if (t1->tag == T_LENS)
        return t2->tag == T_LENS;
    if (t1->tag == T_ARROW && t2->tag == T_ARROW) {
        return subtype(t2->dom, t1->dom) 
            && subtype(t1->img, t2->img);
    }
    return 0;
}

static int type_equal(struct type *t1, struct type *t2) {
    return (t1 == t2) || (subtype(t1, t2) && subtype(t2, t1));
}

/* Return a type T with subtype(T, T1) && subtype(T, T2) */
static struct type *type_meet(struct type *t1, struct type *t2);

/* Return a type T with subtype(T1, T) && subtype(T2, T) */
static struct type *type_join(struct type *t1, struct type *t2) {
    if (t1->tag == T_STRING) {
        if (t2->tag == T_STRING)
            return ref(t1);
        else if (t2->tag == T_REGEXP)
            return ref(t2);
    } else if (t1->tag == T_REGEXP) {
        if (t2->tag == T_STRING)
            return ref(t1);
        else if (t2->tag == T_REGEXP)
            return ref(t1);
    } else if (t1->tag == T_ARROW) {
        if (t2->tag != T_ARROW)
            return NULL;
        struct type *dom = type_meet(t1->dom, t2->dom);
        struct type *img = type_join(t1->img, t2->img);
        if (dom == NULL || img == NULL) {
            unref(dom, type);
            unref(img, type);
            return NULL;
        }
        return make_arrow_type(dom, img);
    } else if (type_equal(t1, t2)) {
        return ref(t1);
    }
    return NULL;
}

/* Return a type T with subtype(T, T1) && subtype(T, T2) */
static struct type *type_meet(struct type *t1, struct type *t2) {
    if (t1->tag == T_STRING) {
        if (t2->tag == T_STRING || t2->tag == T_REGEXP)
            return ref(t1);
    } else if (t1->tag == T_REGEXP) {
        if (t2->tag == T_STRING || t2->tag == T_REGEXP)
            return ref(t2);
    } else if (t1->tag == T_ARROW) {
        if (t2->tag != T_ARROW)
            return NULL;
        struct type *dom = type_join(t1->dom, t2->dom);
        struct type *img = type_meet(t1->img, t2->img);
        if (dom == NULL || img == NULL) {
            unref(dom, type);
            unref(img, type);
            return NULL;
        }
        return make_arrow_type(dom, img);
    } else if (type_equal(t1, t2)) {
        return ref(t1);
    }
    return NULL;
}

static struct type *value_type(struct value *v) {
    switch(v->tag) {
    case V_STRING:
        return make_base_type(T_STRING);
    case V_REGEXP:
        return make_base_type(T_REGEXP);
    case V_LENS:
        return make_base_type(T_LENS);
    case V_TREE:
        return make_base_type(T_TREE);
    case V_NATIVE:
        return ref(v->native->type);
    case V_CLOS:
        return ref(v->func->type);
    default:
        assert(0);
    }
}

/* Coerce V to the type T. Currently, only T_STRING can be coerced to
 * T_REGEXP. Returns a value that is owned by the caller. Trying to perform
 * an impossible coercion is a fatal error.
 */
static struct value *coerce(struct value *v, struct type *t) {
    struct type *vt = value_type(v);
    if (type_equal(vt, t)) {
        unref(vt, type);
        return ref(v);
    }
    if (vt->tag == T_STRING && t->tag == T_REGEXP) {
        struct value *rxp = make_value(V_REGEXP, ref(v->info));
        make_ref(rxp->regexp);
        // FIXME: Escape special characters in v->string; we take
        // it to be a regexp matching v->string literally
        rxp->regexp->pattern = ref(v->string);
        return rxp;
    }
    fatal_error(v->info, "Failed to coerce %s to %s",
                type_name(vt), type_name(t));
}


/* Return one of the expected types (passed as ...).
   Does not give ownership of the returned type */
static struct type *expect_types_arr(struct info *info,
                                     struct type *act, 
                                     int ntypes, struct type *allowed[]) {
    struct type *result = NULL;

    for (int i=0; i < ntypes; i++) {
        if (subtype(act, allowed[i])) {
            result = allowed[i];
            break;
        }
    }
    if (result == NULL) {
        int len = 0;
        for (int i=0; i < ntypes; i++) {
            len += strlen(type_name(allowed[i]));
        }
        len += (ntypes - 1) * 4 + 1;
        char *allowed_names;
        CALLOC(allowed_names, len);
        for (int i=0; i < ntypes; i++) {
            if (i > 0)
                strcat(allowed_names, (i == ntypes - 1) ? ", or " : ", ");
            strcat(allowed_names, type_name(allowed[i]));
        }
        syntax_error(info, "type error: expected %s but found %s",
                     allowed_names, type_name(act));
        free(allowed_names);
    }
    return result;
}

static struct type *expect_types(struct info *info,
                                 struct type *act, int ntypes, ...) {
    va_list ap;
    struct type *allowed[ntypes];

    va_start(ap, ntypes);
    for (int i=0; i < ntypes; i++)
        allowed[i] = va_arg(ap, struct type *);
    va_end(ap);
    return expect_types_arr(info, act, ntypes, allowed);
}

static struct value *apply(struct term *app, struct ctx *ctx);

typedef struct value *(*impl0)(struct info *);
typedef struct value *(*impl1)(struct info *, struct value *);
typedef struct value *(*impl2)(struct info *, struct value *, struct value *);
typedef struct value *(*impl3)(struct info *, struct value *, struct value *,
                               struct value *);
typedef struct value *(*impl4)(struct info *, struct value *, struct value *,
                               struct value *, struct value *);
typedef struct value *(*impl5)(struct info *, struct value *, struct value *,
                               struct value *, struct value *, struct value *);

static struct value *native_call(struct info *info,
                                 struct native *func, struct ctx *ctx) {
    struct value *argv[func->argc];
    struct binding *b = ctx->local;
    struct value *result;

    for (int i = func->argc - 1; i >= 0; i--) {
        argv[i] = b->value;
        b = b->next;
    }

    switch(func->argc) {
    case 0:
        result = ((impl0) *func->impl)(info);
        break;
    case 1:
        result = ((impl1) *func->impl)(info, argv[0]);
        break;
    case 2:
        result = ((impl2) *func->impl)(info, argv[0], argv[1]);
        break;
    case 3:
        result = ((impl3) *func->impl)(info, argv[0], argv[1], argv[2]);
        break;
    case 4:
        result = ((impl4) *func->impl)(info, argv[0], argv[1], argv[2], argv[3]);
        break;
    case 5:
        result = ((impl5) *func->impl)(info, argv[0], argv[1], argv[2], argv[3],
                                       argv[4]);
        break;
    default:
        assert(0);
        break;
    }

    return result;
}

static void type_error1(struct info *info, const char *msg, struct type *type) {
    const char *s = type_string(type);
    syntax_error(info, "Type error: ");
    syntax_error(info, msg, s);
    free((char *) s);
}
                        
static void type_error2(struct info *info, const char *msg, 
                        struct type *type1, struct type *type2) {
    const char *s1 = type_string(type1);
    const char *s2 = type_string(type2);
    syntax_error(info, "Type error: ");
    syntax_error(info, msg, s1, s2);
    free((char *) s1);
    free((char *) s2);
}

static int check_exp(struct term *term, struct ctx *ctx);

static struct type *require_exp_type(struct term *term, struct ctx *ctx, 
                                     int ntypes, ...) {
    va_list ap;
    struct type *allowed[ntypes];
    int r = 1;

    r = check_exp(term, ctx);
    if (! r)
        return NULL;

    va_start(ap, ntypes);
    for (int i=0; i < ntypes; i++)
        allowed[i] = va_arg(ap, struct type *);
    va_end(ap);
    
    return expect_types_arr(term->info, term->type, ntypes, allowed);
}

static int check_concat(struct term *term, struct ctx *ctx) {
    struct type *tl = NULL, *tr = NULL;
    
    if (! check_exp(term->left, ctx))
        return 0;
    tl = term->left->type;

    if (tl->tag == T_ARROW) {
        /* Concatenation (composition) of functions f: a -> b and g: c -> d
           if defined as (f . g) x = g (f x) and is type correct if b <: c
           yielding a function with type a -> d */
        if (! check_exp(term->right, ctx))
            return 0;
        tr = term->right->type;
        if (tr->tag != T_ARROW)
            goto print_error;
        if (! subtype(tl->img, tr->dom))
            goto print_error;
        term->type = make_arrow_type(tl->dom, tr->img);
    } else {
        tl = require_exp_type(term->left, ctx, 
                              3, t_string, t_regexp, t_lens);
        tr = require_exp_type(term->right, ctx, 
                              3, t_string, t_regexp, t_lens);
        if ((tl == NULL) || (tr == NULL))
            return 0;
        term->type = type_join(tl, tr);
        if (term->type == NULL)
            goto print_error;
    }
    return 1;
 print_error:
    type_error2(term->info, 
                "concatenation of %s and %s is not possible",
                term->left->type, term->right->type);
    return 0;
}

/* Return 1 if TERM passes, 0 otherwise */
static int check_exp(struct term *term, struct ctx *ctx) {
    int result = 1;
    switch (term->tag) {
    case A_UNION:
        {
            struct type *tl = require_exp_type(term->left, ctx, 
                                               2, t_regexp, t_lens);
            struct type *tr = require_exp_type(term->right, ctx, 
                                               2, t_regexp, t_lens);
            result = (tl != NULL) && (tr != NULL);
            if (result) {
                term->type = type_join(tl, tr);
                if (term->type == NULL) {
                    type_error2(term->info, 
                                "union of %s and %s is not possible",
                                term->left->type, term->right->type);
                    result = 0;
                }
            }
        }
        break;
    case A_CONCAT:
        result = check_concat(term, ctx);
        break;
    case A_APP:
        result = check_exp(term->left, ctx) & check_exp(term->right, ctx);
        if (result) {
            if (term->left->type->tag != T_ARROW) {
                type_error1(term->info, 
                            "expected function in application but found %s", 
                            term->left->type);
                result = 0;
            };
        }
        if (result) {
            result = expect_types(term->info,
                                  term->right->type,
                                  1, term->left->type->dom) != NULL;
            if (! result) {
                type_error2(term->info, 
                            "application of %s to %s is not possible",
                            term->left->type, term->right->type);
                result = 0;
                
            }
        }
        if (result)
            term->type = ref(term->left->type->img);
        break;
    case A_VALUE:
        /* Type filled in by the parser */
        break;
    case A_IDENT:
        {
            struct type *t = ctx_lookup_type(ctx, term->ident);
            if (t == NULL) {
                syntax_error(term->info, "Undefined variable %s",
                             term->ident->str);
                result = 0;
            } else {
                term->type = ref(t);
            }
        }
        break;
    case A_BRACKET:
        result = check_exp(term->brexp, ctx);
        if (result) {
            term->type = ref(expect_types(term->info, term->brexp->type,
                                          1, t_lens));
            if (term->type == NULL) {
                type_error1(term->info, 
                             "[..] is only defined for lenses, not for %s", 
                            term->brexp->type);
                result = 0;
            }
        }
        break;
    case A_FUNC:
        {
            bind_param(&ctx->local, term->param, NULL);
            result = check_exp(term->body, ctx);
            if (result) {
                term->type =
                    make_arrow_type(term->param->type, term->body->type);
            }
            unbind_param(&ctx->local, term->param);
        }
        break;
    case A_REP:
        result = check_exp(term->exp, ctx);
        if (result) {
            term->type = ref(expect_types(term->info, term->exp->type, 2,
                                          t_regexp, t_lens));
            if (term->type == NULL) {
                type_error1(term->info, 
                            "Incompatible types: repetition is only defined"
                            " for regexp and lens, not for %s", 
                            term->exp->type);
                result = 0;
            }
        }
        break;
    default:
        assert(0);
        break;
    }
    assert(!result || term->type != NULL);
    return result;
}

static int check_decl(struct term *term, struct ctx *ctx) {
    assert(term->tag == A_BIND || term->tag == A_TEST);

    if (term->tag == A_BIND) {
        if (!check_exp(term->exp, ctx))
            return 0;
        term->type = ref(term->exp->type);

        if (bnd_lookup(ctx->local, term->bname) != NULL) {
            syntax_error(term->info,
                         "the name %s is already defined", term->bname);
            return 0;
        }
        bind_type(&ctx->local, term->bname, term->type);
    } else if (term->tag == A_TEST) {
        if (!check_exp(term->test, ctx))
            return 0;
        if (term->result != NULL) {
            if (!check_exp(term->result, ctx))
                return 0;
            if (! type_equal(term->test->type, term->result->type)) {
                type_error2(term->info, 
                            "expected test result of type %s but got %s",
                            term->result->type, term->test->type);
                return 0;
            }
        } else {
            if (expect_types(term->info, term->test->type, 2,
                             t_string, t_tree) == NULL)
                return 0;
        }
        term->type = ref(term->test->type);
    } else {
        assert(0);
    }
    return 1;
}

int typecheck(struct term *term, struct env *global) {
    int ok = 1;
    struct ctx ctx;
    assert(term->tag == A_MODULE);

    ctx.global = global;
    ctx.local = NULL;
    list_for_each(dcl, term->decls) {
        ok &= check_decl(dcl, &ctx);
    }
    list_unref(ctx.local, binding);
    return ok;
}

static struct value *compile_exp(struct info *, struct term *, struct ctx *);

static struct value *compile_union(struct term *exp, struct ctx *ctx) {
    struct value *v1 = compile_exp(exp->info, exp->left, ctx);
    struct value *v2 = compile_exp(exp->info, exp->right, ctx);
    if (v1 == NULL || v2 == NULL) {
        // FIXME: unref v1/v2 ?? Figure out convention for compile
        return NULL;
    }
    struct type *t = exp->type;
    struct info *info = exp->info;
    struct value *v;

    v1 = coerce(v1, t);
    v2 = coerce(v2, t);
    if (t->tag == T_REGEXP) {
        const char *p1 = v1->regexp->pattern->str;
        const char *p2 = v2->regexp->pattern->str;
        v = make_value(V_REGEXP, ref(info));
        make_ref(v->regexp);
        v->regexp->info = ref(info);
        make_ref(v->regexp->pattern);
        CALLOC(v->regexp->pattern->str, strlen(p1) + strlen(p2) + 6);
        char *s = (char *) v->regexp->pattern->str;
        strcpy(s, "(");
        strcat(s, p1);
        strcat(s, ")|(");
        strcat(s, p2);
        strcat(s, ")");
    } else if (t->tag == T_LENS) {
        struct lens *l1 = v1->lens;
        struct lens *l2 = v2->lens;
        v = make_value(V_LENS, ref(info));
        v->lens = lns_make_binop(L_UNION, ref(info), l1, l2);
    } else {
        fatal_error(info, "Tried to union a %s and a %s to yield a %s",
                    type_name(exp->left->type), type_name(exp->right->type),
                    type_name(t));
    }
    unref(t, type);
    unref(v1, value);
    unref(v2, value);
    return v;
}

static struct value *compile_concat(struct term *exp, struct ctx *ctx) {
    struct value *v1 = compile_exp(exp->info, exp->left, ctx);
    struct value *v2 = compile_exp(exp->info, exp->right, ctx);
    if (v1 == NULL || v2 == NULL) {
        // FIXME: unref v1/v2 ?? Figure out convention for compile
        return NULL;
    }
    struct type *t = exp->type;
    struct info *info = exp->info;
    struct value *v;

    v1 = coerce(v1, t);
    v2 = coerce(v2, t);
    if (t->tag == T_STRING) {
        const char *s1 = v1->string->str;
        const char *s2 = v2->string->str;
        v = make_value(V_STRING, ref(info));
        make_ref(v->string);
        CALLOC(v->string->str, strlen(s1) + strlen(s2) + 1);
        char *s = (char *) v->string->str;
        strcpy(s, s1);
        strcat(s, s2);
    } else if (t->tag == T_REGEXP) {
        const char *p1 = v1->regexp->pattern->str;
        const char *p2 = v2->regexp->pattern->str;
        v = make_value(V_REGEXP, ref(info));
        make_ref(v->regexp);
        v->regexp->info = ref(info);
        make_ref(v->regexp->pattern);
        CALLOC(v->regexp->pattern->str, strlen(p1) + strlen(p2) + 5);
        char *s = (char *) v->regexp->pattern->str;
        strcpy(s, "(");
        strcat(s, p1);
        strcat(s, ")(");
        strcat(s, p2);
        strcat(s, ")");
    } else if (t->tag == T_LENS) {
        struct lens *l1 = v1->lens;
        struct lens *l2 = v2->lens;
        v = make_value(V_LENS, ref(info));
        v->lens = lns_make_binop(L_CONCAT, ref(info), l1, l2);
    } else if (t->tag == T_ARROW) {
        /* Build lambda x: exp->right (exp->left x) as a closure */
        char *var = strdup("@0");
        struct term *param = make_param(var, ref(exp->left->type->dom),
                                        ref(info));
        struct term *ident = make_term(A_IDENT, ref(info));
        ident->ident = ref(param->param->name);
        struct term *app = make_app_term(ref(exp->left), ident, ref(info));
        app = make_app_term(ref(exp->right), app, ref(info));
        struct term *func = build_func(param, app);

        if (! check_exp(func, ctx))
            fatal_error(info, "Func for concat failed typecheck");
        assert(type_equal(func->type, exp->type));
        v = make_closure(func, ctx->local);
    } else {
        fatal_error(info, "Tried to concat a %s and a %s to yield a %s",
                    type_name(exp->left->type), type_name(exp->right->type),
                    type_name(t));
    }
    unref(t, type);
    unref(v1, value);
    unref(v2, value);
    return v;
}

static struct value *apply(struct term *app, struct ctx *ctx) {
    struct value *f = compile_exp(app->info, app->left, ctx);
    struct value *arg = compile_exp(app->info, app->right, ctx);

    assert(f->tag == V_CLOS);
    struct value *result;
    struct ctx lctx;

    lctx.global = ctx->global;
    lctx.local = ref(f->bindings);

    arg = coerce(arg, f->func->param->type);
    if (arg == NULL) {
        unref(lctx.local, binding);
        return NULL;
    }

    bind_param(&lctx.local, f->func->param, arg);
    result = compile_exp(app->info, f->func->body, &lctx);
    unbind_param(&lctx.local, f->func->param);

    unref(lctx.local, binding);

    return result;
}

static struct value *compile_bracket(struct term *exp, struct ctx *ctx) {
    struct value *arg = compile_exp(exp->info, exp->brexp, ctx);
    if (arg == NULL)
        return NULL;
    struct value *v = make_value(V_LENS, ref(exp->info));
    assert(arg->tag == V_LENS);
    v->lens = lns_make_unop(L_SUBTREE, ref(exp->info), ref(arg->lens));
    return v;
}

static struct value *compile_rep(struct term *rep, struct ctx *ctx) {
    struct value *arg = compile_exp(rep->info, rep->rexp, ctx);
    struct value *v;
    
    if (arg == NULL)
        return NULL;
    arg = coerce(arg, rep->type);
    if (rep->type->tag == T_REGEXP) {
        v = make_value(V_REGEXP, ref(rep->info));
        make_ref(v->regexp);
        CALLOC(v->regexp->pattern->str, 
               strlen(arg->regexp->pattern->str) + 4);
        char repchar;
        if (rep->quant == Q_STAR) {
            repchar = '*';
        } else if (rep->quant == Q_PLUS) {
            repchar = '+';
        } else if (rep->quant == Q_MAYBE) {
            repchar = '?';
        } else {
            assert(0);
        }
        sprintf((char *) v->regexp->pattern->str, "(%s)%c",
                arg->regexp->pattern->str, repchar);
    } else if (rep->type->tag == T_LENS) {
        v = make_value(V_LENS, ref(rep->info));
        enum lens_tag ltag;
        if (rep->quant == Q_STAR) {
            ltag = L_STAR;
        } else if (rep->quant == Q_PLUS) {
            ltag = L_PLUS;
        } else if (rep->quant == Q_MAYBE) {
            ltag = L_MAYBE;
        } else {
            assert(0);
        }
        v->lens = lns_make_unop(ltag, ref(rep->info), ref(arg->lens));
    } else {
        fatal_error(rep->info, "Tried to repeat a %s to yield a %s",
                    type_name(rep->rexp->type), type_name(rep->type));
    }
    return v;
}

static struct value *compile_exp(struct info *info,
                                 struct term *exp, struct ctx *ctx) {
    struct value *v = NULL;

    switch (exp->tag) {
    case A_UNION:
        v = compile_union(exp, ctx);
        break;
    case A_CONCAT:
        v = compile_concat(exp, ctx);
        break;
    case A_APP:
        v = apply(exp, ctx);
        break;
    case A_VALUE:
        if (exp->value->tag == V_NATIVE) {
            v = native_call(info, exp->value->native, ctx);
        } else {
            v = exp->value;
        }
        break;
    case A_IDENT:
        v = ctx_lookup(ctx, exp->ident);
        break;
    case A_BRACKET:
        v = compile_bracket(exp, ctx);
        break;
    case A_FUNC:
        v = make_closure(exp, ctx->local);
        break;
    case A_REP:
        v = compile_rep(exp, ctx);
        break;
    default:
        assert(0);
        break;
    }

    return v;
}

static int compile_decl(struct term *term, struct ctx *ctx) {
    if (term->tag == A_BIND) {
        struct value *v = compile_exp(term->info, term->exp, ctx);
        if (v != NULL)
            bind(&ctx->local, term->bname, term->type, v);
        else {
            syntax_error(term->info, "Failed to compile %s",
                         term->bname);
        }
        return v != NULL;
    } else if (term->tag == A_TEST) {
        struct value *test = compile_exp(term->info, term->test, ctx);
        if (term->result != NULL) { 
            struct value *result = compile_exp(term->info, term->result, ctx);
            if (result != test)
                FIXME("Verify that test and result are equal");
            return 1;
        } else {
            FIXME("Print test");
            return 1;
        }
    } else {
        assert(0);
    }
}

struct env *compile(struct term *term, struct env *global) {
    struct ctx ctx;
    int ok = 1;
    assert(term->tag == A_MODULE);

    ctx.global = global;
    ctx.local = NULL;
    list_for_each(dcl, term->decls) {
        ok &= compile_decl(dcl, &ctx);
    }
    if (!ok) {
        unref(ctx.local, binding);
        return NULL;
    }

    struct env *env = env_create(term->mname);
    env->bindings = ctx.local;
    return env;
}

/*
 * Defining native functions
 */
static struct info *make_native_info(const char *fname, int line) {
    struct info *info;
    make_ref(info);
    info->first_line = info->last_line = line;
    info->first_column = info->last_column = 0;
    make_ref(info->filename);
    info->filename->str = strdup(fname);
    return info;
}

void define_native_intl(const char *file, int line,
                        struct env *env, const char *name,
                        int argc, void *impl, ...) {
    assert(argc > 0);  /* We have no unit type */
    assert(argc <= 5);
    va_list ap;
    enum type_tag tag;
    struct term *params = NULL, *body;
    struct type *type;
    struct value *v;
    struct info *info = make_native_info(file, line);

    va_start(ap, impl);
    for (int i=0; i < argc; i++) {
        struct term *pterm;
        char ident[10];
        tag = va_arg(ap, enum type_tag);
        type = make_base_type(tag);
        snprintf(ident, 10, "@%d", i);
        pterm = make_param(strdup(ident), type, ref(info));
        list_append(params, pterm);
    }
    tag = va_arg(ap, enum type_tag);
    va_end(ap);

    type = make_base_type(tag);

    make_ref(v);
    v->tag = V_NATIVE;
    v->info = info;

    CALLOC(v->native, 1);
    v->native->argc = argc;
    v->native->type = type;
    v->native->impl = impl;

    make_ref(body);
    body->info = ref(info);
    body->type = ref(type);
    body->tag = A_VALUE;
    body->value = v;

    struct term *func = build_func(params, body);
    struct ctx ctx;
    ctx.global = NULL;
    ctx.local = ref(env->bindings);
    if (! check_exp(func, &ctx)) {
        fatal_error(info, "Typechecking native %s failed",
                    name);
        abort();
    }
    v = make_closure(func, ctx.local);
    bind(&ctx.local, name, func->type, v);

    env->bindings = ctx.local;
}


/* Defined in parser.y */
int augl_parse_file(const char *name, struct term **term);

/* Scaffold for testing */
static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: syntax FILE\n");
        return 1;
    }

    int r;
    struct term *term;

    r = augl_parse_file(argv[1], &term);
    if (r == -1)
        die("Parse error");

    struct env *env = builtin_init();
    dump_env(env);
    if (! typecheck(term, env))
        die("Typecheck error");

    env = compile(term, env);
    if (env == NULL)
        die("Compile error");

    fprintf(stderr, "Compile succeeded\n");
    dump_env(env);
    unref(env, env);
    /*
      Eval ??
    */
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
