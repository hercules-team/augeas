/*
 * lens.c:
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

#include <config.h>
#include <stddef.h>

#include "lens.h"
#include "memory.h"
#include "errcode.h"

static const int const type_offs[] = {
    offsetof(struct lens, ctype),
    offsetof(struct lens, atype),
    offsetof(struct lens, ktype),
    offsetof(struct lens, vtype)
};
static const int ntypes = sizeof(type_offs)/sizeof(type_offs[0]);

#define ltype(lns, t) *((struct regexp **) ((char *) lns + type_offs[t]))

static struct value * typecheck_union(struct info *,
                                      struct lens *l1, struct lens *l2);
static struct value *typecheck_concat(struct info *,
                                      struct lens *l1, struct lens *l2);
static struct value *typecheck_iter(struct info *info, struct lens *l);
static struct value *typecheck_maybe(struct info *info, struct lens *l);

/* Lens names for pretty printing */
static const char *const tags[] = {
    "del", "store", "key", "label", "seq", "counter", "concat", "union",
    "subtree", "star", "maybe", "rec"
};

static const struct string digits_string = {
    .ref = REF_MAX, .str = (char *) "[0123456789]+"
};
static const struct string *const digits_pat = &digits_string;

char *format_lens(struct lens *l) {
    char *inf = format_info(l->info);
    char *result;

    xasprintf(&result, "%s[%s]%s", tags[l->tag - L_DEL], inf,
              l->recursive ? "R" : "r");
    free(inf);
    return result;
}

/* Construct a finite automaton from REGEXP and return it in *FA.
 *
 * Return NULL if REGEXP is valid, if the regexp REGEXP has syntax errors,
 * return an exception.
 */
static struct value *str_to_fa(struct info *info, const char *pattern,
                               struct fa **fa, int nocase) {
    int error;
    struct value *exn = NULL;
    size_t re_err_len;
    char *re_str = NULL, *re_err = NULL;

    *fa = NULL;
    error = fa_compile(pattern, strlen(pattern), fa);
    if (error == REG_NOERROR) {
        if (nocase) {
            error = fa_nocase(*fa);
            ERR_NOMEM(error < 0, info);
        }
        return NULL;
    }

    re_str = escape(pattern, -1);
    ERR_NOMEM(re_str == NULL, info);

    exn = make_exn_value(info, "Invalid regular expression /%s/", re_str);

    re_err_len = regerror(error, NULL, NULL, 0);
    error = ALLOC_N(re_err, re_err_len);
    ERR_NOMEM(error < 0, info);

    regerror(error, NULL, re_err, re_err_len);
    exn_printf_line(exn, "%s", re_err);

 done:
    free(re_str);
    free(re_err);
    return exn;
 error:
    fa_free(*fa);
    *fa = NULL;
    exn = exn_error();
    goto done;
}

static struct value *regexp_to_fa(struct regexp *regexp, struct fa **fa) {
    return str_to_fa(regexp->info, regexp->pattern->str, fa, regexp->nocase);
}

static struct lens *make_lens(enum lens_tag tag, struct info *info) {
    struct lens *lens;
    make_ref(lens);
    lens->tag = tag;
    lens->info = info;

    return lens;
}

static struct lens *make_lens_unop(enum lens_tag tag, struct info *info,
                                  struct lens *child) {
    struct lens *lens = make_lens(tag, info);
    lens->child = child;
    lens->value = child->value;
    lens->key = child->key;
    return lens;
}

typedef struct regexp *regexp_combinator(struct info *, int, struct regexp **);

static struct lens *make_lens_binop(enum lens_tag tag, struct info *info,
                                    struct lens *l1, struct lens *l2,
                                    regexp_combinator *combinator) {
    struct lens *lens = make_lens(tag, info);
    int n1 = (l1->tag == tag) ? l1->nchildren : 1;
    struct regexp **types = NULL;

    if (lens == NULL)
        goto error;

    lens->nchildren = n1;
    lens->nchildren += (l2->tag == tag) ? l2->nchildren : 1;

    if (ALLOC_N(lens->children, lens->nchildren) < 0) {
        lens->nchildren = 0;
        goto error;
    }

    if (l1->tag == tag) {
        for (int i=0; i < l1->nchildren; i++)
            lens->children[i] = ref(l1->children[i]);
        unref(l1, lens);
    } else {
        lens->children[0] = l1;
    }

    if (l2->tag == tag) {
        for (int i=0; i < l2->nchildren; i++)
            lens->children[n1 + i] = ref(l2->children[i]);
        unref(l2, lens);
    } else {
        lens->children[n1] = l2;
    }

    for (int i=0; i < lens->nchildren; i++) {
        lens->value = lens->value || lens->children[i]->value;
        lens->key = lens->key || lens->children[i]->key;
    }

    if (ALLOC_N(types, lens->nchildren) < 0)
        goto error;

    for (int t=0; t < ntypes; t++) {
        for (int i=0; i < lens->nchildren; i++)
            types[i] = ltype(lens->children[i], t);
        ltype(lens, t) = (*combinator)(info, lens->nchildren, types);
    }

    FREE(types);

    for (int i=0; i < lens->nchildren; i++)
        assert(tag != lens->children[i]->tag);

    return lens;
 error:
    unref(lens, lens);
    FREE(types);
    return NULL;
}

static struct value *make_lens_value(struct lens *lens) {
    struct value *v;
    v = make_value(V_LENS, ref(lens->info));
    v->lens = lens;
    return v;
}

struct value *lns_make_union(struct info *info,
                             struct lens *l1, struct lens *l2, int check) {
    struct lens *lens = NULL;
    int consumes_value = l1->consumes_value && l2->consumes_value;

    if (check) {
        struct value *exn = typecheck_union(info, l1, l2);
        if (exn != NULL)
            return exn;
    }

    lens = make_lens_binop(L_UNION, info, l1, l2, regexp_union_n);
    lens->consumes_value = consumes_value;
    return make_lens_value(lens);
}

struct value *lns_make_concat(struct info *info,
                              struct lens *l1, struct lens *l2, int check) {
    struct lens *lens = NULL;
    int consumes_value = l1->consumes_value || l2->consumes_value;

    if (check) {
        struct value *exn = typecheck_concat(info, l1, l2);
        if (exn != NULL) {
            return exn;
        }
    }
    if (l1->value && l2->value) {
        return make_exn_value(info, "Multiple stores in concat");
    }
    if (l1->key && l2->key) {
        return make_exn_value(info, "Multiple keys/labels in concat");
    }

    lens = make_lens_binop(L_CONCAT, info, l1, l2, regexp_concat_n);
    lens->consumes_value = consumes_value;
    return make_lens_value(lens);
}

static struct regexp *subtree_atype(struct info *info,
                                    struct regexp *ktype,
                                    struct regexp *vtype) {
    const char *kpat = (ktype == NULL) ? ENC_NULL : ktype->pattern->str;
    const char *vpat = (vtype == NULL) ? ENC_NULL : vtype->pattern->str;
    char *pat;
    struct regexp *result = NULL;
    char *ks = NULL, *vs = NULL;
    int nocase;

    if (ktype != NULL && vtype != NULL && ktype->nocase != vtype->nocase) {
        ks = regexp_expand_nocase(ktype);
        vs = regexp_expand_nocase(vtype);
        ERR_NOMEM(ks == NULL || vs == NULL, info);
        if (asprintf(&pat, "(%s)%s(%s)%s", ks, ENC_EQ, vs, ENC_SLASH) < 0)
            ERR_NOMEM(true, info);
        nocase = 0;
    } else {
        if (asprintf(&pat, "(%s)%s(%s)%s", kpat, ENC_EQ, vpat, ENC_SLASH) < 0)
            ERR_NOMEM(pat == NULL, info);

        nocase = 0;
        if (ktype != NULL)
            nocase = ktype->nocase;
        else if (vtype != NULL)
            nocase = vtype->nocase;
    }
    result = make_regexp(info, pat, nocase);
 error:
    free(ks);
    free(vs);
    return result;
}

/*
 * A subtree lens l1 = [ l ]
 *
 * Types are assigned as follows:
 *
 * l1->ctype = l->ctype
 * l1->atype = encode(l->ktype, l->vtype)
 * l1->ktype = NULL
 * l1->vtype = NULL
 */
struct value *lns_make_subtree(struct info *info, struct lens *l) {
    struct lens *lens;

    lens = make_lens_unop(L_SUBTREE, info, l);
    lens->ctype = ref(l->ctype);
    lens->atype = subtree_atype(info, l->ktype, l->vtype);
    lens->value = lens->key = 0;
    return make_lens_value(lens);
}

struct value *lns_make_star(struct info *info, struct lens *l, int check) {
    struct lens *lens;

    if (check) {
        struct value *exn = typecheck_iter(info, l);
        if (exn != NULL) {
            return exn;
        }
    }
    if (l->value) {
        return make_exn_value(info, "Multiple stores in iteration");
    }
    if (l->key) {
        return make_exn_value(info, "Multiple keys/labels in iteration");
    }

    lens = make_lens_unop(L_STAR, info, l);
    for (int t = 0; t < ntypes; t++) {
        ltype(lens, t) = regexp_iter(info, ltype(l, t), 0, -1);
    }
    return make_lens_value(lens);
}

struct value *lns_make_plus(struct info *info, struct lens *l, int check) {
    struct value *star, *conc;

    star = lns_make_star(info, l, check);
    if (EXN(star))
        return star;

    conc = lns_make_concat(ref(info), ref(l), ref(star->lens), check);
    unref(star, value);
    return conc;
}

struct value *lns_make_maybe(struct info *info, struct lens *l, int check) {
    struct lens *lens;

    if (check) {
        struct value *exn = typecheck_maybe(info, l);
        if (exn != NULL) {
            return exn;
        }
    }
    lens = make_lens_unop(L_MAYBE, info, l);
    for (int t=0; t < ntypes; t++)
        ltype(lens, t) = regexp_maybe(info, ltype(l, t));
    lens->value = l->value;
    lens->key = l->key;
    return make_lens_value(lens);
}

/*
 * Lens primitives
 */

static struct regexp *make_regexp_from_string(struct info *info,
                                              struct string *string) {
    struct regexp *r;
    make_ref(r);
    if (r != NULL) {
        r->info = ref(info);
        r->pattern = ref(string);
        r->nocase = 0;
    }
    return r;
}

static struct regexp *restrict_regexp(struct regexp *r) {
    char *nre = NULL;
    size_t nre_len;
    int ret;

    ret = fa_restrict_alphabet(r->pattern->str, strlen(r->pattern->str),
                               &nre, &nre_len,
                               RESERVED_FROM, RESERVED_TO);
    assert(nre_len == strlen(nre));
    // FIXME: Tell the user what's wrong
    if (ret != 0)
        return NULL;

    ret = regexp_c_locale(&nre, &nre_len);
    if (ret < 0) {
        free(nre);
        return NULL;
    }

    r = make_regexp(r->info, nre, r->nocase);
    if (regexp_compile(r) != 0)
        abort();
    return r;
}

struct value *lns_make_prim(enum lens_tag tag, struct info *info,
                            struct regexp *regexp, struct string *string) {
    struct lens *lens = NULL;
    struct value *exn = NULL;
    struct fa *fa_slash = NULL;
    struct fa *fa_key = NULL;
    struct fa *fa_isect = NULL;

    /* Typecheck */
    if (tag == L_KEY) {
        exn = str_to_fa(info, "(.|\n)*/(.|\n)*", &fa_slash, regexp->nocase);
        if (exn != NULL)
            goto error;

        exn = regexp_to_fa(regexp, &fa_key);
        if (exn != NULL)
            goto error;

        fa_isect = fa_intersect(fa_slash, fa_key);
        if (! fa_is_basic(fa_isect, FA_EMPTY)) {
            exn = make_exn_value(info,
                                 "The key regexp /%s/ matches a '/'",
                                 regexp->pattern->str);
            goto error;
        }
        fa_free(fa_isect);
        fa_free(fa_key);
        fa_free(fa_slash);
        fa_isect = fa_key = fa_slash = NULL;
    } else if (tag == L_LABEL) {
        if (strchr(string->str, SEP) != NULL) {
            exn = make_exn_value(info,
                                 "The label string \"%s\" contains a '/'",
                                 string->str);
            goto error;
        }
    } else if (tag == L_DEL) {
        int cnt;
        const char *dflt = string->str;
        cnt = regexp_match(regexp, dflt, strlen(dflt), 0, NULL);
        if (cnt != strlen(dflt)) {
            char *s = escape(dflt, -1);
            char *r = regexp_escape(regexp);
            exn = make_exn_value(info,
                   "del: the default value '%s' does not match /%s/",
                   s, r);
            FREE(s);
            FREE(r);
            goto error;
        }
    }

    /* Build the actual lens */
    lens = make_lens(tag, info);
    lens->regexp = regexp;
    lens->string = string;
    lens->key = (tag == L_KEY || tag == L_LABEL || tag == L_SEQ);
    lens->value = (tag == L_STORE);
    lens->consumes_value = (tag == L_STORE);
    lens->atype = regexp_make_empty(info);
    /* Set the ctype */
    if (tag == L_DEL || tag == L_STORE || tag == L_KEY) {
        lens->ctype = ref(regexp);
    } else if (tag == L_LABEL || tag == L_SEQ || tag == L_COUNTER) {
        lens->ctype = regexp_make_empty(info);
    } else {
        assert(0);
    }

    /* Set the ktype */
    if (tag == L_SEQ) {
        lens->ktype =
            make_regexp_from_string(info, (struct string *) digits_pat);
        if (lens->ktype == NULL)
            goto error;
    } else if (tag == L_KEY) {
        lens->ktype = restrict_regexp(lens->regexp);
    } else if (tag == L_LABEL) {
        lens->ktype = make_regexp_literal(info, lens->string->str);
        if (lens->ktype == NULL)
            goto error;
    }

    /* Set the vtype */
    if (tag == L_STORE) {
        lens->vtype = restrict_regexp(lens->regexp);
    }

    return make_lens_value(lens);
 error:
    fa_free(fa_isect);
    fa_free(fa_key);
    fa_free(fa_slash);
    return exn;
}

/*
 * Typechecking of lenses
 */
static struct value *disjoint_check(struct info *info, bool is_get,
                                    struct regexp *r1, struct regexp *r2) {
    struct fa *fa1 = NULL;
    struct fa *fa2 = NULL;
    struct fa *fa = NULL;
    struct value *exn = NULL;
    const char *const msg = is_get ? "union.get" : "tree union.put";

    exn = regexp_to_fa(r1, &fa1);
    if (exn != NULL)
        goto done;

    exn = regexp_to_fa(r2, &fa2);
    if (exn != NULL)
        goto done;

    fa = fa_intersect(fa1, fa2);
    if (! fa_is_basic(fa, FA_EMPTY)) {
        size_t xmpl_len;
        char *xmpl;
        fa_example(fa, &xmpl, &xmpl_len);
        if (! is_get) {
            char *fmt = enc_format(xmpl, xmpl_len);
            if (fmt != NULL) {
                FREE(xmpl);
                xmpl = fmt;
            }
        }
        exn = make_exn_value(ref(info),
                             "overlapping lenses in %s", msg);

        if (is_get)
            exn_printf_line(exn, "Example matched by both: '%s'", xmpl);
        else
            exn_printf_line(exn, "Example matched by both: %s", xmpl);
        free(xmpl);
    }

 done:
    fa_free(fa);
    fa_free(fa1);
    fa_free(fa2);

    return exn;
}

static struct value *typecheck_union(struct info *info,
                                     struct lens *l1, struct lens *l2) {
    struct value *exn = NULL;

    exn = disjoint_check(info, true, l1->ctype, l2->ctype);
    if (exn == NULL) {
        exn = disjoint_check(info, false, l1->atype, l2->atype);
    }
    if (exn != NULL) {
        char *fi = format_info(l1->info);
        exn_printf_line(exn, "First lens: %s", fi);
        free(fi);

        fi = format_info(l2->info);
        exn_printf_line(exn, "Second lens: %s", fi);
        free(fi);
    }
    return exn;
}

static struct value *ambig_check(struct info *info,
                                 struct fa *fa1, struct fa *fa2,
                                 struct regexp *r1, struct regexp *r2,
                                 const char *msg, bool iterated) {
    char *upv, *pv, *v;
    size_t upv_len;
    struct value *exn = NULL;
    int r;

    r = fa_ambig_example(fa1, fa2, &upv, &upv_len, &pv, &v);
    if (r < 0) {
        exn = make_exn_value(ref(info), "not enough memory");
        if (exn != NULL) {
            return exn;
        } else {
            ERR_REPORT(info, AUG_ENOMEM, NULL);
            return exn_error();
        }
    }

    if (upv != NULL) {
        char *e_u = escape(upv, pv - upv);
        char *e_up = escape(upv, v - upv);
        char *e_upv = escape(upv, -1);
        char *e_pv = escape(pv, -1);
        char *e_v = escape(v, -1);
        char *s1 = regexp_escape(r1);
        char *s2 = regexp_escape(r2);
        exn = make_exn_value(ref(info), "%s", msg);
        if (iterated) {
            exn_printf_line(exn, "  Iterated regexp: /%s/", s1);
        } else {
            exn_printf_line(exn, "  First regexp: /%s/", s1);
            exn_printf_line(exn, "  Second regexp: /%s/", s2);
        }
        exn_printf_line(exn, "  '%s' can be split into", e_upv);
        exn_printf_line(exn, "  '%s|=|%s'\n", e_u, e_pv);
        exn_printf_line(exn, " and");
        exn_printf_line(exn, "  '%s|=|%s'\n", e_up, e_v);
        free(e_u);
        free(e_up);
        free(e_upv);
        free(e_pv);
        free(e_v);
        free(s1);
        free(s2);
    }
    free(upv);
    return exn;
}

static struct value *ambig_concat_check(struct info *info, const char *msg,
                                        struct regexp *r1, struct regexp *r2) {
    struct fa *fa1 = NULL;
    struct fa *fa2 = NULL;
    struct value *result = NULL;

    result = regexp_to_fa(r1, &fa1);
    if (result != NULL)
        goto done;

    result = regexp_to_fa(r2, &fa2);
    if (result != NULL)
        goto done;

    result = ambig_check(info, fa1, fa2, r1, r2, msg, false);
 done:
    fa_free(fa1);
    fa_free(fa2);
    return result;
}

static struct value *typecheck_concat(struct info *info,
                                      struct lens *l1, struct lens *l2) {
    struct value *result = NULL;

    result = ambig_concat_check(info, "ambiguous concatenation",
                                l1->ctype, l2->ctype);
    if (result == NULL) {
        result = ambig_concat_check(info, "ambiguous tree concatenation",
                                    l1->atype, l2->atype);
    }
    if (result != NULL) {
        char *fi = format_info(l1->info);
        exn_printf_line(result, "First lens: %s", fi);
        free(fi);
        fi = format_info(l2->info);
        exn_printf_line(result, "Second lens: %s", fi);
        free(fi);
    }
    return result;
}

static struct value *ambig_iter_check(struct info *info, const char *msg,
                                      struct regexp *r) {
    struct fa *fas = NULL, *fa = NULL;
    struct value *result = NULL;

    result = regexp_to_fa(r, &fa);
    if (result != NULL)
        goto done;

    fas = fa_iter(fa, 0, -1);

    result = ambig_check(info, fa, fas, r, r, msg, true);

 done:
    fa_free(fa);
    fa_free(fas);
    return result;
}

static struct value *typecheck_iter(struct info *info, struct lens *l) {
    struct value *result = NULL;

    result = ambig_iter_check(info, "ambiguous iteration", l->ctype);
    if (result == NULL) {
        result = ambig_iter_check(info, "ambiguous tree iteration", l->atype);
    }
    if (result != NULL) {
        char *fi = format_info(l->info);
        exn_printf_line(result, "Iterated lens: %s", fi);
        free(fi);
    }
    return result;
}

static struct value *typecheck_maybe(struct info *info, struct lens *l) {
    /* Check (r)? as (<e>|r) where <e> is the empty language */
    struct value *exn = NULL;

    if (regexp_matches_empty(l->ctype)) {
        exn = make_exn_value(ref(info),
                "illegal optional expression: /%s/ matches the empty word",
                l->ctype->pattern->str);
    }

    /* Typecheck the put direction; the check passes if
       (1) the atype does not match the empty string, because we can tell
           from looking at tree nodes whether L should be applied or not
       (2) L handles a value; with that, we know whether to apply L or not
           depending on whether the current node has a non NULL value or not
    */
    if (exn == NULL && ! l->consumes_value) {
        if (regexp_matches_empty(l->atype)) {
            exn = make_exn_value(ref(info),
               "optional expression matches the empty tree but does not consume a value");
        }
    }
    return exn;
}

void free_lens(struct lens *lens) {
    if (lens == NULL)
        return;
    assert(lens->ref == 0);

    unref(lens->info, info);
    for (int t=0; t < ntypes; t++)
        unref(ltype(lens, t), regexp);

    switch (lens->tag) {
    case L_DEL:
        unref(lens->regexp, regexp);
        unref(lens->string, string);
        break;
    case L_STORE:
    case L_KEY:
        unref(lens->regexp, regexp);
        break;
    case L_LABEL:
    case L_SEQ:
    case L_COUNTER:
        unref(lens->string, string);
        break;
    case L_SUBTREE:
    case L_STAR:
    case L_MAYBE:
        unref(lens->child, lens);
        break;
    case L_CONCAT:
    case L_UNION:
        for (int i=0; i < lens->nchildren; i++)
            unref(lens->children[i], lens);
        free(lens->children);
        break;
    default:
        assert(0);
        break;
    }
    free(lens);
}

void lens_release(struct lens *lens) {
    if (lens == NULL)
        return;

    for (int t=0; t < ntypes; t++)
        regexp_release(ltype(lens, t));

    if (lens->tag == L_KEY || lens->tag == L_STORE)
        regexp_release(lens->regexp);

    if (lens->tag == L_SUBTREE || lens->tag == L_STAR
        || lens->tag == L_MAYBE) {
        lens_release(lens->child);
    }

    if (lens->tag == L_UNION || lens->tag == L_CONCAT) {
        for (int i=0; i < lens->nchildren; i++) {
            lens_release(lens->children[i]);
        }
    }
}

/*
 * Encoding of tree levels
 */
char *enc_format(const char *e, size_t len) {
    size_t size = 0;
    char *result = NULL, *r;
    const char *k = e;

    while (*k && k - e < len) {
        char *eq,  *slash, *v;
        eq = strchr(k, ENC_EQ_CH);
        slash = strchr(eq, ENC_SLASH_CH);
        assert(eq != NULL && slash != NULL);
        v = eq + 1;

        size += 6;     /* Surrounding braces */
        if (k != eq)
            size += 1 + (eq - k) + 1;
        if (v != slash)
            size += 4 + (slash - v) + 1;
        k = slash + 1;
    }
    if (ALLOC_N(result, size + 1) < 0)
        return NULL;

    k = e;
    r = result;
    while (*k && k - e < len) {
        char *eq,  *slash, *v;
        eq = strchr(k, ENC_EQ_CH);
        slash = strchr(eq, ENC_SLASH_CH);
        assert(eq != NULL && slash != NULL);
        v = eq + 1;

        r = stpcpy(r, " { ");
        if (k != eq) {
            r = stpcpy(r, "\"");
            r = stpncpy(r, k, eq - k);
            r = stpcpy(r, "\"");
        }
        if (v != slash) {
            r = stpcpy (r, " = \"");
            r = stpncpy(r, v, slash - v);
            r = stpcpy(r, "\"");
        }
        r = stpcpy(r, " }");
        k = slash + 1;
    }
    return result;
}

static int lns_format_subtree_atype(struct lens *l, char **buf) {
    char *k = NULL, *v = NULL;
    const struct regexp *ktype = l->child->ktype;
    const struct regexp *vtype = l->child->vtype;
    int r, result = -1;

    if (ktype != NULL) {
        k = regexp_escape(ktype);
        if (k == NULL)
            goto done;
    }
    if (vtype != NULL) {
        v = regexp_escape(vtype);
        if (v == NULL)
            goto done;
        if (k == NULL)
            r = xasprintf(buf, "{ = /%s/ }", k, v);
        else
            r = xasprintf(buf, "{ /%s/ = /%s/ }", k, v);
    } else {
        if (k == NULL)
            r = xasprintf(buf, "{ }", k);
        else
            r = xasprintf(buf, "{ /%s/ }", k);
    }
    if (r < 0)
        goto done;

    result = 0;
 done:
    FREE(v);
    FREE(k);
    return result;
}

static int lns_format_rep_atype(struct lens *l, char **buf, char quant) {
    char *a = NULL;
    int r, result = -1;

    r = lns_format_atype(l->child, &a);
    if (r < 0)
        goto done;
    if (strlen(a) == 0) {
        *buf = a;
        a = NULL;
        result = 0;
        goto done;
    }

    if (l->child->tag == L_CONCAT || l->child->tag == L_UNION)
        r = xasprintf(buf, "(%s)%c", a, quant);
    else
        r = xasprintf(buf, "%s%c", a, quant);

    if (r < 0)
        goto done;

    result = 0;
 done:
    FREE(a);
    return result;
}

static int lns_format_concat_atype(struct lens *l, char **buf) {
    char **c = NULL, *s = NULL, *p;
    int r, result = -1;
    size_t len = 0, nconc = 0;

    if (ALLOC_N(c, l->nchildren) < 0)
        goto done;

    for (int i=0; i < l->nchildren; i++) {
        r = lns_format_atype(l->children[i], c+i);
        if (r < 0)
            goto done;
        len += strlen(c[i]) + 2;
        if (strlen(c[i]) > 0)
            nconc += 1;
        if (l->children[i]->tag == L_UNION)
            len += 2;
    }

    if (ALLOC_N(s, len+1) < 0)
        goto done;
    p = s;
    for (int i=0; i < l->nchildren; i++) {
        bool needs_parens = nconc > 1 && l->children[i]->tag == L_UNION;
        if (strlen(c[i]) == 0)
            continue;
        if (needs_parens)
            *p++ = '(';
        p = stpcpy(p, c[i]);
        if (needs_parens)
            *p++ = ')';
    }

    *buf = s;
    s = NULL;
    result = 0;
 done:
    if (c != NULL)
        for (int i=0; i < l->nchildren; i++)
            FREE(c[i]);
    FREE(c);
    FREE(s);
    return result;
}

static int lns_format_union_atype(struct lens *l, char **buf) {
    char **c = NULL, *s = NULL, *p;
    int r, result = -1;
    size_t len = 0;

    if (ALLOC_N(c, l->nchildren) < 0)
        goto done;

    for (int i=0; i < l->nchildren; i++) {
        r = lns_format_atype(l->children[i], c+i);
        if (r < 0)
            goto done;
        len += strlen(c[i]) + 2;
    }
    len += l->nchildren - 1;

    if (ALLOC_N(s, len+1) < 0)
        goto done;

    p = s;
    for (int i=0; i < l->nchildren; i++) {
        if (i > 0)
            p = stpcpy(p, " | ");
        if (strlen(c[i]) == 0)
            p = stpcpy(p, "()");
        else
            p = stpcpy(p, c[i]);
    }
    *buf = s;
    s = NULL;
    result = 0;
 done:
    if (c != NULL)
        for (int i=0; i < l->nchildren; i++)
            FREE(c[i]);
    FREE(c);
    FREE(s);
    return result;
}

int lns_format_atype(struct lens *l, char **buf) {
    *buf = NULL;

    switch(l->tag) {
    case L_DEL:
    case L_STORE:
    case L_KEY:
    case L_LABEL:
    case L_SEQ:
    case L_COUNTER:
        *buf = strdup("");
        return (*buf == NULL) ? -1 : 0;
        break;
    case L_SUBTREE:
        return lns_format_subtree_atype(l, buf);
        break;
    case L_STAR:
        return lns_format_rep_atype(l, buf, '*');
        break;
    case L_MAYBE:
        return lns_format_rep_atype(l, buf, '?');
        break;
    case L_CONCAT:
        return lns_format_concat_atype(l, buf);
        break;
    case L_UNION:
        return lns_format_union_atype(l, buf);
        break;
    default:
        assert(0);
        break;
    };
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
