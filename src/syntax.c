/*
 * syntax.c:
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

#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <glob.h>
#include <argz.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "memory.h"
#include "syntax.h"
#include "augeas.h"
#include "transform.h"
#include "errcode.h"

/* Extension of source files */
#define AUG_EXT ".aug"

#define LNS_TYPE_CHECK(ctx) ((ctx)->aug->flags & AUG_TYPE_CHECK)

static const char *const builtin_module = "Builtin";

static const struct type string_type    = { .ref = UINT_MAX, .tag = T_STRING };
static const struct type regexp_type    = { .ref = UINT_MAX, .tag = T_REGEXP };
static const struct type lens_type      = { .ref = UINT_MAX, .tag = T_LENS };
static const struct type tree_type      = { .ref = UINT_MAX, .tag = T_TREE };
static const struct type filter_type    = { .ref = UINT_MAX, .tag = T_FILTER };
static const struct type transform_type =
                                       { .ref = UINT_MAX, .tag = T_TRANSFORM };
static const struct type unit_type      = { .ref = UINT_MAX, .tag = T_UNIT };

const struct type *const t_string    = &string_type;
const struct type *const t_regexp    = &regexp_type;
const struct type *const t_lens      = &lens_type;
const struct type *const t_tree      = &tree_type;
const struct type *const t_filter    = &filter_type;
const struct type *const t_transform = &transform_type;
const struct type *const t_unit      = &unit_type;

static const char *const type_names[] = {
    "string", "regexp", "lens", "tree", "filter",
    "transform", "function", "unit", NULL
};

/* The anonymous identifier which we will never bind */
static const char const anon_ident[] = "_";

static void print_value(FILE *out, struct value *v);

/* The evaluation context with all loaded modules and the bindings for the
 * module we are working on in LOCAL
 */
struct ctx {
    const char     *name;     /* The module we are working on */
    struct augeas  *aug;
    struct binding *local;
};

static void format_error(struct info *info, aug_errcode_t code,
                         const char *format, va_list ap) {
    struct error *error = info->error;
    char *si = NULL, *sf = NULL, *sd = NULL;
    int r;

    error->code = code;
    /* Only syntax errors are cumulative */
    if (code != AUG_ESYNTAX)
        FREE(error->details);

    si = format_info(info);
    r = vasprintf(&sf, format, ap);
    if (r < 0)
        sf = NULL;
    if (error->details != NULL) {
        r = xasprintf(&sd, "%s\n%s%s", error->details,
                      (si == NULL) ? "(no location)" : si,
                      (sf == NULL) ? "(no details)" : sf);
    } else {
        r = xasprintf(&sd, "%s%s",
                      (si == NULL) ? "(no location)" : si,
                      (sf == NULL) ? "(no details)" : sf);
    }
    if (r >= 0) {
        free(error->details);
        error->details = sd;
    }
    free(si);
    free(sf);
}

void syntax_error(struct info *info, const char *format, ...) {
    struct error *error = info->error;
    va_list ap;

    if (error->code != AUG_NOERROR && error->code != AUG_ESYNTAX)
        return;

	va_start(ap, format);
    format_error(info, AUG_ESYNTAX, format, ap);
    va_end(ap);
}

void fatal_error(struct info *info, const char *format, ...) {
    struct error *error = info->error;
    va_list ap;

    if (error->code == AUG_EINTERNAL)
        return;

	va_start(ap, format);
    format_error(info, AUG_EINTERNAL, format, ap);
    va_end(ap);
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

void free_term(struct term *term) {
    if (term == NULL)
        return;
    assert(term->ref == 0);
    switch(term->tag) {
    case A_MODULE:
        free(term->mname);
        free(term->autoload);
        unref(term->decls, term);
        break;
    case A_BIND:
        free(term->bname);
        unref(term->exp, term);
        break;
    case A_COMPOSE:
    case A_UNION:
    case A_MINUS:
    case A_CONCAT:
    case A_APP:
    case A_LET:
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
    free(module->name);
    unref(module->next, module);
    unref(module->bindings, binding);
    unref(module->autoload, transform);
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

static void free_exn(struct exn *exn) {
    if (exn == NULL)
        return;

    unref(exn->info, info);
    free(exn->message);
    for (int i=0; i < exn->nlines; i++) {
        free(exn->lines[i]);
    }
    free(exn->lines);
    free(exn);
}

void free_value(struct value *v) {
    if (v == NULL)
        return;
    assert(v->ref == 0);

    switch(v->tag) {
    case V_STRING:
        unref(v->string, string);
        break;
    case V_REGEXP:
        unref(v->regexp, regexp);
        break;
    case V_LENS:
        unref(v->lens, lens);
        break;
    case V_TREE:
        free_tree(v->origin);
        break;
    case V_FILTER:
        unref(v->filter, filter);
        break;
    case V_TRANSFORM:
        unref(v->transform, transform);
        break;
    case V_NATIVE:
        if (v->native)
            unref(v->native->type, type);
        free(v->native);
        break;
    case V_CLOS:
        unref(v->func, term);
        unref(v->bindings, binding);
        break;
    case V_EXN:
        free_exn(v->exn);
        break;
    case V_UNIT:
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
  if (make_ref(term) < 0) {
      unref(info, info);
  } else {
      term->tag = tag;
      term->info = info;
  }
  return term;
}

struct term *make_param(char *name, struct type *type, struct info *info) {
  struct term *term = make_term(A_FUNC, info);
  if (term == NULL)
      goto error;
  make_ref_err(term->param);
  term->param->info = ref(term->info);
  make_ref_err(term->param->name);
  term->param->name->str = name;
  term->param->type = type;
  return term;
 error:
  unref(term, term);
  return NULL;
}

struct value *make_value(enum value_tag tag, struct info *info) {
    struct value *value = NULL;
    if (make_ref(value) < 0) {
        unref(info, info);
    } else {
        value->tag = tag;
        value->info = info;
    }
    return value;
}

struct value *make_unit(struct info *info) {
    return make_value(V_UNIT, info);
}

struct term *make_app_term(struct term *lambda, struct term *arg,
                           struct info *info) {
  struct term *app = make_term(A_APP, info);
  if (app == NULL) {
      unref(lambda, term);
      unref(arg, term);
  } else {
      app->left = lambda;
      app->right = arg;
  }
  return app;
}

struct term *make_app_ident(char *id, struct term *arg, struct info *info) {
    struct term *ident = make_term(A_IDENT, ref(info));
    ident->ident = make_string(id);
    if (ident->ident == NULL) {
        unref(arg, term);
        unref(info, info);
        unref(ident, term);
        return NULL;
    }
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
    struct value *v = NULL;
    if (make_ref(v) == 0) {
        v->tag  = V_CLOS;
        v->info = ref(func->info);
        v->func = ref(func);
        v->bindings = ref(bnds);
    }
    return v;
}

struct value *make_exn_value(struct info *info,
                             const char *format, ...) {
    va_list ap;
    int r;
    struct value *v;
    char *message;

    va_start(ap, format);
    r = vasprintf(&message, format, ap);
    va_end(ap);
    if (r == -1)
        return NULL;

    v = make_value(V_EXN, ref(info));
    CALLOC(v->exn, 1);
    v->exn->info = info;
    v->exn->message = message;

    return v;
}

void exn_add_lines(struct value *v, int nlines, ...) {
    assert(v->tag == V_EXN);

    va_list ap;
    if (REALLOC_N(v->exn->lines, v->exn->nlines + nlines) == -1)
        return;
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
    int r;
    char *line;

    va_start(ap, format);
    r = vasprintf(&line, format, ap);
    va_end(ap);
    if (r >= 0)
        exn_add_lines(exn, 1, line);
}

struct value *exn_error(void) {
    static const struct exn exn = {
        .info = NULL, .seen = 1, .error = 1,
        .message = (char *) "Error during evaluation",
        .nlines = 0, .lines = NULL };
    static const struct value value = {
        .ref = REF_MAX, /* Protect against being freed */
        .info = NULL, .tag = V_EXN,
        { .exn = (struct exn *) &exn } };
    return (struct value *) &value;
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
        if (STRCASEEQ(e->name, name))
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

static char *modname_of_qname(const char *qname) {
    char *dot = strchr(qname, '.');
    if (dot == NULL)
        return NULL;

    return strndup(qname, dot - qname);
}

static int lookup_internal(struct augeas *aug, const char *ctx_modname,
                           const char *name, struct binding **bnd) {
    char *modname = modname_of_qname(name);

    *bnd = NULL;

    if (modname == NULL) {
        struct module *builtin =
            module_find(aug->modules, builtin_module);
        assert(builtin != NULL);
        *bnd = bnd_lookup(builtin->bindings, name);
        return 0;
    }

 qual_lookup:
    list_for_each(module, aug->modules) {
        if (STRCASEEQ(module->name, modname)) {
            *bnd = bnd_lookup(module->bindings, name + strlen(modname) + 1);
            free(modname);
            return 0;
        }
    }
    /* Try to load the module */
    if (streqv(modname, ctx_modname)) {
        free(modname);
        return 0;
    }
    int loaded = load_module(aug, modname) == 0;
    if (loaded)
        goto qual_lookup;

    free(modname);
    return -1;
}

struct lens *lens_lookup(struct augeas *aug, const char *qname) {
    struct binding *bnd = NULL;

    if (lookup_internal(aug, NULL, qname, &bnd) < 0)
        return NULL;
    if (bnd == NULL || bnd->value->tag != V_LENS)
        return NULL;
    return bnd->value->lens;
}

static struct binding *ctx_lookup_bnd(struct info *info,
                                      struct ctx *ctx, const char *name) {
    struct binding *b = NULL;
    int nlen = strlen(ctx->name);

    if (STREQLEN(ctx->name, name, nlen) && name[nlen] == '.')
        name += nlen + 1;

    b = bnd_lookup(ctx->local, name);
    if (b != NULL)
        return b;

    if (ctx->aug != NULL) {
        int r;
        r = lookup_internal(ctx->aug, ctx->name, name, &b);
        if (r == 0)
            return b;
        char *modname = modname_of_qname(name);
        syntax_error(info, "Could not load module %s for %s",
                     modname, name);
        free(modname);
        return NULL;
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

    if (STREQ(name, anon_ident))
        return NULL;
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
    ref(*bnds);
    list_cons(*bnds, b);
}

static void unbind_param(struct binding **bnds, ATTRIBUTE_UNUSED struct param *param) {
    struct binding *b = *bnds;
    assert(b->ident == param->name);
    assert(b->next != *bnds);
    *bnds = b->next;
    unref(b, binding);
}

/* Takes ownership of VALUE */
static void bind(struct binding **bnds,
                 const char *name, struct type *type, struct value *value) {
    struct binding *b = NULL;

    if (STRNEQ(name, anon_ident)) {
        b = bind_type(bnds, name, type);
        b->value = ref(value);
    }
}

/*
 * Some debug printing
 */

static char *type_string(struct type *t);

static void dump_bindings(struct binding *bnds) {
    list_for_each(b, bnds) {
        char *st = type_string(b->type);
        fprintf(stderr, "    %s: %s", b->ident->str, st);
        fprintf(stderr, " = ");
        print_value(stderr, b->value);
        fputc('\n', stderr);
        free(st);
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
    fprintf(stderr, "Context: %s\n", ctx->name);
    dump_bindings(ctx->local);
    if (ctx->aug != NULL) {
        list_for_each(m, ctx->aug->modules)
            dump_module(m);
    }
}

/*
 * Values
 */
static void print_tree(FILE *out, int indent, struct tree *tree) {
    if (tree == NULL) {
        fprintf(out, "(null tree)\n");
        return;
    }
    list_for_each(t, tree) {
        for (int i=0; i < indent; i++) fputc(' ', out);
        fprintf(out, "{ ");
        if (t->label != NULL)
            fprintf(out, "\"%s\"", t->label);
        if (t->value != NULL)
            fprintf(out, " = \"%s\"", t->value);
        if (t->children != NULL) {
            fputc('\n', out);
            print_tree(out, indent + 2, t->children);
            for (int i=0; i < indent; i++) fputc(' ', out);
        } else {
            fputc(' ', out);
        }
        fprintf(out, "}\n");
    }
}

static void print_value(FILE *out, struct value *v) {
    if (v == NULL) {
        fprintf(out, "<null>");
        return;
    }

    switch(v->tag) {
    case V_STRING:
        fprintf(out, "\"%s\"", v->string->str);
        break;
    case V_REGEXP:
        fprintf(out, "/%s/", v->regexp->pattern->str);
        break;
    case V_LENS:
        fprintf(out, "<lens:");
        print_info(out, v->lens->info);
        fprintf(out, ">");
        break;
    case V_TREE:
        print_tree(out, 0, v->origin);
        break;
    case V_FILTER:
        fprintf(out, "<filter:");
        list_for_each(f, v->filter) {
            fprintf(out, "%c%s%c", f->include ? '+' : '-', f->glob->str,
                   (f->next != NULL) ? ':' : '>');
        }
        break;
    case V_TRANSFORM:
        fprintf(out, "<transform:");
        print_info(out, v->transform->lens->info);
        fprintf(out, ">");
        break;
    case V_NATIVE:
        fprintf(out, "<native:");
        print_info(out, v->info);
        fprintf(out, ">");
        break;
    case V_CLOS:
        fprintf(out, "<closure:");
        print_info(out, v->func->info);
        fprintf(out, ">");
        break;
    case V_EXN:
        if (! v->exn->seen) {
            print_info(out, v->exn->info);
            fprintf(out, "exception: %s\n", v->exn->message);
            for (int i=0; i < v->exn->nlines; i++) {
                fprintf(out, "    %s\n", v->exn->lines[i]);
            }
            v->exn->seen = 1;
        }
        break;
    case V_UNIT:
        fprintf(out, "()");
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
        return tree_equal(v1->origin->children, v2->origin->children);
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
        abort();
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
    else if (tag == T_FILTER)
        return (struct type *) t_filter;
    else if (tag == T_TRANSFORM)
        return (struct type *) t_transform;
    else if (tag == T_UNIT)
        return (struct type *) t_unit;
    assert(0);
    abort();
}

static const char *type_name(struct type *t) {
    for (int i = 0; type_names[i] != NULL; i++)
        if (i == t->tag)
            return type_names[i];
    assert(0);
    abort();
}

static char *type_string(struct type *t) {
    if (t->tag == T_ARROW) {
        char *s = NULL;
        int r;
        char *sd = type_string(t->dom);
        char *si = type_string(t->img);
        if (t->dom->tag == T_ARROW)
            r = asprintf(&s, "(%s) -> %s", sd, si);
        else
            r = asprintf(&s, "%s -> %s", sd, si);
        free(sd);
        free(si);
        return (r == -1) ? NULL : s;
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
    if (t1->tag == T_ARROW && t2->tag == T_ARROW) {
        return subtype(t2->dom, t1->dom)
            && subtype(t1->img, t2->img);
    }
    return t1->tag == t2->tag;
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
    case V_UNIT:
        return make_base_type(T_UNIT);
    case V_NATIVE:
        return ref(v->native->type);
    case V_CLOS:
        return ref(v->func->type);
    case V_EXN:   /* Fail on exceptions */
    default:
        assert(0);
        abort();
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
    return make_exn_value(v->info, "Type %s can not be coerced to %s",
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
        char *act_str = type_string(act);
        syntax_error(info, "type error: expected %s but found %s",
                     allowed_names, act_str);
        free(act_str);
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
        abort();
        break;
    }

    return result;
}

static void type_error1(struct info *info, const char *msg, struct type *type) {
    char *s = type_string(type);
    syntax_error(info, "Type error: ");
    syntax_error(info, msg, s);
    free(s);
}

static void type_error2(struct info *info, const char *msg,
                        struct type *type1, struct type *type2) {
    char *s1 = type_string(type1);
    char *s2 = type_string(type2);
    syntax_error(info, "Type error: ");
    syntax_error(info, msg, s1, s2);
    free(s1);
    free(s2);
}

static void type_error_binop(struct info *info, const char *opname,
                             struct type *type1, struct type *type2) {
    char *s1 = type_string(type1);
    char *s2 = type_string(type2);
    syntax_error(info, "Type error: ");
    syntax_error(info, "%s of %s and %s is not possible", opname, s1, s2);
    free(s1);
    free(s2);
}

static int check_exp(struct term *term, struct ctx *ctx);

static struct type *require_exp_type(struct term *term, struct ctx *ctx,
                                     int ntypes, struct type *allowed[]) {
    int r = 1;

    if (term->type == NULL) {
        r = check_exp(term, ctx);
        if (! r)
            return NULL;
    }

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
    } else if (tl->tag == T_UNIT) {
        if (! check_exp(term->right, ctx))
            return 0;
        term->type = ref(term->right->type);
    } else {
        goto print_error;
    }
    return 1;
 print_error:
    type_error_binop(term->info,
                     "composition", term->left->type, term->right->type);
    return 0;
}

static int check_binop(const char *opname, struct term *term,
                       struct ctx *ctx, int ntypes, ...) {
    va_list ap;
    struct type *allowed[ntypes];
    struct type *tl = NULL, *tr = NULL;

    va_start(ap, ntypes);
    for (int i=0; i < ntypes; i++)
        allowed[i] = va_arg(ap, struct type *);
    va_end(ap);

    tl = require_exp_type(term->left, ctx, ntypes, allowed);
    if (tl == NULL)
        return 0;

    tr = require_exp_type(term->right, ctx, ntypes, allowed);
    if (tr == NULL)
        return 0;

    term->type = type_join(tl, tr);
    if (term->type == NULL)
        goto print_error;
    return 1;
 print_error:
    type_error_binop(term->info, opname, term->left->type, term->right->type);
    return 0;
}

static int check_value(struct value *v) {
    const char *msg;

    if (v->tag == V_REGEXP) {
        if (regexp_check(v->regexp, &msg) == -1) {
            syntax_error(v->info, "Invalid regular expression: %s", msg);
            return 0;
        }
    }
    return 1;
}

/* Return 1 if TERM passes, 0 otherwise */
static int check_exp(struct term *term, struct ctx *ctx) {
    int result = 1;
    assert(term->type == NULL || term->tag == A_VALUE || term->ref > 1);
    if (term->type != NULL && term->tag != A_VALUE)
        return 1;

    switch (term->tag) {
    case A_UNION:
        result = check_binop("union", term, ctx, 2, t_regexp, t_lens);
        break;
    case A_MINUS:
        result = check_binop("minus", term, ctx, 1, t_regexp);
        break;
    case A_COMPOSE:
        result = check_compose(term, ctx);
        break;
    case A_CONCAT:
        result = check_binop("concatenation", term, ctx,
                             4, t_string, t_regexp, t_lens, t_filter);
        break;
    case A_LET:
        {
            result = check_exp(term->right, ctx);
            if (result) {
                struct term *func = term->left;
                assert(func->tag == A_FUNC);
                assert(func->param->type == NULL);
                func->param->type = ref(term->right->type);

                result = check_exp(func, ctx);
                if (result) {
                    term->tag = A_APP;
                    term->type = ref(func->type->img);
                }
            }
        }
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
                type_error_binop(term->info, "application",
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

static int typecheck(struct term *term, struct augeas *aug) {
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

    ctx.aug = aug;
    ctx.local = NULL;
    ctx.name = term->mname;
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
    struct value *v = NULL;

    v1 = coerce(v1, t);
    if (EXN(v1))
        return v1;
    v2 = coerce(v2, t);
    if (EXN(v2)) {
        unref(v1, value);
        return v2;
    }

    if (t->tag == T_REGEXP) {
        v = make_value(V_REGEXP, ref(info));
        v->regexp = regexp_union(info, v1->regexp, v2->regexp);
    } else if (t->tag == T_LENS) {
        struct lens *l1 = v1->lens;
        struct lens *l2 = v2->lens;
        v = lns_make_union(ref(info), ref(l1), ref(l2), LNS_TYPE_CHECK(ctx));
    } else {
        fatal_error(info, "Tried to union a %s and a %s to yield a %s",
                    type_name(exp->left->type), type_name(exp->right->type),
                    type_name(t));
    }
    unref(v1, value);
    unref(v2, value);
    return v;
}

static struct value *compile_minus(struct term *exp, struct ctx *ctx) {
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
        struct regexp *re1 = v1->regexp;
        struct regexp *re2 = v2->regexp;
        struct regexp *re = regexp_minus(info, re1, re2);
        if (re == NULL) {
            v = make_exn_value(ref(info),
                   "Regular expression subtraction 'r1 - r2' failed");
            exn_printf_line(v, "r1: /%s/", re1->pattern->str);
            exn_printf_line(v, "r2: /%s/", re2->pattern->str);
        } else {
            v = make_value(V_REGEXP, ref(info));
            v->regexp = re;
        }
    } else {
        fatal_error(info, "Tried to subtract a %s and a %s to yield a %s",
                    type_name(exp->left->type), type_name(exp->right->type),
                    type_name(t));
    }
    unref(v1, value);
    unref(v2, value);
    return v;
}

static struct value *compile_compose(struct term *exp, struct ctx *ctx) {
    struct info *info = exp->info;
    struct value *v;

    if (exp->left->type->tag == T_ARROW) {
        // FIXME: This is really crufty, and should be desugared in the
        // parser so that we don't have to do all this manual type
        // computation. Should we write function compostion as
        // concatenation instead of using a separate syntax ?

        /* Build lambda x: exp->right (exp->left x) as a closure */
        char *var = strdup("@0");
        struct term *param = make_param(var, ref(exp->left->type->dom),
                                        ref(info));
        param->type = ref(exp->left->type);
        struct term *ident = make_term(A_IDENT, ref(info));
        ident->ident = ref(param->param->name);
        ident->type = ref(param->type);
        struct term *app = make_app_term(ref(exp->left), ident, ref(info));
        app->type = ref(app->left->type->img);
        app = make_app_term(ref(exp->right), app, ref(info));
        app->type = ref(app->left->type->img);

        struct term *func = build_func(param, app);

        if (!type_equal(func->type, exp->type)) {
            char *f = type_string(func->type);
            char *e = type_string(exp->type);
            fatal_error(info,
              "Composition has type %s but should have type %s", f, e);
            free(f);
            free(e);
            unref(func, term);
            return exn_error();
        }
        v = make_closure(func, ctx->local);
        unref(func, term);
    } else {
        v = compile_exp(exp->info, exp->left, ctx);
        unref(v, value);
        v = compile_exp(exp->info, exp->right, ctx);
    }
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
        char *s = v->string->str;
        strcpy(s, s1);
        strcat(s, s2);
    } else if (t->tag == T_REGEXP) {
        v = make_value(V_REGEXP, ref(info));
        v->regexp = regexp_concat(info, v1->regexp, v2->regexp);
    } else if (t->tag == T_FILTER) {
        struct filter *f1 = v1->filter;
        struct filter *f2 = v2->filter;
        v = make_value(V_FILTER, ref(info));
        if (v2->ref == 1 && f2->ref == 1) {
            list_append(f2, ref(f1));
            v->filter = ref(f2);
        } else if (v1->ref == 1 && f1->ref == 1) {
            list_append(f1, ref(f2));
            v->filter = ref(f1);
        } else {
            struct filter *cf1, *cf2;
            cf1 = make_filter(ref(f1->glob), f1->include);
            cf2 = make_filter(ref(f2->glob), f2->include);
            cf1->next = ref(f1->next);
            cf2->next = ref(f2->next);
            list_append(cf1, cf2);
            v->filter = cf1;
        }
    } else if (t->tag == T_LENS) {
        struct lens *l1 = v1->lens;
        struct lens *l2 = v2->lens;
        v = lns_make_concat(ref(info), ref(l1), ref(l2), LNS_TYPE_CHECK(ctx));
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

    lctx.aug = ctx->aug;
    lctx.local = ref(f->bindings);
    lctx.name = ctx->name;

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
    unref(arg, value);
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
    struct value *v = NULL;

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
            abort();
        }
        v = make_value(V_REGEXP, ref(rep->info));
        v->regexp = regexp_iter(rep->info, arg->regexp, min, max);
    } else if (rep->type->tag == T_LENS) {
        int c = LNS_TYPE_CHECK(ctx);
        if (rep->quant == Q_STAR) {
            v = lns_make_star(ref(rep->info), ref(arg->lens), c);
        } else if (rep->quant == Q_PLUS) {
            v = lns_make_plus(ref(rep->info), ref(arg->lens), c);
        } else if (rep->quant == Q_MAYBE) {
            v = lns_make_maybe(ref(rep->info), ref(arg->lens), c);
        } else {
            assert(0);
        }
    } else {
        fatal_error(rep->info, "Tried to repeat a %s to yield a %s",
                    type_name(rep->rexp->type), type_name(rep->type));
    }
    unref(arg, value);
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
    case A_MINUS:
        v = compile_minus(exp, ctx);
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
            print_value(stdout, actual);
            printf("\n");
            ret = 0;
        }
    } else {
        if (EXN(actual)) {
            print_info(stdout, term->info);
            printf("exception thrown in test\n");
            print_value(stdout, actual);
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
                print_value(stdout, expect);
                printf("\n");
                printf(" Actual:\n");
                print_value(stdout, actual);
                printf("\n");
                ret = 0;
            }
        } else {
            printf("Test result: ");
            print_info(stdout, term->info);
            printf("\n");
            if (actual->tag == V_TREE) {
                print_tree(stdout, 2, actual->origin->children);
            } else {
                print_value(stdout, actual);
            }
            printf("\n");
        }
    }
 done:
    reset_error(term->info->error);
    unref(actual, value);
    unref(expect, value);
    return ret;
}

static int compile_decl(struct term *term, struct ctx *ctx) {
    if (term->tag == A_BIND) {
        int result;

        struct value *v = compile_exp(term->info, term->exp, ctx);
        bind(&ctx->local, term->bname, term->type, v);

        if (EXN(v) && !v->exn->seen) {
            struct error *error = term->info->error;
            struct memstream ms;

            init_memstream(&ms);

            syntax_error(term->info, "Failed to compile %s",
                         term->bname);
            fprintf(ms.stream, "%s\n", error->details);
            print_value(ms.stream, v);
            close_memstream(&ms);

            v->exn->seen = 1;
            free(error->details);
            error->details = ms.buf;
        }
        result = ! EXN(v);
        unref(v, value);
        return result;
    } else if (term->tag == A_TEST) {
        return compile_test(term, ctx);
    }
    assert(0);
    abort();
}

static struct module *compile(struct term *term, struct augeas *aug) {
    struct ctx ctx;
    struct transform *autoload = NULL;
    assert(term->tag == A_MODULE);

    ctx.aug = aug;
    ctx.local = NULL;
    ctx.name = term->mname;
    list_for_each(dcl, term->decls) {
        if (!compile_decl(dcl, &ctx))
            goto error;
    }

    if (term->autoload != NULL) {
        struct binding *bnd = bnd_lookup(ctx.local, term->autoload);
        if (bnd == NULL) {
            syntax_error(term->info, "Undefined transform in autoload %s",
                         term->autoload);
            goto error;
        }
        if (expect_types(term->info, bnd->type, 1, t_transform) == NULL)
            goto error;
        autoload = bnd->value->transform;
    }
    struct module *module = module_create(term->mname);
    module->bindings = ctx.local;
    module->autoload = ref(autoload);
    return module;
 error:
    unref(ctx.local, binding);
    return NULL;
}

/*
 * Defining native functions
 */
static struct info *
make_native_info(struct error *error, const char *fname, int line) {
    struct info *info;
    if (make_ref(info) < 0)
        goto error;
    info->first_line = info->last_line = line;
    info->first_column = info->last_column = 0;
    info->error = error;
    if (make_ref(info->filename) < 0)
        goto error;
    info->filename->str = strdup(fname);
    return info;
 error:
    unref(info, info);
    return NULL;
}

int define_native_intl(const char *file, int line,
                       struct error *error,
                       struct module *module, const char *name,
                       int argc, void *impl, ...) {
    assert(argc > 0);  /* We have no unit type */
    assert(argc <= 5);
    va_list ap;
    enum type_tag tag;
    struct term *params = NULL, *body = NULL, *func = NULL;
    struct type *type;
    struct value *v = NULL;
    struct info *info = NULL;
    struct ctx ctx;

    info = make_native_info(error, file, line);
    if (info == NULL)
        goto error;

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
    if (v == NULL)
        goto error;
    v->tag = V_NATIVE;
    v->info = info;
    info = NULL;

    if (ALLOC(v->native) < 0)
        goto error;
    v->native->argc = argc;
    v->native->type = type;
    v->native->impl = impl;

    make_ref(body);
    if (body == NULL)
        goto error;
    body->info = ref(info);
    body->type = ref(type);
    body->tag = A_VALUE;
    body->value = v;
    v = NULL;

    func = build_func(params, body);
    if (func == NULL)
        goto error;
    body = NULL;

    ctx.aug = NULL;
    ctx.local = ref(module->bindings);
    ctx.name = module->name;
    if (! check_exp(func, &ctx)) {
        fatal_error(info, "Typechecking native %s failed",
                    name);
        abort();
    }
    v = make_closure(func, ctx.local);
    if (v == NULL) {
        unref(module->bindings, binding);
        goto error;
    }
    bind(&ctx.local, name, func->type, v);
    unref(v, value);
    unref(func, term);
    unref(module->bindings, binding);

    module->bindings = ctx.local;
    return 0;
 error:
    unref(v, value);
    unref(body, term);
    unref(func, term);
    return -1;
}


/* Defined in parser.y */
int augl_parse_file(struct augeas *aug, const char *name, struct term **term);

static char *module_basename(const char *modname) {
    char *fname;

    if (asprintf(&fname, "%s" AUG_EXT, modname) == -1)
        return NULL;
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

        if (REALLOC_N(filename, len) == -1)
            goto error;
        sprintf(filename, "%s/%s", dir, name);
        if (stat(filename, &st) == 0)
            goto done;
    }
 error:
    FREE(filename);
 done:
    free(name);
    return filename;
}

int load_module_file(struct augeas *aug, const char *filename) {
    struct term *term = NULL;
    int result = -1;

    augl_parse_file(aug, filename, &term);
    ERR_BAIL(aug);

    if (! typecheck(term, aug))
        goto error;

    struct module *module = compile(term, aug);
    ERR_THROW(module == NULL, aug, AUG_ESYNTAX,
              "Failed to load %s", filename);

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

    if (load_module_file(aug, filename) == -1)
        goto error;

    free(filename);
    return 0;

 error:
    free(filename);
    return -1;
}

int interpreter_init(struct augeas *aug) {
    int r;

    aug->modules = builtin_init(aug->error);

    if (aug->flags & AUG_NO_MODL_AUTOLOAD)
        return 0;

    // For now, we just load every file on the search path
    const char *dir = NULL;
    glob_t globbuf;
    int gl_flags = GLOB_NOSORT;

    MEMZERO(&globbuf, 1);

    while ((dir = argz_next(aug->modpathz, aug->nmodpath, dir)) != NULL) {
        char *globpat;
        r = asprintf(&globpat, "%s/*.aug", dir);
        ERR_NOMEM(r < 0, aug);

        r = glob(globpat, gl_flags, NULL, &globbuf);
        if (r != 0 && r != GLOB_NOMATCH) {
            /* This really has to be an allocation failure; glob is not
             * supposed to return GLOB_ABORTED here */
            aug_errcode_t code =
                r == GLOB_NOSPACE ? AUG_ENOMEM : AUG_EINTERNAL;
            ERR_REPORT(aug, code, "glob failure for %s", globpat);
            free(globpat);
            goto error;
        }
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
    globfree(&globbuf);
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
