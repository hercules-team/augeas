/*
 * jmt.c: Earley parser for lenses based on Jim/Mandelbaum transducers
 *
 * Copyright (C) 2009-2011 David Lutterkort
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
 * Author: David Lutterkort <lutter@redhat.com>
 */

#include <config.h>

#include "jmt.h"
#include "internal.h"
#include "memory.h"
#include "errcode.h"

/* This is an implementation of the Earley parser described in the paper
 * "Efficient Earley Parsing with Regular Right-hand Sides" by Trever Jim
 * and Yitzhak Mandelbaum.
 *
 * Since we only deal in lenses, they are both terminals and nonterminals:
 * recursive lenses (those with lens->recursive set) are nonterminals, and
 * non-recursive lenses are terminals, with the exception of non-recursive
 * lenses that match the empty word. Lenses are also the semantic actions
 * attached to a SCAN or COMPLETE during recosntruction of the parse
 * tree. Our SCAN makes sure that we only ever scan nonempty words - that
 * means that for nonrecursive lenses which match the empty word (e.g., del
 * [ \t]* "") we need to make sure we get a COMPLETE when the lens matched
 * the empty word.
 *
 * That is achieved by treating such a lens t as the construct (t|T) with
 * T := eps.
 */

/*
 * Data structures for the Jim/Mandelbaum parser
 */

#define IND_MAX UINT32_MAX

struct array {
    size_t elem_size;
    ind_t used;
    ind_t size;
    void *data;
};

#define array_elem(arr, ind, typ)                               \
    (((typ *) (arr).data) + ind)

#define array_for_each(i, arr)                                  \
    for(ind_t i = 0; i < (arr).used; i++)

#define array_each_elem(elt, arr, typ)                          \
    for(typ *elt = (typ *) (arr).data + 0;                      \
        elt - (typ *) (arr).data < (arr).used;                  \
        elt++)

static void array_init(struct array *arr, size_t elem_size) {
    MEMZERO(arr, 1);
    arr->elem_size = elem_size;
}

ATTRIBUTE_RETURN_CHECK
static int array_add(struct array *arr, ind_t *ind) {
    if (arr->used >= arr->size) {
        int r;
        ind_t expand = arr->size;
        if (expand < 8)
            expand = 8;
        r = mem_realloc_n(&(arr->data), arr->elem_size, arr->size + expand);
        if (r < 0)
            return -1;
        memset((char *) arr->data + arr->elem_size*arr->size, 0,
               arr->elem_size * expand);
        arr->size += expand;
    }
    *ind = arr->used;
    arr->used += 1;
    return 0;
}

/* Insert a new entry into the array at index IND. Shift all entries from
 * IND on back by one. */
ATTRIBUTE_RETURN_CHECK
static int array_insert(struct array *arr, ind_t ind) {
    ind_t last;

    if (array_add(arr, &last) < 0)
        return -1;

    if (ind >= last)
        return 0;

    memmove((char *) arr->data + arr->elem_size*(ind+1),
            (char *) arr->data + arr->elem_size*ind,
            arr->elem_size * (arr->used - ind - 1));
    memset((char *) arr->data + arr->elem_size*ind, 0, arr->elem_size);
    return 0;
}

static void array_release(struct array *arr) {
    if (arr != NULL) {
        free(arr->data);
        arr->used = arr->size = 0;
    }
}

static void array_remove(struct array *arr, ind_t ind) {
    char *data = arr->data;
    memmove(data + ind * arr->elem_size,
            data + (ind + 1) * arr->elem_size,
            (arr->used - ind - 1) * arr->elem_size);
    arr->used -= 1;
}

ATTRIBUTE_RETURN_CHECK
static int array_join(struct array *dst, struct array *src) {
    int r;

    if (dst->elem_size != src->elem_size)
        return -1;

    r = mem_realloc_n(&(dst->data), dst->elem_size,
                      dst->used + src->used);
    if (r < 0)
        return -1;

    memcpy(((char *) dst->data) + dst->used * dst->elem_size,
           src->data,
           src->used * src->elem_size);
    dst->used += src->used;
    dst->size = dst->used;
    return 0;
}

/* Special lens indices - these don't reference lenses, but
 * some pseudo actions. We stash them at the top of the range
 * of IND_T, with LENS_MAX giving us the maximal lens index for
 * a real lens
 */
enum trans_op {
    EPS = IND_MAX,
    CALL = EPS - 1,
    LENS_MAX = CALL - 1
};

struct trans {
    struct state *to;
    ind_t lens;
};

struct state {
    struct state *next;      /* Linked list for memory management */
    struct array trans;      /* Array of struct trans */
    ind_t  nret;             /* Number of returned lenses */
    ind_t  *ret;             /* The returned lenses */
    ind_t  num;              /* Counter for num of new states */
    unsigned int reachable : 1;
    unsigned int live : 1;
};

/* Sets of states used in determinizing the NFA to a DFA */
struct nfa_state {
    struct state *state;  /* The new state in the DFA */
    struct array     set; /* Set of (struct state *) in the NFA, sorted */
};

/* For recursive lenses (nonterminals), the mapping of nonterminal to
 * state. We also store nonrecursive lenses here; in that case, the state
 * will be NULL */
struct jmt_lens {
    struct lens  *lens;
    struct state *state;
};

/* A Jim/Mandelbaum transducer */
struct jmt {
    struct error *error;
    struct array lenses;       /* Array of struct jmt_lens */
    struct state *start;
    ind_t  lens;               /* The start symbol of the grammar */
    ind_t  state_count;
};

enum item_reason {
    R_ROOT = 1,
    R_COMPLETE = R_ROOT << 1,
    R_PREDICT = R_COMPLETE << 1,
    R_SCAN = R_PREDICT << 1
};

/* The reason an item was added for parse reconstruction; can be R_SCAN,
 * R_COMPLETE, R_PREDICT or R_COMPLETE|R_PREDICT */
struct link {
    enum item_reason reason;
    ind_t            lens;       /* R_COMPLETE, R_SCAN */
    ind_t            from_set;   /* R_COMPLETE, R_PREDICT, R_SCAN */
    ind_t            from_item;  /* R_COMPLETE, R_PREDICT, R_SCAN */
    ind_t            to_item;    /* R_COMPLETE */
    ind_t            caller;     /* number of a state */
};

struct item {
    /* The 'classical' Earley item (state, parent) */
    struct state    *state;
    ind_t            parent;
    /* Backlinks to why item was added */
    ind_t            nlinks;
    struct link     *links;
};

struct item_set {
    struct array items;
};

struct jmt_parse {
    struct jmt       *jmt;
    struct error     *error;
    const char       *text;
    ind_t             nsets;
    struct item_set **sets;
};

#define for_each_item(it, set)                                  \
    array_each_elem(it, (set)->items, struct item)

#define for_each_trans(t, s)                                    \
    array_each_elem(t, (s)->trans, struct trans)

#define parse_state(parse, ind)                                 \
    array_elem(parse->states, ind, struct state)

static struct item *set_item(struct jmt_parse *parse, ind_t set,
                                 ind_t item) {
    ensure(parse->sets[set] != NULL, parse);
    ensure(item < parse->sets[set]->items.used, parse);
    return array_elem(parse->sets[set]->items, item, struct item);
 error:
    return NULL;
}

static struct state *item_state(struct jmt_parse *parse, ind_t set,
                                    ind_t item) {
    return set_item(parse, set, item)->state;
}

static struct trans *state_trans(struct state *state, ind_t t) {
    return array_elem(state->trans, t, struct trans);
}

static ind_t item_parent(struct jmt_parse *parse, ind_t set, ind_t item) {
    return set_item(parse, set, item)->parent;
}

static bool is_return(const struct state *s) {
    return s->nret > 0;
}

static struct lens *lens_of_parse(struct jmt_parse *parse, ind_t lens) {
    return array_elem(parse->jmt->lenses, lens, struct jmt_lens)->lens;
}

/*
 * The parser
 */

/*
 * Manipulate the Earley graph. We denote edges in the graph as
 *   [j, (s,i)] -> [k, item_k] => [l, item_l]
 * to indicate that we are adding item (s,i) to E_j and record that its
 * child is the item with index item_k in E_k and its sibling is item
 * item_l in E_l.
 */

/* Add item (s, k) to E_j. Note that the item was caused by action reason
 * using lens starting at from_item in E_{from_set}
 *
 * [j, (s,k)] -> [from_set, from_item] => [j, to_item]
 */
static ind_t parse_add_item(struct jmt_parse *parse, ind_t j,
                            struct state *s, ind_t k,
                            enum item_reason reason, ind_t lens,
                            ind_t from_set,
                            ind_t from_item, ind_t to_item,
                            ind_t caller) {

    ensure(from_item == EPS || from_item < parse->sets[from_set]->items.used,
           parse);
    ensure(to_item == EPS || to_item < parse->sets[j]->items.used,
           parse);

    int r;
    struct item_set *set = parse->sets[j];
    struct item *item = NULL;
    ind_t result = IND_MAX;

    if (set == NULL) {
        r = ALLOC(parse->sets[j]);
        ERR_NOMEM(r < 0, parse);
        array_init(&parse->sets[j]->items, sizeof(struct item));
        set = parse->sets[j];
    }

    for (ind_t i=0; i < set->items.used; i++) {
        if (item_state(parse, j, i) == s
            && item_parent(parse, j, i) == k) {
            result = i;
            item = set_item(parse, j, i);
            break;
        }
    }

    if (result == IND_MAX) {
        r = array_add(&set->items, &result);
        ERR_NOMEM(r < 0, parse);

        item = set_item(parse, j, result);
        item->state = s;
        item->parent = k;
    }

    for (ind_t i = 0; i < item->nlinks; i++) {
        struct link *lnk = item->links + i;
        if (lnk->reason == reason && lnk->lens == lens
            && lnk->from_set == from_set && lnk->from_item == from_item
            && lnk->to_item == to_item && lnk->caller == caller)
            return result;
    }

    r = REALLOC_N(item->links, item->nlinks + 1);
    ERR_NOMEM(r < 0, parse);

    struct link *lnk = item->links+ item->nlinks;
    item->nlinks += 1;

    lnk->reason = reason;
    lnk->lens = lens;
    lnk->from_set = from_set;
    lnk->from_item = from_item;
    lnk->to_item = to_item;
    lnk->caller = caller;
 error:
    return result;
}

/* Add item (s, i) to set E_j and record that it was added because
 * a parse of nonterminal lens, starting at itemk in E_k, was completed
 * by item item in E_j
 *
 * [j, (s,i)] -> [j, item] => [k, itemk]
 */
static void parse_add_complete(struct jmt_parse *parse, ind_t j,
                               struct state *s, ind_t i,
                               ind_t k, ind_t itemk,
                               ind_t lens,
                               ind_t item) {
    parse_add_item(parse, j, s, i, R_COMPLETE, lens, k, itemk, item, IND_MAX);
}

/* Same as parse_add_complete, but mark the item also as a predict
 *
 * [j, (s,i)] -> [j, item] => [k, itemk]
 */
static void parse_add_predict_complete(struct jmt_parse *parse, ind_t j,
                                       struct state *s, ind_t i,
                                       ind_t k, ind_t itemk,
                                       ind_t lens,
                                       ind_t item, ind_t caller) {
    parse_add_item(parse, j, s, i, R_COMPLETE|R_PREDICT,
                   lens, k, itemk, item, caller);
}

/* Add item (s, j) to E_j and record that it was added because of a
 * prediction from item from in E_j
 */
static ind_t parse_add_predict(struct jmt_parse *parse, ind_t j,
                               struct state *s, ind_t from) {

    ensure(from < parse->sets[j]->items.used, parse);
    struct state *t = item_state(parse, j, from);

    return parse_add_item(parse, j, s, j, R_PREDICT, EPS, j, from, EPS,
                          t->num);
 error:
    return IND_MAX;
}

/* Add item (s,i) to E_j and record that it was added because of scanning
 * with lens starting from item item in E_k.
 *
 * [j, (s,i)] -> [k, item]
 */
static void parse_add_scan(struct jmt_parse *parse, ind_t j,
                           struct state *s, ind_t i,
                           ind_t lens, ind_t k, ind_t item) {
    ensure(item < parse->sets[k]->items.used, parse);

    parse_add_item(parse, j, s, i, R_SCAN, lens, k, item, EPS, IND_MAX);
 error:
    return;
}

ATTRIBUTE_PURE
static bool is_complete(const struct link *lnk) {
    return lnk->reason & R_COMPLETE;
}

ATTRIBUTE_PURE
static bool is_predict(const struct link *lnk) {
    return lnk->reason & R_PREDICT;
}

ATTRIBUTE_PURE
static bool is_scan(const struct link *lnk) {
    return lnk->reason & R_SCAN;
}

ATTRIBUTE_PURE
static bool is_last_sibling(const struct link *lnk) {
    if (is_complete(lnk))
        return false;
    return lnk->reason & (R_PREDICT|R_ROOT);
}

ATTRIBUTE_PURE
static bool returns(const struct state *s, ind_t l) {
    for (ind_t i = 0; i < s->nret; i++)
        if (s->ret[i] == l)
            return true;
    return false;
}

static void state_add_return(struct jmt *jmt, struct state *s, ind_t l) {
    int r;

    if (s == NULL || returns(s, l))
        return;

    r = REALLOC_N(s->ret, s->nret + 1);
    ERR_NOMEM(r < 0, jmt);
    s->ret[s->nret] = l;
    s->nret += 1;
 error:
    return;
}

static void state_merge_returns(struct jmt *jmt, struct state *dst,
                                const struct state *src) {
    for (ind_t l = 0; l < src->nret; l++)
        state_add_return(jmt, dst, src->ret[l]);
}

static void nncomplete(struct jmt_parse *parse, ind_t j,
                       struct state *t, ind_t k, ind_t item) {

    for (ind_t itemk = 0; itemk < parse->sets[k]->items.used; itemk++) {
        struct state *u = item_state(parse, k, itemk);
        for_each_trans(y, u) {
            if (returns(t, y->lens)) {
                ind_t parent = item_parent(parse, k, itemk);
                parse_add_complete(parse, j,
                                   y->to, parent,
                                   k, itemk, y->lens, item);
            }
        }
    }
}

/* NCALLER for (t, i) in E_j, which has index item in E_j, and t -> s a
 * call in the transducer and s a return. The item (s,j) has index pred in
 * E_j
 */
static void ncaller(struct jmt_parse *parse, ind_t j, ind_t item,
                    struct state *t, ind_t i, struct state *s, ind_t pred) {
    for_each_trans(u, t) {
        if (returns(s, u->lens)) {
            /* [j, (u->to, i)] -> [j, pred] => [j, item] */
            parse_add_predict_complete(parse, j,
                                       u->to, i,
                                       j, item, u->lens,
                                       pred, t->num);
        }
    }
}

/* NCALLEE for (t, parent) in E_j, which has index item in E_j, and t -> s
 * a call in the transducer and s a return. The item (s,j) has index pred
 * in E_j
 */
static void ncallee(struct jmt_parse *parse, ind_t j, ATTRIBUTE_UNUSED ind_t item,
                    ATTRIBUTE_UNUSED struct state *t, ATTRIBUTE_UNUSED ind_t parent, struct state *s, ind_t pred) {
    for_each_trans(u, s) {
        if (returns(s, u->lens)) {
            /* [j, (u->to, j)] -> [j, item] => [j, item] */
            parse_add_predict_complete(parse, j,
                                       u->to, j,
                                       j, pred, u->lens,
                                       pred, t->num);
        }
    }
}

static struct jmt_parse *parse_init(struct jmt *jmt,
                                    const char *text, size_t text_len) {
    int r;
    struct jmt_parse *parse;

    r = ALLOC(parse);
    ERR_NOMEM(r < 0, jmt);

    parse->jmt = jmt;
    parse->error = jmt->error;
    parse->text = text;
    parse->nsets = text_len + 1;
    r = ALLOC_N(parse->sets, parse->nsets);
    ERR_NOMEM(r < 0, jmt);
    return parse;
 error:
    if (parse != NULL)
        free(parse->sets);
    free(parse);
    return NULL;
}

void jmt_free_parse(struct jmt_parse *parse) {
    if (parse == NULL)
        return;
    for (int i=0; i < parse->nsets; i++) {
        struct item_set *set = parse->sets[i];
        if (set != NULL) {
            array_each_elem(x, set->items, struct item)
                free(x->links);
            array_release(&set->items);
            free(set);
        }
    }
    free(parse->sets);
    free(parse);
}

static struct state *lens_state(struct jmt *jmt, ind_t l);

static void flens(FILE *fp, ind_t l) {
    if (l == 0)
        fprintf(fp, "%c", 'S');
    else if (l < 'S' - 'A')
        fprintf(fp, "%c", 'A' + l - 1);
    else if (l <= 'Z' - 'A')
        fprintf(fp, "%c", 'A' + l);
    else
        fprintf(fp, "%u", l);
}

static void parse_dot_link(FILE *fp, struct jmt_parse *parse,
                           ind_t k, struct item *x, struct link *lnk) {
    char *lens_label = NULL;
    if (is_complete(lnk) || is_scan(lnk)) {
        struct state *sA = lens_state(parse->jmt, lnk->lens);
        int r;

        if (sA == NULL)
            r = xasprintf(&lens_label, "<%d>", lnk->lens);
        else
            r = xasprintf(&lens_label, "%d", lnk->lens);
        if (r < 0) {
            fprintf(fp, "// Internal error generating lens_label\n");
            return;
        }
    }
    fprintf(fp, "    n%d_%d_%d [ label = \"(%d, %d)\"];\n",
            k, x->state->num, x->parent, x->state->num, x->parent);
    if (is_complete(lnk)) {
        struct item *y = set_item(parse, k, lnk->to_item);
        const char *pred = is_predict(lnk) ? "p" : "";
        fprintf(fp, "    n%d_%d_%d -> n%s%d_%d_%d [ style = dashed ];\n",
                k, x->state->num, x->parent,
                pred, k, y->state->num, y->parent);
        if (is_predict(lnk)) {
            fprintf(fp, "    n%s%d_%d_%d [ label = \"\" ];\n",
                    pred, k, y->state->num, y->parent);
            fprintf(fp, "    n%s%d_%d_%d -> n%d_%d_%d [ style = bold ];\n",
                    pred, k, y->state->num, y->parent,
                    k, y->state->num, y->parent);
        }
        y = set_item(parse, lnk->from_set, lnk->from_item);
        fprintf(fp,
                "    n%d_%d_%d -> n%d_%d_%d [ style = dashed, label = \"",
                k, x->state->num, x->parent,
                lnk->from_set, y->state->num, y->parent);
        flens(fp, lnk->lens);
        fprintf(fp, "\" ];\n");
    } else if (is_scan(lnk)) {
        struct item *y =
            set_item(parse, lnk->from_set, lnk->from_item);
        fprintf(fp,
                "    n%d_%d_%d -> n%d_%d_%d [ label = \"",
                k, x->state->num, x->parent,
                lnk->from_set, y->state->num, y->parent);
        for (ind_t i=lnk->from_set; i < k; i++)
            fprintf(fp, "%c", parse->text[i]);
        fprintf(fp, "\" ];\n");

    } else if (is_predict(lnk)) {
        struct item *y =
            set_item(parse, lnk->from_set, lnk->from_item);
        fprintf(fp,
                "    n%d_%d_%d -> n%d_%d_%d [ style = bold ];\n",
                k, x->state->num, x->parent,
                lnk->from_set, y->state->num, y->parent);

    }
    free(lens_label);
}

static void parse_dot(struct jmt_parse *parse, const char *fname) {
    FILE *fp = debug_fopen("%s", fname);
    if (fp == NULL)
        return;

    fprintf(fp, "digraph \"jmt_parse\" {\n");
    fprintf(fp, "  rankdir = RL;\n");
    for (int k=0; k < parse->nsets; k++) {
        struct item_set *set = parse->sets[k];

        if (set == NULL)
            continue;

        fprintf(fp, "  subgraph \"cluster_E_%d\" {\n", k);
        fprintf(fp, "    rankdir=RL;\n    rank=same;\n");
        fprintf(fp, "    title%d [ label=\"E%d\", shape=plaintext ]\n", k, k);
        for_each_item(x, set) {
            for (int i=0; i < x->nlinks; i++) {
                struct link *lnk = x->links + i;
                parse_dot_link(fp, parse, k, x, lnk);
            }
        }
        fprintf(fp, "}\n");
    }
    fprintf(fp, "}\n");
    fclose(fp);
}

struct jmt_parse *
jmt_parse(struct jmt *jmt, const char *text, size_t text_len)
{
    struct jmt_parse *parse = NULL;

    parse = parse_init(jmt, text, text_len);
    ERR_BAIL(jmt);

    /* INIT */
    parse_add_item(parse, 0, jmt->start, 0, R_ROOT, EPS, EPS, EPS, EPS,
                   jmt->lens);
    /* NINIT */
    if (is_return(jmt->start)) {
        for_each_trans(x, jmt->start) {
            if (returns(jmt->start, x->lens))
                parse_add_predict_complete(parse, 0, x->to, 0,
                                           0, 0, x->lens, 0, 0);
        }
    }

    for (int j=0; j <= text_len; j++) {
        struct item_set *set = parse->sets[j];
        if (set == NULL)
            continue;

        for (int item=0; item < set->items.used; item++) {
            struct state *t = item_state(parse, j, item);
            ind_t i = item_parent(parse, j, item);

            if (is_return(t) && i != j) {
                /* NNCOMPLETE */
                nncomplete(parse, j, t, i, item);
            }

            for_each_trans(x, t) {
                if (x->lens == CALL) {
                    /* PREDICT */
                    ind_t pred = parse_add_predict(parse, j, x->to, item);
                    ERR_BAIL(parse);
                    if (is_return(x->to)) {
                        /* NCALLER */
                        ncaller(parse, j, item, t, i, x->to, pred);
                        /* NCALLEE */
                        ncallee(parse, j, item, t, i, x->to, pred);
                    }
                } else {
                    int count;
                    struct lens *lens = lens_of_parse(parse, x->lens);
                    struct state *sA = lens_state(parse->jmt, x->lens);
                    if (! lens->recursive && sA == NULL) {
                        /* SCAN, terminal */
                        // FIXME: We really need to find every k so that
                        // text[j..k] matches lens->ctype, not just one
                        count = regexp_match(lens->ctype, text, text_len, j, NULL);
                        if (count > 0) {
                            parse_add_scan(parse, j+count,
                                           x->to, i,
                                           x->lens, j, item);
                        }
                    }
                }
            }
        }
    }
    if (debugging("cf.jmt.parse"))
        parse_dot(parse, "jmt_parse.dot");
    return parse;
 error:
    jmt_free_parse(parse);
    return NULL;
}

/*
 * Reconstruction of the parse tree
 */

static void
build_nullable(struct jmt_parse *parse, ind_t pos,
               struct jmt_visitor *visitor, struct lens *lens, int lvl) {
    if (! lens->recursive) {
        if (visitor->terminal != NULL) {
            (*visitor->terminal)(lens, pos, pos, visitor->data);
            ERR_BAIL(parse);
        }
    } else {
        if (visitor->enter != NULL) {
            (*visitor->enter)(lens, pos, pos, visitor->data);
            ERR_BAIL(parse);
        }

        switch(lens->tag) {
        case L_REC:
            build_nullable(parse, pos, visitor, lens->body, lvl+1);
            break;
        case L_CONCAT:
            for (int i=0; i < lens->nchildren; i++)
                build_nullable(parse, pos, visitor, lens->children[i], lvl+1);
            break;
        case L_UNION:
            for (int i=0; i < lens->nchildren; i++)
                if (lens->children[i]->ctype_nullable)
                    build_nullable(parse, pos, visitor,
                                   lens->children[i], lvl+1);
            break;
        case L_SUBTREE:
        case L_SQUARE:
            build_nullable(parse, pos, visitor, lens->child, lvl+1);
            break;
        case L_STAR:
        case L_MAYBE:
            break;
        default:
            BUG_ON(true, parse, "Unexpected lens tag %d", lens->tag);
        }

        if (visitor->exit != NULL) {
            (*visitor->exit)(lens, pos, pos, visitor->data);
            ERR_BAIL(parse);
        }
    }
 error:
    return;
}

static void build_trace(const char *msg, ind_t start, ind_t end,
                        struct item *x, int lvl) {
    for (int i=0; i < lvl; i++) putc(' ', stderr);
    if (x != NULL) {
        printf("%s %d..%d: (%d, %d) %d %s%s%s\n", msg,
               start, end, x->state->num,
               x->parent, x->links->lens,
               is_complete(x->links) ? "c" : "",
               is_predict(x->links) ? "p" : "",
               is_scan(x->links) ? "s" : "");
    } else {
        printf("%s %d..%d\n", msg, start, end);
    }
}

static int add_sibling(struct array *siblings, ind_t lnk) {
    int r;
    ind_t ind;

    r = array_add(siblings, &ind);
    if (r < 0)
        return -1;
    *array_elem(*siblings, ind, ind_t) = lnk;
    return 0;
}

/* Return true if CALLER is a possible caller for the link LNK which starts
 * at item X.
 *
 * FIXME: We can get rid of the caller field on a link if we distinguish
 * between NCALLER and NCALLEE in the Earley graph, rather than collapse
 * them both into links with reason PREDICT|COMPLETE
 */
static bool is_caller(struct item *x, struct link *lnk, ind_t caller) {
    if (lnk->reason & R_ROOT)
        return caller == lnk->caller;

    if (! is_predict(lnk))
        return false;

    if (is_complete(lnk)) {
        /* NCALLER: caller == t
         * NCALLEE: caller == s */
        return caller == lnk->caller;
    }
    /* PREDICT: caller == t || caller == s */
    return caller == lnk->caller || caller == x->state->num;
}

/* Traverse the siblings of x and check that the callee's set of callers
 * contains CALLER. When a path ending with a call from CALLER exists,
 * record the number of the corresponding link for each item in
 * SIBLINGS. The links are recorded in left-to-right order, i.e. the number
 * of the first link to follow is in the last entry in SIBLINGS.
 *
 * Returns 0 if there is a path ending in a callee of CALLER. Return -1 if
 * there is none. Return -2 if there are multiple such paths. Return -3 for
 * any other error.
 */
static int filter_siblings(struct jmt_visitor *visitor, struct lens *lens,
                          ind_t k, ind_t item, ind_t caller,
                          struct array *siblings) {
    struct jmt_parse *parse = visitor->parse;
    struct item *x = set_item(parse, k, item);
    ind_t nlast = 0;
    int r;

    for (ind_t lnk = 0; lnk < x->nlinks; lnk++)
        if (is_last_sibling(x->links + lnk))
            nlast += 1;

    if (nlast > 0 && nlast < x->nlinks)
        goto ambig;

    if (nlast == x->nlinks) {
        for (ind_t lnk = 0; lnk < x->nlinks; lnk++) {
            if (is_caller(x, x->links + lnk, caller)) {
                siblings->used = 0;
                r = add_sibling(siblings, lnk);
                if (r < 0) {
                    ERR_REPORT(parse, AUG_ENOMEM, NULL);
                    return -3;
                }
                return 0;
            }
        }
        return -1;
    } else {
        /* nlast == 0 */
        ind_t found = IND_MAX;
        for (ind_t lnk = 0; lnk < x->nlinks; lnk++) {
            struct link *l = x->links + lnk;
            r = filter_siblings(visitor, lens,
                                l->from_set, l->from_item, caller,
                                siblings);
            if (r == -1)
                continue;
            if (r == 0) {
                if (found != IND_MAX)
                    goto ambig;
                else
                    found = lnk;
            } else {
                return r;
            }
        }
        if (found == IND_MAX) {
            return -1;
        } else {
            r = add_sibling(siblings, found);
            if (r < 0) {
                ERR_REPORT(parse, AUG_ENOMEM, NULL);
                return -3;
            }
            return 0;
        }
    }
 ambig:
    (*visitor->error)(lens, visitor->data, k,
                      "Ambiguous parse: %d links in state (%d, %d) in E_%d",
                      x->nlinks, x->state->num, x->parent, k);
    return -2;
}

static void visit_enter(struct jmt_visitor *visitor, struct lens *lens,
                        size_t start, size_t end,
                        struct item *x, int lvl) {
    if (debugging("cf.jmt.visit"))
        build_trace("{", start, end, x, lvl);
    if (visitor->enter != NULL)
        (*visitor->enter)(lens, start, end, visitor->data);
}

static void visit_exit(struct jmt_visitor *visitor, struct lens *lens,
                       size_t start, size_t end,
                       struct item *x, int lvl) {
    if (debugging("cf.jmt.visit"))
        build_trace("}", start, end, x, lvl);
    if (visitor->exit != NULL)
        (*visitor->exit)(lens, start, end, visitor->data);
}

static int
build_children(struct jmt_parse *parse, ind_t k, ind_t item,
               struct jmt_visitor *visitor, int lvl, ind_t caller);

static int
build_tree(struct jmt_parse *parse, ind_t k, ind_t item, struct lens *lens,
           struct jmt_visitor *visitor, int lvl) {
    struct item *x = set_item(parse, k, item);
    ind_t start = x->links->from_set;
    ind_t end = k;
    struct item *old_x = x;

    if (start == end) {
        /* This completion corresponds to a nullable nonterminal
         * that match epsilon. Reconstruct the full parse tree
         * for matching epsilon */
        if (debugging("cf.jmt.visit"))
            build_trace("N", x->links->from_set, k, x, lvl);
        build_nullable(parse, start, visitor, lens, lvl);
        return end;
    }

    ensure(is_complete(x->links), parse);

    visit_enter(visitor, lens, start, end, x, lvl);
    ERR_BAIL(parse);

    /* x is a completion item. (k, x->to_item) is its first child in the
     * parse tree. */
    if (! is_predict(x->links)) {
        struct link *lnk = x->links;
        struct item *sib = set_item(parse, lnk->from_set, lnk->from_item);
        ind_t caller = sib->state->num;

        item = lnk->to_item;
        x = set_item(parse, k, item);
        build_children(parse, k, item, visitor, lvl, caller);
        ERR_BAIL(parse);
    }

    visit_exit(visitor, lens, start, end, old_x, lvl);
    ERR_BAIL(parse);
 error:
    return end;
}

static int
build_children(struct jmt_parse *parse, ind_t k, ind_t item,
               struct jmt_visitor *visitor, int lvl, ind_t caller) {
    struct item *x = set_item(parse, k, item);
    struct lens *lens = lens_of_parse(parse, x->links->lens);
    struct array siblings;
    ind_t end = k;
    int r;

    array_init(&siblings, sizeof(ind_t));
    r = filter_siblings(visitor, lens, k, item, caller, &siblings);
    if (r < 0)
        goto error;

    /* x the first item in a list of siblings; visit items (x->from_set,
     * x->from_item) in order, which will visit x and its siblings in the
     * parse tree from right to left */
    for (ind_t i = siblings.used - 1; i > 0; i--) {
        ind_t lnk = *array_elem(siblings, i, ind_t);
        struct lens *sub = lens_of_parse(parse, x->links[lnk].lens);
        if (sub->recursive) {
            build_tree(parse, k, item, sub, visitor, lvl+1);
            ERR_BAIL(parse);
        } else {
            if (debugging("cf.jmt.visit"))
                build_trace("T", x->links->from_set, k, x, lvl+1);
            if (visitor->terminal != NULL) {
                (*visitor->terminal)(sub,
                                     x->links->from_set, k, visitor->data);
                ERR_BAIL(parse);
            }
        }
        k = x->links[lnk].from_set;
        item = x->links[lnk].from_item;
        x = set_item(parse, k, item);
    }
 error:
    array_release(&siblings);
    return end;
}

int jmt_visit(struct jmt_visitor *visitor, size_t *len) {
    struct jmt_parse *parse = visitor->parse;
    ind_t k = parse->nsets - 1;     /* Current Earley set */
    ind_t item;
    struct item_set *set = parse->sets[k];

    if (set == NULL)
        goto noparse;

    for (item = 0; item < set->items.used; item++) {
        struct item *x = set_item(parse, k, item);
        if (x->parent == 0 && returns(x->state, parse->jmt->lens)) {
            for (ind_t i = 0; i < x->nlinks; i++) {
                if (is_complete(x->links + i) || is_scan(x->links + i)) {
                    if (debugging("cf.jmt.visit"))
                        printf("visit: found (%d, %d) in E_%d\n",
                               x->state->num, x->parent, k);
                    goto found;
                }
            }
        }
    }
 found:
    if (item >= parse->sets[k]->items.used)
        goto noparse;
    struct lens *lens = lens_of_parse(parse, parse->jmt->lens);

    visit_enter(visitor, lens, 0, k, NULL, 0);
    ERR_BAIL(parse);

    *len = build_children(parse, k, item, visitor, 0,
                          parse->jmt->start->num);
    ERR_BAIL(parse);

    visit_exit(visitor, lens, 0, k, NULL, 0);
    ERR_BAIL(parse);
    return 1;
 error:
    return -1;
 noparse:
    for (; k > 0; k--)
        if (parse->sets[k] != NULL) break;
    *len = k;
    return 0;
}

/*
 * Build the automaton
 */


static struct state *make_state(struct jmt *jmt) {
    struct state *s;
    int r;

    r = ALLOC(s);
    ERR_NOMEM(r < 0, jmt);
    s->num = jmt->state_count++;
    array_init(&s->trans, sizeof(struct trans));
    if (jmt->start != NULL)
        list_cons(jmt->start->next, s);
    else
        jmt->start = s;
    return s;
 error:
    return NULL;
}

static ind_t add_lens(struct jmt *jmt, struct lens *lens) {
    int r;
    ind_t l;
    struct state *sA = NULL;
    int nullable = 0;

    r = array_add(&jmt->lenses, &l);
    ERR_NOMEM(r < 0, jmt);
    ERR_NOMEM(l == IND_MAX, jmt);

    if (! lens->recursive)
        nullable = regexp_matches_empty(lens->ctype);

    array_elem(jmt->lenses, l, struct jmt_lens)->lens = lens;
    /* A nonrecursive lens that matches epsilon is both a terminal
     * and a nonterminal */
    if (lens->recursive || nullable) {
        sA = make_state(jmt);
        ERR_NOMEM(sA == NULL, jmt);
        array_elem(jmt->lenses, l, struct jmt_lens)->state = sA;
        if (! lens->recursive) {
            /* Add lens again, so that l refers to the nonterminal T
             * for the lens, and l+1 refers to the terminal t for it */
            ind_t m;
            r = array_add(&jmt->lenses, &m);
            ERR_NOMEM(r < 0, jmt);
            ERR_NOMEM(m == IND_MAX, jmt);

            array_elem(jmt->lenses, m, struct jmt_lens)->lens = lens;
        }
    }

    if (debugging("cf.jmt")) {
        if (sA == NULL) {
            printf("add_lens: ");
            print_regexp(stdout, lens->ctype);
            printf(" %s\n", format_lens(lens));
        } else {
            printf("add_lens: ");
            flens(stdout, l);
            printf(" %u %s\n", sA->num, format_lens(lens));
            if (nullable) {
                printf("add_lens: // %s\n", format_lens(lens));
            }
        }
    }

    return l;
 error:
    return IND_MAX;
}

static struct trans *
add_new_trans(struct jmt *jmt,
              struct state *from, struct state *to, ind_t lens) {
    struct trans *t;
    ind_t i;
    int r;

    if (from == NULL || to == NULL)
        return NULL;

    r = array_add(&from->trans, &i);
    ERR_NOMEM(r < 0, jmt);
    t = array_elem(from->trans, i, struct trans);
    t->to = to;
    t->lens = lens;
    return t;
 error:
    return NULL;
}

static struct trans *
add_eps_trans(struct jmt *jmt, struct state *from, struct state *to) {
    return add_new_trans(jmt, from, to, EPS);
}

static struct lens *lens_of_jmt(struct jmt *jmt, ind_t l) {
    return array_elem(jmt->lenses, l, struct jmt_lens)->lens;
}

static ind_t lens_index(struct jmt *jmt, struct lens *lens) {
    array_for_each(i, jmt->lenses)
        if (lens_of_jmt(jmt, i) == lens)
            return i;
    return IND_MAX;
}

static struct state *lens_state(struct jmt *jmt, ind_t l) {
    return array_elem(jmt->lenses, l, struct jmt_lens)->state;
}

static void print_lens_symbol(FILE *fp, struct jmt *jmt, struct lens *lens) {
    ind_t l = lens_index(jmt, lens);
    struct state *sA = lens_state(jmt, l);

    if (sA == NULL)
        print_regexp(fp, lens->ctype);
    else
        flens(fp, l);
}

static void print_grammar(struct jmt *jmt, struct lens *lens) {
    ind_t l = lens_index(jmt, lens);
    struct state *sA = lens_state(jmt, l);

    if (sA == NULL || (lens->tag == L_REC && lens->rec_internal))
        return;

    printf("  ");
    print_lens_symbol(stdout, jmt, lens);
    printf(" := ");

    if (! lens->recursive) {
        /* Nullable regexps */
        print_regexp(stdout, lens->ctype);
        printf("\n");
        return;
    }

    switch (lens->tag) {
    case L_CONCAT:
        print_lens_symbol(stdout, jmt, lens->children[0]);
        for (int i=1; i < lens->nchildren; i++) {
            printf(" . ");
            print_lens_symbol(stdout, jmt, lens->children[i]);
        }
        printf("\n");
        for (int i=0; i < lens->nchildren; i++)
            print_grammar(jmt, lens->children[i]);
        break;
    case L_UNION:
        print_lens_symbol(stdout, jmt, lens->children[0]);
        for (int i=1; i < lens->nchildren; i++) {
            printf(" | ");
            print_lens_symbol(stdout, jmt, lens->children[i]);
        }
        printf("\n");
        for (int i=0; i < lens->nchildren; i++)
            print_grammar(jmt, lens->children[i]);
        break;
    case L_SUBTREE:
        print_lens_symbol(stdout, jmt, lens->child);
        printf("\n");
        print_grammar(jmt, lens->child);
        break;
    case L_STAR:
        print_lens_symbol(stdout, jmt, lens->child);
        printf("*\n");
        print_grammar(jmt, lens->child);
        break;
    case L_MAYBE:
        print_lens_symbol(stdout, jmt, lens->child);
        printf("?\n");
        print_grammar(jmt, lens->child);
        break;
    case L_REC:
        print_lens_symbol(stdout, jmt, lens->body);
        printf("\n");
        print_grammar(jmt, lens->body);
        break;
    case L_SQUARE:
       print_lens_symbol(stdout, jmt, lens->child);
       printf("\n");
       print_grammar(jmt, lens->child);
       break;
    default:
        BUG_ON(true, jmt, "Unexpected lens tag %d", lens->tag);
        break;
    }
 error:
    return;
}

static void print_grammar_top(struct jmt *jmt, struct lens *lens) {
    printf("Grammar:\n");
    print_grammar(jmt, lens);
    if (lens->tag == L_REC) {
        printf("  ");
        print_lens_symbol(stdout, jmt, lens->alias);
        printf(" := ");
        print_lens_symbol(stdout, jmt, lens->alias->body);
        printf("\n");
    }
}

static void index_lenses(struct jmt *jmt, struct lens *lens) {
    ind_t l;

    l = lens_index(jmt, lens);
    if (l == IND_MAX) {
        l = add_lens(jmt, lens);
        ERR_BAIL(jmt);
    }

    if (! lens->recursive)
        return;

    switch (lens->tag) {
    case L_CONCAT:
    case L_UNION:
        for (int i=0; i < lens->nchildren; i++)
            index_lenses(jmt, lens->children[i]);
        break;
    case L_SUBTREE:
    case L_STAR:
    case L_MAYBE:
    case L_SQUARE:
        index_lenses(jmt, lens->child);
        break;
    case L_REC:
        if (! lens->rec_internal)
            index_lenses(jmt, lens->body);
        break;
    default:
        BUG_ON(true, jmt, "Unexpected lens tag %d", lens->tag);
        break;
    }
 error:
    return;
}

static void thompson(struct jmt *jmt, struct lens *lens,
                     struct state **s, struct state **f) {
    ind_t l = lens_index(jmt, lens);
    struct state *sA = lens_state(jmt, l);
    ensure(l < jmt->lenses.used, jmt);

    *s = make_state(jmt);
    *f = make_state(jmt);
    ERR_BAIL(jmt);

    if (lens->recursive) {
        /* A nonterminal */
        add_new_trans(jmt, *s, *f, l);
        add_new_trans(jmt, *s, sA, CALL);
    } else if (sA == NULL) {
        /* A terminal that never matches epsilon */
        add_new_trans(jmt, *s, *f, l);
    } else {
        /* A terminal that matches epsilon */
        add_new_trans(jmt, *s, *f, l);
        add_new_trans(jmt, *s, sA, CALL);
        add_new_trans(jmt, *s, *f, l+1);
    }
 error:
    return;
}

static void conv(struct jmt *jmt, struct lens *lens,
                 struct state **s, struct state **e,
                 struct state **f) {
    ind_t l = lens_index(jmt, lens);
    ensure(l < jmt->lenses.used, jmt);
    struct state *sA = lens_state(jmt, l);

    *s = NULL;
    *e = NULL;
    *f = NULL;

    if (lens->recursive) {
        /* A nonterminal */
        *s = make_state(jmt);
        *f = make_state(jmt);
        ERR_BAIL(jmt);
        add_new_trans(jmt, *s, *f, l);
        ERR_BAIL(jmt);
        ensure(sA != NULL, jmt);
        add_new_trans(jmt, *s, sA, EPS);
        ERR_BAIL(jmt);
    } else if (sA == NULL) {
        /* A terminal that never matches epsilon */
        *s = make_state(jmt);
        *f = make_state(jmt);
        ERR_BAIL(jmt);
        add_new_trans(jmt, *s, *f, l);
        ERR_BAIL(jmt);
    } else {
        /* A terminal that matches epsilon */
        *s = make_state(jmt);
        *f = make_state(jmt);
        ERR_BAIL(jmt);
        add_new_trans(jmt, *s, *f, l);
        add_new_trans(jmt, *s, *f, l+1);
        add_new_trans(jmt, *s, sA, EPS);
        ERR_BAIL(jmt);
    }
 error:
    return;
}

static void conv_concat(struct jmt *jmt, struct lens *lens,
                        struct state **s, struct state **e,
                        struct state **f) {
    struct state *s2, *f2, *e2;

    conv(jmt, lens->children[0], &s2, &e2, &f2);
    *s = make_state(jmt);
    add_new_trans(jmt, *s, s2, EPS);

    for (int i=1; i < lens->nchildren; i++) {
        struct state *s3, *e3, *f3, *scall, *fcall;
        conv(jmt, lens->children[i], &s3, &e3, &f3);
        thompson(jmt, lens->children[i], &scall, &fcall);
        ERR_BAIL(jmt);
        add_eps_trans(jmt, f2, scall);
        add_eps_trans(jmt, e2, s3);
        *f = make_state(jmt);
        add_eps_trans(jmt, f3, *f);
        add_eps_trans(jmt, fcall, *f);
        *e = make_state(jmt);
        add_eps_trans(jmt, e3, *e);
        f2 = *f;
        e2 = *e;
    }
 error:
    return;
}

static void conv_union(struct jmt *jmt, struct lens *lens,
                        struct state **s, struct state **e,
                        struct state **f) {

    *s = make_state(jmt);
    *e = make_state(jmt);
    *f = make_state(jmt);
    ERR_BAIL(jmt);

    for (int i = 0; i < lens->nchildren; i++) {
        struct state *s2, *e2, *f2;

        conv(jmt, lens->children[i], &s2, &e2, &f2);
        ERR_BAIL(jmt);

        add_eps_trans(jmt, *s, s2);
        add_eps_trans(jmt, e2, *e);
        add_eps_trans(jmt, f2, *f);
    }

 error:
    return;
}

static void conv_star(struct jmt *jmt, struct lens *lens,
                      struct state **s, struct state **e,
                      struct state **f) {

    *s = make_state(jmt);
    *e = make_state(jmt);
    *f = make_state(jmt);
    ERR_BAIL(jmt);

    struct state *si, *ei, *fi, *scall, *fcall;
    conv(jmt, lens->child, &si, &ei, &fi);
    thompson(jmt, lens->child, &scall, &fcall);
    ERR_BAIL(jmt);

    add_eps_trans(jmt, *s, si);
    add_eps_trans(jmt, ei, si);
    add_eps_trans(jmt, *s, *e);
    add_eps_trans(jmt, ei, *e);
    add_eps_trans(jmt, fi, scall);
    add_eps_trans(jmt, fcall, scall);
    add_eps_trans(jmt, fi, *f);
    add_eps_trans(jmt, fcall, *f);
    ERR_BAIL(jmt);

 error:
    return;
}

static void conv_rhs(struct jmt *jmt, ind_t l) {
    struct lens *lens = lens_of_jmt(jmt, l);
    struct state *s = NULL, *e = NULL, *f = NULL;
    struct state *sA = lens_state(jmt, l);

    if (! lens->recursive) {
        /* Nothing to do for terminals that do not match epsilon */
        if (sA != NULL)
            state_add_return(jmt, sA, l);
        return;
    }

    /* All other nonterminals/recursive lenses */

    /* Maintain P1 */
    if (lens->ctype_nullable)
        state_add_return(jmt, sA, l);

    switch (lens->tag) {
    case L_REC:
        conv(jmt, lens->body, &s, &e, &f);
        break;
    case L_CONCAT:
        conv_concat(jmt, lens, &s, &e, &f);
        break;
    case L_UNION:
        conv_union(jmt, lens, &s, &e, &f);
        break;
    case L_SUBTREE:
        conv(jmt, lens->child, &s, &e, &f);
        break;
    case L_STAR:
        conv_star(jmt, lens, &s, &e, &f);
        break;
    case L_MAYBE:
        conv(jmt, lens->child, &s, &e, &f);
        add_new_trans(jmt, s, e, EPS);
        break;
    case L_SQUARE:
       conv(jmt, lens->child, &s, &e, &f);
       break;
    default:
        BUG_ON(true, jmt, "Unexpected lens tag %d", lens->tag);
    }

    ensure(sA != NULL, jmt);

    add_eps_trans(jmt, sA, s);
    state_add_return(jmt, e, l);
    state_add_return(jmt, f, l);

 error:
    return;
}

ATTRIBUTE_RETURN_CHECK
static int push_state(struct array *worklist, struct state *s) {
    int r;
    ind_t ind;

    r = array_add(worklist, &ind);
    if (r < 0)
        return -1;
    *array_elem(*worklist, ind, struct state *) = s;
    return 0;
}

static struct state *pop_state(struct array *worklist) {
    if (worklist->used > 0) {
        worklist->used -= 1;
        return *array_elem(*worklist, worklist->used, struct state *);
    } else {
        return NULL;
    }
}

static void free_state(struct state *s) {
    if (s == NULL)
        return;
    free(s->ret);
    array_release(&s->trans);
    free(s);
}

static void collect(struct jmt *jmt) {
    struct array worklist;
    size_t count, removed;
    int r;

    count = 0;
    list_for_each(s, jmt->start) {
        s->live = 0;
        s->reachable = 0;
        count += 1;
    }

    array_init(&worklist, sizeof(struct state *));
    jmt->start->reachable = 1;
    for (struct state *s = jmt->start;
         s != NULL;
         s = pop_state(&worklist)) {
        for_each_trans(t, s) {
            if (! t->to->reachable) {
                t->to->reachable = 1;
                r = push_state(&worklist, t->to);
                ERR_NOMEM(r < 0, jmt);
            }
        }
    }

    list_for_each(s, jmt->start)
        if (s->reachable && is_return(s))
            s->live = 1;

    bool changed;
    do {
        changed = false;
        list_for_each(s, jmt->start) {
            if (! s->live && s->reachable) {
                for_each_trans(t, s) {
                    if (t->lens != CALL && t->to->live) {
                        s->live = 1;
                        changed = true;
                        break;
                    }
                }
            }
        }
    } while (changed);

    list_for_each(s, jmt->start) {
        if (s->live && s->reachable) {
            for (ind_t i = 0; i < s->trans.used; ) {
                struct trans *t = state_trans(s, i);
                if (! (t->to->live && t->to->reachable))
                    array_remove(&s->trans, i);
                else
                    i += 1;
            }
        }
    }

    removed = 0;
    for (struct state *s = jmt->start;
         s->next != NULL; ) {
        struct state *p = s->next;
        if (p->live && p->reachable) {
            s = p;
        } else {
            s->next = p->next;
            free_state(p);
            removed += 1;
        }
    }

 error:
    array_release(&worklist);
    return;
}

static void dedup(struct state *s) {
    array_for_each(i, s->trans) {
        struct trans *t = state_trans(s, i);
        for (ind_t j = i+1; j < s->trans.used;) {
            struct trans *u = state_trans(s, j);
            if (t->to == u->to && t->lens == u->lens)
                array_remove(&s->trans, j);
            else
                j += 1;
        }
    }
}

static void unepsilon(struct jmt *jmt) {
    int r;

    if (debugging("cf.jmt.build"))
        jmt_dot(jmt, "jmt_10_raw.dot");
    collect(jmt);

    /* Get rid of epsilon transitions */
    bool changed;
    do {
        changed = false;
        list_for_each(s, jmt->start) {
            array_for_each(i , s->trans) {
                struct trans *t = state_trans(s, i);
                if (t->lens == EPS) {
                    struct state *to = t->to;
                    array_remove(&s->trans, i);
                    r = array_join(&s->trans, &to->trans);
                    ERR_NOMEM(r < 0, jmt);
                    state_merge_returns(jmt, s, to);
                    dedup(s);
                    changed = true;
                }
            }
        }
    } while (changed);

    collect(jmt);
    if (debugging("cf.jmt.build"))
        jmt_dot(jmt, "jmt_20_uneps.dot");
 error:
    return;
}

static bool is_deterministic(struct jmt *jmt) {
    list_for_each(s, jmt->start) {
        array_for_each(i, s->trans) {
            struct trans *t = state_trans(s, i);
            for (ind_t j = i+1; j < s->trans.used; j++) {
                struct trans *u = state_trans(s, j);
                if (t->lens == u->lens)
                    return false;
            }
        }
    }
    return true;
}

static void free_nfa_state(struct nfa_state *s) {
    if (s == NULL)
        return;
    array_release(&s->set);
    free(s);
}

static struct nfa_state *make_nfa_state(struct jmt *jmt) {
    struct nfa_state *result = NULL;
    int r;

    r = ALLOC(result);
    ERR_NOMEM(r < 0, jmt);

    array_init(&result->set, sizeof(struct state *));

    return result;
 error:
    FREE(result);
    return NULL;
}

static ind_t nfa_state_add(struct jmt *jmt, struct nfa_state *nfa,
                           struct state *s) {
    ind_t i;
    int r;

    array_for_each(j, nfa->set) {
        struct state *q = *array_elem(nfa->set, j, struct state *);
        if (q == s)
            return j;
    }

    /* Keep the list of states sorted */
    i = nfa->set.used;
    for (int j=0; j + 1 < nfa->set.used; j++) {
        if (s < *array_elem(nfa->set, j, struct state *)) {
            i = j;
            break;
        }
    }
    r = array_insert(&nfa->set, i);
    ERR_NOMEM(r < 0, jmt);

    *array_elem(nfa->set, i, struct state *) = s;
    return i;
 error:
    return IND_MAX;
}

static bool nfa_same_set(struct nfa_state *s1, struct nfa_state *s2) {
    if (s1->set.used != s2->set.used)
        return false;

    array_for_each(i, s1->set) {
        struct state *q1 = *array_elem(s1->set, i, struct state *);
        struct state *q2 = *array_elem(s2->set, i, struct state *);
        if (q1 != q2)
            return false;
    }

    return true;
}

static struct nfa_state *nfa_uniq(struct jmt *jmt, struct array *newstates,
                                  struct nfa_state *s) {
    ind_t ind;
    int r;

    array_each_elem(q, *newstates, struct nfa_state *) {
        if (nfa_same_set(s, *q)) {
            if (s == *q)
                return s;
            free_nfa_state(s);
            return *q;
        }
    }

    r = array_add(newstates, &ind);
    ERR_NOMEM(r < 0, jmt);
    *array_elem(*newstates, ind, struct nfa_state *) = s;

    if (s->state == NULL) {
        s->state = make_state(jmt);
        ERR_BAIL(jmt);
    }

    /* This makes looking at pictures easier */
    if (s->set.used == 1)
        s->state->num = (*array_elem(s->set, 0, struct state *))->num;

    return s;

 error:
    return NULL;
}

static void det_target(struct jmt *jmt, struct array *newstates,
                       struct nfa_state *nfas, ind_t l) {
    struct nfa_state *to = NULL;

    array_each_elem(s, nfas->set, struct state *) {
        for_each_trans(t, *s) {
            if (t->lens == l) {
                if (to == NULL) {
                    to = make_nfa_state(jmt);
                    ERR_RET(jmt);
                }
                nfa_state_add(jmt, to, t->to);
                ERR_RET(jmt);
            }
        }
    }

    if (to != NULL) {
        to = nfa_uniq(jmt, newstates, to);
        ERR_RET(jmt);
        add_new_trans(jmt, nfas->state, to->state, l);
    }
}

static void determinize(struct jmt *jmt) {
    struct nfa_state *ini = NULL;
    struct array *newstates = NULL;
    int r;
    ind_t ind, nlenses;

    if (is_deterministic(jmt))
        return;

    r = ALLOC(newstates);
    ERR_NOMEM(r < 0, jmt);
    array_init(newstates, sizeof(struct nfa_state *));

    nlenses = jmt->lenses.used;

    /* The initial state consists of just the start state */
    ini = make_nfa_state(jmt);
    ERR_BAIL(jmt);
    nfa_state_add(jmt, ini, jmt->start);
    ERR_BAIL(jmt);

    /* Make a new initial state */
    ini->state = make_state(jmt);
    ini->state->num = jmt->start->num;  /* Makes lokking at pictures easier */
    ERR_BAIL(jmt);
    jmt->start->next = ini->state->next;
    ini->state->next = jmt->start;
    jmt->start = ini->state;

    r = array_add(newstates, &ind);
    ERR_NOMEM(r < 0, jmt);

    *array_elem(*newstates, ind, struct nfa_state *) = ini;
    ini = NULL;

    for (ind_t i = 0; i < newstates->used; i++) {
        struct nfa_state *nfas = *array_elem(*newstates, i, struct nfa_state *);

        for (int j = 0; j < nfas->set.used; j++) {
            struct state *s = *array_elem(nfas->set, j, struct state *);
            state_merge_returns(jmt, nfas->state, s);
        }

        for (ind_t l = 0; l < nlenses; l++) {
            det_target(jmt, newstates, nfas, l);
            ERR_BAIL(jmt);
        }
        det_target(jmt, newstates, nfas, CALL);
        ERR_BAIL(jmt);
    }

    collect(jmt);

 done:
    if (newstates) {
        array_each_elem(s, *newstates, struct nfa_state *) {
            free_nfa_state(*s);
        }
        array_release(newstates);
        FREE(newstates);
    }
    free_nfa_state(ini);
    return;
 error:
    goto done;
}

struct jmt *jmt_build(struct lens *lens) {
    struct jmt *jmt = NULL;
    int r;

    r = ALLOC(jmt);
    ERR_NOMEM(r < 0, lens->info);

    jmt->error = lens->info->error;
    array_init(&jmt->lenses, sizeof(struct jmt_lens));

    index_lenses(jmt, lens);

    if (debugging("cf.jmt"))
        print_grammar_top(jmt, lens);

    for (ind_t i=0; i < jmt->lenses.used; i++) {
        conv_rhs(jmt, i);
        ERR_BAIL(jmt);
    }

    unepsilon(jmt);
    ERR_BAIL(jmt);

    determinize(jmt);
    ERR_BAIL(jmt);

    if (debugging("cf.jmt.build"))
        jmt_dot(jmt, "jmt_30_dfa.dot");

    return jmt;
 error:
    jmt_free(jmt);
    return NULL;
}

void jmt_free(struct jmt *jmt) {
    if (jmt == NULL)
        return;
    array_release(&jmt->lenses);
    struct state *s = jmt->start;
    while (s != NULL) {
        struct state *del = s;
        s = del->next;
        free_state(del);
    }
    free(jmt);
}

void jmt_dot(struct jmt *jmt, const char *fname) {
    FILE *fp = debug_fopen("%s", fname);
    if (fp == NULL)
        return;

    fprintf(fp, "digraph \"jmt\" {\n");
    fprintf(fp, "  rankdir = LR;\n");
    list_for_each(s, jmt->start) {
        if (is_return(s)) {
            fprintf(fp, "  %u [ shape = doublecircle, label = \"%u (",
                    s->num, s->num);
            flens(fp, s->ret[0]);
            for (ind_t i = 1; i < s->nret; i++) {
                fprintf(fp, ", ");
                flens(fp, s->ret[i]);
            }
            fprintf(fp, ")\" ];\n");
        }
        for_each_trans(t, s) {
            fprintf(fp, "  %u -> %u ", s->num, t->to->num);
            if (t->lens == EPS)
                fprintf(fp, ";\n");
            else if (t->lens == CALL)
                fprintf(fp, "[ label = \"call\" ];\n");
            else if (lens_state(jmt, t->lens) == NULL) {
                struct lens *lens = lens_of_jmt(jmt, t->lens);
                fprintf(fp, "[ label = \"");
                print_regexp(fp, lens->ctype);
                fprintf(fp, "\" ];\n");
            } else {
                fprintf(fp, "[ label = \"");
                flens(fp, t->lens);
                fprintf(fp, "\" ];\n");
            }
        }
    }
    fprintf(fp, "}\n");
    fclose(fp);
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
