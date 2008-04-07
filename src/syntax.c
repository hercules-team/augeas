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
#include <ctype.h>
#include <glob.h>
#include <argz.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "syntax.h"
#include "config.h"

/* Extension of source files */
#define AUG_EXT ".aug"

static const char *const builtin_module = "Builtin";

static const struct type string_type    = { .ref = UINT_MAX, .tag = T_STRING };
static const struct type regexp_type    = { .ref = UINT_MAX, .tag = T_REGEXP };
static const struct type lens_type      = { .ref = UINT_MAX, .tag = T_LENS };
static const struct type tree_type      = { .ref = UINT_MAX, .tag = T_TREE };
static const struct type filter_type    = { .ref = UINT_MAX, .tag = T_FILTER };
static const struct type transform_type =
                                       { .ref = UINT_MAX, .tag = T_TRANSFORM };

const struct type *const t_string    = &string_type;
const struct type *const t_regexp    = &regexp_type;
const struct type *const t_lens      = &lens_type;
const struct type *const t_tree      = &tree_type;
const struct type *const t_filter    = &filter_type;
const struct type *const t_transfrom = &transform_type;

static const char *const type_names[] = {
    "string", "regexp", "lens", "tree", "filter",
    "transform", "function", NULL
};

/* The evaluation context with all loaded modules and the bindings for the
 * module we are working on in LOCAL 
 */
struct ctx {
    struct augeas  *augeas;
    struct binding *local;
};

char *format_info(struct info *info) {
    const char *fname;
    char *result;
    int fl = info->first_line, ll = info->last_line;
    int fc = info->first_column, lc = info->last_column;
    fname = (info->filename != NULL) ? info->filename->str : "(unknown file)";

    if (fl > 0) {
        if (fl == ll) {
            if (fc == lc) {
                asprintf(&result, "%s:%d.%d", fname, fl, fc);
            } else {
                asprintf(&result, "%s:%d.%d-.%d", fname, fl, fc, lc);
            }
        } else {
            asprintf(&result, "%s:%d.%d-%d.%d", fname, fl, fc, ll, lc);
        }
    }
    return result;
}

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

void assert_error_at(const char *srcfile, int srclineno, struct info *info,
                     const char *format, ...) {
    va_list ap;

    fprintf(stderr, "%s:%d:(", srcfile, srclineno);
    print_info(stderr, info);
    fprintf(stderr,"):Internal error:"); 
	va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void free_string(struct string *string) {
    if (string == NULL)
        return;
    assert(string->ref == 0);
    free((char *) string->str);
    free(string);
}

void free_info(struct info *info) {
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

static void free_term(struct term *term) {
    if (term == NULL)
        return;
    assert(term->ref == 0);
    switch(term->tag) {
    case A_MODULE:
        free((char *) term->mname);
        unref(term->decls, term);
        break;
    case A_BIND:
        free((char *) term->bname);
        unref(term->exp, term);
        break;
    case A_COMPOSE:
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
    case A_TEST:
        unref(term->test, term);
        unref(term->result, term);
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
    unref(binding->next, binding);
    unref(binding->ident, string);
    unref(binding->type, type);
    unref(binding->value, value);
    free(binding);
}

void free_module(struct module *module) {
    if (module == NULL)
        return;
    assert(module->ref == 0);
    free((char *) module->name);
    unref(module->next, module);
    unref(module->bindings, binding);
    free(module);
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

void free_value(struct value *v) {
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
    case V_TREE:
        free_tree(v->tree);
        break;
    case V_FILTER:
        FIXME("Free filter");
        break;
    case V_TRANSFORM:
        FIXME("Free transform");
        break;
    case V_NATIVE:
        unref(v->native->type, type);
        free(v->native);
        break;
    case V_CLOS:
        unref(v->func, term);
        unref(v->bindings, binding);
        break;
    case V_EXN:
        unref(v->exn->info, info);
        free((char *) v->exn->message);
        for (int i=0; i < v->exn->nlines; i++) {
            free(v->exn->lines[i]);
        }
        free(v->exn->lines);
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

struct value *make_exn_value(struct info *info,
                             const char *format, ...) {
    va_list ap;
    struct value *v;
    char *message;

    va_start(ap, format);
    vasprintf(&message, format, ap);
    va_end(ap);

    v = make_value(V_EXN, ref(info));
    CALLOC(v->exn, 1);
    v->exn->info = info;
    v->exn->message = message;

    return v;
}

void exn_add_lines(struct value *v, int nlines, ...) {
    assert(v->tag == V_EXN);

    va_list ap;
    REALLOC(v->exn->lines, v->exn->nlines + nlines);
    va_start(ap, nlines);
    for (int i=0; i < nlines; i++) {
        char *line = va_arg(ap, char *);
        v->exn->lines[v->exn->nlines + i] = line;
    }
    va_end(ap);
    v->exn->nlines += nlines;
}

void exn_printf_line(struct value *exn, const char *format, ...) {
    va_list ap;
    char *line;

    va_start(ap, format);
    vasprintf(&line, format, ap);
    va_end(ap);

    exn_add_lines(exn, 1, line);
}

/*
 * Modules
 */
static int load_module(struct augeas *aug, const char *name);
static char *module_basename(const char *modname);

struct module *module_create(const char *name) {
    struct module *module;
    make_ref(module);
    module->name = strdup(name);
    return module;
}

static struct module *module_find(struct module *module, const char *name) {
    list_for_each(e, module) {
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

static struct binding *ctx_lookup_bnd(struct info *info,
                                      struct ctx *ctx, const char *name) {
    struct binding *b = bnd_lookup(ctx->local, name);
    if (b != NULL)
        return b;

    if (ctx->augeas != NULL) {
        char *dot = strchr(name, '.');
        if (dot != NULL) {
        qual_lookup:
            list_for_each(module, ctx->augeas->modules) {
                if (STREQLEN(module->name, name, strlen(module->name))
                    && dot - name == strlen(module->name))
                    return bnd_lookup(module->bindings, dot + 1);
            }
            /* Try to load the module */
            char *modname = strndup(name, dot - name);
            int loaded = load_module(ctx->augeas, modname) == 0;
            if (loaded) {
                free(modname);
                goto qual_lookup;
            } else {
                syntax_error(info, "Could not load module %s for %s",
                             modname, name);
                free(modname);
            }
        } else {
            struct module *builtin = 
                module_find(ctx->augeas->modules, builtin_module);
            assert(builtin != NULL);
            return bnd_lookup(builtin->bindings, name);
        }
    }
    return NULL;
}

static struct value *ctx_lookup(struct info *info,
                                struct ctx *ctx, struct string *ident) {
    struct binding *b = ctx_lookup_bnd(info, ctx, ident->str);
    return b == NULL ? NULL : b->value;
}

static struct type *ctx_lookup_type(struct info *info,
                                    struct ctx *ctx, struct string *ident) {
    struct binding *b = ctx_lookup_bnd(info, ctx, ident->str);
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
    ref(*bnds);
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
    ref(*bnds);
    list_cons(*bnds, b);
}

static void unbind_param(struct binding **bnds, struct param *param) {
    struct binding *b = *bnds;
    assert(b->ident == param->name);
    assert(b->next != *bnds);
    *bnds = b->next;
    unref(b, binding);
}

/* Takes ownership of TYPE and VALUE */
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

static void dump_module(struct module *module) {
    if (module == NULL)
        return;
    fprintf(stderr, "Module %s\n:", module->name);
    dump_bindings(module->bindings);
    dump_module(module->next);
}

ATTRIBUTE_UNUSED
static void dump_ctx(struct ctx *ctx) {
    fprintf(stderr, "Context:\n");
    dump_bindings(ctx->local);
    if (ctx->augeas != NULL) {
        list_for_each(m, ctx->augeas->modules)
            dump_module(m);
    }
}

/*
 * Values
 */
static void print_value(struct value *v) {
    if (v == NULL) {
        printf("<null>");
        return;
    }

    switch(v->tag) {
    case V_STRING:
        printf("\"%s\"", v->string->str);
        break;
    case V_REGEXP:
        printf("/%s/", v->regexp->pattern->str);
        break;
    case V_LENS:
        printf("<lens:");
        print_info(stdout, v->lens->info);
        printf(">");
        break;
    case V_TREE:
        list_for_each(t, v->tree) {
            print_tree(t, stdout, (t->children == NULL) ? NULL : t->label, 1);
        }
        break;
    case V_FILTER:
        printf("<filter:");
        list_for_each(f, v->filter) {
            printf("%c%s%c", f->include ? '+' : '-', f->glob,
                   (f->next != NULL) ? ':' : '>');
        }
        break;
    case V_TRANSFORM:
        printf("<transform:");
        print_info(stdout, v->transform->lens->info);
        printf(">");
        break;
    case V_NATIVE:
        printf("<native:");
        print_info(stdout, v->info);
        printf(">");
        break;
    case V_CLOS:
        printf("<closure:");
        print_info(stdout, v->func->info);
        printf(">");
        break;
    case V_EXN:
        print_info(stdout, v->exn->info);
        printf("exception: %s\n", v->exn->message);
        for (int i=0; i < v->exn->nlines; i++) {
            printf("    %s\n", v->exn->lines[i]);
        }
        break;
    default:
        assert(0);
        break;
    }
}

static int value_equal(struct value *v1, struct value *v2) {
    if (v1 == NULL && v2 == NULL)
        return 1;
    if (v1 == NULL || v2 == NULL)
        return 0;
    if (v1->tag != v2->tag)
        return 0;
    switch (v1->tag) {
    case V_STRING:
        return STREQ(v1->string->str, v2->string->str);
        break;
    case V_REGEXP:
        // FIXME: Should probably build FA's and compare them
        return STREQ(v1->regexp->pattern->str, v2->regexp->pattern->str);
        break;
    case V_LENS:
        return v1->lens == v2->lens;
        break;
    case V_TREE:
        return tree_equal(v1->tree, v2->tree);
        break;
    case V_FILTER:
        return v1->filter == v2->filter;
        break;
    case V_TRANSFORM:
        return v1->transform == v2->transform;
        break;
    case V_NATIVE:
        return v1->native == v2->native;
        break;
    case V_CLOS:
        return v1->func == v2->func && v1->bindings == v2->bindings;
        break;
    default:
        assert(0);
        break;
    }
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
        if (t2->tag == T_STRING || t2->tag == T_REGEXP)
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
    case V_FILTER:
        return make_base_type(T_FILTER);
    case V_TRANSFORM:
        return make_base_type(T_TRANSFORM);
    case V_NATIVE:
        return ref(v->native->type);
    case V_CLOS:
        return ref(v->func->type);
    case V_EXN:   /* Fail on exceptions */
    default:
        assert(0);
    }
}

/* Coerce V to the type T. Currently, only T_STRING can be coerced to
 * T_REGEXP. Returns a value that is owned by the caller. Trying to perform
 * an impossible coercion is a fatal error. Receives ownership of V.
 */
static struct value *coerce(struct value *v, struct type *t) {
    struct type *vt = value_type(v);
    if (type_equal(vt, t)) {
        unref(vt, type);
        return v;
    }
    if (vt->tag == T_STRING && t->tag == T_REGEXP) {
        struct value *rxp = make_value(V_REGEXP, ref(v->info));
        rxp->regexp = make_regexp_literal(v->info, v->string->str);
        unref(v, value);
        unref(vt, type);
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

static int check_compose(struct term *term, struct ctx *ctx) {
    struct type *tl = NULL, *tr = NULL;

    if (! check_exp(term->left, ctx))
        return 0;
    tl = term->left->type;

    if (tl->tag == T_ARROW) {
        /* Composition of functions f: a -> b and g: c -> d is defined as
           (f . g) x = g (f x) and is type correct if b <: c yielding a
           function with type a -> d */
        if (! check_exp(term->right, ctx))
            return 0;
        tr = term->right->type;
        if (tr->tag != T_ARROW)
            goto print_error;
        if (! subtype(tl->img, tr->dom))
            goto print_error;
        term->type = make_arrow_type(tl->dom, tr->img);
    } else {
        goto print_error;
    }
    return 1;
 print_error:
    type_error2(term->info,
                "composition of %s and %s is not possible",
                term->left->type, term->right->type);
    return 0;
}

static int check_concat(struct term *term, struct ctx *ctx) {
    struct type *tl = NULL, *tr = NULL;

    if (! check_exp(term->left, ctx))
        return 0;
    tl = term->left->type;

    tl = require_exp_type(term->left, ctx, 
                          3, t_string, t_regexp, t_lens);
    tr = require_exp_type(term->right, ctx, 
                          3, t_string, t_regexp, t_lens);
    if ((tl == NULL) || (tr == NULL))
        return 0;
    term->type = type_join(tl, tr);
    if (term->type == NULL)
        goto print_error;
    return 1;
 print_error:
    type_error2(term->info,
                "concatenation of %s and %s is not possible",
                term->left->type, term->right->type);
    return 0;
}

static int check_value(struct value *v) {
    if (v->tag == V_REGEXP) {
        if (regexp_compile(v->regexp) == -1)
            return 0;
    }
    return 1;
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
    case A_COMPOSE:
        result = check_compose(term, ctx);
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
        result = check_value(term->value);
        break;
    case A_IDENT:
        {
            struct type *t = ctx_lookup_type(term->info, ctx, term->ident);
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

static int typecheck(struct term *term, struct augeas *augeas) {
    int ok = 1;
    struct ctx ctx;
    char *fname;
    const char *basenam;

    assert(term->tag == A_MODULE);

    /* Check that the module name is consistent with the filename */
    fname = module_basename(term->mname);

    basenam = strrchr(term->info->filename->str, SEP);
    if (basenam == NULL)
        basenam = term->info->filename->str;
    else
        basenam += 1;
    if (STRNEQ(fname, basenam)) {
        syntax_error(term->info, 
                     "The module %s must be in a file named %s",
                     term->mname, fname);
        free(fname);
        return 0;
    }
    free(fname);

    ctx.augeas = augeas;
    ctx.local = NULL;
    list_for_each(dcl, term->decls) {
        ok &= check_decl(dcl, &ctx);
    }
    unref(ctx.local, binding);
    return ok;
}

static struct value *compile_exp(struct info *, struct term *, struct ctx *);

static struct value *compile_union(struct term *exp, struct ctx *ctx) {
    struct value *v1 = compile_exp(exp->info, exp->left, ctx);
    if (EXN(v1))
        return v1;
    struct value *v2 = compile_exp(exp->info, exp->right, ctx);
    if (EXN(v2)) {
        unref(v1, value);
        return v2;
    }

    struct type *t = exp->type;
    struct info *info = exp->info;
    struct value *v;

    v1 = coerce(v1, t);
    v2 = coerce(v2, t);
    if (t->tag == T_REGEXP) {
        v = make_value(V_REGEXP, ref(info));
        v->regexp = regexp_union(info, v1->regexp, v2->regexp);
    } else if (t->tag == T_LENS) {
        struct lens *l1 = v1->lens;
        struct lens *l2 = v2->lens;
        v = lns_make_union(ref(info), ref(l1), ref(l2));
    } else {
        fatal_error(info, "Tried to union a %s and a %s to yield a %s",
                    type_name(exp->left->type), type_name(exp->right->type),
                    type_name(t));
    }
    unref(v1, value);
    unref(v2, value);
    return v;
}

static struct value *compile_compose(struct term *exp, struct ctx *ctx) {
    struct value *v1 = compile_exp(exp->info, exp->left, ctx);
    if (EXN(v1))
        return v1;
    struct value *v2 = compile_exp(exp->info, exp->right, ctx);
    if (EXN(v2)) {
        unref(v1, value);
        return v2;
    }

    struct type *t = exp->type;
    struct info *info = exp->info;
    struct value *v;

    v1 = coerce(v1, t);
    v2 = coerce(v2, t);
    if (t->tag == T_ARROW) {
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
        fatal_error(info, "Tried to compose a %s and a %s to yield a %s",
                    type_name(exp->left->type), type_name(exp->right->type),
                    type_name(t));
    }
    unref(v1, value);
    unref(v2, value);
    return v;
}

static struct value *compile_concat(struct term *exp, struct ctx *ctx) {
    struct value *v1 = compile_exp(exp->info, exp->left, ctx);
    if (EXN(v1))
        return v1;
    struct value *v2 = compile_exp(exp->info, exp->right, ctx);
    if (EXN(v2)) {
        unref(v1, value);
        return v2;
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
        v = make_value(V_REGEXP, ref(info));
        v->regexp = regexp_concat(info, v1->regexp, v2->regexp);
    } else if (t->tag == T_LENS) {
        struct lens *l1 = v1->lens;
        struct lens *l2 = v2->lens;
        v = lns_make_concat(ref(info), ref(l1), ref(l2));
    } else {
        fatal_error(info, "Tried to concat a %s and a %s to yield a %s",
                    type_name(exp->left->type), type_name(exp->right->type),
                    type_name(t));
    }
    unref(v1, value);
    unref(v2, value);
    return v;
}

static struct value *apply(struct term *app, struct ctx *ctx) {
    struct value *f = compile_exp(app->info, app->left, ctx);
    struct value *result = NULL;
    struct ctx lctx;

    if (EXN(f))
        return f;

    struct value *arg = compile_exp(app->info, app->right, ctx);
    if (EXN(arg)) {
        unref(f, value);
        return arg;
    }

    assert(f->tag == V_CLOS);

    lctx.augeas = ctx->augeas;
    lctx.local = ref(f->bindings);

    arg = coerce(arg, f->func->param->type);
    if (arg == NULL)
        goto done;

    bind_param(&lctx.local, f->func->param, arg);
    result = compile_exp(app->info, f->func->body, &lctx);
    unref(result->info, info);
    result->info = ref(app->info);
    unbind_param(&lctx.local, f->func->param);

 done:
    unref(lctx.local, binding);
    unref(f, value);
    return result;
}

static struct value *compile_bracket(struct term *exp, struct ctx *ctx) {
    struct value *arg = compile_exp(exp->info, exp->brexp, ctx);
    if (EXN(arg))
        return arg;
    assert(arg->tag == V_LENS);

    struct value *v = lns_make_subtree(ref(exp->info), ref(arg->lens));
    unref(arg, value);
        
    return v;
}

static struct value *compile_rep(struct term *rep, struct ctx *ctx) {
    struct value *arg = compile_exp(rep->info, rep->rexp, ctx);
    struct value *v;

    if (EXN(arg))
        return arg;

    arg = coerce(arg, rep->type);
    if (rep->type->tag == T_REGEXP) {
        int min, max;
        if (rep->quant == Q_STAR) {
            min = 0; max = -1;
        } else if (rep->quant == Q_PLUS) {
            min = 1; max = -1;
        } else if (rep->quant == Q_MAYBE) {
            min = 0; max = 1;
        } else {
            assert(0);
        }
        v = make_value(V_REGEXP, ref(rep->info));
        v->regexp = regexp_iter(rep->info, arg->regexp, min, max);
    } else if (rep->type->tag == T_LENS) {
        if (rep->quant == Q_STAR) {
            v = lns_make_star(ref(rep->info), ref(arg->lens));
        } else if (rep->quant == Q_PLUS) {
            v = lns_make_plus(ref(rep->info), ref(arg->lens));
        } else if (rep->quant == Q_MAYBE) {
            v = lns_make_maybe(ref(rep->info), ref(arg->lens));
        } else {
            assert(0);
        }
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
    case A_COMPOSE:
        v = compile_compose(exp, ctx);
        break;
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
            v = ref(exp->value);
        }
        break;
    case A_IDENT:
        v = ref(ctx_lookup(exp->info, ctx, exp->ident));
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

static int compile_test(struct term *term, struct ctx *ctx) {
    struct value *actual = compile_exp(term->info, term->test, ctx);
    struct value *expect = NULL;
    int ret = 1;

    if (term->tr_tag == TR_EXN) {
        if (!EXN(actual)) {
            printf("Test run should have produced exception, but produced\n");
            print_value(actual);
            printf("\n");
        }
    } else {
        if (EXN(actual)) {
            printf("Test run encountered exception:\n");
            print_value(actual);
            printf("\n");
            ret = 0;
        } else if (term->tr_tag == TR_CHECK) {
            expect = compile_exp(term->info, term->result, ctx);
            if (EXN(expect))
                goto done;
            if (! value_equal(actual, expect)) {
                printf("Test failure:");
                print_info(stdout, term->info);
                printf("\n");
                printf(" Expected:\n");
                print_value(expect);
                printf("\n");
                printf(" Actual:\n");
                print_value(actual);
                printf("\n");
            }
        } else {
            printf("Test result: ");
            print_info(stdout, term->info);
            printf("\n");
            print_value(actual);
            printf("\n");
        }
    }
 done:
    unref(actual, value);
    unref(expect, value);
    return ret;
}

static int compile_decl(struct term *term, struct ctx *ctx) {
    if (term->tag == A_BIND) {
        struct value *v = compile_exp(term->info, term->exp, ctx);
        bind(&ctx->local, term->bname, term->type, v);
        if (!EXN(v))
            return 1;

        syntax_error(term->info, "Failed to compile %s",
                     term->bname);
        print_value(v);
        printf("\n");
        return 0;
    } else if (term->tag == A_TEST) {
        return compile_test(term, ctx);
    } else {
        assert(0);
    }
}

static struct module *compile(struct term *term, struct augeas *augeas) {
    struct ctx ctx;
    int ok = 1;
    assert(term->tag == A_MODULE);

    ctx.augeas = augeas;
    ctx.local = NULL;
    list_for_each(dcl, term->decls) {
        ok &= compile_decl(dcl, &ctx);
    }
    if (!ok) {
        unref(ctx.local, binding);
        return NULL;
    }

    struct module *module = module_create(term->mname);
    module->bindings = ctx.local;
    return module;
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
                        struct module *module, const char *name,
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
    ctx.augeas = NULL;
    ctx.local = ref(module->bindings);
    if (! check_exp(func, &ctx)) {
        fatal_error(info, "Typechecking native %s failed",
                    name);
        abort();
    }
    v = make_closure(func, ctx.local);
    bind(&ctx.local, name, func->type, v);

    module->bindings = ctx.local;
}


/* Defined in parser.y */
int augl_parse_file(const char *name, struct term **term);

static char *module_basename(const char *modname) {
    char *fname;

    asprintf(&fname, "%s" AUG_EXT, modname);
    for (int i=0; i < strlen(modname); i++)
        fname[i] = tolower(fname[i]);
    return fname;
}

static char *module_filename(struct augeas *aug, const char *modname) {
    char *dir = NULL;
    char *filename = NULL;
    char *name = module_basename(modname);
    
    while ((dir = argz_next(aug->modpathz, aug->nmodpath, dir)) != NULL) {
        int len = strlen(name) + strlen(dir) + 2;
        struct stat st;

        REALLOC(filename, len);
        sprintf(filename, "%s/%s", dir, name);
        if (stat(filename, &st) == 0)
            goto done;
    }
    free(filename);
    filename = NULL;
 done:
    free(name);
    return filename;
}

int __aug_load_module_file(struct augeas *aug, const char *filename) {
    struct term *term = NULL;
    int result = -1;

    if (augl_parse_file(filename, &term) == -1)
        goto error;

    if (! typecheck(term, aug))
        goto error;

    struct module *module = compile(term, aug);
    if (module == NULL)
        goto error;

    list_append(aug->modules, module);
    result = 0;
 error:
    // FIXME: This leads to a bad free of a string used in a del lens
    // To reproduce run lenses/tests/test_yum.aug
    unref(term, term);
    return result;
}

static int load_module(struct augeas *aug, const char *name) {
    char *filename = NULL;

    if (module_find(aug->modules, name) != NULL)
        return 0;

    if ((filename = module_filename(aug, name)) == NULL)
        return -1;

    if (__aug_load_module_file(aug, filename) == -1)
        goto error;

    free(filename);
    return 0;

 error:
    free(filename);
    return -1;
}

int interpreter_init(struct augeas *aug) {
    int r;

    aug->modules = builtin_init();

    if (aug->flags & AUG_NO_DEFAULT_LOAD)
        return 0;

    // For now, we just load every file on the search path
    const char *dir = NULL;
    glob_t globbuf;
    int gl_flags = GLOB_NOSORT;

    while ((dir = argz_next(aug->modpathz, aug->nmodpath, dir)) != NULL) {
        char *globpat;
        asprintf(&globpat, "%s/*.aug", dir);
        r = glob(globpat, gl_flags, NULL, &globbuf);
        if (r != 0 && r != GLOB_NOMATCH)
            FIXME("glob failure for %s", globpat);
        gl_flags |= GLOB_APPEND;
        free(globpat);
    }
    
    for (int i=0; i < globbuf.gl_pathc; i++) {
        char *name, *p, *q;
        p = strrchr(globbuf.gl_pathv[i], SEP);
        if (p == NULL)
            p = globbuf.gl_pathv[i];
        else
            p += 1;
        q = strchr(p, '.');
        name = strndup(p, q - p);
        name[0] = toupper(name[0]);
        if (load_module(aug, name) == -1)
            goto error;
        free(name);
    }
    globfree(&globbuf);
    return 0;
 error:
    FIXME("Cleanup loaded modules");
    return -1;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
