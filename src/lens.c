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

#include "lens.h"
#include "memory.h"

static struct value * typecheck_union(struct info *,
                                      struct lens *l1, struct lens *l2);
static struct value *typecheck_concat(struct info *,
                                      struct lens *l1, struct lens *l2);
static struct value *typecheck_iter(struct info *info, struct lens *l);
static struct value *typecheck_maybe(struct info *info, struct lens *l);

static struct regexp *lns_key_regexp(struct lens *l, struct value **exn);
static struct regexp *make_key_regexp(struct info *info, const char *pat);

/* Lens names for pretty printing */
static const char *const tags[] = {
    "del", "store", "key", "label", "seq", "counter", "concat", "union",
    "subtree", "star", "maybe"
};

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

    for (int i=0; i < lens->nchildren; i++)
        types[i] = lens->children[i]->ctype;
    lens->ctype = (*combinator)(info, lens->nchildren, types);

    for (int i=0; i < lens->nchildren; i++)
        types[i] = lens->children[i]->atype;
    lens->atype = (*combinator)(info, lens->nchildren, types);

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

    if (check) {
        struct value *exn = typecheck_union(info, l1, l2);
        if (exn != NULL)
            return exn;
    }

    lens = make_lens_binop(L_UNION, info, l1, l2, regexp_union_n);
    lens->consumes_value = l1->consumes_value && l2->consumes_value;
    return make_lens_value(lens);
}

struct value *lns_make_concat(struct info *info,
                              struct lens *l1, struct lens *l2, int check) {
    struct lens *lens = NULL;

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
    lens->consumes_value = l1->consumes_value || l2->consumes_value;
    return make_lens_value(lens);
}

struct value *lns_make_subtree(struct info *info, struct lens *l) {
    struct lens *lens;
    struct value *exn;
    struct regexp *atype;

    atype = lns_key_regexp(l, &exn);
    if (exn != NULL)
        return exn;

    lens = make_lens_unop(L_SUBTREE, info, l);
    lens->ctype = ref(l->ctype);
    lens->atype = atype;
    lens->value = lens->key = 0;
    if (lens->atype == NULL)
        lens->atype = make_key_regexp(info, "");
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
    lens->ctype = regexp_iter(info, l->ctype, 0, -1);
    lens->atype = regexp_iter(info, l->atype, 0, -1);
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
    lens->ctype = regexp_maybe(info, l->ctype);
    lens->atype = regexp_maybe(info, l->atype);
    lens->value = l->value;
    lens->key = l->key;
    return make_lens_value(lens);
}

/*
 * Lens primitives
 */
struct value *lns_make_prim(enum lens_tag tag, struct info *info,
                            struct regexp *regexp, struct string *string) {
    struct lens *lens = NULL;
    struct value *exn = NULL;
    fa_t fa_slash = NULL;
    fa_t fa_key = NULL;
    fa_t fa_isect = NULL;

    /* Typecheck */
    if (tag == L_KEY) {
        int error = fa_compile("(.|\n)*/(.|\n)*", &fa_slash);
        if (error != REG_NOERROR) {
            exn = make_exn_value(info,
                                 "unexpected error from fa_compile %d", error);
            goto error;
        }
        fa_key = regexp_to_fa(regexp);
        if (fa_key == NULL) {
            exn = make_exn_value(info, "fa_compile of key failed");
            goto error;
        }
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
            char *r = escape(regexp->pattern->str, -1);
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
    if (tag == L_DEL || tag == L_STORE || tag == L_KEY) {
        lens->ctype = ref(regexp);
    } else if (tag == L_LABEL || tag == L_SEQ || tag == L_COUNTER) {
        lens->ctype = regexp_make_empty(info);
    } else {
        assert(0);
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
static struct value *disjoint_check(struct info *info, const char *msg,
                                    struct regexp *r1, struct regexp *r2) {
    fa_t fa1 = regexp_to_fa(r1);
    fa_t fa2 = regexp_to_fa(r2);
    fa_t fa = NULL;
    struct value *exn = NULL;

    if (fa1 == NULL || fa2 == NULL) {
        fa_free(fa1);
        fa_free(fa2);
        return make_exn_value(ref(info),
              "internal error: compile in disjoint_check failed");
    }

    fa = fa_intersect(fa1, fa2);
    if (! fa_is_basic(fa, FA_EMPTY)) {
        char *xmpl = fa_example(fa);
        exn = make_exn_value(ref(info),
                             "overlapping lenses in %s", msg);

        exn_printf_line(exn, "Example matched by both: '%s'", xmpl);
        free(xmpl);
    }

    fa_free(fa);
    fa_free(fa1);
    fa_free(fa2);

    return exn;
}

static struct value *typecheck_union(struct info *info,
                                     struct lens *l1, struct lens *l2) {
    struct value *exn = NULL;

    exn = disjoint_check(info, "union.get", l1->ctype, l2->ctype);
    if (exn == NULL) {
        exn = disjoint_check(info, "tree union.put", l1->atype, l2->atype);
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

static struct value *ambig_check(struct info *info, fa_t fa1, fa_t fa2,
                                 const char *msg) {
    char *upv, *pv, *v;
    upv = fa_ambig_example(fa1, fa2, &pv, &v);
    struct value *exn = NULL;

    if (upv != NULL) {
        char *e_u = escape(upv, pv - upv);
        char *e_up = escape(upv, v - upv);
        char *e_upv = escape(upv, -1);
        char *e_pv = escape(pv, -1);
        char *e_v = escape(v, -1);
        exn = make_exn_value(ref(info), "%s", msg);
        exn_printf_line(exn, "  '%s' can be split into", e_upv);
        exn_printf_line(exn, "  '%s|=|%s'\n", e_u, e_pv);
        exn_printf_line(exn, " and");
        exn_printf_line(exn, "  '%s|=|%s'\n", e_up, e_v);
        free(e_u);
        free(e_up);
        free(e_upv);
        free(e_pv);
        free(e_v);
    }
    free(upv);
    return exn;
}

static struct value *ambig_concat_check(struct info *info, const char *msg,
                                        struct regexp *r1, struct regexp *r2) {
    fa_t fa1 = regexp_to_fa(r1);
    fa_t fa2 = regexp_to_fa(r2);
    struct value *result = NULL;

    if (fa1 == NULL || fa2 == NULL) {
        fa_free(fa1);
        fa_free(fa2);
        return make_exn_value(ref(info), "Internal error: regexp_to_fa failed");
    }

    result = ambig_check(info, fa1, fa2, msg);
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
    fa_t fas, fa;
    struct value *result = NULL;

    fa = regexp_to_fa(r);
    fas = fa_iter(fa, 0, -1);

    result = ambig_check(info, fa, fas, msg);

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
    fa_t fa, eps;
    struct value *exn = NULL;

    fa = regexp_to_fa(l->ctype);
    if (fa == NULL)
        return make_exn_value(ref(info), "Internal error: regexp_to_fa failed");

    eps = fa_make_basic(FA_EPSILON);
    if (fa_contains(eps, fa)) {
        exn = make_exn_value(ref(info),
                "illegal optional expression: /%s/ matches the empty word",
                l->ctype->pattern->str);
    }
    fa_free(fa);

    /* Typecheck the put direction; the check passes if
       (1) the atype does not match the empty string, because we can tell
           from looking at tree nodes whether L should be applied or not
       (2) L handles a value; with that, we know whether to apply L or not
           depending on whether the current node has a non NULL value or not
    */
    if (exn == NULL && ! l->consumes_value) {
        fa = regexp_to_fa(l->atype);
        if (fa == NULL)
            return make_exn_value(ref(info),
                                  "Internal error: regexp_to_fa failed");

        eps = fa_make_basic(FA_EPSILON);
        if (fa_contains(eps, fa)) {
            exn = make_exn_value(ref(info),
                                 "optional expression matches the empty tree");
        }
        fa_free(fa);
    }
    fa_free(eps);
    return exn;
}

static struct regexp *make_key_regexp(struct info *info, const char *pat) {
    struct regexp *regexp;
    size_t len = strlen(pat) + 4;

    make_ref(regexp);
    make_ref(regexp->pattern);
    regexp->info = ref(info);
    CALLOC(regexp->pattern->str, len);
    snprintf(regexp->pattern->str, len, "(%s)/", pat);
    return regexp;
}

static struct regexp *make_regexp_from_string(struct info *info,
                                              struct string *string) {
    struct regexp *r;
    make_ref(r);
    if (r != NULL) {
        r->info = ref(info);
        r->pattern = ref(string);
    }
    return r;
}

/* Calculate the regexp that matches the labels if the trees that L can
   generate.

   We have some headache here because of the behavior of STORE: since STORE
   creates a tree with no label (a leaf, really), its key regexp should be
   "/", but only of there is no KEY or LABEL statement that fills in the
   label of the tree that STORE created.
 */
static struct regexp *lns_key_regexp(struct lens *l, struct value **exn) {
    static const struct string digits_string = {
        .ref = REF_MAX, .str = (char *) "[0-9]+/"
    };
    static const struct string *const digits_pat = &digits_string;

    *exn = NULL;
    switch(l->tag) {
    case L_STORE:
    case L_DEL:
    case L_COUNTER:
        return NULL;
    case L_SEQ:
        return make_regexp_from_string(l->info, (struct string *) digits_pat);
    case L_KEY:
        return make_key_regexp(l->info, l->regexp->pattern->str);
    case L_LABEL:
        {
            struct regexp *r = make_regexp_literal(l->info, l->string->str);
            if (r == NULL)
                return NULL;
            if (REALLOC_N(r->pattern->str, strlen(r->pattern->str) + 2) == -1) {
                unref(r, regexp);
                return NULL;
            }
            strcat(r->pattern->str, "/");
            return r;
        }
    case L_CONCAT:
        {
            struct regexp *k = NULL;
            for (int i=0; i < l->nchildren; i++) {
                struct regexp *r = lns_key_regexp(l->children[i], exn);
                if (*exn != NULL) {
                    free_regexp(k);
                    return NULL;
                }
                if (r != NULL) {
                    if (k != NULL) {
                        *exn = make_exn_value(ref(l->info),
                                              "More than one key");
                        unref(r, regexp);
                        unref(k, regexp);
                        return NULL;
                    } else {
                        k = r;
                    }
                }
            }
            return k;
        }
        break;
    case L_UNION:
        {
            struct regexp *k = NULL;
            for (int i=0; i < l->nchildren; i++) {
                struct regexp *r = lns_key_regexp(l->children[i], exn);
                if (*exn != NULL)
                    return NULL;
                if (k == NULL) {
                    k = r;
                } else {
                    struct regexp *u = regexp_union(l->info, k, r);
                    unref(k, regexp);
                    unref(r, regexp);
                    k = u;
                }
            }
            return k;
        }
        break;
    case L_SUBTREE:
        return NULL;
        break;
    case L_STAR:
    case L_MAYBE:
        return lns_key_regexp(l->child, exn);
    default:
        assert(0);
    }
    return NULL;
}

void free_lens(struct lens *lens) {
    if (lens == NULL)
        return;
    assert(lens->ref == 0);

    unref(lens->info, info);
    unref(lens->ctype, regexp);
    unref(lens->atype, regexp);
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
    regexp_release(lens->ctype);
    regexp_release(lens->atype);
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
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
