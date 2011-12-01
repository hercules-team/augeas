/*
 * lens.c:
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
#include <stddef.h>

#include "lens.h"
#include "memory.h"
#include "errcode.h"
#include "internal.h"

/* This enum must be kept in sync with type_offs and ntypes */
enum lens_type {
    CTYPE, ATYPE, KTYPE, VTYPE
};

static const int const type_offs[] = {
    offsetof(struct lens, ctype),
    offsetof(struct lens, atype),
    offsetof(struct lens, ktype),
    offsetof(struct lens, vtype)
};
static const int ntypes = sizeof(type_offs)/sizeof(type_offs[0]);

static const char *lens_type_names[] =
    { "ctype", "atype", "ktype", "vtype" };

#define ltype(lns, t) *((struct regexp **) ((char *) lns + type_offs[t]))

static struct value * typecheck_union(struct info *,
                                      struct lens *l1, struct lens *l2);
static struct value *typecheck_concat(struct info *,
                                      struct lens *l1, struct lens *l2);
static struct value *typecheck_iter(struct info *info, struct lens *l);
static struct value *typecheck_maybe(struct info *info, struct lens *l);

/* Lens names for pretty printing */
/* keep order in sync with enum type */
static const char *const tags[] = {
    "del", "store", "value", "key", "label", "seq", "counter",
    "concat", "union",
    "subtree", "star", "maybe", "rec", "square"
};

#define ltag(lens) (tags[lens->tag - L_DEL])

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

#define BUG_LENS_TAG(lns)  bug_lens_tag(lns, __FILE__, __LINE__)

static void bug_lens_tag(struct lens *lens, const char *file, int lineno) {
    char *s = format_lens(lens);

    if (lens != NULL && lens->info != NULL && lens->info->error != NULL) {
        bug_on(lens->info->error, file, lineno, "Unexpected lens tag %s", s);
    } else {
        /* We are really screwed */
        assert(0);
    }
    free(s);
    return;
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

    re_str = escape(pattern, -1, RX_ESCAPES);
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

    lens->recursive = l1->recursive || l2->recursive;
    lens->rec_internal = l1->rec_internal || l2->rec_internal;

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

    if (! lens->rec_internal) {
        /* Inside a recursive lens, we assign types with lns_check_rec
         * once we know the entire lens */
        for (int t=0; t < ntypes; t++) {
            if (lens->recursive && t == CTYPE)
                continue;
            for (int i=0; i < lens->nchildren; i++)
                types[i] = ltype(lens->children[i], t);
            ltype(lens, t) = (*combinator)(info, lens->nchildren, types);
        }
    }
    FREE(types);

    for (int i=0; i < lens->nchildren; i++)
        ensure(tag != lens->children[i]->tag, lens->info);

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
    int recursive = l1->recursive || l2->recursive;
    int ctype_nullable = l1->ctype_nullable || l2->ctype_nullable;

    if (check) {
        struct value *exn = typecheck_union(info, l1, l2);
        if (exn != NULL)
            return exn;
    }

    lens = make_lens_binop(L_UNION, info, l1, l2, regexp_union_n);
    lens->consumes_value = consumes_value;
    if (! recursive)
        lens->ctype_nullable = ctype_nullable;
    return make_lens_value(lens);
}

struct value *lns_make_concat(struct info *info,
                              struct lens *l1, struct lens *l2, int check) {
    struct lens *lens = NULL;
    int consumes_value = l1->consumes_value || l2->consumes_value;
    int recursive = l1->recursive || l2->recursive;
    int ctype_nullable = l1->ctype_nullable && l2->ctype_nullable;

    if (check) {
        struct value *exn = typecheck_concat(info, l1, l2);
        if (exn != NULL)
            return exn;
    }
    if (l1->value && l2->value) {
        return make_exn_value(info, "Multiple stores in concat");
    }
    if (l1->key && l2->key) {
        return make_exn_value(info, "Multiple keys/labels in concat");
    }

    lens = make_lens_binop(L_CONCAT, info, l1, l2, regexp_concat_n);
    lens->consumes_value = consumes_value;
    if (! recursive)
        lens->ctype_nullable = ctype_nullable;
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
    if (! l->recursive)
        lens->atype = subtree_atype(info, l->ktype, l->vtype);
    lens->value = lens->key = 0;
    lens->recursive = l->recursive;
    lens->rec_internal = l->rec_internal;
    if (! l->recursive)
        lens->ctype_nullable = l->ctype_nullable;
    return make_lens_value(lens);
}

struct value *lns_make_star(struct info *info, struct lens *l, int check) {
    struct lens *lens;

    if (check) {
        struct value *exn = typecheck_iter(info, l);
        if (exn != NULL)
            return exn;
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
    lens->recursive = l->recursive;
    lens->rec_internal = l->rec_internal;
    lens->ctype_nullable = 1;
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
        if (exn != NULL)
            return exn;
    }
    lens = make_lens_unop(L_MAYBE, info, l);
    for (int t=0; t < ntypes; t++)
        ltype(lens, t) = regexp_maybe(info, ltype(l, t));
    lens->value = l->value;
    lens->key = l->key;
    lens->recursive = l->recursive;
    lens->rec_internal = l->rec_internal;
    lens->ctype_nullable = 1;
    return make_lens_value(lens);
}

/* Build a square lens as
 *   key REG . lns . del REG MATCHED
 * where MATCHED is whatever the key lens matched (the inability to express
 * this with other lenses makes the square primitve necessary
 */
struct value *lns_make_square(struct info *info,
                              struct regexp *reg,
                              struct lens *lns, int check) {
    struct value *key = NULL, *del = NULL;
    struct value *cnt1 = NULL, *cnt2 = NULL, *res = NULL;
    struct lens *sqr = NULL;

    res = lns_make_prim(L_KEY, ref(info), ref(reg), NULL);
    if (EXN(res))
        goto error;
    key = res;

    res = lns_make_prim(L_DEL, ref(info), ref(reg), NULL);
    if (EXN(res))
        goto error;
    del = res;

    // typechecking is handled when concatenating lenses
    res = lns_make_concat(ref(info), ref(key->lens), ref(lns), check);
    if (EXN(res))
        goto error;
    cnt1 = res;

    res = lns_make_concat(ref(info), ref(cnt1->lens), ref(del->lens), check);
    if (EXN(res))
        goto error;
    cnt2 = res;

    sqr = make_lens_unop(L_SQUARE, ref(info), ref(cnt2->lens));
    ERR_NOMEM(sqr == NULL, info);

    for (int t=0; t < ntypes; t++)
        ltype(sqr, t) = ref(ltype(cnt2->lens, t));
    sqr->recursive = cnt2->lens->recursive;
    sqr->rec_internal = cnt2->lens->rec_internal;
    sqr->consumes_value = cnt2->lens->consumes_value;

    res = make_lens_value(sqr);
    ERR_NOMEM(res == NULL, info);
    sqr = NULL;

 error:
    unref(info, info);
    unref(reg, regexp);
    unref(lns, lens);

    unref(key, value);
    unref(del, value);
    unref(cnt1, value);
    unref(cnt2, value);
    unref(sqr, lens);
    return res;
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
    struct regexp *result = NULL;
    size_t nre_len;
    int ret;

    ret = fa_restrict_alphabet(r->pattern->str, strlen(r->pattern->str),
                               &nre, &nre_len,
                               RESERVED_FROM, RESERVED_TO);
    ERR_NOMEM(ret == REG_ESPACE || ret < 0, r->info);
    BUG_ON(ret != 0, r->info, NULL);
    ensure(nre_len == strlen(nre), r->info);

    ret = regexp_c_locale(&nre, &nre_len);
    ERR_NOMEM(ret < 0, r->info);

    result = make_regexp(r->info, nre, r->nocase);
    nre = NULL;
    BUG_ON(regexp_compile(result) != 0, r->info,
           "Could not compile restricted regexp");
 done:
    free(nre);
    return result;
 error:
    unref(result, regexp);
    goto done;
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
    } else if (tag == L_DEL && string != NULL) {
        int cnt;
        const char *dflt = string->str;
        cnt = regexp_match(regexp, dflt, strlen(dflt), 0, NULL);
        if (cnt != strlen(dflt)) {
            char *s = escape(dflt, -1, RX_ESCAPES);
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
    lens->value = (tag == L_STORE || tag == L_VALUE);
    lens->consumes_value = (tag == L_STORE || tag == L_VALUE);
    lens->atype = regexp_make_empty(info);
    /* Set the ctype */
    if (tag == L_DEL || tag == L_STORE || tag == L_KEY) {
        lens->ctype = ref(regexp);
        lens->ctype_nullable = regexp_matches_empty(lens->ctype);
    } else if (tag == L_LABEL || tag == L_VALUE
               || tag == L_SEQ || tag == L_COUNTER) {
        lens->ctype = regexp_make_empty(info);
        lens->ctype_nullable = 1;
    } else {
        BUG_LENS_TAG(lens);
        goto error;
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
    } else if (tag == L_VALUE) {
        lens->vtype = make_regexp_literal(info, lens->string->str);
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

    if (r1 == NULL || r2 == NULL)
        return NULL;

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

static struct value *
ambig_check(struct info *info, struct fa *fa1, struct fa *fa2,
            enum lens_type typ,  struct lens *l1, struct lens *l2,
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
        char *e_u, *e_up, *e_upv, *e_pv, *e_v;
        char *s1, *s2;

        if (typ == ATYPE) {
            e_u = enc_format(upv, pv - upv);
            e_up = enc_format(upv, v - upv);
            e_upv = enc_format(upv, upv_len);
            e_pv = enc_format(pv, strlen(pv));
            e_v = enc_format(v, strlen(v));
            lns_format_atype(l1, &s1);
            lns_format_atype(l2, &s2);
        } else {
            e_u = escape(upv, pv - upv, RX_ESCAPES);
            e_up = escape(upv, v - upv, RX_ESCAPES);
            e_upv = escape(upv, -1, RX_ESCAPES);
            e_pv = escape(pv, -1, RX_ESCAPES);
            e_v = escape(v, -1, RX_ESCAPES);
            s1 = regexp_escape(ltype(l1, typ));
            s2 = regexp_escape(ltype(l2, typ));
        }
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

static struct value *
ambig_concat_check(struct info *info, const char *msg,
                   enum lens_type typ, struct lens *l1, struct lens *l2) {
    struct fa *fa1 = NULL;
    struct fa *fa2 = NULL;
    struct value *result = NULL;
    struct regexp *r1 = ltype(l1, typ);
    struct regexp *r2 = ltype(l2, typ);

    if (r1 == NULL || r2 == NULL)
        return NULL;

    result = regexp_to_fa(r1, &fa1);
    if (result != NULL)
        goto done;

    result = regexp_to_fa(r2, &fa2);
    if (result != NULL)
        goto done;

    result = ambig_check(info, fa1, fa2, typ, l1, l2, msg, false);
 done:
    fa_free(fa1);
    fa_free(fa2);
    return result;
}

static struct value *typecheck_concat(struct info *info,
                                      struct lens *l1, struct lens *l2) {
    struct value *result = NULL;

    result = ambig_concat_check(info, "ambiguous concatenation",
                                CTYPE, l1, l2);
    if (result == NULL) {
        result = ambig_concat_check(info, "ambiguous tree concatenation",
                                    ATYPE, l1, l2);
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

static struct value *
ambig_iter_check(struct info *info, const char *msg,
                 enum lens_type typ, struct lens *l) {
    struct fa *fas = NULL, *fa = NULL;
    struct value *result = NULL;
    struct regexp *r = ltype(l, typ);

    if (r == NULL)
        return NULL;

    result = regexp_to_fa(r, &fa);
    if (result != NULL)
        goto done;

    fas = fa_iter(fa, 0, -1);

    result = ambig_check(info, fa, fas, typ, l, l, msg, true);

 done:
    fa_free(fa);
    fa_free(fas);
    return result;
}

static struct value *typecheck_iter(struct info *info, struct lens *l) {
    struct value *result = NULL;

    result = ambig_iter_check(info, "ambiguous iteration", CTYPE, l);
    if (result == NULL) {
        result = ambig_iter_check(info, "ambiguous tree iteration", ATYPE, l);
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

    if (l->ctype != NULL && regexp_matches_empty(l->ctype)) {
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
        if (l->atype != NULL && regexp_matches_empty(l->atype)) {
            exn = make_exn_value(ref(info),
               "optional expression matches the empty tree but does not consume a value");
        }
    }
    return exn;
}

void free_lens(struct lens *lens) {
    if (lens == NULL)
        return;
    ensure(lens->ref == 0, lens->info);

    if (debugging("lenses"))
        dump_lens_tree(lens);
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
    case L_VALUE:
        unref(lens->string, string);
        break;
    case L_SUBTREE:
    case L_STAR:
    case L_MAYBE:
    case L_SQUARE:
        unref(lens->child, lens);
        break;
    case L_CONCAT:
    case L_UNION:
        for (int i=0; i < lens->nchildren; i++)
            unref(lens->children[i], lens);
        free(lens->children);
        break;
    case L_REC:
        if (!lens->rec_internal) {
            unref(lens->body, lens);
            jmt_free(lens->jmt);
        }
        break;
    default:
        BUG_LENS_TAG(lens);
        break;
    }

    for (int t=0; t < ntypes; t++)
        unref(ltype(lens, t), regexp);

    unref(lens->info, info);

    free(lens);
 error:
    return;
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

    if (lens->tag == L_REC) {
        jmt_free(lens->jmt);
        lens->jmt = NULL;
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
        assert(eq != NULL);
        slash = strchr(eq, ENC_SLASH_CH);
        assert(slash != NULL);
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

static int lns_format_rec_atype(struct lens *l, char **buf) {
    int r;

    if (l->rec_internal) {
        *buf = strdup("<<rec>>");
        return (*buf == NULL) ? -1 : 0;
    }

    char *c = NULL;
    r = lns_format_atype(l->body, &c);
    if (r < 0)
        return -1;
    r = xasprintf(buf, "<<rec:%s>>", c);
    free(c);
    return (r < 0) ? -1 : 0;
}

int lns_format_atype(struct lens *l, char **buf) {
    *buf = NULL;

    switch(l->tag) {
    case L_DEL:
    case L_STORE:
    case L_KEY:
    case L_LABEL:
    case L_VALUE:
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
    case L_REC:
        return lns_format_rec_atype(l, buf);
        break;
    case L_SQUARE:
        return lns_format_concat_atype(l->child, buf);
        break;
    default:
        BUG_LENS_TAG(l);
        break;
    };
    return -1;
}

/*
 * Recursive lenses
 */
struct value *lns_make_rec(struct info *info) {
    struct lens *l = make_lens(L_REC, info);
    l->recursive = 1;
    l->rec_internal = 1;

    return make_lens_value(l);
}

/* Transform a recursive lens into a recursive transition network
 *
 * First, we transform the lens into context free grammar, considering any
 * nonrecursive lens as a terminal
 *
 * cfg: lens -> nonterminal -> production list
 *
 * cfg(primitive, N) -> N := regexp(primitive)
 * cfg(l1 . l2, N)   -> N := N1 . N2 + cfg(l1, N1) + cfg(l2, N2)
 * cfg(l1 | l2, N)   -> N := N1 | N2 + cfg(l1, N1) + cfg(l2, N2)
 * cfg(l*, N)        -> N := N . N' | eps + cfg(l, N')
 * cfg([ l ], N)     -> N := N' + cfg(l, N')
 *
 * We use the lenses as nonterminals themselves; this also means that our
 * productions are normalized such that the RHS is either a terminal
 * (regexp) or entirely consists of nonterminals
 *
 * In a few places, we need to know that a nonterminal corresponds to a
 * subtree combinator ([ l ]); this is the main reason that the rule (cfg[
 * l ], N) introduces a useless production N := N'.
 *
 * Computing the types for a recursive lens r is (fairly) straightforward,
 * given the above grammar, which we convert to an automaton following
 * http://arxiv.org/abs/cs/9910022; the only complication arises from the
 * subtree combinator, since it can be used in recursive lenses to
 * construct trees of arbitrary depth, but we need to approximate the types
 * of r in a way that fits with our top-down tree automaton in put.c.
 *
 * To handle subtree combinators, remember that the type rules for a lens
 * m = [ l ] are:
 *
 *   m.ktype = NULL
 *   m.vtype = NULL
 *   m.ctype = l.ctype
 *   m.atype = enc(l.ktype, l.vtype)
 *     ( enc is a function regexp -> regexp -> regexp)
 *
 * We compute types for r by modifying its automaton according to
 * Nederhof's paper and reducing it to a regular expression of lenses. This
 * has to happen in the following steps:
 *   r.ktype : approximate by using [ .. ].ktype = NULL
 *   r.vtype : same as r.ktype
 *   r.ctype : approximate by treating [ l ] as l
 *   r.atype : approximate by using r.ktype and r.vtype from above
 *             in lens expressions [ f(r) ]
 */

/* Transitions go to a state and are labeled with a lens. For epsilon
 * transitions, lens may be NULL. When lens is a simple (nonrecursive
 * lens), PROD will be NULL. When we modify the automaton to splice
 * nonterminals in, we remember the production for the nonterminal in PROD.
 */
struct trans {
    struct state  *to;
    struct lens   *lens;
    struct regexp *re;
};

struct state {
    struct state  *next;   /* Linked list for memory management */
    size_t         ntrans;
    struct trans  *trans;
};

/* Productions for lens LENS. Start state START and end state END. If we
   start with START, END is the only accepting state. */
struct prod {
    struct lens  *lens;
    struct state *start;
    struct state *end;
};

/* A recursive transition network used to compute regular approximations
 * to the types */
struct rtn {
    struct info *info;
    size_t        nprod;
    struct prod **prod;
    struct state *states;  /* Linked list through next of all states in all
                              prods; the states for each production are on
                              the part of the list from prod->start to
                              prod->end */
    struct value *exn;
    enum lens_type lens_type;
    unsigned int check : 1;
};

#define RTN_BAIL(rtn) if ((rtn)->exn != NULL ||                     \
                          (rtn)->info->error->code != AUG_NOERROR)  \
                         goto error;

static void free_prod(struct prod *prod) {
    if (prod == NULL)
        return;
    unref(prod->lens, lens);
    free(prod);
}

static void free_rtn(struct rtn *rtn) {
    if (rtn == NULL)
        return;
    for (int i=0; i < rtn->nprod; i++)
        free_prod(rtn->prod[i]);
    free(rtn->prod);
    list_for_each(s, rtn->states) {
        for (int i=0; i < s->ntrans; i++) {
            unref(s->trans[i].lens, lens);
            unref(s->trans[i].re, regexp);
        }
        free(s->trans);
    }
    list_free(rtn->states);
    unref(rtn->info, info);
    unref(rtn->exn, value);
    free(rtn);
}

static struct state *add_state(struct prod *prod) {
    struct state *result = NULL;
    int r;

    r = ALLOC(result);
    ERR_NOMEM(r < 0, prod->lens->info);

    list_cons(prod->start->next, result);
 error:
    return result;
}

static struct trans *add_trans(struct rtn *rtn, struct state *state,
                               struct state *to, struct lens *l) {
    int r;
    struct trans *result = NULL;

    for (int i=0; i < state->ntrans; i++)
        if (state->trans[i].to == to && state->trans[i].lens == l)
            return state->trans + i;

    r = REALLOC_N(state->trans, state->ntrans+1);
    ERR_NOMEM(r < 0, rtn->info);

    result = state->trans + state->ntrans;
    state->ntrans += 1;

    MEMZERO(result, 1);
    result->to = to;
    if (l != NULL) {
        result->lens = ref(l);
        result->re = ref(ltype(l, rtn->lens_type));
    }
 error:
    return result;
}

static struct prod *make_prod(struct rtn *rtn, struct lens *l) {
    struct prod *result = NULL;
    int r;

    r = ALLOC(result);
    ERR_NOMEM(r < 0, l->info);

    result->lens = ref(l);
    r = ALLOC(result->start);
    ERR_NOMEM(r < 0, l->info);

    result->end = add_state(result);
    ERR_BAIL(l->info);

    result->end->next = rtn->states;
    rtn->states = result->start;

    return result;
 error:
    free_prod(result);
    return NULL;
}

static struct prod *prod_for_lens(struct rtn *rtn, struct lens *l) {
    if (l == NULL)
        return NULL;
    for (int i=0; i < rtn->nprod; i++) {
        if (rtn->prod[i]->lens == l)
            return rtn->prod[i];
    }
    return NULL;
}

static void rtn_dot(struct rtn *rtn, const char *stage) {
    FILE *fp;
    int r = 0;

    fp = debug_fopen("rtn_%s_%s.dot", stage, lens_type_names[rtn->lens_type]);
    if (fp == NULL)
        return;

    fprintf(fp, "digraph \"l1\" {\n  rankdir=LR;\n");
    list_for_each(s, rtn->states) {
        char *label = NULL;
        for (int p=0; p < rtn->nprod; p++) {
            if (s == rtn->prod[p]->start) {
                r = xasprintf(&label, "s%d", p);
            } else if (s == rtn->prod[p]->end) {
                r = xasprintf(&label, "e%d", p);
            }
            ERR_NOMEM(r < 0, rtn->info);
        }
        if (label == NULL) {
            r = xasprintf(&label, "%p", s);
            ERR_NOMEM(r < 0, rtn->info);
        }
        fprintf(fp, "  n%p [label = \"%s\"];\n", s, label == NULL ? "" : label);
        FREE(label);
        for (int i=0; i < s->ntrans; i++) {
            fprintf(fp, "  n%p -> n%p", s, s->trans[i].to);
            if (s->trans[i].re != NULL) {
                label = regexp_escape(s->trans[i].re);
                for (char *t = label; *t; t++)
                    if (*t == '\\')
                        *t = '~';
                fprintf(fp, " [ label = \"%s\" ]", label);
                FREE(label);
            }
            fprintf(fp, ";\n");
        }
    }
 error:
    fprintf(fp, "}\n");
    fclose(fp);
}

/* Add transitions to RTN corresponding to cfg(l, N) */
static void rtn_rules(struct rtn *rtn, struct lens *l) {
    if (! l->recursive)
        return;

    struct prod *prod = prod_for_lens(rtn, l);
    if (prod != NULL)
        return;

    int r = REALLOC_N(rtn->prod, rtn->nprod+1);
    ERR_NOMEM(r < 0, l->info);

    prod =  make_prod(rtn, l);
    rtn->prod[rtn->nprod] = prod;
    RTN_BAIL(rtn);
    rtn->nprod += 1;

    struct state *start = prod->start;

    switch (l->tag) {
    case L_UNION:
        /* cfg(l1|..|ln, N) -> N := N1 | N2 | ... | Nn */
        for (int i=0; i < l->nchildren; i++) {
            add_trans(rtn, start, prod->end, l->children[i]);
            RTN_BAIL(rtn);
            rtn_rules(rtn, l->children[i]);
            RTN_BAIL(rtn);
        }
        break;
    case L_CONCAT:
        /* cfg(l1 . l2 ... ln, N) -> N := N1 . N2 ... Nn */
        for (int i=0; i < l->nchildren-1; i++) {
            struct state *s = add_state(prod);
            RTN_BAIL(rtn);
            add_trans(rtn, start, s, l->children[i]);
            RTN_BAIL(rtn);
            start = s;
            rtn_rules(rtn, l->children[i]);
            RTN_BAIL(rtn);
        }
        {
            struct lens *c = l->children[l->nchildren - 1];
            add_trans(rtn, start, prod->end, c);
            RTN_BAIL(rtn);
            rtn_rules(rtn, c);
            RTN_BAIL(rtn);
        }
        break;
    case L_STAR: {
        /* cfg(l*, N) -> N := N . N' | eps */
        struct state *s = add_state(prod);
        RTN_BAIL(rtn);
        add_trans(rtn, start, s, l);
        RTN_BAIL(rtn);
        add_trans(rtn, s, prod->end, l->child);
        RTN_BAIL(rtn);
        add_trans(rtn, start, prod->end, NULL);
        RTN_BAIL(rtn);
        rtn_rules(rtn, l->child);
        RTN_BAIL(rtn);
        break;
    }
    case L_SUBTREE:
        switch (rtn->lens_type) {
        case KTYPE:
        case VTYPE:
            /* cfg([ l ], N) -> N := eps */
            add_trans(rtn, start, prod->end, NULL);
            break;
        case CTYPE:
            /* cfg([ l ], N) -> N := N' plus cfg(l, N') */
            add_trans(rtn, start, prod->end, l->child);
            RTN_BAIL(rtn);
            rtn_rules(rtn, l->child);
            RTN_BAIL(rtn);
            break;
        case ATYPE: {
            /* At this point, we have propagated ktype and vtype */
            /* cfg([ l ], N) -> N := enc(l->ktype, l->vtype) */
            struct trans *t = add_trans(rtn, start, prod->end, NULL);
            RTN_BAIL(rtn);
            t->re = subtree_atype(l->info, l->child->ktype, l->child->vtype);
            break;
        }
        default:
            BUG_ON(true, rtn->info, "Unexpected lens type %d", rtn->lens_type);
            break;
        }
        break;
    case L_MAYBE:
        /* cfg(l?, N) -> N := N' | eps plus cfg(l, N') */
        add_trans(rtn, start, prod->end, l->child);
        RTN_BAIL(rtn);
        add_trans(rtn, start, prod->end, NULL);
        RTN_BAIL(rtn);
        rtn_rules(rtn, l->child);
        RTN_BAIL(rtn);
        break;
    case L_REC:
        /* cfg(l, N) -> N := N' plus cfg(l->body, N') */
        add_trans(rtn, start, prod->end, l->body);
        RTN_BAIL(rtn);
        rtn_rules(rtn, l->body);
        RTN_BAIL(rtn);
        break;
    default:
        BUG_LENS_TAG(l);
        break;
    }
 error:
    return;
}

/* Replace transition t with two epsilon transitions s => p->start and
 * p->end => s->trans[i].to where s is the start of t. Instead of adding
 * epsilon transitions, we expand the epsilon transitions.
 */
static void prod_splice(struct rtn *rtn,
                        struct prod *from, struct prod *to, struct trans *t) {

    add_trans(rtn, to->end, t->to, NULL);
    ERR_BAIL(from->lens->info);
    t->to = to->start;
    unref(t->re, regexp);

 error:
    return;
}

static void rtn_splice(struct rtn *rtn, struct prod *prod) {
    for (struct state *s = prod->start; s != prod->end; s = s->next) {
        for (int i=0; i < s->ntrans; i++) {
            struct prod *p = prod_for_lens(rtn, s->trans[i].lens);
            if (p != NULL) {
                prod_splice(rtn, prod, p, s->trans+i);
                RTN_BAIL(rtn);
            }
        }
    }
 error:
    return;
}

static struct rtn *rtn_build(struct lens *rec, enum lens_type lt) {
    int r;
    struct rtn *rtn;

    r = ALLOC(rtn);
    ERR_NOMEM(r < 0, rec->info);

    rtn->info = ref(rec->info);
    rtn->lens_type = lt;

    rtn_rules(rtn, rec);
    RTN_BAIL(rtn);
    if (debugging("cf.approx"))
        rtn_dot(rtn, "10-rules");

    for (int i=0; i < rtn->nprod; i++) {
        rtn_splice(rtn, rtn->prod[i]);
        RTN_BAIL(rtn);
    }
    if (debugging("cf.approx"))
        rtn_dot(rtn, "11-splice");

 error:
    return rtn;
}

/* Compare transitions lexicographically by (to, lens) */
static int trans_to_cmp(const void *v1, const void *v2) {
    const struct trans *t1 = v1;
    const struct trans *t2 = v2;

    if (t1->to != t2->to)
        return (t1->to < t2->to) ? -1 : 1;

    if (t1->lens == t2->lens)
        return 0;
    return (t1->lens < t2->lens) ? -1 : 1;
}

/* Collapse a transition S1 -> S -> S2 by adding a transition S1 -> S2 with
 * lens R1 . (LOOP)* . R2 | R3 where R3 is the regexp on the possibly
 * existing transition S1 -> S2. If LOOP is NULL or R3 does not exist,
 * label the transition with a simplified regexp by treating NULL as
 * epsilon */
static void collapse_trans(struct rtn *rtn,
                           struct state *s1, struct state *s2,
                           struct regexp *r1, struct regexp *loop,
                           struct regexp *r2) {

    struct trans *t = NULL;
    struct regexp *r = NULL;

    for (int i=0; i < s1->ntrans; i++) {
        if (s1->trans[i].to == s2) {
            t = s1->trans + i;
            break;
        }
    }

    /* Set R = R1 . (LOOP)* . R2, treating NULL's as epsilon */
    if (loop == NULL) {
        if (r1 == NULL)
            r = ref(r2);
        else if (r2 == NULL)
            r = ref(r1);
        else
            r = regexp_concat(rtn->info, r1, r2);
    } else {
        struct regexp *s = regexp_iter(rtn->info, loop, 0, -1);
        ERR_NOMEM(s == NULL, rtn->info);
        struct regexp *c = NULL;
        if (r1 == NULL) {
            c = s;
            s = NULL;
        } else {
            c = regexp_concat(rtn->info, r1, s);
            unref(s, regexp);
            ERR_NOMEM(c == NULL, rtn->info);
        }
        if (r2 == NULL) {
            r = c;
            c = NULL;
        } else {
            r = regexp_concat(rtn->info, c, r2);
            unref(c, regexp);
            ERR_NOMEM(r == NULL, rtn->info);
        }
    }

    if (t == NULL) {
        t = add_trans(rtn, s1, s2, NULL);
        ERR_NOMEM(t == NULL, rtn->info);
        t->re = r;
    } else if (t->re == NULL) {
        if (r == NULL || regexp_matches_empty(r))
            t->re = r;
        else {
            t->re = regexp_maybe(rtn->info, r);
            unref(r, regexp);
            ERR_NOMEM(t->re == NULL, rtn->info);
        }
    } else if (r == NULL) {
        if (!regexp_matches_empty(t->re)) {
            r = regexp_maybe(rtn->info, t->re);
            unref(t->re, regexp);
            t->re = r;
            ERR_NOMEM(r == NULL, rtn->info);
        }
    } else {
        struct regexp *u = regexp_union(rtn->info, r, t->re);
        unref(r, regexp);
        unref(t->re, regexp);
        t->re = u;
        ERR_NOMEM(u == NULL, rtn->info);
    }

    return;
 error:
    rtn->exn = exn_error();
    return;
}

/* Reduce the automaton with start state rprod->start and only accepting
 * state rprod->end so that we have a single transition rprod->start =>
 * rprod->end labelled with the overall approximating regexp for the
 * automaton.
 *
 * This is the same algorithm as fa_as_regexp in fa.c
 */
static struct regexp *rtn_reduce(struct rtn *rtn, struct lens *rec) {
    struct prod *prod = prod_for_lens(rtn, rec);
    int r;

    ERR_THROW(prod == NULL, rtn->info, AUG_EINTERNAL,
              "No production for recursive lens");

    /* Eliminate epsilon transitions and turn transitions between the same
     * two states into a regexp union */
    list_for_each(s, rtn->states) {
        qsort(s->trans, s->ntrans, sizeof(*s->trans), trans_to_cmp);
        for (int i=0; i < s->ntrans; i++) {
            int j = i+1;
            for (;j < s->ntrans && s->trans[i].to == s->trans[j].to;
                 j++);
            if (j > i+1) {
                struct regexp *u, **v;
                r = ALLOC_N(v, j - i);
                ERR_NOMEM(r < 0, rtn->info);
                for (int k=i; k < j; k++)
                    v[k-i] = s->trans[k].re;
                u = regexp_union_n(rtn->info, j - i, v);
                if (u == NULL) {
                    // FIXME: The calling convention for regexp_union_n
                    // is bad, since we can't distinguish between alloc
                    // failure and unioning all NULL's
                    for (int k=0; k < j-i; k++)
                        if (v[k] != NULL) {
                            FREE(v);
                            ERR_NOMEM(true, rtn->info);
                        }
                }
                FREE(v);
                for (int k=i; k < j; k++) {
                    unref(s->trans[k].lens, lens);
                    unref(s->trans[k].re, regexp);
                }
                s->trans[i].re = u;
                MEMMOVE(s->trans + (i+1),
                        s->trans + j,
                        s->ntrans - j);
                s->ntrans -= j - (i + 1);
            }
        }
    }

    /* Introduce new start and end states with epsilon transitions to/from
     * the old start and end states */
    struct state *end = NULL;
    struct state *start = NULL;
    if (ALLOC(start) < 0 || ALLOC(end) < 0) {
        FREE(start);
        FREE(end);
        ERR_NOMEM(true, rtn->info);
    }
    list_insert_before(start, prod->start, rtn->states);
    end->next = prod->end->next;
    prod->end->next = end;

    add_trans(rtn, start, prod->start, NULL);
    RTN_BAIL(rtn);
    add_trans(rtn, prod->end, end, NULL);
    RTN_BAIL(rtn);

    prod->start = start;
    prod->end = end;

    /* Eliminate states S (except for INI and FIN) one by one:
     *     Let LOOP the regexp for the transition S -> S if it exists, epsilon
     *     otherwise.
     *     For all S1, S2 different from S with S1 -> S -> S2
     *       Let R1 the regexp of S1 -> S
     *           R2 the regexp of S -> S2
     *           R3 the regexp of S1 -> S2 (or the regexp matching nothing
     *                                      if no such transition)
     *        set the regexp on the transition S1 -> S2 to
     *          R1 . (LOOP)* . R2 | R3 */
    // FIXME: This does not go over all states
    list_for_each(s, rtn->states) {
        if (s == prod->end || s == prod->start)
            continue;
        struct regexp *loop = NULL;
        for (int i=0; i < s->ntrans; i++) {
            if (s == s->trans[i].to) {
                ensure(loop == NULL, rtn->info);
                loop = s->trans[i].re;
            }
        }
        list_for_each(s1, rtn->states) {
            if (s == s1)
                continue;
            for (int t1=0; t1 < s1->ntrans; t1++) {
                if (s == s1->trans[t1].to) {
                    for (int t2=0; t2 < s->ntrans; t2++) {
                        struct state *s2 = s->trans[t2].to;
                        if (s2 == s)
                            continue;
                        collapse_trans(rtn, s1, s2,
                                       s1->trans[t1].re, loop,
                                       s->trans[t2].re);
                        RTN_BAIL(rtn);
                    }
                }
            }
        }
    }

    /* Find the overall regexp */
    struct regexp *result = NULL;
    for (int i=0; i < prod->start->ntrans; i++) {
        if (prod->start->trans[i].to == prod->end) {
            ensure(result == NULL, rtn->info);
            result = ref(prod->start->trans[i].re);
        }
    }
    return result;
 error:
    return NULL;
}

static void propagate_type(struct lens *l, enum lens_type lt) {
    struct regexp **types = NULL;
    int r;

    if (! l->recursive || ltype(l, lt) != NULL)
        return;

    switch(l->tag) {
    case L_CONCAT:
        r = ALLOC_N(types, l->nchildren);
        ERR_NOMEM(r < 0, l->info);
        for (int i=0; i < l->nchildren; i++) {
            propagate_type(l->children[i], lt);
            types[i] = ltype(l->children[i], lt);
        }
        ltype(l, lt) = regexp_concat_n(l->info, l->nchildren, types);
        FREE(types);
        break;
    case L_UNION:
        r = ALLOC_N(types, l->nchildren);
        ERR_NOMEM(r < 0, l->info);
        for (int i=0; i < l->nchildren; i++) {
            propagate_type(l->children[i], lt);
            types[i] = ltype(l->children[i], lt);
        }
        ltype(l, lt) = regexp_union_n(l->info, l->nchildren, types);
        FREE(types);
        break;
    case L_SUBTREE:
        propagate_type(l->child, lt);
        if (lt == ATYPE)
            l->atype = subtree_atype(l->info, l->child->ktype, l->child->vtype);
        if (lt == CTYPE)
            l->ctype = ref(l->child->ctype);
        break;
    case L_STAR:
        propagate_type(l->child, lt);
        ltype(l, lt) = regexp_iter(l->info, ltype(l->child, lt), 0, -1);
        break;
    case L_MAYBE:
        propagate_type(l->child, lt);
        ltype(l, lt) = regexp_maybe(l->info, ltype(l->child, lt));
        break;
    case L_REC:
        /* Nothing to do */
        break;
    case L_SQUARE:
        propagate_type(l->child, lt);
        ltype(l, lt) = ref(ltype(l->child, lt));
        break;
    default:
        BUG_LENS_TAG(l);
        break;
    }

 error:
    FREE(types);
}

static struct value *typecheck(struct lens *l, int check);

typedef struct value *typecheck_n_make(struct info *,
                                       struct lens *, struct lens *, int);

static struct info *merge_info(struct info *i1, struct info *i2) {
    struct info *info;
    make_ref(info);
    ERR_NOMEM(info == NULL, i1);

    info->filename = ref(i1->filename);
    info->first_line = i1->first_line;
    info->first_column = i1->first_column;
    info->last_line    = i2->last_line;
    info->last_column  = i2->last_column;
    info->error        = i1->error;
    return info;

 error:
    unref(info, info);
    return NULL;
}

static struct value *typecheck_n(struct lens *l,
                                 typecheck_n_make *make, int check) {
    struct value *exn = NULL;
    struct lens *acc = NULL;

    ensure(l->tag == L_CONCAT || l->tag == L_UNION, l->info);
    for (int i=0; i < l->nchildren; i++) {
        exn = typecheck(l->children[i], check);
        if (exn != NULL)
            goto error;
    }
    acc = ref(l->children[0]);
    for (int i=1; i < l->nchildren; i++) {
        struct info *info = merge_info(acc->info, l->children[i]->info);
        ERR_BAIL(acc->info);
        exn = (*make)(info, acc, ref(l->children[i]), check);
        if (EXN(exn))
            goto error;
        ensure(exn->tag == V_LENS, l->info);
        acc = ref(exn->lens);
        unref(exn, value);
    }
    l->value = acc->value;
    l->key = acc->key;
 error:
    unref(acc, lens);
    return exn;
}

static struct value *typecheck(struct lens *l, int check) {
    struct value *exn = NULL;

    /* Nonrecursive lenses are typechecked at build time */
    if (! l->recursive)
        return NULL;

    switch(l->tag) {
    case L_CONCAT:
        exn = typecheck_n(l, lns_make_concat, check);
        break;
    case L_UNION:
        exn = typecheck_n(l, lns_make_union, check);
        break;
    case L_SUBTREE:
    case L_SQUARE:
        exn = typecheck(l->child, check);
        break;
    case L_STAR:
        if (check)
            exn = typecheck_iter(l->info, l->child);
        if (exn == NULL && l->value)
            exn = make_exn_value(l->info, "Multiple stores in iteration");
        if (exn == NULL && l->key)
            exn = make_exn_value(l->info, "Multiple keys/labels in iteration");
        break;
    case L_MAYBE:
        if (check)
            exn = typecheck_maybe(l->info, l->child);
        l->key = l->child->key;
        l->value = l->child->value;
        break;
    case L_REC:
        /* Nothing to do */
        break;
    default:
        BUG_LENS_TAG(l);
        break;
    }

    return exn;
}

static struct value *rtn_approx(struct lens *rec, enum lens_type lt) {
    struct rtn *rtn = NULL;
    struct value *result = NULL;

    rtn = rtn_build(rec, lt);
    RTN_BAIL(rtn);
    ltype(rec, lt) = rtn_reduce(rtn, rec);
    RTN_BAIL(rtn);
    if (debugging("cf.approx"))
        rtn_dot(rtn, "50-reduce");

    propagate_type(rec->body, lt);
    ERR_BAIL(rec->info);

 done:
    free_rtn(rtn);

    if (debugging("cf.approx")) {
        printf("approx %s  => ", lens_type_names[lt]);
        print_regexp(stdout, ltype(rec, lt));
        printf("\n");
    }

    return result;
 error:
    if (rtn->exn == NULL)
        result = exn_error();
    else
        result = ref(rtn->exn);
    goto done;
}

static struct value *
exn_multiple_epsilons(struct lens *lens,
                      struct lens *l1, struct lens *l2) {
    char *fi = NULL;
    struct value *exn = NULL;

    exn = make_exn_value(ref(lens->info),
                         "more than one nullable branch in a union");
    fi = format_info(l1->info);
    exn_printf_line(exn, "First nullable lens: %s", fi);
    FREE(fi);

    fi = format_info(l2->info);
    exn_printf_line(exn, "Second nullable lens: %s", fi);
    FREE(fi);

    return exn;
}

/* Update lens->ctype_nullable and return 1 if there was a change,
 * 0 if there was none */
static int ctype_nullable(struct lens *lens, struct value **exn) {
    int nullable = 0;
    int ret = 0;
    struct lens *null_lens = NULL;

    if (! lens->recursive)
        return 0;

    switch(lens->tag) {
    case L_CONCAT:
        nullable = 1;
        for (int i=0; i < lens->nchildren; i++) {
            if (ctype_nullable(lens->children[i], exn))
                ret = 1;
            if (! lens->children[i]->ctype_nullable)
                nullable = 0;
        }
        break;
    case L_UNION:
        for (int i=0; i < lens->nchildren; i++) {
            if (ctype_nullable(lens->children[i], exn))
                ret = 1;
            if (lens->children[i]->ctype_nullable) {
                if (nullable) {
                    *exn = exn_multiple_epsilons(lens, null_lens,
                                                 lens->children[i]);
                    return 0;
                }
                nullable = 1;
                null_lens = lens->children[i];
            }
        }
        break;
    case L_SUBTREE:
    case L_SQUARE:
        ret = ctype_nullable(lens->child, exn);
        nullable = lens->child->ctype_nullable;
        break;
    case L_STAR:
    case L_MAYBE:
        nullable = 1;
        break;
    case L_REC:
        nullable = lens->body->ctype_nullable;
        break;
    default:
        BUG_LENS_TAG(lens);
        break;
    }
    if (*exn != NULL)
        return 0;
    if (nullable != lens->ctype_nullable) {
        ret = 1;
        lens->ctype_nullable = nullable;
    }
    return ret;
}

struct value *lns_check_rec(struct info *info,
                            struct lens *body, struct lens *rec,
                            int check) {
    /* The types in the order of approximation */
    static const enum lens_type types[] = { KTYPE, VTYPE, ATYPE };
    struct value *result = NULL;

    ensure(rec->tag == L_REC, info);
    ensure(rec->rec_internal, info);

    /* The user might have written down a regular lens with 'let rec' */
    if (! body->recursive) {
        result = make_lens_value(ref(body));
        ERR_NOMEM(result == NULL, info);
        return result;
    }

    /* To help memory management, we avoid the cycle inherent ina recursive
     * lens by using two instances of an L_REC lens. One is marked with
     * rec_internal, and used inside the body of the lens. The other is the
     * "toplevel" which receives external references.
     *
     * The internal instance of the recursive lens is REC, the external one
     * is TOP, constructed below
     */
    rec->body = body;                          /* REC does not own BODY */

    for (int i=0; i < ARRAY_CARDINALITY(types); i++) {
        result = rtn_approx(rec, types[i]);
        ERR_BAIL(info);
    }

    if (rec->atype == NULL) {
        result = make_exn_value(ref(rec->info),
        "recursive lens generates the empty language for its %s",
         rec->ctype == NULL ? "ctype" : "atype");
        goto error;
    }

    rec->key = rec->body->key;
    rec->value = rec->body->value;
    rec->consumes_value = rec->body->consumes_value;

    while(ctype_nullable(rec->body, &result));
    if (result != NULL)
        goto error;
    rec->ctype_nullable = rec->body->ctype_nullable;

    result = typecheck(rec->body, check);
    if (result != NULL)
        goto error;

    result = lns_make_rec(ref(rec->info));
    struct lens *top = result->lens;
    for (int t=0; t < ntypes; t++)
        ltype(top, t) = ref(ltype(rec, t));
    top->value = rec->value;
    top->key = rec->key;
    top->consumes_value = rec->consumes_value;
    top->ctype_nullable = rec->ctype_nullable;
    top->body = ref(body);
    top->alias = rec;
    top->rec_internal = 0;
    rec->alias = top;

    top->jmt = jmt_build(top);
    ERR_BAIL(info);

    return result;
 error:
    if (result != NULL && result->tag != V_EXN)
        unref(result, value);
    if (result == NULL)
        result = exn_error();
    return result;
}

#if ENABLE_DEBUG
void dump_lens_tree(struct lens *lens){
    static int count = 0;
    FILE *fp;

    fp = debug_fopen("lens_%02d_%s.dot", count++, ltag(lens));
    if (fp == NULL)
        return;

    fprintf(fp, "digraph \"%s\" {\n", "lens");
    dump_lens(fp, lens);
    fprintf(fp, "}\n");

    fclose(fp);
}

void dump_lens(FILE *out, struct lens *lens){
    int i = 0;
    struct regexp *re;

    fprintf(out, "\"%p\" [ shape = box, label = \"%s\\n", lens, ltag(lens));

    for (int t=0; t < ntypes; t++) {
        re = ltype(lens, t);
        if (re == NULL)
            continue;
        fprintf(out, "%s=",lens_type_names[t]);
        print_regexp(out, re);
        fprintf(out, "\\n");
    }

    fprintf(out, "recursive=%x\\n", lens->recursive);
    fprintf(out, "rec_internal=%x\\n", lens->rec_internal);
    fprintf(out, "consumes_value=%x\\n", lens->consumes_value);
    fprintf(out, "ctype_nullable=%x\\n", lens->ctype_nullable);
    fprintf(out, "\"];\n");
    switch(lens->tag){
    case L_DEL:
        break;
    case L_STORE:
        break;
    case L_VALUE:
        break;
    case L_KEY:
        break;
    case L_LABEL:
        break;
    case L_SEQ:
        break;
    case L_COUNTER:
        break;
    case L_CONCAT:
        for(i = 0; i<lens->nchildren;i++){
            fprintf(out, "\"%p\" -> \"%p\"\n", lens, lens->children[i]);
            dump_lens(out, lens->children[i]);
        }
        break;
    case L_UNION:
        for(i = 0; i<lens->nchildren;i++){
            fprintf(out, "\"%p\" -> \"%p\"\n", lens, lens->children[i]);
            dump_lens(out, lens->children[i]);
        }
        break;
    case L_SUBTREE:
        fprintf(out, "\"%p\" -> \"%p\"\n", lens, lens->child);
        dump_lens(out, lens->child);
        break;
    case L_STAR:
        fprintf(out, "\"%p\" -> \"%p\"\n", lens, lens->child);
        dump_lens(out, lens->child);

        break;
    case L_MAYBE:
        fprintf(out, "\"%p\" -> \"%p\"\n", lens, lens->child);
        dump_lens(out, lens->child);

        break;
    case L_REC:
        if (lens->rec_internal == 0){
            fprintf(out, "\"%p\" -> \"%p\"\n", lens, lens->child);
            dump_lens(out, lens->body);
        }
        break;
    case L_SQUARE:
        fprintf(out, "\"%p\" -> \"%p\"\n", lens, lens->child);
        dump_lens(out, lens->child);
        break;
    default:
        fprintf(out, "ERROR\n");
        break;
    }
}
#endif

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
