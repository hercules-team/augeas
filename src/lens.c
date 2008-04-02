/*
 * lens.c: 
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

#include "lens.h"

static struct regexp *regexp_digits = NULL;

static int typecheck_union(struct info *, struct lens *l1, struct lens *l2);
static int typecheck_concat(struct info *, struct lens *l1, struct lens *l2);
static int typecheck_iter(struct info *info, struct lens *l);
static int typecheck_maybe(struct info *info, struct lens *l);

static struct regexp *lns_key_regexp(struct lens *l);
static struct regexp *make_key_regexp(struct info *info, const char *pat);

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
    return lens;
}

static struct lens *make_lens_binop(enum lens_tag tag, struct info *info,
                                    struct lens *l1, struct lens *l2) {
    struct lens *lens = make_lens(tag, info);
    CALLOC(lens->children, 2);
    lens->nchildren = 2;
    lens->children[0] = l1;
    lens->children[1] = l2;
    return lens;
}

struct lens *lns_make_union(struct info *info,
                            struct lens *l1, struct lens *l2) {
    struct lens *lens = NULL;
    if (typecheck_union(info, l1, l2) == -1)
        return NULL;
    lens = make_lens_binop(L_UNION, info, l1, l2);
    lens->ctype = regexp_union(info, l1->ctype, l2->ctype);
    lens->atype = regexp_union(info, l1->atype, l2->atype);
    return lens;
}

struct lens *lns_make_concat(struct info *info,
                             struct lens *l1, struct lens *l2) {
    struct lens *lens = NULL;
    if (typecheck_concat(info, l1, l2) == -1)
        return NULL;
    lens = make_lens_binop(L_CONCAT, info, l1, l2);
    lens->ctype = regexp_concat(info, l1->ctype, l2->ctype);
    lens->atype = regexp_concat(info, l1->atype, l2->atype);
    return lens;
}

struct lens *lns_make_subtree(struct info *info, struct lens *l) {
    struct lens *lens = make_lens_unop(L_SUBTREE, info, l);
    lens->ctype = ref(l->ctype);
    lens->atype = lns_key_regexp(l);
    if (lens->atype == NULL)
        lens->atype = make_key_regexp(info, "");
    return lens;
}

struct lens *lns_make_star(struct info *info, struct lens *l) {
    struct lens *lens;

    if (typecheck_iter(info, l) == -1)
        return NULL;
    lens = make_lens_unop(L_STAR, info, l);
    lens->ctype = regexp_iter(info, l->ctype, 0, -1);
    lens->atype = regexp_iter(info, l->atype, 0, -1);
    return lens;
}

struct lens *lns_make_plus(struct info *info, struct lens *l) {
    struct lens *lens;

    if (typecheck_iter(info, l) == -1)
        return NULL;
    lens = make_lens_unop(L_PLUS, info, l);
    lens->ctype = regexp_iter(info, l->ctype, 1, -1);
    lens->atype = regexp_iter(info, l->atype, 1, -1);
    return lens;
}

struct lens *lns_make_maybe(struct info *info, struct lens *l) {
    struct lens *lens;

    if (typecheck_maybe(info, l) == -1)
        return NULL;
    lens = make_lens_unop(L_MAYBE, info, l);
    lens->ctype = regexp_maybe(info, l->ctype);
    lens->atype = regexp_maybe(info, l->atype);
    return lens;
}

/*
 * Lens primitives
 */
struct lens *lns_make_prim(enum lens_tag tag, struct info *info,
                           struct regexp *regexp, struct string *string) {
    struct lens *lens = make_lens(tag, info);
    lens->regexp = regexp;
    lens->string = string;
    switch(tag) {
    case L_DEL:
    case L_STORE:
    case L_KEY:
        lens->ctype = ref(regexp);
        lens->atype = regexp_make_empty(info);
        break;
    case L_LABEL:
    case L_SEQ:
    case L_COUNTER:
        lens->ctype = regexp_make_empty(info);
        lens->atype = regexp_make_empty(info);
        break;
    default:
        fatal_error(info, "Illegal primitive tag %d", tag);
    }
    return lens;
}

/*
 * Typechecking of lenses
 */
static fa_t regexp_to_fa(struct regexp *regexp) {
    fa_t fa;
    int error = fa_compile(regexp->pattern->str, &fa);
    if (error != REG_NOERROR) {
        syntax_error(regexp->info,
                     "unexpected error from fa_compile %d", error);
        return NULL;
    }
    return fa;
}

static int typecheck_union(struct info *info,
                           struct lens *l1, struct lens *l2) {
    fa_t fa1 = regexp_to_fa(l1->ctype);
    fa_t fa2 = regexp_to_fa(l2->ctype);

    if (fa1 == NULL || fa2 == NULL) {
        fa_free(fa1);
        fa_free(fa2);
        return -1;
    }

    fa_t isect = fa_intersect(fa1, fa2);
    if (! fa_is_basic(isect, FA_EMPTY)) {
        char *xmpl = fa_example(isect);
        char *esc = escape(xmpl, -1);
        syntax_error(info,
         "warning: nonempty intersection in union: '%s' matched by both\n",
                     esc);
        free(xmpl);
        free(esc);
        /* FIXME: We leave it with a warning since fixing this
           error with the curent language is a huge pain and it is
           fairly harmless as we always use the first match in a
           union, so it's predictable to the user what happens */
    }
    fa_free(isect);
    fa_free(fa1);
    fa_free(fa2);
    return 0;
}

static int is_ambiguous(struct info *info, fa_t fa1, fa_t fa2,
                        const char *msg) {
    char *upv, *pv, *v;
    upv = fa_ambig_example(fa1, fa2, &pv, &v);
    int result = (upv != NULL);
    if (upv != NULL) {
        char *e_u = escape(upv, pv - upv);
        char *e_up = escape(upv, v - upv);
        char *e_upv = escape(upv, -1);
        char *e_pv = escape(pv, -1);
        char *e_v = escape(v, -1);
        syntax_error(info,
                      "%s:\n"
                      "  '%s' can be split into\n"
                      "  '%s|=|%s'\n"
                      " and\n"
                      "  '%s|=|%s'", msg, e_upv, e_u, e_pv, e_up, e_v);
        free(e_u);
        free(e_up);
        free(e_upv);
        free(e_pv);
        free(e_v);
    }
    free(upv);
    return result;
}

static int typecheck_concat(struct info *info,
                            struct lens *l1, struct lens *l2) {
    fa_t fa1 = regexp_to_fa(l1->ctype);
    fa_t fa2 = regexp_to_fa(l2->ctype);
    int result = 0;

    if (fa1 == NULL || fa2 == NULL) {
        fa_free(fa1);
        fa_free(fa2);
        return -1;
    }

    if (is_ambiguous(info, fa1, fa2, "ambiguous concatenation"))
        result = -1;

    fa_free(fa1);
    fa_free(fa2);
    return result;
}

static int typecheck_iter(struct info *info, struct lens *l) {
    fa_t fas, fa;
    int result = 0;

    fa = regexp_to_fa(l->ctype);
    fas = fa_iter(fa, 0, -1);
    if (is_ambiguous(info, fa, fas, "ambiguous iteration")) {
        result = -1;
    }
    fa_free(fa);
    fa_free(fas);
    return result;
}

static int typecheck_maybe(struct info *info, struct lens *l) {
    /* Check (r)? as (<e>|r) where <e> is the empty language */
    fa_t fa, eps;
    int result = 0;

    fa = regexp_to_fa(l->ctype);
    if (fa == NULL)
        return -1;

    eps = fa_make_basic(FA_EPSILON);
    if (fa_contains(eps, fa)) {
        syntax_error(info,
          "illegal optional expression: expression matches the empty word");
        result = -1;
    }
    fa_free(eps);
    fa_free(fa);
    return result;
}

static struct regexp *make_key_regexp(struct info *info, const char *pat) {
    struct regexp *regexp;
    size_t len = strlen(pat) + 2;

    make_ref(regexp);
    make_ref(regexp->pattern);
    regexp->info = ref(info);
    CALLOC(regexp->pattern->str, len);
    snprintf((char *) regexp->pattern->str, len, "%s/", pat);
    return regexp;
}

static struct regexp *lns_key_regexp(struct lens *l) {
    switch(l->tag) {
    case L_DEL:
    case L_STORE:
    case L_COUNTER:
        return NULL;
    case L_SEQ:
        if (regexp_digits == NULL) {
            regexp_digits = make_regexp(l->info, strdup("[0-9]+/"));
            return regexp_digits;
        } else {
            return ref(regexp_digits);
        }
        break;
    case L_KEY:
        return make_key_regexp(l->info, l->regexp->pattern->str);
    case L_LABEL:
        return make_key_regexp(l->info, l->string->str);
    case L_CONCAT:
    case L_UNION:
        {
            struct regexp *k = NULL;
            for (int i=0; i < l->nchildren; i++) {
                struct regexp *r = lns_key_regexp(l->children[i]);
                if (r != NULL) {
                    if (k != NULL)
                        syntax_error(l->info, "More than one key");
                    else
                        k = r;
                }
            }
            return k;
        }
        break;
    case L_SUBTREE:
        return NULL;
        break;
    case L_STAR:
    case L_PLUS:
    case L_MAYBE:
        return lns_key_regexp(l->child);
    default:
        assert(0);
    }
    return NULL;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
