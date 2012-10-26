/*
 * fa.c: finite automata
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

/*
 * This implementation follows closely the Java dk.brics.automaton package
 * by Anders Moeller. The project's website is
 * http://www.brics.dk/automaton/.
 *
 * It is by no means a complete reimplementation of that package; only a
 * subset of what Automaton provides is implemented here.
 */

#include <config.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>

#include "internal.h"
#include "memory.h"
#include "ref.h"
#include "hash.h"
#include "fa.h"

#define UCHAR_NUM (UCHAR_MAX+1)
#define UCHAR_MIN 0
typedef unsigned char uchar;

#define E(cond) if (cond) goto error
#define F(expr) if ((expr) < 0) goto error

/* Which algorithm to use in FA_MINIMIZE */
int fa_minimization_algorithm = FA_MIN_HOPCROFT;

/* A finite automaton. INITIAL is both the initial state and the head of
 * the list of all states. Any state that is allocated for this automaton
 * is put on this list. Dead/unreachable states are cleared from the list
 * at opportune times (e.g., during minimization) It's poor man's garbage
 * collection
 *
 * Normally, transitions are on a character range [min..max]; in
 * fa_as_regexp, we store regexps on transitions in the re field of each
 * transition. TRANS_RE indicates that we do that, and is used by fa_dot to
 * produce proper graphs of an automaton transitioning on regexps.
 *
 * For case-insensitive regexps (nocase == 1), the FA never has transitions
 * on uppercase letters [A-Z], effectively removing these letters from the
 * alphabet.
 */
struct fa {
    struct state *initial;
    int           deterministic : 1;
    int           minimal : 1;
    unsigned int  nocase : 1;
    int           trans_re : 1;
};

/* A state in a finite automaton. Transitions are never shared between
   states so that we can free the list when we need to free the state */
struct state {
    struct state *next;
    hash_val_t    hash;
    unsigned int  accept : 1;
    unsigned int  live : 1;
    unsigned int  reachable : 1;
    unsigned int  visited : 1;   /* Used in various places to track progress */
    /* Array of transitions. The TUSED first entries are used, the array
       has allocated room for TSIZE */
    size_t        tused;
    size_t        tsize;
    struct trans *trans;
};

/* A transition. If the input has a character in the inclusive
 * range [MIN, MAX], move to TO
 */
struct trans {
    struct state *to;
    union {
        struct {
            uchar         min;
            uchar         max;
        };
        struct re *re;
    };
};

/*
 * Bitsets
 */
#define UINT_BIT (sizeof(unsigned int) * CHAR_BIT)

typedef unsigned int bitset;

static bitset *bitset_init(size_t nbits) {
    bitset *bs;
    if (ALLOC_N(bs, (nbits + UINT_BIT) / UINT_BIT) == -1)
        return NULL;
    return bs;
}

static inline void bitset_clr(bitset *bs, unsigned int bit) {
    bs[bit/UINT_BIT] &= ~(1 << (bit % UINT_BIT));
}

static inline void bitset_set(bitset *bs, unsigned int bit) {
    bs[bit/UINT_BIT] |= 1 << (bit % UINT_BIT);
}

ATTRIBUTE_PURE
static inline int bitset_get(const bitset * const bs, unsigned int bit) {
    return (bs[bit/UINT_BIT] >> bit % UINT_BIT) & 1;
}

ATTRIBUTE_PURE
static inline bool bitset_disjoint(const bitset *const bs1,
                                   const bitset *const bs2,
                                   size_t nbits) {
    for (int i=0; i < (nbits + UINT_BIT) / UINT_BIT; i++) {
        if (bs1[i] & bs2[i])
            return false;
    }
    return true;
}

static void bitset_free(bitset *bs) {
    free(bs);
}

static void bitset_negate(bitset *bs, size_t nbits) {
    for (int i=0; i < (nbits + UINT_BIT) / UINT_BIT; i++)
        bs[i] = ~ bs[i];
}

/*
 * Representation of a parsed regular expression. The regular expression is
 * parsed according to the following grammar by PARSE_REGEXP:
 *
 * regexp:        concat_exp ('|' regexp)?
 * concat_exp:    repeated_exp concat_exp?
 * repeated_exp:  simple_exp
 *              | simple_exp '*'
 *              | simple_exp '+'
 *              | simple_exp '?'
 *              | simple_exp '{' INT (',' INT '}')?
 * simple_exp:  char_class
 *            | '.'
 *            |  '(' regexp ')'
 *            |  CHAR
 * char_class: '[' char_exp ']'
 *           | '[' '^' char_exp ']'
 * char_exp:   CHAR '-' CHAR
 *           | CHAR
 */

enum re_type {
    UNION,
    CONCAT,
    CSET,
    CHAR,
    ITER,
    EPSILON
};

#define re_unref(r) unref(r, re)

struct re {
    ref_t        ref;
    enum re_type type;
    union {
        struct {                  /* UNION, CONCAT */
            struct re *exp1;
            struct re *exp2;
        };
        struct {                  /* CSET */
            bool    negate;
            bitset *cset;
            /* Whether we can use character ranges when converting back
             * to a string */
            unsigned int no_ranges:1;
        };
        struct {                  /* CHAR */
            uchar c;
        };
        struct {                  /* ITER */
            struct re *exp;
            int min;
            int max;
        };
    };
};

/* Used to keep state of the regex parse; RX may contain NUL's */
struct re_parse {
    const char *rx;          /* Current position in regex */
    const char *rend;        /* Last char of rx+ 1 */
    int         error;       /* error code */
    /* Whether new CSET's should have the no_ranges flag set */
    unsigned int no_ranges:1;
};

/* String with explicit length, used when converting re to string */
struct re_str {
    char  *rx;
    size_t len;
};

static struct re *parse_regexp(struct re_parse *parse);

/* A map from a set of states to a state. */
typedef hash_t state_set_hash;

static hash_val_t ptr_hash(const void *p);

static const int array_initial_size = 4;
static const int array_max_expansion   = 128;

enum state_set_init_flags {
    S_NONE   = 0,
    S_SORTED = (1 << 0),
    S_DATA   = (1 << 1)
};

struct state_set {
    size_t            size;
    size_t            used;
    unsigned int      sorted : 1;
    unsigned int      with_data : 1;
    struct state    **states;
    void            **data;
};

struct state_set_list {
    struct state_set_list *next;
    struct state_set      *set;
};

/* Clean up FA by removing dead transitions and states and reducing
 * transitions. Unreachable states are freed. The return value is the same
 * as FA; returning it is merely a convenience.
 *
 * Only automata in this state should be returned to the user
 */
ATTRIBUTE_RETURN_CHECK
static int collect(struct fa *fa);

ATTRIBUTE_RETURN_CHECK
static int totalize(struct fa *fa);

/* Print an FA into a (fairly) fixed file if the environment variable
 * FA_DOT_DIR is set. This code is only used for debugging
 */
#define FA_DOT_DIR "FA_DOT_DIR"

ATTRIBUTE_UNUSED
static void fa_dot_debug(struct fa *fa, const char *tag) {
    const char *dot_dir;
    static int count = 0;
    int r;
    char *fname;
    FILE *fp;

    if ((dot_dir = getenv(FA_DOT_DIR)) == NULL)
        return;

    r = asprintf(&fname, "%s/fa_%02d_%s.dot", dot_dir, count++, tag);
    if (r == -1)
        return;

    fp = fopen(fname, "w");
    fa_dot(fp, fa);
    fclose(fp);
    free(fname);
}

static void print_char_set(struct re *set) {
    int from, to;

    if (set->negate)
        printf("[^");
    else
        printf("[");
    for (from = UCHAR_MIN; from <= UCHAR_MAX; from = to+1) {
        while (bitset_get(set->cset, from) == set->negate)
            from += 1;
        if (from > UCHAR_MAX)
            break;
        for (to = from;
             to < UCHAR_MAX && (bitset_get(set->cset, to+1) == !set->negate);
             to++);
        if (to == from) {
            printf("%c", from);
        } else {
            printf("%c-%c", from, to);
        }
    }
    printf("]");
}

ATTRIBUTE_UNUSED
static void print_re(struct re *re) {
    switch(re->type) {
    case UNION:
        print_re(re->exp1);
        printf("|");
        print_re(re->exp2);
        break;
    case CONCAT:
        print_re(re->exp1);
        printf(".");
        print_re(re->exp2);
        break;
    case CSET:
        print_char_set(re);
        break;
    case CHAR:
        printf("%c", re->c);
        break;
    case ITER:
        printf("(");
        print_re(re->exp);
        printf("){%d,%d}", re->min, re->max);
        break;
    case EPSILON:
        printf("<>");
        break;
    default:
        printf("(**)");
        break;
    }
}

/*
 * struct re_str
 */
static void release_re_str(struct re_str *str) {
    if (str == NULL)
        return;
    FREE(str->rx);
    str->len = 0;
}

static void free_re_str(struct re_str *str) {
    if (str == NULL)
        return;
    release_re_str(str);
    FREE(str);
}

static struct re_str *make_re_str(const char *s) {
    struct re_str *str;

    if (ALLOC(str) < 0)
        return NULL;
    if (s != NULL) {
        str->rx = strdup(s);
        str->len = strlen(s);
        if (str->rx == NULL) {
            FREE(str);
            return NULL;
        }
    }
    return str;
}

static int re_str_alloc(struct re_str *str) {
    return ALLOC_N(str->rx, str->len + 1);
}

/*
 * Memory management
 */

static void free_trans(struct state *s) {
    free(s->trans);
    s->trans = NULL;
    s->tused = s->tsize = 0;
}

static void gut(struct fa *fa) {
    list_for_each(s, fa->initial) {
        free_trans(s);
    }
    list_free(fa->initial);
    fa->initial = NULL;
}

void fa_free(struct fa *fa) {
    if (fa == NULL)
        return;
    gut(fa);
    free(fa);
}

static struct state *make_state(void) {
    struct state *s;
    if (ALLOC(s) == -1)
        return NULL;
    s->hash = ptr_hash(s);
    return s;
}

static struct state *add_state(struct fa *fa, int accept) {
    struct state *s = make_state();
    if (s) {
        s->accept = accept;
        if (fa->initial == NULL) {
            fa->initial = s;
        } else {
            list_cons(fa->initial->next, s);
        }
    }
    return s;
}

#define last_trans(s)  ((s)->trans + (s)->tused - 1)

#define for_each_trans(t, s)                                            \
    for (struct trans *t = (s)->trans;                                  \
         (t - (s)->trans) < (s)->tused;                                 \
         t++)

ATTRIBUTE_RETURN_CHECK
static int add_new_trans(struct state *from, struct state *to,
                         uchar min, uchar max) {
    assert(to != NULL);

    if (from->tused == from->tsize) {
        size_t tsize = from->tsize;
        if (tsize == 0)
            tsize = array_initial_size;
        else if (from->tsize > array_max_expansion)
            tsize += array_max_expansion;
        else
            tsize *= 2;
        if (REALLOC_N(from->trans, tsize) == -1)
            return -1;
        from->tsize = tsize;
    }
    from->trans[from->tused].to  = to;
    from->trans[from->tused].min = min;
    from->trans[from->tused].max = max;
    from->tused += 1;
    return 0;
}

ATTRIBUTE_RETURN_CHECK
static int add_epsilon_trans(struct state *from,
                             struct state *to) {
    int r;
    from->accept |= to->accept;
    for_each_trans(t, to) {
        r = add_new_trans(from, t->to, t->min, t->max);
        if (r < 0)
            return -1;
    }
    return 0;
}

static void set_initial(struct fa *fa, struct state *s) {
    list_remove(s, fa->initial);
    list_cons(fa->initial, s);
}

/* Merge automaton FA2 into FA1. This simply adds FA2's states to FA1
   and then frees FA2. It has no influence on the language accepted by FA1
*/
static void fa_merge(struct fa *fa1, struct fa **fa2) {
    list_append(fa1->initial, (*fa2)->initial);
    free(*fa2);
    *fa2 = NULL;
}

/*
 * Operations on STATE_SET
 */
static void state_set_free(struct state_set *set) {
    if (set == NULL)
        return;
    free(set->states);
    free(set->data);
    free(set);
}

static int state_set_init_data(struct state_set *set) {
    set->with_data = 1;
    if (set->data == NULL)
        return ALLOC_N(set->data, set->size);
    else
        return 0;
}

/* Create a new STATE_SET with an initial size of SIZE. If SIZE is -1, use
   the default size ARRAY_INITIAL_SIZE. FLAGS is a bitmask indicating
   some options:
   - S_SORTED: keep the states in the set sorted by their address, and use
     binary search for lookups. If it is not set, entries are kept in the
     order in which they are added and lookups scan linearly through the
     set of states.
   - S_DATA: allocate the DATA array in the set, and keep its size in sync
     with the size of the STATES array.
*/
static struct state_set *state_set_init(int size, int flags) {
    struct state_set *set = NULL;

    F(ALLOC(set));

    set->sorted = (flags & S_SORTED) ? 1 : 0;
    set->with_data = (flags & S_DATA) ? 1 : 0;
    if (size > 0) {
        set->size = size;
        F(ALLOC_N(set->states, set->size));
        if (set->with_data)
            F(state_set_init_data(set));
    }
    return set;
 error:
    state_set_free(set);
    return NULL;
}

ATTRIBUTE_RETURN_CHECK
static int state_set_expand(struct state_set *set) {
    if (set->size == 0)
        set->size = array_initial_size;
    else if (set->size > array_max_expansion)
        set->size += array_max_expansion;
    else
        set->size *= 2;
    if (REALLOC_N(set->states, set->size) < 0)
        goto error;
    if (set->with_data)
        if (REALLOC_N(set->data, set->size) < 0)
            goto error;
    return 0;
 error:
    /* We do this to provoke a SEGV as early as possible */
    FREE(set->states);
    FREE(set->data);
    return -1;
}

/* Return the index where S belongs in SET->STATES to keep it sorted.  S
   may not be in SET->STATES. The returned index is in the interval [0
   .. SET->USED], with the latter indicating that S is larger than all
   values in SET->STATES
*/
static int state_set_pos(const struct state_set *set, const struct state *s) {
    int l = 0, h = set->used;
    while (l < h) {
        int m = (l + h)/2;
        if (set->states[m] > s)
            h = m;
        else if (set->states[m] < s)
            l = m + 1;
        else
            return m;
    }
    return l;
}

ATTRIBUTE_RETURN_CHECK
static int state_set_push(struct state_set *set, struct state *s) {
    if (set->size == set->used)
        if (state_set_expand(set) < 0)
            return -1;
    if (set->sorted) {
        int p = state_set_pos(set, s);
        if (set->size == set->used)
            if (state_set_expand(set) < 0)
                return -1;
        while (p < set->used && set->states[p] <= s)
            p += 1;
        if (p < set->used) {
            memmove(set->states + p + 1, set->states + p,
                    sizeof(*set->states) * (set->used - p));
            if (set->data != NULL)
                memmove(set->data + p + 1, set->data + p,
                        sizeof(*set->data) * (set->used - p));
        }
        set->states[p] = s;
        set->used += 1;
        return p;
    } else {
        set->states[set->used++] = s;
        return set->used - 1;
    }
}

ATTRIBUTE_RETURN_CHECK
static int state_set_push_data(struct state_set *set, struct state *s,
                               void *d) {
    int i = state_set_push(set, s);
    if (i == -1)
        return -1;
    set->data[i] = d;
    return i;
}

static int state_set_index(const struct state_set *set,
                           const struct state *s) {
    if (set->sorted) {
        int p = state_set_pos(set, s);
        return (p < set->used && set->states[p] == s) ? p : -1;
    } else {
        for (int i=0; i < set->used; i++) {
            if (set->states[i] == s)
                return i;
        }
    }
    return -1;
}

static void state_set_remove(struct state_set *set,
                             const struct state *s) {
   if (set->sorted) {
       int p = state_set_index(set, s);
       if (p == -1) return;
       memmove(set->states + p, set->states + p + 1,
               sizeof(*set->states) * (set->used - p - 1));
       if (set->data != NULL)
           memmove(set->data + p, set->data + p + 1,
                   sizeof(*set->data) * (set->used - p - 1));
   } else {
       int p = state_set_index(set, s);
       if (p >= 0) {
           set->states[p] = set->states[--set->used];
       }
   }
}

/* Only add S if it's not in SET yet. Return 1 if S was added, 0 if it was
   already in the set and -1 on error. */
ATTRIBUTE_RETURN_CHECK
static int state_set_add(struct state_set *set, struct state *s) {
    if (set->sorted) {
        int p = state_set_pos(set, s);
        if (p < set->used && set->states[p] == s)
            return 0;
        if (set->size == set->used)
            if (state_set_expand(set) < 0)
                return -1;
        while (p < set->used && set->states[p] <= s)
            p += 1;
        if (p < set->used) {
            memmove(set->states + p + 1, set->states + p,
                    sizeof(*set->states) * (set->used - p));
            if (set->data != NULL)
                memmove(set->data + p + 1, set->data + p,
                        sizeof(*set->data) * (set->used - p));
        }
        set->states[p] = s;
        set->used += 1;
    } else {
        if (state_set_index(set, s) >= 0)
            return 0;
        if (state_set_push(set, s) < 0)
            goto error;
    }
    return 1;
 error:
    /* We do this to provoke a SEGV as early as possible */
    FREE(set->states);
    FREE(set->data);
    return -1;
}

static struct state *state_set_pop(struct state_set *set) {
    struct state *s = NULL;
    if (set->used > 0)
        s = set->states[--set->used];
    return s;
}

static struct state *state_set_pop_data(struct state_set *set, void **d) {
    struct state *s = NULL;
    s = state_set_pop(set);
    *d = set->data[set->used];
    return s;
}

static void *state_set_find_data(struct state_set *set, struct state *s) {
    int i = state_set_index(set, s);
    if (i >= 0)
        return set->data[i];
    else
        return NULL;
}

static int state_set_equal(const struct state_set *set1,
                           const struct state_set *set2) {
    if (set1->used != set2->used)
        return 0;
    if (set1->sorted && set2->sorted) {
        for (int i = 0; i < set1->used; i++)
            if (set1->states[i] != set2->states[i])
                return 0;
        return 1;
    } else {
        for (int i=0; i < set1->used; i++)
            if (state_set_index(set2, set1->states[i]) == -1)
                return 0;
        return 1;
    }
}

#if 0
static void state_set_compact(struct state_set *set) {
    while (set->used > 0 && set->states[set->used] == NULL)
        set->used -= 1;
    for (int i=0; i < set->used; i++) {
        if (set->states[i] == NULL) {
            set->used -= 1;
            set->states[i] = set->states[set->used];
            if (set->data)
                set->data[i] = set->data[set->used];
        }
        while (set->used > 0 && set->states[set->used] == NULL)
            set->used -= 1;
    }
}
#endif

/* Add an entry (FST, SND) to SET. FST is stored in SET->STATES, and SND is
   stored in SET->DATA at the same index.
*/
ATTRIBUTE_RETURN_CHECK
static int state_pair_push(struct state_set **set,
                           struct state *fst, struct state *snd) {
    if (*set == NULL)
        *set = state_set_init(-1, S_DATA);
    if (*set == NULL)
        return -1;
    int i = state_set_push(*set, fst);
    if (i == -1)
        return -1;
    (*set)->data[i] = snd;

    return 0;
}

/* Return the index of the pair (FST, SND) in SET, or -1 if SET contains no
   such pair.
 */
static int state_pair_find(struct state_set *set, struct state *fst,
                           struct state *snd) {
    for (int i=0; i < set->used; i++)
        if (set->states[i] == fst && set->data[i] == snd)
            return i;
    return -1;
}

/* Jenkins' hash for void* */
static hash_val_t ptr_hash(const void *p) {
    hash_val_t hash = 0;
    char *c = (char *) &p;
    for (int i=0; i < sizeof(p); i++) {
        hash += c[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

typedef hash_t state_triple_hash;

static hash_val_t pair_hash(const void *key) {
    register struct state *const *pair = key;
    return pair[0]->hash + pair[1]->hash;
}

static int pair_cmp(const void *key1, const void *key2) {
    return memcmp(key1, key2, 2*sizeof(struct state *));
}

static void state_triple_node_free(hnode_t *node, ATTRIBUTE_UNUSED void *ctx) {
    free((void *) hnode_getkey(node));
    free(node);
}

static state_triple_hash *state_triple_init(void) {
    state_triple_hash *hash;

    hash = hash_create(HASHCOUNT_T_MAX, pair_cmp, pair_hash);
    if (hash == NULL)
        return NULL;
    hash_set_allocator(hash, NULL, state_triple_node_free, NULL);
    return hash;
}

ATTRIBUTE_RETURN_CHECK
static int state_triple_push(state_triple_hash *hash,
                             struct state *s1,
                             struct state *s2,
                             struct state *s3) {
    struct state **pair;
    if (ALLOC_N(pair, 2) < 0)
        return -1;
    pair[0] = s1;
    pair[1] = s2;
    return hash_alloc_insert(hash, pair, s3);
}

static struct state * state_triple_thd(state_triple_hash *hash,
                                       struct state *s1,
                                       struct state *s2) {
    struct state *pair[2];
    hnode_t *node;
    pair[0] = s1;
    pair[1] = s2;
    node = hash_lookup(hash, pair);
    return (node == NULL) ? NULL : (struct state *) hnode_get(node);
}

static void state_triple_free(state_triple_hash *hash) {
    if (hash != NULL) {
        hash_free_nodes(hash);
        hash_destroy(hash);
    }
}

/*
 * State operations
 */
ATTRIBUTE_RETURN_CHECK
static int mark_reachable(struct fa *fa) {
    struct state_set *worklist = state_set_init(-1, S_NONE);
    int result = -1;

    E(worklist == NULL);

    list_for_each(s, fa->initial) {
        s->reachable = 0;
    }
    fa->initial->reachable = 1;

    for (struct state *s = fa->initial;
         s != NULL;
         s = state_set_pop(worklist)) {
        for_each_trans(t, s) {
            if (! t->to->reachable) {
                t->to->reachable = 1;
                F(state_set_push(worklist, t->to));
            }
        }
    }
    result = 0;

 error:
    state_set_free(worklist);
    return result;
}

/* Return all reachable states. As a sideeffect, all states have their
   REACHABLE flag set appropriately.
 */
static struct state_set *fa_states(struct fa *fa) {
    struct state_set *visited = state_set_init(-1, S_NONE);
    int r;

    r = mark_reachable(fa);
    E(visited == NULL || r < 0);

    list_for_each(s, fa->initial) {
        if (s->reachable)
            F(state_set_push(visited, s));
    }
    return visited;
 error:
    state_set_free(visited);
    return NULL;
}

/* Return all reachable accepting states. As a sideeffect, all states have
   their REACHABLE flag set appropriately.
 */
static struct state_set *fa_accept_states(struct fa *fa) {
    struct state_set *accept = state_set_init(-1, S_NONE);
    int r;

    r = mark_reachable(fa);
    E(r < 0);

    list_for_each(s, fa->initial) {
        if (s->reachable && s->accept)
            F(state_set_push(accept, s));
    }
    return accept;
 error:
    return NULL;
}

/* Mark all live states, i.e. states from which an accepting state can be
   reached. All states have their REACHABLE and LIVE flags set
   appropriately.
 */
ATTRIBUTE_RETURN_CHECK
static int mark_live(struct fa *fa) {
    int changed;

    F(mark_reachable(fa));

    list_for_each(s, fa->initial) {
        s->live = s->reachable && s->accept;
    }

    do {
        changed = 0;
        list_for_each(s, fa->initial) {
            if (! s->live && s->reachable) {
                for_each_trans(t, s) {
                    if (t->to->live) {
                        s->live = 1;
                        changed = 1;
                        break;
                    }
                }
            }
        }
    } while (changed);
    return 0;
 error:
    return -1;
}

/*
 * Reverse an automaton in place. Change FA so that it accepts the
 * language that is the reverse of the input automaton.
 *
 * Returns a list of the new initial states of the automaton. The list must
 * be freed by the caller.
 */
static struct state_set *fa_reverse(struct fa *fa) {
    struct state_set *all = NULL;
    struct state_set *accept = NULL;
    int r;

    all = fa_states(fa);
    accept = fa_accept_states(fa);

    F(state_set_init_data(all));

    /* Reverse all transitions */
    int *tused;
    F(ALLOC_N(tused, all->used));
    for (int i=0; i < all->used; i++) {
        all->data[i] = all->states[i]->trans;
        tused[i] = all->states[i]->tused;
        all->states[i]->trans = NULL;
        all->states[i]->tsize = 0;
        all->states[i]->tused = 0;
    }
    for (int i=0; i < all->used; i++) {
        struct state *s = all->states[i];
        struct trans *t = all->data[i];
        s->accept = 0;
        for (int j=0; j < tused[i]; j++) {
            r = add_new_trans(t[j].to, s, t[j].min, t[j].max);
            if (r < 0)
                goto error;
        }
        free(t);
    }
    free(tused);

    /* Make new initial and final states */
    struct state *s = add_state(fa, 0);
    E(s == NULL);

    fa->initial->accept = 1;
    set_initial(fa, s);
    for (int i=0; i < accept->used; i++) {
        r = add_epsilon_trans(s, accept->states[i]);
        if (r < 0)
            goto error;
    }

    fa->deterministic = 0;
    fa->minimal = 0;
    state_set_free(all);
    return accept;
 error:
    state_set_free(all);
    state_set_free(accept);
    return NULL;
}

/*
 * Return a sorted array of all interval start points in FA. The returned
 * array is a string (null terminated)
 */
static uchar* start_points(struct fa *fa, int *npoints) {
    char pointset[UCHAR_NUM];
    uchar *points = NULL;

    F(mark_reachable(fa));
    MEMZERO(pointset, UCHAR_NUM);
    list_for_each(s, fa->initial) {
        if (! s->reachable)
            continue;
        pointset[0] = 1;
        for_each_trans(t, s) {
            pointset[t->min] = 1;
            if (t->max < UCHAR_MAX)
                pointset[t->max+1] = 1;
        }
    }

    *npoints = 0;
    for(int i=0; i < UCHAR_NUM; *npoints += pointset[i], i++);

    F(ALLOC_N(points, *npoints+1));
    for (int i=0, n=0; i < UCHAR_NUM; i++) {
        if (pointset[i])
            points[n++] = (uchar) i;
    }

    return points;
 error:
    free(points);
    return NULL;
}

/*
 * Operations on STATE_SET_HASH
 */
static int state_set_hash_contains(state_set_hash *smap,
                               struct state_set *set) {
    return hash_lookup(smap, set) != NULL;
}

/*
 * Find the set in SMAP that has the same states as SET. If the two are
 * different, i.e. they point to different memory locations, free SET and
 * return the set found in SMAP
 */
static struct state_set *state_set_hash_uniq(state_set_hash *smap,
                                             struct state_set *set) {
    hnode_t *node = hash_lookup(smap, set);
    const struct state_set *orig_set = hnode_getkey(node);
    if (orig_set != set) {
        state_set_free(set);
    }
    return (struct state_set *) orig_set;
}

static struct state *state_set_hash_get_state(state_set_hash *smap,
                                             struct state_set *set) {
    hnode_t *node = hash_lookup(smap, set);
    return (struct state *) hnode_get(node);
}

static hash_val_t set_hash(const void *key) {
    hash_val_t hash = 0;
    const struct state_set *set = key;

    for (int i = 0; i < set->used; i++) {
        hash += set->states[i]->hash;
    }
    return hash;
}

static int set_cmp(const void *key1, const void *key2) {
    const struct state_set *set1 = key1;
    const struct state_set *set2 = key2;

    return state_set_equal(set1, set2) ? 0 : 1;
}

static void set_destroy(hnode_t *node, ATTRIBUTE_UNUSED void *ctx) {
    struct state_set *set = (struct state_set *) hnode_getkey(node);
    state_set_free(set);
    free(node);
}

ATTRIBUTE_RETURN_CHECK
static int state_set_hash_add(state_set_hash **smap,
                              struct state_set *set, struct fa *fa) {
    if (*smap == NULL) {
        *smap = hash_create(HASHCOUNT_T_MAX, set_cmp, set_hash);
        E(*smap == NULL);
        hash_set_allocator(*smap, NULL, set_destroy, NULL);
    }
    struct state *s = add_state(fa, 0);
    E(s == NULL);
    F(hash_alloc_insert(*smap, set, s));
    return 0;
 error:
    return -1;
}

static void state_set_hash_free(state_set_hash *smap,
                                struct state_set *protect) {
    if (protect != NULL) {
        hnode_t *node = hash_lookup(smap, protect);
        hash_delete(smap, node);
        hnode_getkey(node) = NULL;
        set_destroy(node, NULL);
    }
    hash_free_nodes(smap);
    hash_destroy(smap);
}

static int state_set_list_add(struct state_set_list **list,
                              struct state_set *set) {
    struct state_set_list *elt;
    if (ALLOC(elt) < 0)
        return -1;
    elt->set = set;
    list_cons(*list, elt);
    return 0;
}

static struct state_set *state_set_list_pop(struct state_set_list **list) {
    struct state_set_list *elt = *list;
    struct state_set      *set = elt->set;

    *list = elt->next;
    free(elt);
    return set;
}

/* Compare transitions lexicographically by (to, min, reverse max) */
static int trans_to_cmp(const void *v1, const void *v2) {
    const struct trans *t1 = v1;
    const struct trans *t2 = v2;

    if (t1->to != t2->to) {
        return (t1->to < t2->to) ? -1 : 1;
    }
    if (t1->min < t2->min)
        return -1;
    if (t1->min > t2->min)
        return 1;
    if (t1->max > t2->max)
        return -1;
    return (t1->max < t2->max) ? 1 : 0;
}

/* Compare transitions lexicographically by (min, reverse max, to) */
static int trans_intv_cmp(const void *v1, const void *v2) {
    const struct trans *t1 = v1;
    const struct trans *t2 = v2;

    if (t1->min < t2->min)
        return -1;
    if (t1->min > t2->min)
        return 1;
    if (t1->max > t2->max)
        return -1;
    if (t1->max < t2->max)
        return 1;
    if (t1->to != t2->to) {
        return (t1->to < t2->to) ? -1 : 1;
    }
    return 0;
}

/*
 * Reduce an automaton by combining overlapping and adjacent edge intervals
 * with the same destination.
 */
static void reduce(struct fa *fa) {
    list_for_each(s, fa->initial) {
        if (s->tused == 0)
            continue;

        qsort(s->trans, s->tused, sizeof(*s->trans), trans_to_cmp);
        int i=0, j=1;
        struct trans *t = s->trans;
        while (j < s->tused) {
            if (t[i].to == t[j].to && t[j].min <= t[i].max + 1) {
                if (t[j].max > t[i].max)
                    t[i].max = t[j].max;
                j += 1;
            } else {
                i += 1;
                if (i < j)
                    memmove(s->trans + i, s->trans + j,
                            sizeof(*s->trans) * (s->tused - j));
                s->tused -= j - i;
                j = i + 1;
            }
        }
        s->tused = i+1;
        /* Shrink if we use less than half the allocated size */
        if (s->tsize > array_initial_size && 2*s->tused < s->tsize) {
            int r;
            r = REALLOC_N(s->trans, s->tsize);
            if (r == 0)
                s->tsize = s->tused;
        }
    }
}

/*
 * Remove dead transitions from an FA; a transition is dead is it does not
 * lead to a live state. This also removes any states that are not
 * reachable any longer from FA->INITIAL.
 *
 * Returns the same FA as a convenience
 */

static void collect_trans(struct fa *fa) {
    list_for_each(s, fa->initial) {
        if (! s->live) {
            free_trans(s);
        } else {
            int i=0;
            while (i < s->tused) {
                if (! s->trans[i].to->live) {
                    s->tused -= 1;
                    memmove(s->trans + i, s->trans + s->tused,
                            sizeof(*s->trans));
                } else {
                    i += 1;
                }
            }
        }
    }
}

static void collect_dead_states(struct fa *fa) {
    /* Remove all dead states and free their storage */
    for (struct state *s = fa->initial; s->next != NULL; ) {
        if (! s->next->live) {
            struct state *del = s->next;
            s->next = del->next;
            free_trans(del);
            free(del);
        } else {
            s = s->next;
        }
    }
}

static int collect(struct fa *fa) {
    F(mark_live(fa));

    if (! fa->initial->live) {
        /* This automaton accepts nothing, make it the canonical
         * epsilon automaton
         */
        list_for_each(s, fa->initial) {
            free_trans(s);
        }
        list_free(fa->initial->next);
        fa->deterministic = 1;
    } else {
        collect_trans(fa);
        collect_dead_states(fa);
    }
    reduce(fa);
    return 0;
 error:
    return -1;
}

static void swap_initial(struct fa *fa) {
    struct state *s = fa->initial;
    if (s->next != NULL) {
        fa->initial = s->next;
        s->next = fa->initial->next;
        fa->initial->next = s;
    }
}

/*
 * Make a finite automaton deterministic using the given set of initial
 * states with the subset construction. This also eliminates dead states
 * and transitions and reduces and orders the transitions for each state
 */
static int determinize(struct fa *fa, struct state_set *ini) {
    int npoints;
    int make_ini = (ini == NULL);
    const uchar *points = NULL;
    state_set_hash *newstate = NULL;
    struct state_set_list *worklist = NULL;
    int ret = 0;

    if (fa->deterministic)
        return 0;

    points = start_points(fa, &npoints);
    E(points == NULL);
    if (make_ini) {
        ini = state_set_init(-1, S_NONE);
        if (ini == NULL || state_set_push(ini, fa->initial) < 0)
            goto error;
    }

    F(state_set_list_add(&worklist, ini));
    F(state_set_hash_add(&newstate, ini, fa));
    // Make the new state the initial state
    swap_initial(fa);
    while (worklist != NULL) {
        struct state_set *sset = state_set_list_pop(&worklist);
        struct state *r = state_set_hash_get_state(newstate, sset);
        for (int q=0; q < sset->used; q++) {
            r->accept |= sset->states[q]->accept;
        }
        for (int n=0; n < npoints; n++) {
            struct state_set *pset = state_set_init(-1, S_SORTED);
            E(pset == NULL);
            for(int q=0 ; q < sset->used; q++) {
                for_each_trans(t, sset->states[q]) {
                    if (t->min <= points[n] && points[n] <= t->max) {
                        F(state_set_add(pset, t->to));
                    }
                }
            }
            if (!state_set_hash_contains(newstate, pset)) {
                F(state_set_list_add(&worklist, pset));
                F(state_set_hash_add(&newstate, pset, fa));
            }
            pset = state_set_hash_uniq(newstate, pset);

            struct state *q = state_set_hash_get_state(newstate, pset);
            uchar min = points[n];
            uchar max = UCHAR_MAX;
            if (n+1 < npoints)
                max = points[n+1] - 1;
            if (add_new_trans(r, q, min, max) < 0)
                goto error;
        }
    }
    fa->deterministic = 1;

 done:
    if (newstate)
        state_set_hash_free(newstate, make_ini ? NULL : ini);
    free((void *) points);
    if (collect(fa) < 0)
        ret = -1;
    return ret;
 error:
    ret = -1;
    goto done;
}

/*
 * Minimization. As a sideeffect of minimization, the transitions are
 * reduced and ordered.
 */

static struct state *step(struct state *s, uchar c) {
    for_each_trans(t, s) {
        if (t->min <= c && c <= t->max)
            return t->to;
    }
    return NULL;
}

struct state_list {
    struct state_list_node *first;
    struct state_list_node *last;
    unsigned int            size;
};

struct state_list_node {
    struct state_list      *sl;
    struct state_list_node *next;
    struct state_list_node *prev;
    struct state           *state;
};

static struct state_list_node *state_list_add(struct state_list *sl,
                                              struct state *s) {
    struct state_list_node *n;

    if (ALLOC(n) < 0)
        return NULL;

    n->state = s;
    n->sl = sl;

    if (sl->size++ == 0) {
        sl->first = n;
        sl->last = n;
    } else {
        sl->last->next = n;
        n->prev = sl->last;
        sl->last = n;
    }
    return n;
}

static void state_list_remove(struct state_list_node *n) {
    struct state_list *sl = n->sl;
    sl->size -= 1;
    if (sl->first == n)
        sl->first = n->next;
    else
        n->prev->next = n->next;
    if (sl->last == n)
        sl->last = n->prev;
    else
        n->next->prev = n->prev;

    free(n);
}

static void state_list_free(struct state_list *sl) {
    if (sl)
        list_free(sl->first);
    free(sl);
}

/* The linear index of element (q,c) in an NSTATES * NSIGMA matrix */
#define INDEX(q, c) (q * nsigma + c)

static int minimize_hopcroft(struct fa *fa) {
    struct state_set *states = NULL;
    uchar *sigma = NULL;
    struct state_set **reverse = NULL;
    bitset *reverse_nonempty = NULL;
    struct state_set **partition = NULL;
    unsigned int *block = NULL;
    struct state_list **active = NULL;
    struct state_list_node **active2 = NULL;
    int *pending = NULL;
    bitset *pending2 = NULL;
    struct state_set *split = NULL;
    bitset *split2 = NULL;
    int *refine = NULL;
    bitset *refine2 = NULL;
    struct state_set **splitblock = NULL;
    struct state_set *newstates = NULL;
    int *nsnum = NULL;
    int *nsind = NULL;
    int result = -1;

    F(determinize(fa, NULL));

    /* Total automaton, nothing to do */
    if (fa->initial->tused == 1
        && fa->initial->trans[0].to == fa->initial
        && fa->initial->trans[0].min == UCHAR_MIN
        && fa->initial->trans[0].max == UCHAR_MAX)
        return 0;

    F(totalize(fa));

    /* make arrays for numbered states and effective alphabet */
    states = state_set_init(-1, S_NONE);
    E(states == NULL);

    list_for_each(s, fa->initial) {
        F(state_set_push(states, s));
    }
    unsigned int nstates = states->used;

    int nsigma;
    sigma = start_points(fa, &nsigma);
    E(sigma == NULL);

    /* initialize data structures */

    /* An ss->used x nsigma matrix of lists of states */
    F(ALLOC_N(reverse, nstates * nsigma));
    reverse_nonempty = bitset_init(nstates * nsigma);
    E(reverse_nonempty == NULL);
    F(ALLOC_N(partition, nstates));
    F(ALLOC_N(block, nstates));

    F(ALLOC_N(active, nstates * nsigma));
    F(ALLOC_N(active2, nstates * nsigma));

    /* PENDING is an array of pairs of ints. The i'th pair is stored in
     * PENDING[2*i] and PENDING[2*i + 1]. There are NPENDING pairs in
     * PENDING at any time. SPENDING is the maximum number of pairs
     * allocated for PENDING.
     */
    size_t npending = 0, spending = 0;
    pending2 = bitset_init(nstates * nsigma);
    E(pending2 == NULL);

    split = state_set_init(-1, S_NONE);
    split2 = bitset_init(nstates);
    E(split == NULL || split2 == NULL);

    F(ALLOC_N(refine, nstates));
    refine2 = bitset_init(nstates);
    E(refine2 == NULL);

    F(ALLOC_N(splitblock, nstates));

    for (int q = 0; q < nstates; q++) {
        splitblock[q] = state_set_init(-1, S_NONE);
        partition[q] = state_set_init(-1, S_NONE);
        E(splitblock[q] == NULL || partition[q] == NULL);
        for (int x = 0; x < nsigma; x++) {
            reverse[INDEX(q, x)] = state_set_init(-1, S_NONE);
            E(reverse[INDEX(q, x)] == NULL);
            F(ALLOC_N(active[INDEX(q, x)], 1));
        }
    }

    /* find initial partition and reverse edges */
    for (int q = 0; q < nstates; q++) {
        struct state *qq = states->states[q];
        int j;
        if (qq->accept)
            j = 0;
        else
            j = 1;
        F(state_set_push(partition[j], qq));
        block[q] = j;
        for (int x = 0; x < nsigma; x++) {
            uchar y = sigma[x];
            struct state *p = step(qq, y);
            assert(p != NULL);
            int pn = state_set_index(states, p);
            assert(pn >= 0);
            F(state_set_push(reverse[INDEX(pn, x)], qq));
            bitset_set(reverse_nonempty, INDEX(pn, x));
        }
    }

    /* initialize active sets */
    for (int j = 0; j <= 1; j++)
        for (int x = 0; x < nsigma; x++)
            for (int q = 0; q < partition[j]->used; q++) {
                struct state *qq = partition[j]->states[q];
                int qn = state_set_index(states, qq);
                if (bitset_get(reverse_nonempty, INDEX(qn, x))) {
                    active2[INDEX(qn, x)] =
                        state_list_add(active[INDEX(j, x)], qq);
                    E(active2[INDEX(qn, x)] == NULL);
                }
            }

    /* initialize pending */
    F(ALLOC_N(pending, 2*nsigma));
    npending = nsigma;
    spending = nsigma;
    for (int x = 0; x < nsigma; x++) {
        int a0 = active[INDEX(0,x)]->size;
        int a1 = active[INDEX(1,x)]->size;
        int j;
        if (a0 <= a1)
            j = 0;
        else
            j = 1;
        pending[2*x] = j;
        pending[2*x+1] = x;
        bitset_set(pending2, INDEX(j, x));
    }

    /* process pending until fixed point */
    int k = 2;
    while (npending-- > 0) {
        int p = pending[2*npending];
        int x = pending[2*npending+1];
        bitset_clr(pending2, INDEX(p, x));
        int ref = 0;
        /* find states that need to be split off their blocks */
        struct state_list *sh = active[INDEX(p,x)];
        for (struct state_list_node *m = sh->first; m != NULL; m = m->next) {
            int q = state_set_index(states, m->state);
            struct state_set *rev = reverse[INDEX(q, x)];
            for (int r =0; r < rev->used; r++) {
                struct state *rs = rev->states[r];
                int s = state_set_index(states, rs);
                if (! bitset_get(split2, s)) {
                    bitset_set(split2, s);
                    F(state_set_push(split, rs));
                    int j = block[s];
                    F(state_set_push(splitblock[j], rs));
                    if (!bitset_get(refine2, j)) {
                        bitset_set(refine2, j);
                        refine[ref++] = j;
                    }
                }
            }
        }
        // refine blocks
        for(int rr=0; rr < ref; rr++) {
            int j = refine[rr];
            struct state_set *sp = splitblock[j];
            if (sp->used < partition[j]->used) {
                struct state_set *b1 = partition[j];
                struct state_set *b2 = partition[k];
                for (int s = 0; s < sp->used; s++) {
                    state_set_remove(b1, sp->states[s]);
                    F(state_set_push(b2, sp->states[s]));
                    int snum = state_set_index(states, sp->states[s]);
                    block[snum] = k;
                    for (int c = 0; c < nsigma; c++) {
                        struct state_list_node *sn = active2[INDEX(snum, c)];
                        if (sn != NULL && sn->sl == active[INDEX(j,c)]) {
                            state_list_remove(sn);
                            active2[INDEX(snum, c)] =
                                state_list_add(active[INDEX(k, c)],
                                               sp->states[s]);
                            E(active2[INDEX(snum, c)] == NULL);
                        }
                    }
                }
                // update pending
                for (int c = 0; c < nsigma; c++) {
                    int aj = active[INDEX(j, c)]->size;
                    int ak = active[INDEX(k, c)]->size;
                    if (npending + 1 > spending) {
                        spending *= 2;
                        F(REALLOC_N(pending, 2 * spending));
                    }
                    pending[2*npending + 1] = c;
                    if (!bitset_get(pending2, INDEX(j, c))
                        && 0 < aj && aj <= ak) {
                        bitset_set(pending2, INDEX(j, c));
                        pending[2*npending] = j;
                    } else {
                        bitset_set(pending2, INDEX(k, c));
                        pending[2*npending] = k;
                    }
                    npending += 1;
                }
                k++;
            }
            for (int s = 0; s < sp->used; s++) {
                int snum = state_set_index(states, sp->states[s]);
                bitset_clr(split2, snum);
            }
            bitset_clr(refine2, j);
            sp->used = 0;
        }
        split->used = 0;
    }

    /* make a new state for each equivalence class, set initial state */
    newstates = state_set_init(k, S_NONE);
    E(newstates == NULL);
    F(ALLOC_N(nsnum, k));
    F(ALLOC_N(nsind, nstates));

    for (int n = 0; n < k; n++) {
        struct state *s = make_state();
        E(s == NULL);
        newstates->states[n] = s;
        struct state_set *partn = partition[n];
        for (int q=0; q < partn->used; q++) {
            struct state *qs = partn->states[q];
            int qnum = state_set_index(states, qs);
            if (qs == fa->initial)
                s->live = 1;     /* Abuse live to flag the new intial state */
            nsnum[n] = qnum;     /* select representative */
            nsind[qnum] = n;     /* and point from partition to new state */
        }
    }

    /* build transitions and set acceptance */
    for (int n = 0; n < k; n++) {
        struct state *s = newstates->states[n];
        s->accept = states->states[nsnum[n]]->accept;
        for_each_trans(t, states->states[nsnum[n]]) {
            int toind = state_set_index(states, t->to);
            struct state *nto = newstates->states[nsind[toind]];
            F(add_new_trans(s, nto, t->min, t->max));
        }
    }

    /* Get rid of old states and transitions and turn NEWTSTATES into
       a linked list */
    gut(fa);
    for (int n=0; n < k; n++)
        if (newstates->states[n]->live) {
            struct state *ini = newstates->states[n];
            newstates->states[n] = newstates->states[0];
            newstates->states[0] = ini;
        }
    for (int n=0; n < k-1; n++)
        newstates->states[n]->next = newstates->states[n+1];
    fa->initial = newstates->states[0];

    result = 0;

    /* clean up */
 done:
    free(nsind);
    free(nsnum);
    state_set_free(states);
    free(sigma);
    bitset_free(reverse_nonempty);
    free(block);
    for (int i=0; i < nstates*nsigma; i++) {
        if (reverse)
            state_set_free(reverse[i]);
        if (active)
            state_list_free(active[i]);
    }
    free(reverse);
    free(active);
    free(active2);
    free(pending);
    bitset_free(pending2);
    state_set_free(split);
    bitset_free(split2);
    free(refine);
    bitset_free(refine2);
    for (int q=0; q < nstates; q++) {
        if (splitblock)
            state_set_free(splitblock[q]);
        if (partition)
            state_set_free(partition[q]);
    }
    free(splitblock);
    free(partition);
    state_set_free(newstates);

    if (collect(fa) < 0)
        result = -1;
    return result;
 error:
    result = -1;
    goto done;
}

static int minimize_brzozowski(struct fa *fa) {
    struct state_set *set;

    /* Minimize using Brzozowski's algorithm */
    set = fa_reverse(fa);
    E(set == NULL);
    F(determinize(fa, set));
    state_set_free(set);

    set = fa_reverse(fa);
    E(set == NULL);
    F(determinize(fa, set));
    state_set_free(set);
    return 0;
 error:
    return -1;
}

int fa_minimize(struct fa *fa) {
    int r;

    if (fa == NULL)
        return -1;
    if (fa->minimal)
        return 0;

    if (fa_minimization_algorithm == FA_MIN_BRZOZOWSKI) {
        r = minimize_brzozowski(fa);
    } else {
        r = minimize_hopcroft(fa);
    }

    if (r == 0)
        fa->minimal = 1;
    return r;
}

/*
 * Construction of fa
 */

static struct fa *fa_make_empty(void) {
    struct fa *fa;

    if (ALLOC(fa) < 0)
        return NULL;
    if (add_state(fa, 0) == NULL) {
        fa_free(fa);
        return NULL;
    }
    fa->deterministic = 1;
    fa->minimal = 1;
    return fa;
}

static struct fa *fa_make_epsilon(void) {
    struct fa *fa = fa_make_empty();
    if (fa) {
        fa->initial->accept = 1;
        fa->deterministic= 1;
        fa->minimal = 1;
    }
    return fa;
}

static struct fa *fa_make_char(uchar c) {
    struct fa *fa = fa_make_empty();
    if (! fa)
        return NULL;

    struct state *s = fa->initial;
    struct state *t = add_state(fa, 1);
    int r;

    if (t == NULL)
        goto error;

    r = add_new_trans(s, t, c, c);
    if (r < 0)
        goto error;
    fa->deterministic = 1;
    fa->minimal = 1;
    return fa;
 error:
    fa_free(fa);
    return NULL;
}

struct fa *fa_make_basic(unsigned int basic) {
    int r;

    if (basic == FA_EMPTY) {
        return fa_make_empty();
    } else if (basic == FA_EPSILON) {
        return fa_make_epsilon();
    } else if (basic == FA_TOTAL) {
        struct fa *fa = fa_make_epsilon();
        r = add_new_trans(fa->initial, fa->initial, UCHAR_MIN, UCHAR_MAX);
        if (r < 0) {
            fa_free(fa);
            fa = NULL;
        }
        return fa;
    }
    return NULL;
}

int fa_is_basic(struct fa *fa, unsigned int basic) {
    if (basic == FA_EMPTY) {
        return ! fa->initial->accept && fa->initial->tused == 0;
    } else if (basic == FA_EPSILON) {
        return fa->initial->accept && fa->initial->tused == 0;
    } else if (basic == FA_TOTAL) {
        if (! fa->initial->accept)
            return 0;
        if (fa->nocase) {
            if (fa->initial->tused != 2)
                return 0;
            struct trans *t1 = fa->initial->trans;
            struct trans *t2 = fa->initial->trans + 1;
            if (t1->to != fa->initial || t2->to != fa->initial)
                return 0;
            if (t2->max != UCHAR_MAX) {
                t1 = t2;
                t2 = fa->initial->trans;
            }
            return (t1->min == UCHAR_MIN && t1->max == 'A' - 1 &&
                    t2->min == 'Z' + 1 && t2->max == UCHAR_MAX);
        } else {
            struct trans *t = fa->initial->trans;
            return fa->initial->tused == 1 &&
                t->to == fa->initial &&
                t->min == UCHAR_MIN && t->max == UCHAR_MAX;
        }
    }
    return 0;
}

static struct fa *fa_clone(struct fa *fa) {
    struct fa *result = NULL;
    struct state_set *set = state_set_init(-1, S_DATA|S_SORTED);
    int r;

    if (fa == NULL || set == NULL || ALLOC(result) < 0)
        goto error;

    result->deterministic = fa->deterministic;
    result->minimal = fa->minimal;
    result->nocase = fa->nocase;
    list_for_each(s, fa->initial) {
        int i = state_set_push(set, s);
        E(i < 0);

        struct state *q = add_state(result, s->accept);
        if (q == NULL)
            goto error;
        set->data[i] = q;
        q->live = s->live;
        q->reachable = s->reachable;
    }
    for (int i=0; i < set->used; i++) {
        struct state *s = set->states[i];
        struct state *sc = set->data[i];
        for_each_trans(t, s) {
            int to = state_set_index(set, t->to);
            assert(to >= 0);
            struct state *toc = set->data[to];
            r = add_new_trans(sc, toc, t->min, t->max);
            if (r < 0)
                goto error;
        }
    }
    state_set_free(set);
    return result;
 error:
    state_set_free(set);
    fa_free(result);
    return NULL;
}

static int case_expand(struct fa *fa);

/* Compute FA1|FA2 and set FA1 to that automaton. FA2 is freed */
ATTRIBUTE_RETURN_CHECK
static int union_in_place(struct fa *fa1, struct fa **fa2) {
    struct state *s;
    int r;

    if (fa1->nocase != (*fa2)->nocase) {
        if (case_expand(fa1) < 0)
            return -1;
        if (case_expand(*fa2) < 0)
            return -1;
    }

    s = add_state(fa1, 0);
    if (s == NULL)
        return -1;
    r = add_epsilon_trans(s, fa1->initial);
    if (r < 0)
        return -1;
    r = add_epsilon_trans(s, (*fa2)->initial);
    if (r < 0)
        return -1;

    fa1->deterministic = 0;
    fa1->minimal = 0;
    fa_merge(fa1, fa2);

    set_initial(fa1, s);

    return 0;
}

struct fa *fa_union(struct fa *fa1, struct fa *fa2) {
    fa1 = fa_clone(fa1);
    fa2 = fa_clone(fa2);
    if (fa1 == NULL || fa2 == NULL)
        goto error;

    F(union_in_place(fa1, &fa2));

    return fa1;
 error:
    fa_free(fa1);
    fa_free(fa2);
    return NULL;
}

/* Concat FA2 onto FA1; frees FA2 and changes FA1 to FA1.FA2 */
ATTRIBUTE_RETURN_CHECK
static int concat_in_place(struct fa *fa1, struct fa **fa2) {
    int r;

    if (fa1->nocase != (*fa2)->nocase) {
        if (case_expand(fa1) < 0)
            return -1;
        if (case_expand(*fa2) < 0)
            return -1;
    }

    list_for_each(s, fa1->initial) {
        if (s->accept) {
            s->accept = 0;
            r = add_epsilon_trans(s, (*fa2)->initial);
            if (r < 0)
                return -1;
        }
    }

    fa1->deterministic = 0;
    fa1->minimal = 0;
    fa_merge(fa1, fa2);

    return 0;
}

struct fa *fa_concat(struct fa *fa1, struct fa *fa2) {
    fa1 = fa_clone(fa1);
    fa2 = fa_clone(fa2);

    if (fa1 == NULL || fa2 == NULL)
        goto error;

    F(concat_in_place(fa1, &fa2));

    F(collect(fa1));

    return fa1;

 error:
    fa_free(fa1);
    fa_free(fa2);
    return NULL;
}

static struct fa *fa_make_char_set(bitset *cset, int negate) {
    struct fa *fa = fa_make_empty();
    if (!fa)
        return NULL;

    struct state *s = fa->initial;
    struct state *t = add_state(fa, 1);
    int from = 0;
    int r;

    if (t == NULL)
        goto error;

    while (from <= UCHAR_MAX) {
        while (from <= UCHAR_MAX && bitset_get(cset, from) == negate)
            from += 1;
        if (from > UCHAR_MAX)
            break;
        int to = from;
        while (to < UCHAR_MAX && (bitset_get(cset, to + 1) == !negate))
            to += 1;
        r = add_new_trans(s, t, from, to);
        if (r < 0)
            goto error;
        from = to + 1;
    }

    fa->deterministic = 1;
    fa->minimal = 1;
    return fa;

 error:
    fa_free(fa);
    return NULL;
}

static struct fa *fa_star(struct fa *fa) {
    struct state *s;
    int r;

    fa = fa_clone(fa);
    if (fa == NULL)
        return NULL;

    s = add_state(fa, 1);
    if (s == NULL)
        goto error;

    r = add_epsilon_trans(s, fa->initial);
    if (r < 0)
        goto error;

    set_initial(fa, s);
    list_for_each(p, fa->initial->next) {
        if (p->accept) {
            r = add_epsilon_trans(p, s);
            if (r < 0)
                goto error;
        }
    }
    fa->deterministic = 0;
    fa->minimal = 0;

    return fa;

 error:
    fa_free(fa);
    return NULL;
}

/* Form the automaton (FA){N}; FA is not modified */
static struct fa *repeat(struct fa *fa, int n) {
    if (n == 0) {
        return fa_make_epsilon();
    } else if (n == 1) {
        return fa_clone(fa);
    } else {
        struct fa *cfa = fa_clone(fa);
        if (cfa == NULL)
            return NULL;
        while (n > 1) {
            struct fa *tfa = fa_clone(fa);
            if (tfa == NULL) {
                fa_free(cfa);
                return NULL;
            }
            if (concat_in_place(cfa, &tfa) < 0) {
                fa_free(cfa);
                fa_free(tfa);
                return NULL;
            }
            n -= 1;
        }
        return cfa;
    }
}

struct fa *fa_iter(struct fa *fa, int min, int max) {
    int r;

    if (min < 0)
        min = 0;

    if (min > max && max != -1) {
        return fa_make_empty();
    }
    if (max == -1) {
        struct fa *sfa = fa_star(fa);
        if (min == 0)
            return sfa;
        if (! sfa)
            return NULL;
        struct fa *cfa = repeat(fa, min);
        if (! cfa) {
            fa_free(sfa);
            return NULL;
        }
        if (concat_in_place(cfa, &sfa) < 0) {
            fa_free(sfa);
            fa_free(cfa);
            return NULL;
        }
        return cfa;
    } else {
        struct fa *cfa = NULL;

        max -= min;
        cfa = repeat(fa, min);
        if (cfa == NULL)
            return NULL;
        if (max > 0) {
            struct fa *cfa2 = fa_clone(fa);
            if (cfa2 == NULL) {
                fa_free(cfa);
                return NULL;
            }
            while (max > 1) {
                struct fa *cfa3 = fa_clone(fa);
                if (cfa3 == NULL) {
                    fa_free(cfa);
                    fa_free(cfa2);
                    return NULL;
                }
                list_for_each(s, cfa3->initial) {
                    if (s->accept) {
                        r = add_epsilon_trans(s, cfa2->initial);
                        if (r < 0) {
                            fa_free(cfa);
                            fa_free(cfa2);
                            fa_free(cfa3);
                            return NULL;
                        }
                    }
                }
                fa_merge(cfa3, &cfa2);
                cfa2 = cfa3;
                max -= 1;
            }
            list_for_each(s, cfa->initial) {
                if (s->accept) {
                    r = add_epsilon_trans(s, cfa2->initial);
                    if (r < 0) {
                        fa_free(cfa);
                        fa_free(cfa2);
                        return NULL;
                    }
                }
            }
            fa_merge(cfa, &cfa2);
            cfa->deterministic = 0;
            cfa->minimal = 0;
        }
        if (collect(cfa) < 0) {
            fa_free(cfa);
            cfa = NULL;
        }
        return cfa;
    }
}

static void sort_transition_intervals(struct fa *fa) {
    list_for_each(s, fa->initial) {
        qsort(s->trans, s->tused, sizeof(*s->trans), trans_intv_cmp);
    }
}

struct fa *fa_intersect(struct fa *fa1, struct fa *fa2) {
    int ret;
    struct fa *fa = NULL;
    struct state_set *worklist = NULL;
    state_triple_hash *newstates = NULL;

    if (fa1 == fa2)
        return fa_clone(fa1);

    if (fa_is_basic(fa1, FA_EMPTY) || fa_is_basic(fa2, FA_EMPTY))
        return fa_make_empty();

    if (fa1->nocase != fa2->nocase) {
        F(case_expand(fa1));
        F(case_expand(fa2));
    }

    fa = fa_make_empty();
    worklist = state_set_init(-1, S_NONE);
    newstates = state_triple_init();
    if (fa == NULL || worklist == NULL || newstates == NULL)
        goto error;

    sort_transition_intervals(fa1);
    sort_transition_intervals(fa2);

    F(state_set_push(worklist, fa1->initial));
    F(state_set_push(worklist, fa2->initial));
    F(state_set_push(worklist, fa->initial));
    F(state_triple_push(newstates,
                         fa1->initial, fa2->initial, fa->initial));
    while (worklist->used) {
        struct state *s  = state_set_pop(worklist);
        struct state *p2 = state_set_pop(worklist);
        struct state *p1 = state_set_pop(worklist);
        s->accept = p1->accept && p2->accept;

        struct trans *t1 = p1->trans;
        struct trans *t2 = p2->trans;
        for (int n1 = 0, b2 = 0; n1 < p1->tused; n1++) {
            while (b2 < p2->tused && t2[b2].max < t1[n1].min)
                b2++;
            for (int n2 = b2;
                 n2 < p2->tused && t1[n1].max >= t2[n2].min;
                 n2++) {
                if (t2[n2].max >= t1[n1].min) {
                    struct state *r = state_triple_thd(newstates,
                                                       t1[n1].to, t2[n2].to);
                    if (r == NULL) {
                        r = add_state(fa, 0);
                        E(r == NULL);
                        F(state_set_push(worklist, t1[n1].to));
                        F(state_set_push(worklist, t2[n2].to));
                        F(state_set_push(worklist, r));
                        F(state_triple_push(newstates,
                                             t1[n1].to, t2[n2].to, r));
                    }
                    char min = t1[n1].min > t2[n2].min
                        ? t1[n1].min : t2[n2].min;
                    char max = t1[n1].max < t2[n2].max
                        ? t1[n1].max : t2[n2].max;
                    ret = add_new_trans(s, r, min, max);
                    if (ret < 0)
                        goto error;
                }
            }
        }
    }
    fa->deterministic = fa1->deterministic && fa2->deterministic;
    fa->nocase = fa1->nocase && fa2->nocase;
 done:
    state_set_free(worklist);
    state_triple_free(newstates);
    if (fa != NULL) {
        if (collect(fa) < 0) {
            fa_free(fa);
            fa = NULL;
        }
    }

    return fa;
 error:
    fa_free(fa);
    fa = NULL;
    goto done;
}

int fa_contains(struct fa *fa1, struct fa *fa2) {
    int result = 0;
    struct state_set *worklist = NULL;  /* List of pairs of states */
    struct state_set *visited = NULL;   /* List of pairs of states */

    if (fa1 == NULL || fa2 == NULL)
        return -1;

    if (fa1 == fa2)
        return 1;

    F(determinize(fa2, NULL));
    sort_transition_intervals(fa1);
    sort_transition_intervals(fa2);

    F(state_pair_push(&worklist, fa1->initial, fa2->initial));
    F(state_pair_push(&visited, fa1->initial, fa2->initial));
    while (worklist->used) {
        struct state *p1, *p2;
        void *v2;
        p1 = state_set_pop_data(worklist, &v2);
        p2 = v2;

        if (p1->accept && !p2->accept)
            goto done;

        struct trans *t1 = p1->trans;
        struct trans *t2 = p2->trans;
        for(int n1 = 0, b2 = 0; n1 < p1->tused; n1++) {
            while (b2 < p2->tused && t2[b2].max < t1[n1].min)
                b2++;
            int min1 = t1[n1].min, max1 = t1[n1].max;
            for (int n2 = b2;
                 n2 < p2->tused && t1[n1].max >= t2[n2].min;
                 n2++) {
                if (t2[n2].min > min1)
                    goto done;
                if (t2[n2].max < CHAR_MAX)
                    min1 = t2[n2].max + 1;
                else {
                    min1 = UCHAR_MAX;
                    max1 = UCHAR_MIN;
                }
                if (state_pair_find(visited, t1[n1].to, t2[n2].to) == -1) {
                    F(state_pair_push(&worklist, t1[n1].to, t2[n2].to));
                    F(state_pair_push(&visited, t1[n1].to, t2[n2].to));
                }
            }
            if (min1 <= max1)
                goto done;
        }
    }

    result = 1;
 done:
    state_set_free(worklist);
    state_set_free(visited);
    return result;
 error:
    result = -1;
    goto done;
}

static int add_crash_trans(struct fa *fa, struct state *s, struct state *crash,
                           int min, int max) {
    int result;

    if (fa->nocase) {
        /* Never transition on anything in [A-Z] */
        if (min > 'Z' || max < 'A') {
            result = add_new_trans(s, crash, min, max);
        } else if (min >= 'A' && max <= 'Z') {
            result = 0;
        } else if (max <= 'Z') {
            /* min < 'A' */
            result = add_new_trans(s, crash, min, 'A' - 1);
        } else if (min >= 'A') {
            /* max > 'Z' */
            result = add_new_trans(s, crash, 'Z' + 1, max);
        } else {
            /* min < 'A' && max > 'Z' */
            result = add_new_trans(s, crash, min, 'A' - 1);
            if (result == 0)
                result = add_new_trans(s, crash, 'Z' + 1, max);
        }
    } else {
        result = add_new_trans(s, crash, min, max);
    }
    return result;
}

static int totalize(struct fa *fa) {
    int r;
    struct state *crash = add_state(fa, 0);

    E(crash == NULL);
    F(mark_reachable(fa));
    sort_transition_intervals(fa);

    r = add_crash_trans(fa, crash, crash, UCHAR_MIN, UCHAR_MAX);
    if (r < 0)
        return -1;

    list_for_each(s, fa->initial) {
        int next = UCHAR_MIN;
        int tused = s->tused;
        for (int i=0; i < tused; i++) {
            uchar min = s->trans[i].min, max = s->trans[i].max;
            if (min > next) {
                r = add_crash_trans(fa, s, crash, next, min - 1);
                if (r < 0)
                    return -1;
            }
            if (max + 1 > next)
                next = max + 1;
        }
        if (next <= UCHAR_MAX) {
            r = add_crash_trans(fa, s, crash, next, UCHAR_MAX);
            if (r < 0)
                return -1;
        }
    }
    return 0;
 error:
    return -1;
}

struct fa *fa_complement(struct fa *fa) {
    fa = fa_clone(fa);
    E(fa == NULL);
    F(determinize(fa, NULL));
    F(totalize(fa));
    list_for_each(s, fa->initial)
        s->accept = ! s->accept;

    F(collect(fa));
    return fa;
 error:
    fa_free(fa);
    return NULL;
}

struct fa *fa_minus(struct fa *fa1, struct fa *fa2) {
    if (fa1 == NULL || fa2 == NULL)
        return NULL;

    if (fa_is_basic(fa1, FA_EMPTY) || fa1 == fa2)
        return fa_make_empty();
    if (fa_is_basic(fa2, FA_EMPTY))
        return fa_clone(fa1);

    struct fa *cfa2 = fa_complement(fa2);
    if (cfa2 == NULL)
        return NULL;

    struct fa *result = fa_intersect(fa1, cfa2);
    fa_free(cfa2);
    return result;
}

static int accept_to_accept(struct fa *fa) {
    int r;
    struct state *s = add_state(fa, 0);
    if (s == NULL)
        return -1;

    F(mark_reachable(fa));
    list_for_each(a, fa->initial) {
        if (a->accept && a->reachable) {
            r = add_epsilon_trans(s, a);
            if (r < 0)
                return -1;
        }
    }

    set_initial(fa, s);
    fa->deterministic = fa->minimal = 0;
    return 0;
 error:
    return -1;
}

struct fa *fa_overlap(struct fa *fa1, struct fa *fa2) {
    struct fa *fa = NULL, *eps = NULL, *result = NULL;
    struct state_set *map = NULL;

    if (fa1 == NULL || fa2 == NULL)
        return NULL;

    fa1 = fa_clone(fa1);
    fa2 = fa_clone(fa2);
    if (fa1 == NULL || fa2 == NULL)
        goto error;

    if (determinize(fa1, NULL) < 0)
        goto error;
    if (accept_to_accept(fa1) < 0)
        goto error;

    map = fa_reverse(fa2);
    state_set_free(map);
    if (determinize(fa2, NULL) < 0)
        goto error;
    if (accept_to_accept(fa2) < 0)
        goto error;
    map = fa_reverse(fa2);
    state_set_free(map);
    if (determinize(fa2, NULL) < 0)
        goto error;

    fa = fa_intersect(fa1, fa2);
    if (fa == NULL)
        goto error;

    eps = fa_make_epsilon();
    if (eps == NULL)
        goto error;

    result = fa_minus(fa, eps);
    if (result == NULL)
        goto error;

 error:
    fa_free(fa1);
    fa_free(fa2);
    fa_free(fa);
    fa_free(eps);
    return result;
}

int fa_equals(struct fa *fa1, struct fa *fa2) {
    if (fa1 == NULL || fa2 == NULL)
        return -1;

    /* fa_contains(fa1, fa2) && fa_contains(fa2, fa1) with error checking */
    int c1 = fa_contains(fa1, fa2);
    if (c1 < 0)
        return -1;
    if (c1 == 0)
        return 0;
    return fa_contains(fa2, fa1);
}

static unsigned int chr_score(char c) {
    if (isalpha(c)) {
        return 2;
    } else if (isalnum(c)) {
        return 3;
    } else if (isprint(c)) {
        return 7;
    } else if (c == '\0') {
        return 10000;
    } else {
        return 100;
    }
}

static unsigned int str_score(const struct re_str *str) {
    unsigned int score = 0;
    for (int i=0; i < str->len; i++) {
        score += chr_score(str->rx[i]);
    }
    return score;
}

/* See if we get a better string for DST by appending C to SRC. If DST is
 * NULL or empty, always use SRC + C
 */
static struct re_str *string_extend(struct re_str *dst,
                                    const struct re_str *src,
                                    char c) {
    if (dst == NULL
        || dst->len == 0
        || str_score(src) + chr_score(c) < str_score(dst)) {
        int slen = src->len;
        if (dst == NULL)
            dst = make_re_str(NULL);
        if (dst == NULL)
            return NULL;
        if (REALLOC_N(dst->rx, slen+2) < 0) {
            free(dst);
            return NULL;
        }
        memcpy(dst->rx, src->rx, slen);
        dst->rx[slen] = c;
        dst->rx[slen + 1] = '\0';
        dst->len = slen + 1;
    }
    return dst;
}

static char pick_char(struct trans *t) {
    for (int c = t->min; c <= t->max; c++)
        if (isalpha(c)) return c;
    for (int c = t->min; c <= t->max; c++)
        if (isalnum(c)) return c;
    for (int c = t->min; c <= t->max; c++)
        if (isprint(c)) return c;
    return t->max;
}

/* Generate an example string for FA. Traverse all transitions and record
 * at each turn the "best" word found for that state.
 */
int fa_example(struct fa *fa, char **example, size_t *example_len) {
    struct re_str *word = NULL;
    struct state_set *path = NULL, *worklist = NULL;
    struct re_str *str = NULL;

    *example = NULL;
    *example_len = 0;

    /* Sort to avoid any ambiguity because of reordering of transitions */
    sort_transition_intervals(fa);

    /* Map from state to string */
    path = state_set_init(-1, S_DATA|S_SORTED);
    str = make_re_str("");
    if (path == NULL || str == NULL)
        goto error;
    F(state_set_push_data(path, fa->initial, str));
    str = NULL;

    /* List of states still to visit */
    worklist = state_set_init(-1, S_NONE);
    if (worklist == NULL)
        goto error;
    F(state_set_push(worklist, fa->initial));

    while (worklist->used) {
        struct state *s = state_set_pop(worklist);
        struct re_str *ps = state_set_find_data(path, s);
        for_each_trans(t, s) {
            char c = pick_char(t);
            int toind = state_set_index(path, t->to);
            if (toind == -1) {
                struct re_str *w = string_extend(NULL, ps, c);
                E(w == NULL);
                F(state_set_push(worklist, t->to));
                F(state_set_push_data(path, t->to, w));
            } else {
                path->data[toind] = string_extend(path->data[toind], ps, c);
            }
        }
    }

    for (int i=0; i < path->used; i++) {
        struct state *p = path->states[i];
        struct re_str *ps = path->data[i];
        if (p->accept &&
            (word == NULL || word->len == 0
             || (ps->len > 0 && str_score(word) > str_score(ps)))) {
            free_re_str(word);
            word = ps;
        } else {
            free_re_str(ps);
        }
    }
    state_set_free(path);
    state_set_free(worklist);
    if (word != NULL) {
        *example_len = word->len;
        *example = word->rx;
        free(word);
    }
    return 0;
 error:
    state_set_free(path);
    state_set_free(worklist);
    free_re_str(word);
    free_re_str(str);
    return -1;
}

struct enum_intl {
    int       limit;
    int       nwords;
    char    **words;
    char     *buf;
    size_t    bsize;
};

static int fa_enumerate_intl(struct state *s, struct enum_intl *ei, int pos) {
    int result = -1;

    if (ei->bsize <= pos + 1) {
        ei->bsize *= 2;
        F(REALLOC_N(ei->buf, ei->bsize));
    }

    ei->buf[pos] = '\0';
    for_each_trans(t, s) {
        if (t->to->visited)
            return -2;
        t->to->visited = 1;
        for (int i=t->min; i <= t->max; i++) {
            ei->buf[pos] = i;
            if (t->to->accept) {
                if (ei->nwords >= ei->limit)
                    return -2;
                ei->words[ei->nwords] = strdup(ei->buf);
                E(ei->words[ei->nwords] == NULL);
                ei->nwords += 1;
            }
            result = fa_enumerate_intl(t->to, ei, pos+1);
            E(result < 0);
        }
        t->to->visited = 0;
    }
    ei->buf[pos] = '\0';
    result = 0;
 error:
    return result;
}

int fa_enumerate(struct fa *fa, int limit, char ***words) {
    struct enum_intl ei;
    int result = -1;

    *words = NULL;
    MEMZERO(&ei, 1);
    ei.bsize = 8;                    /* Arbitrary initial size */
    ei.limit = limit;
    F(ALLOC_N(ei.words, limit));
    F(ALLOC_N(ei.buf, ei.bsize));

    /* We use the visited bit to track which states we already visited
     * during the construction of a word to detect loops */
    list_for_each(s, fa->initial)
        s->visited = 0;
    fa->initial->visited = 1;
    if (fa->initial->accept) {
        if (ei.nwords >= limit)
            return -2;
        ei.words[0] = strdup("");
        E(ei.words[0] == NULL);
        ei.nwords = 1;
    }
    result = fa_enumerate_intl(fa->initial, &ei, 0);
    E(result < 0);

    result = ei.nwords;
    *words = ei.words;
    ei.words = NULL;
 done:
    free(ei.buf);
    return result;

 error:
    for (int i=0; i < ei.nwords; i++)
        free(ei.words[i]);
    free(ei.words);
    goto done;
}

/* Expand the automaton FA by replacing every transition s(c) -> p from
 * state s to p on character c by two transitions s(X) -> r, r(c) -> p via
 * a new state r.
 * If ADD_MARKER is true, also add for each original state s a new a loop
 * s(Y) -> q and q(X) -> s through a new state q.
 *
 * The end result is that an automaton accepting "a|ab" is turned into one
 * accepting "Xa|XaXb" if add_marker is false and "(YX)*Xa|(YX)*Xa(YX)*Xb"
 * when add_marker is true.
 *
 * The returned automaton is a copy of FA, FA is not modified.
 */
static struct fa *expand_alphabet(struct fa *fa, int add_marker,
                                  char X, char Y) {
    int ret;

    fa = fa_clone(fa);
    if (fa == NULL)
        return NULL;

    F(mark_reachable(fa));
    list_for_each(p, fa->initial) {
        if (! p->reachable)
            continue;

        struct state *r = add_state(fa, 0);
        r->trans = p->trans;
        r->tused = p->tused;
        r->tsize = p->tsize;
        p->trans = NULL;
        p->tused = p->tsize = 0;
        ret = add_new_trans(p, r, X, X);
        if (ret < 0)
            goto error;
        if (add_marker) {
            struct state *q = add_state(fa, 0);
            ret = add_new_trans(p, q, Y, Y);
            if (ret < 0)
                goto error;
            ret = add_new_trans(q, p, X, X);
            if (ret < 0)
                goto error;
        }
    }
    return fa;
 error:
    fa_free(fa);
    return NULL;
}

static bitset *alphabet(struct fa *fa) {
    bitset *bs = bitset_init(UCHAR_NUM);

    list_for_each(s, fa->initial) {
        for (int i=0; i < s->tused; i++) {
            for (uint c = s->trans[i].min; c <= s->trans[i].max; c++)
                bitset_set(bs, c);
        }
    }
    return bs;
}

static bitset *last_chars(struct fa *fa) {
    bitset *bs = bitset_init(UCHAR_NUM);

    list_for_each(s, fa->initial) {
        for (int i=0; i < s->tused; i++) {
            if (s->trans[i].to->accept) {
                for (uint c = s->trans[i].min; c <= s->trans[i].max; c++)
                    bitset_set(bs, c);
            }
        }
    }
    return bs;
}

static bitset *first_chars(struct fa *fa) {
    bitset *bs = bitset_init(UCHAR_NUM);
    struct state *s = fa->initial;

    for (int i=0; i < s->tused; i++) {
        for (uint c = s->trans[i].min; c <= s->trans[i].max; c++)
            bitset_set(bs, c);
    }
    return bs;
}

/* Return true if F1 and F2 are known to be unambiguously concatenable
 * according to simple heuristics. Return false if they need to be checked
 * further to decide ambiguity */
static bool is_splittable(struct fa *fa1, struct fa *fa2) {
    bitset *alpha1 = NULL;
    bitset *alpha2 = NULL;
    bitset *last1 = NULL;
    bitset *first2 = NULL;
    bool result = false;

    alpha2 = alphabet(fa2);
    last1 = last_chars(fa1);
    if (bitset_disjoint(last1, alpha2, UCHAR_NUM)) {
        result = true;
        goto done;
    }

    alpha1 = alphabet(fa1);
    first2 = first_chars(fa2);
    if (bitset_disjoint(first2, alpha1, UCHAR_NUM)) {
        result = true;
        goto done;
    }
 done:
    bitset_free(alpha1);
    bitset_free(alpha2);
    bitset_free(last1);
    bitset_free(first2);
    return result;
}

/* This algorithm is due to Anders Moeller, and can be found in class
 * AutomatonOperations in dk.brics.grammar
 */
int fa_ambig_example(struct fa *fa1, struct fa *fa2,
                     char **upv, size_t *upv_len,
                     char **pv, char **v) {
    static const char X = '\001';
    static const char Y = '\002';
    char *result = NULL, *s = NULL;
    int ret = -1, r;
    struct fa *mp = NULL, *ms = NULL, *sp = NULL, *ss = NULL, *amb = NULL;
    struct fa *a1f = NULL, *a1t = NULL, *a2f = NULL, *a2t = NULL;
    struct fa *b1 = NULL, *b2 = NULL;

    *upv = NULL;
    *upv_len = 0;
    if (pv != NULL)
        *pv = NULL;
    if (v != NULL)
        *v = NULL;

    if (is_splittable(fa1, fa2))
        return 0;

#define Xs "\001"
#define Ys "\002"
#define MPs Ys Xs "(" Xs "(.|\n))+"
#define MSs Ys Xs "(" Xs "(.|\n))*"
#define SPs "(" Xs "(.|\n))+" Ys Xs
#define SSs "(" Xs "(.|\n))*" Ys Xs
    /* These could become static constants */
    r = fa_compile( MPs, strlen(MPs), &mp);
    if (r != REG_NOERROR)
        goto error;
    r = fa_compile( MSs, strlen(MSs), &ms);
    if (r != REG_NOERROR)
        goto error;
    r = fa_compile( SPs, strlen(SPs), &sp);
    if (r != REG_NOERROR)
        goto error;
    r = fa_compile( SSs, strlen(SSs), &ss);
    if (r != REG_NOERROR)
        goto error;
#undef SSs
#undef SPs
#undef MSs
#undef MPs
#undef Xs
#undef Ys

    a1f = expand_alphabet(fa1, 0, X, Y);
    a1t = expand_alphabet(fa1, 1, X, Y);
    a2f = expand_alphabet(fa2, 0, X, Y);
    a2t = expand_alphabet(fa2, 1, X, Y);
    if (a1f == NULL || a1t == NULL || a2f == NULL || a2t == NULL)
        goto error;

    /* Compute b1 = ((a1f . mp) & a1t) . ms */
    if (concat_in_place(a1f, &mp) < 0)
        goto error;
    b1 = fa_intersect(a1f, a1t);
    if (b1 == NULL)
        goto error;
    if (concat_in_place(b1, &ms) < 0)
        goto error;

    /* Compute b2 = ss . ((sp . a2f) & a2t) */
    if (concat_in_place(sp, &a2f) < 0)
        goto error;
    b2 = fa_intersect(sp, a2t);
    if (b2 == NULL)
        goto error;
    if (concat_in_place(ss, &b2) < 0)
        goto error;
    b2 = ss;
    ss = NULL;

    /* The automaton we are really interested in */
    amb = fa_intersect(b1, b2);
    if (amb == NULL)
        goto error;

    size_t s_len = 0;
    r = fa_example(amb, &s, &s_len);
    if (r < 0)
        goto error;

    if (s != NULL) {
        char *t;
        F(ALLOC_N(result, (strlen(s)-1)/2 + 1));
        t = result;
        int i = 0;
        for (i=0; s[2*i] == X; i++)
            *t++ = s[2*i + 1];
        if (pv != NULL)
            *pv = t;
        i += 1;

        for ( ;s[2*i] == X; i++)
            *t++ = s[2*i + 1];
        if (v != NULL)
            *v = t;
        i += 1;

        for (; 2*i+1 < strlen(s); i++)
            *t++ = s[2*i + 1];
    }
    ret = 0;

 done:
    /* Clean up intermediate automata */
    fa_free(mp);
    fa_free(ms);
    fa_free(ss);
    fa_free(sp);
    fa_free(a1f);
    fa_free(a1t);
    fa_free(a2f);
    fa_free(a2t);
    fa_free(b1);
    fa_free(b2);
    fa_free(amb);

    FREE(s);
    *upv = result;
    if (result != NULL)
        *upv_len = strlen(result);
    return ret;
 error:
    FREE(result);
    ret = -1;
    goto done;
}

/*
 * Construct an fa from a regular expression
 */
static struct fa *fa_from_re(struct re *re) {
    struct fa *result = NULL;

    switch(re->type) {
    case UNION:
        {
            result = fa_from_re(re->exp1);
            if (result == NULL)
                goto error;
            struct fa *fa2 = fa_from_re(re->exp2);
            if (fa2 == NULL)
                goto error;
            if (union_in_place(result, &fa2) < 0)
                goto error;
        }
        break;
    case CONCAT:
        {
            result = fa_from_re(re->exp1);
            if (result == NULL)
                goto error;
            struct fa *fa2 = fa_from_re(re->exp2);
            if (fa2 == NULL)
                goto error;
            if (concat_in_place(result, &fa2) < 0)
                goto error;
        }
        break;
    case CSET:
        result = fa_make_char_set(re->cset, re->negate);
        break;
    case ITER:
        {
            struct fa *fa = fa_from_re(re->exp);
            if (fa == NULL)
                goto error;
            result = fa_iter(fa, re->min, re->max);
            fa_free(fa);
        }
        break;
    case EPSILON:
        result = fa_make_epsilon();
        break;
    case CHAR:
        result = fa_make_char(re->c);
        break;
    default:
        assert(0);
        break;
    }
    return result;
 error:
    fa_free(result);
    return NULL;
}

static void free_re(struct re *re) {
    if (re == NULL)
        return;
    assert(re->ref == 0);

    if (re->type == UNION || re->type == CONCAT) {
        re_unref(re->exp1);
        re_unref(re->exp2);
    } else if (re->type == ITER) {
        re_unref(re->exp);
    } else if (re->type == CSET) {
        bitset_free(re->cset);
    }
    free(re);
}

int fa_compile(const char *regexp, size_t size, struct fa **fa) {
    struct re *re = NULL;
    struct re_parse parse;

    *fa = NULL;

    parse.rx = regexp;
    parse.rend = regexp + size;
    parse.error = REG_NOERROR;

    re = parse_regexp(&parse);
    if (re == NULL)
        return parse.error;

    *fa = fa_from_re(re);
    re_unref(re);

    if (*fa == NULL || collect(*fa) < 0)
        parse.error = REG_ESPACE;
    return parse.error;
}

/* We represent a case-insensitive FA by using only transitions on
 * lower-case letters.
 */
int fa_nocase(struct fa *fa) {
    if (fa == NULL || fa->nocase)
        return 0;

    fa->nocase = 1;
    list_for_each(s, fa->initial) {
        int tused = s->tused;
        /* For every transition on characters in [A-Z] add a corresponding
         * transition on [a-z]; remove any portion covering [A-Z] */
        for (int i=0; i < tused; i++) {
            struct trans *t = s->trans + i;
            int lc_min = t->min < 'A' ? 'a' : tolower(t->min);
            int lc_max = t->max > 'Z' ? 'z' : tolower(t->max);

            if (t->min > 'Z' || t->max < 'A')
                continue;
            if (t->min >= 'A' && t->max <= 'Z') {
                t->min = tolower(t->min);
                t->max = tolower(t->max);
            } else if (t->max <= 'Z') {
                /* t->min < 'A' */
                t->max = 'A' - 1;
                F(add_new_trans(s, t->to, lc_min, lc_max));
            } else if (t->min >= 'A') {
                /* t->max > 'Z' */
                t->min = 'Z' + 1;
                F(add_new_trans(s, t->to, lc_min, lc_max));
            } else {
                /* t->min < 'A' && t->max > 'Z' */
                F(add_new_trans(s, t->to, 'Z' + 1, t->max));
                s->trans[i].max = 'A' - 1;
                F(add_new_trans(s, s->trans[i].to, lc_min, lc_max));
            }
        }
    }
    F(collect(fa));
    return 0;
 error:
    return -1;
}

int fa_is_nocase(struct fa *fa) {
    return fa->nocase;
}

/* If FA is case-insensitive, turn it into a case-sensitive automaton by
 * adding transitions on upper-case letters for each existing transition on
 * lower-case letters */
static int case_expand(struct fa *fa) {
    if (! fa->nocase)
        return 0;

    fa->nocase = 0;
    list_for_each(s, fa->initial) {
        int tused = s->tused;
        /* For every transition on characters in [a-z] add a corresponding
         * transition on [A-Z] */
        for (int i=0; i < tused; i++) {
            struct trans *t = s->trans + i;
            int lc_min = t->min < 'a' ? 'A' : toupper(t->min);
            int lc_max = t->max > 'z' ? 'Z' : toupper(t->max);

            if (t->min > 'z' || t->max < 'a')
                continue;
            F(add_new_trans(s, t->to, lc_min, lc_max));
        }
    }
    F(collect(fa));
    return 0;
 error:
    return -1;
}

/*
 * Regular expression parser
 */

static struct re *make_re(enum re_type type) {
    struct re *re;
    if (make_ref(re) == 0)
        re->type = type;
    return re;
}

static struct re *make_re_rep(struct re *exp, int min, int max) {
    struct re *re = make_re(ITER);
    if (re) {
        re->exp = exp;
        re->min = min;
        re->max = max;
    } else {
        re_unref(exp);
    }
    return re;
}

static struct re *make_re_binop(enum re_type type, struct re *exp1,
                                struct re *exp2) {
    struct re *re = make_re(type);
    if (re) {
        re->exp1 = exp1;
        re->exp2 = exp2;
    } else {
        re_unref(exp1);
        re_unref(exp2);
    }
    return re;
}

static struct re *make_re_char(uchar c) {
    struct re *re = make_re(CHAR);
    if (re)
        re->c = c;
    return re;
}

static struct re *make_re_char_set(bool negate, bool no_ranges) {
    struct re *re = make_re(CSET);
    if (re) {
        re->negate = negate;
        re->no_ranges = no_ranges;
        re->cset = bitset_init(UCHAR_NUM);
        if (re->cset == NULL)
            re_unref(re);
    }
    return re;
}

static bool more(struct re_parse *parse) {
    return parse->rx < parse->rend;
}

static bool match(struct re_parse *parse, char m) {
    if (!more(parse))
        return false;
    if (*parse->rx == m) {
        parse->rx += 1;
        return true;
    }
    return false;
}

static bool peek(struct re_parse *parse, const char *chars) {
    return *parse->rx != '\0' && strchr(chars, *parse->rx) != NULL;
}

static bool next(struct re_parse *parse, char *c) {
    if (!more(parse))
        return false;
    *c = *parse->rx;
    parse->rx += 1;
    return true;
}

static bool parse_char(struct re_parse *parse, int quoted, char *c) {
    if (!more(parse))
        return false;
    if (quoted && *parse->rx == '\\') {
        parse->rx += 1;
        return next(parse, c);
    } else {
        return next(parse, c);
    }
}

static void add_re_char(struct re *re, uchar from, uchar to) {
    assert(re->type == CSET);
    for (unsigned int c = from; c <= to; c++)
        bitset_set(re->cset, c);
}

static void parse_char_class(struct re_parse *parse, struct re *re) {
    if (! more(parse)) {
        parse->error = REG_EBRACK;
        goto error;
    }
    char from, to;
    parse_char(parse, 0, &from);
    to = from;
    if (match(parse, '-')) {
        if (! more(parse)) {
            parse->error = REG_EBRACK;
            goto error;
        }
        if (peek(parse, "]")) {
            if (from > to) {
                parse->error = REG_ERANGE;
                goto error;
            }
            add_re_char(re, from, to);
            add_re_char(re, '-', '-');
            return;
        } else if (!parse_char(parse, 0, &to)) {
            parse->error = REG_ERANGE;
            goto error;
        }
    }
    if (from > to) {
        parse->error = REG_ERANGE;
        goto error;
    }
    add_re_char(re, from, to);
 error:
    return;
}

static struct re *parse_simple_exp(struct re_parse *parse) {
    struct re *re = NULL;

    if (match(parse, '[')) {
        bool negate = match(parse, '^');
        re = make_re_char_set(negate, parse->no_ranges);
        if (re == NULL) {
            parse->error = REG_ESPACE;
            goto error;
        }
        parse_char_class(parse, re);
        if (parse->error != REG_NOERROR)
            goto error;
        while (more(parse) && ! peek(parse, "]")) {
            parse_char_class(parse, re);
            if (parse->error != REG_NOERROR)
                goto error;
        }
        if (! match(parse, ']')) {
            parse->error = REG_EBRACK;
            goto error;
        }
    } else if (match(parse, '(')) {
        if (match(parse, ')')) {
            re = make_re(EPSILON);
            if (re == NULL) {
                parse->error = REG_ESPACE;
                goto error;
            }
        } else {
            re = parse_regexp(parse);
            if (re == NULL)
                goto error;
            if (! match(parse, ')')) {
                parse->error = REG_EPAREN;
                goto error;
            }
        }
    } else if (match(parse, '.')) {
        re = make_re_char_set(1, parse->no_ranges);
        if (re == NULL) {
            parse->error = REG_ESPACE;
            goto error;
        }
        add_re_char(re, '\n', '\n');
    } else if (more(parse)) {
        char c;
        if (!parse_char(parse, 1, &c)) {
            parse->error = REG_EESCAPE;
            goto error;
        }
        re = make_re_char(c);
        if (re == NULL) {
            parse->error = REG_ESPACE;
            goto error;
        }
    } else {
        re = make_re(EPSILON);
        if (re == NULL) {
            parse->error = REG_ESPACE;
            goto error;
        }
    }
    return re;
 error:
    re_unref(re);
    return NULL;
}

static int parse_int(struct re_parse *parse) {
    const char *lim;
    char *end;
    size_t used;
    long l;

    /* We need to be careful that strtoul will never access
     * memory beyond parse->rend
     */
    for (lim = parse->rx; lim < parse->rend && *lim >= '0' && *lim <= '9';
         lim++);
    if (lim < parse->rend) {
        l = strtoul(parse->rx, &end, 10);
        used = end - parse->rx;
    } else {
        char *s = strndup(parse->rx, parse->rend - parse->rx);
        if (s == NULL) {
            parse->error = REG_ESPACE;
            return -1;
        }
        l = strtoul(s, &end, 10);
        used = end - s;
        free(s);
    }

    if (used == 0)
        return -1;
    parse->rx += used;
    if ((l<0) || (l > INT_MAX)) {
        parse->error = REG_BADBR;
        return -1;
    }
    return (int) l;
}

static struct re *parse_repeated_exp(struct re_parse *parse) {
    struct re *re = parse_simple_exp(parse);
    if (re == NULL)
        goto error;
    if (match(parse, '?')) {
        re = make_re_rep(re, 0, 1);
    } else if (match(parse, '*')) {
        re = make_re_rep(re, 0, -1);
    } else if (match(parse, '+')) {
        re = make_re_rep(re, 1, -1);
    } else if (match(parse, '{')) {
        int min, max;
        min = parse_int(parse);
        if (min == -1)
            goto error;
        if (match(parse, ',')) {
            max = parse_int(parse);
            if (max == -1)
                max = -1;      /* If it's not an int, it means 'unbounded' */
            if (! match(parse, '}')) {
                parse->error = REG_EBRACE;
                goto error;
            }
        } else if (match(parse, '}')) {
            max = min;
        } else {
            parse->error = REG_EBRACE;
            goto error;
        }
        if (min > max && max != -1) {
            parse->error = REG_BADBR;
            goto error;
        }
        re = make_re_rep(re, min, max);
    }
    if (re == NULL)
        parse->error = REG_ESPACE;
    return re;
 error:
    re_unref(re);
    return NULL;
}

static struct re *parse_concat_exp(struct re_parse *parse) {
    struct re *re = parse_repeated_exp(parse);
    if (re == NULL)
        goto error;

    if (more(parse) && ! peek(parse, ")|")) {
        struct re *re2 = parse_concat_exp(parse);
        if (re2 == NULL)
            goto error;
        re = make_re_binop(CONCAT, re, re2);
        if (re == NULL) {
            parse->error = REG_ESPACE;
            goto error;
        }
    }
    return re;

 error:
    re_unref(re);
    return NULL;
}

static struct re *parse_regexp(struct re_parse *parse) {
    struct re *re = NULL;

    /* Something like (|r) */
    if (peek(parse, "|"))
        re = make_re(EPSILON);
    else
        re = parse_concat_exp(parse);
    if (re == NULL)
        goto error;

    if (match(parse, '|')) {
        struct re *re2 = NULL;
        /* Something like (r|) */
        if (peek(parse, ")"))
            re2 = make_re(EPSILON);
        else
            re2 = parse_regexp(parse);
        if (re2 == NULL)
            goto error;

        re = make_re_binop(UNION, re, re2);
        if (re == NULL) {
            parse->error = REG_ESPACE;
            goto error;
        }
    }
    return re;

 error:
    re_unref(re);
    return NULL;
}

/*
 * Convert a STRUCT RE to a string. Code is complicated by the fact that
 * we try to be clever and avoid unneeded parens and concatenation with
 * epsilon etc.
 */
static int re_as_string(const struct re *re, struct re_str *str);

static int re_binop_count(enum re_type type, const struct re *re) {
    assert(type == CONCAT || type == UNION);
    if (re->type == type) {
        return re_binop_count(type, re->exp1) + re_binop_count(type, re->exp2);
    } else {
        return 1;
    }
}

static int re_binop_store(enum re_type type, const struct re *re,
                          const struct re **list) {
    int pos = 0;
    if (type == re->type) {
        pos = re_binop_store(type, re->exp1, list);
        pos += re_binop_store(type, re->exp2, list + pos);
    } else {
        list[0] = re;
        pos = 1;
    }
    return pos;
}

static int re_union_as_string(const struct re *re, struct re_str *str) {
    assert(re->type == UNION);

    int result = -1;
    const struct re **res = NULL;
    struct re_str *strings = NULL;
    int nre = 0, r;

    nre = re_binop_count(re->type, re);
    r = ALLOC_N(res, nre);
    if (r < 0)
        goto done;

    re_binop_store(re->type, re, res);

    r = ALLOC_N(strings, nre);
    if (r < 0)
        goto error;

    str->len = 0;
    for (int i=0; i < nre; i++) {
        if (re_as_string(res[i], strings + i) < 0)
            goto error;
        str->len += strings[i].len;
    }
    str->len += nre-1;

    r = re_str_alloc(str);
    if (r < 0)
        goto error;

    char *p = str->rx;
    for (int i=0; i < nre; i++) {
        if (i>0)
            *p++ = '|';
        memcpy(p, strings[i].rx, strings[i].len);
        p += strings[i].len;
    }

    result = 0;
 done:
    free(res);
    if (strings != NULL) {
        for (int i=0; i < nre; i++)
            release_re_str(strings + i);
    }
    free(strings);
    return result;
 error:
    release_re_str(str);
    result = -1;
    goto done;
}

ATTRIBUTE_PURE
static int re_needs_parens_in_concat(const struct re *re) {
    return (re->type != CHAR && re->type != CSET && re->type != ITER);
}

static int re_concat_as_string(const struct re *re, struct re_str *str) {
    assert(re->type == CONCAT);

    const struct re **res = NULL;
    struct re_str *strings = NULL;
    int nre = 0, r;
    int result = -1;

    nre = re_binop_count(re->type, re);
    r = ALLOC_N(res, nre);
    if (r < 0)
        goto error;
    re_binop_store(re->type, re, res);

    r = ALLOC_N(strings, nre);
    if (r < 0)
        goto error;

    str->len = 0;
    for (int i=0; i < nre; i++) {
        if (res[i]->type == EPSILON)
            continue;
        if (re_as_string(res[i], strings + i) < 0)
            goto error;
        str->len += strings[i].len;
        if (re_needs_parens_in_concat(res[i]))
            str->len += 2;
    }

    r = re_str_alloc(str);
    if (r < 0)
        goto error;

    char *p = str->rx;
    for (int i=0; i < nre; i++) {
        if (res[i]->type == EPSILON)
            continue;
        if (re_needs_parens_in_concat(res[i]))
            *p++ = '(';
        p = memcpy(p, strings[i].rx, strings[i].len);
        p += strings[i].len;
        if (re_needs_parens_in_concat(res[i]))
            *p++ = ')';
    }

    result = 0;
 done:
    free(res);
    if (strings != NULL) {
        for (int i=0; i < nre; i++)
            release_re_str(strings + i);
    }
    free(strings);
    return result;
 error:
    release_re_str(str);
    result = -1;
    goto done;
}

static bool cset_contains(const struct re *cset, int c) {
    return bitset_get(cset->cset, c) != cset->negate;
}

static int re_cset_as_string(const struct re *re, struct re_str *str) {
    const uchar rbrack = ']';
    const uchar dash = '-';
    const uchar nul = '\0';

    static const char *const empty_set = "[]";
    static const char *const total_set = "(.|\n)";
    static const char *const not_newline = ".";

    char *s;
    int from, to, negate;
    size_t len;
    int incl_rbrack, incl_dash;
    int r;

    str->len = strlen(empty_set);

    /* We can not include NUL explicitly in a CSET since we use ordinary
       NUL delimited strings to represent them. That means that we need to
       use negated representation if NUL is to be included (and vice versa)
    */
    negate = cset_contains(re, nul);
    if (negate) {
        for (from = UCHAR_MIN;
             from <= UCHAR_MAX && cset_contains(re, from);
             from += 1);
        if (from > UCHAR_MAX) {
            /* Special case: the set matches every character */
            str->rx = strdup(total_set);
            goto done;
        }
        if (from == '\n') {
            for (from += 1;
                 from <= UCHAR_MAX && cset_contains(re, from);
                 from += 1);
            if (from > UCHAR_MAX) {
                /* Special case: the set matches everything but '\n' */
                str->rx = strdup(not_newline);
                goto done;
            }
        }
    }

    /* See if ']' and '-' will be explicitly included in the character set
       (INCL_RBRACK, INCL_DASH) As we loop over the character set, we reset
       these flags if they are in the set, but not mentioned explicitly
    */
    incl_rbrack = cset_contains(re, rbrack) != negate;
    incl_dash = cset_contains(re, dash) != negate;

    if (re->no_ranges) {
        for (from = UCHAR_MIN; from <= UCHAR_MAX; from++)
            if (cset_contains(re, from) != negate)
                str->len += 1;
    } else {
        for (from = UCHAR_MIN; from <= UCHAR_MAX; from = to+1) {
            while (from <= UCHAR_MAX && cset_contains(re, from) == negate)
                from += 1;
            if (from > UCHAR_MAX)
                break;
            for (to = from;
                 to < UCHAR_MAX && (cset_contains(re, to+1) != negate);
                 to++);

            if (to == from && (from == rbrack || from == dash))
                continue;
            if (from == rbrack || from == dash)
                from += 1;
            if (to == rbrack || to == dash)
                to -= 1;

            len = (to == from) ? 1 : ((to == from + 1) ? 2 : 3);

            if (from < rbrack && rbrack < to)
                incl_rbrack = 0;
            if (from < dash && dash < to)
                incl_dash = 0;
            str->len += len;
        }
        str->len += incl_rbrack + incl_dash;
    }
    if (negate)
        str->len += 1;        /* For the ^ */

    r = re_str_alloc(str);
    if (r < 0)
        goto error;

    s = str->rx;
    *s++ = '[';
    if (negate)
        *s++ = '^';
    if (incl_rbrack)
        *s++ = rbrack;

    if (re->no_ranges) {
        for (from = UCHAR_MIN; from <= UCHAR_MAX; from++) {
            if (from == rbrack || from == dash)
                continue;
            if (cset_contains(re, from) != negate)
                *s++ = from;
        }
    } else {
        for (from = UCHAR_MIN; from <= UCHAR_MAX; from = to+1) {
            while (from <= UCHAR_MAX && cset_contains(re, from) == negate)
                from += 1;
            if (from > UCHAR_MAX)
                break;
            for (to = from;
                 to < UCHAR_MAX && (cset_contains(re, to+1) != negate);
                 to++);

            if (to == from && (from == rbrack || from == dash))
                continue;
            if (from == rbrack || from == dash)
                from += 1;
            if (to == rbrack || to == dash)
                to -= 1;

            if (to == from) {
                *s++ = from;
            } else if (to == from + 1) {
                *s++ = from;
                *s++ = to;
            } else {
                *s++ = from;
                *s++ = '-';
                *s++ = to;
            }
        }
    }
    if (incl_dash)
        *s++ = dash;

    *s = ']';
 done:
    if (str->rx == NULL)
        goto error;
    str->len = strlen(str->rx);
    return 0;
 error:
    release_re_str(str);
    return -1;
}

static int re_iter_as_string(const struct re *re, struct re_str *str) {
    const char *quant = NULL;
    char *iter = NULL;
    int r, result = -1;

    if (re_as_string(re->exp, str) < 0)
        return -1;

    if (re->min == 0 && re->max == -1) {
        quant = "*";
    } else if (re->min == 1 && re->max == -1) {
        quant = "+";
    } else if (re->min == 0 && re->max == 1) {
        quant = "?";
    } else if (re->max == -1) {
        r = asprintf(&iter, "{%d,}", re->min);
        if (r < 0)
            return -1;
        quant = iter;
    } else {
        r = asprintf(&iter, "{%d,%d}", re->min, re->max);
        if (r < 0)
            return -1;
        quant = iter;
    }

    if (re->exp->type == CHAR || re->exp->type == CSET) {
        if (REALLOC_N(str->rx, str->len + strlen(quant) + 1) < 0)
            goto error;
        strcpy(str->rx + str->len, quant);
        str->len += strlen(quant);
    } else {
        /* Format '(' + str->rx ')' + quant */
        if (REALLOC_N(str->rx, str->len + strlen(quant) + 1 + 2) < 0)
            goto error;
        memmove(str->rx + 1, str->rx, str->len);
        str->rx[0] = '(';
        str->rx[str->len + 1] = ')';
        str->len += 2;
        strcpy(str->rx + str->len, quant);
        str->len += strlen(quant);
    }

    result = 0;
 done:
    FREE(iter);
    return result;
 error:
    release_re_str(str);
    goto done;
}

static int re_as_string(const struct re *re, struct re_str *str) {
    /* Characters that must be escaped */
    static const char * const special_chars = ".()[]{}*|+?\\^$";
    int result = 0;

    switch(re->type) {
    case UNION:
        result = re_union_as_string(re, str);
        break;
    case CONCAT:
        result = re_concat_as_string(re, str);
        break;
    case CSET:
        result = re_cset_as_string(re, str);
        break;
    case CHAR:
        if (re->c == '\0' || strchr(special_chars, re->c) == NULL) {
            if (ALLOC_N(str->rx, 2) < 0)
                goto error;
            str->rx[0] = re->c;
            str->len = 1;
        } else {
            if (ALLOC_N(str->rx, 3) < 0)
                goto error;
            str->rx[0] = '\\';
            str->rx[1] = re->c;
            str->len = strlen(str->rx);
        }
        break;
    case ITER:
        result = re_iter_as_string(re, str);
        break;
    case EPSILON:
        if (ALLOC_N(str->rx, 3) < 0)
            goto error;
        strcpy(str->rx, "()");
        str->len = strlen(str->rx);
        break;
    default:
        assert(0);
        abort();
        break;
    }
    return result;
 error:
    release_re_str(str);
    return -1;
}

static int convert_trans_to_re(struct state *s) {
    struct re *re = NULL;
    size_t nto = 1;
    struct trans *trans = NULL;
    int r;

    if (s->tused == 0)
        return 0;

    qsort(s->trans, s->tused, sizeof(*s->trans), trans_to_cmp);
    for (int i = 0; i < s->tused - 1; i++) {
        if (s->trans[i].to != s->trans[i+1].to)
            nto += 1;
    }
    r = ALLOC_N(trans, nto);
    if (r < 0)
        goto error;

    struct state *to = s->trans[0].to;
    int tind = 0;
    for_each_trans(t, s) {
        if (t->to != to) {
            trans[tind].to = to;
            trans[tind].re = re;
            tind += 1;
            re = NULL;
            to = t->to;
        }
        if (re == NULL) {
            re = make_re_char_set(0, 0);
            if (re == NULL)
                goto error;
        }
        add_re_char(re, t->min, t->max);
    }
    assert(nto == tind + 1);
    trans[tind].to = to;
    trans[tind].re = re;

    /* Simplify CSETs with a single char to a CHAR */
    for (int t=0; t < nto; t++) {
        int cnt = 0;
        uchar chr = UCHAR_MIN;
        for (int c = 0; c < UCHAR_NUM; c++) {
            if (bitset_get(trans[t].re->cset, c)) {
                cnt += 1;
                chr = c;
            }
        }
        if (cnt == 1) {
            re_unref(trans[t].re);
            trans[t].re = make_re_char(chr);
            if (trans[t].re == NULL)
                goto error;
        }
    }
    free_trans(s);
    s->trans = trans;
    s->tused = s->tsize = nto;
    return 0;

 error:
    if (trans)
        for (int i=0; i < nto; i++)
            unref(trans[i].re, re);
    free(trans);
    return -1;
}

ATTRIBUTE_RETURN_CHECK
static int add_new_re_trans(struct state *s1, struct state *s2,
                            struct re *re) {
    int r;
    r = add_new_trans(s1, s2, 0, 0);
    if (r < 0)
        return -1;
    last_trans(s1)->re = re;
    return 0;
}

/* Add the regular expression R1 . LOOP* . R2 to the transition
   from S1 to S2. */
static int re_collapse_trans(struct state *s1, struct state *s2,
                             struct re *r1, struct re *loop, struct re *r2) {
    struct re *re = NULL;

    if (loop->type != EPSILON) {
        loop = make_re_rep(ref(loop), 0, -1);
        if (loop == NULL)
            goto error;
    }

    if (r1->type == EPSILON) {
        if (loop->type == EPSILON) {
            re = ref(r2);
        } else {
            re = make_re_binop(CONCAT, loop, ref(r2));
        }
    } else {
        if (loop->type == EPSILON) {
            if (r2->type == EPSILON) {
                re = ref(r1);
            } else {
                re = make_re_binop(CONCAT, ref(r1), ref(r2));
            }
        } else {
            re = make_re_binop(CONCAT, ref(r1), loop);
            if (re != NULL && r2->type != EPSILON) {
                re = make_re_binop(CONCAT, re, ref(r2));
            }
        }
    }
    if (re == NULL)
        goto error;

    struct trans *t = NULL;
    for (t = s1->trans; t <= last_trans(s1) && t->to != s2; t += 1);
    if (t > last_trans(s1)) {
        if (add_new_re_trans(s1, s2, re) < 0)
            goto error;
    } else {
        if (t->re == NULL) {
            t->re = re;
        } else {
            t->re = make_re_binop(UNION, re, t->re);
            if (t->re == NULL)
                goto error;
        }
    }
    return 0;
 error:
    // FIXME: make sure we don't leak loop
    return -1;
}

static int convert_strings(struct fa *fa) {
    struct state_set *worklist = state_set_init(-1, S_NONE);
    int result = -1;

    E(worklist == NULL);

    /* Abuse hash to count indegree, reachable to mark visited states */
    list_for_each(s, fa->initial) {
        s->hash = 0;
        s->reachable = 0;
    }

    list_for_each(s, fa->initial) {
        for_each_trans(t, s) {
            t->to->hash += 1;
        }
    }

    for (struct state *s = fa->initial;
         s != NULL;
         s = state_set_pop(worklist)) {
        for (int i=0; i < s->tused; i++) {
            struct trans *t = s->trans + i;
            struct state *to = t->to;
            while (to->hash == 1 && to->tused == 1 && ! to->accept) {
                if (t->re == NULL) {
                    t->re = to->trans->re;
                    to->trans->re = NULL;
                } else {
                    t->re = make_re_binop(CONCAT, t->re, to->trans->re);
                    if (t->re == NULL)
                        goto error;
                }
                t->to = to->trans->to;
                to->tused = 0;
                to->hash -= 1;
                to = t->to;
                for (int j=0; j < s->tused; j++) {
                    if (j != i && s->trans[j].to == to) {
                        /* Combine transitions i and j; remove trans j */
                        t->re = make_re_binop(UNION, t->re, s->trans[j].re);
                        if (t->re == NULL)
                            goto error;
                        memmove(s->trans + j, s->trans + j + 1,
                                sizeof(s->trans[j]) * (s->tused - j - 1));
                        to->hash -= 1;
                        s->tused -= 1;
                        if (j < i) {
                            i = i - 1;
                            t = s->trans + i;
                        }
                    }
                }
            }

            if (! to->reachable) {
                to->reachable = 1;
                F(state_set_push(worklist, to));
            }
        }
    }

    for (struct state *s = fa->initial; s->next != NULL; ) {
        if (s->next->hash == 0 && s->next->tused == 0) {
            struct state *del = s->next;
            s->next = del->next;
            free(del->trans);
            free(del);
        } else {
            s = s->next;
        }
    }
    result = 0;

 error:
    state_set_free(worklist);
    return result;
}

/* Convert an FA to a regular expression.
 * The strategy is the following:
 * (1) For all states S1 and S2, convert the transitions between them
 *     into one transition whose regexp is a CSET
 * (2) Add a new initial state INI and a new final state FIN to the automaton,
 *     a transition from INI to the old initial state of FA, and a transition
 *     from all accepting states of FA to FIN; the regexp on those transitions
 *     matches only the empty word
 * (3) Eliminate states S (except for INI and FIN) one by one:
 *     Let LOOP the regexp for the transition S -> S if it exists, epsilon
 *     otherwise.
 *     For all S1, S2 different from S with S1 -> S -> S2
 *       Let R1 the regexp of S1 -> S
 *           R2 the regexp of S -> S2
 *           R3 the regexp S1 -> S2 (or epsilon if no such transition)
 *        set the regexp on the transition S1 -> S2 to
 *          R1 . (LOOP)* . R2 | R3
 * (4) The regexp for the whole FA can now be found as the regexp of
 *     the transition INI -> FIN
 * (5) Convert that STRUCT RE to a string with RE_AS_STRING
 */
int fa_as_regexp(struct fa *fa, char **regexp, size_t *regexp_len) {
    int r;
    struct state *fin = NULL, *ini = NULL;
    struct re *eps = NULL;

    *regexp = NULL;
    *regexp_len = 0;
    fa = fa_clone(fa);
    if (fa == NULL)
        goto error;

    eps = make_re(EPSILON);
    if (eps == NULL)
        goto error;

    fin = add_state(fa,1);
    if (fin == NULL)
        goto error;

    fa->trans_re = 1;

    list_for_each(s, fa->initial) {
        r = convert_trans_to_re(s);
        if (r < 0)
            goto error;
        if (s->accept && s != fin) {
            r = add_new_re_trans(s, fin, ref(eps));
            if (r < 0)
                goto error;
            s->accept = 0;
        }
    }

    ini = add_state(fa, 0);
    if (ini == NULL)
        goto error;

    r = add_new_re_trans(ini, fa->initial, ref(eps));
    if (r < 0)
        goto error;
    set_initial(fa, ini);

    convert_strings(fa);

    list_for_each(s, fa->initial->next) {
        if (s == fin)
            continue;
        /* Eliminate S */
        struct re *loop = eps;
        for_each_trans(t, s) {
            if (t->to == s)
                loop = t->re;
        }
        list_for_each(s1, fa->initial) {
            if (s == s1)
                continue;
            for (int t1 = 0; t1 < s1->tused; t1++) {
                if (s1->trans[t1].to == s) {
                    for (int t = 0; t < s->tused; t++) {
                        if (s->trans[t].to == s)
                            continue;
                        r = re_collapse_trans(s1, s->trans[t].to,
                                              s1->trans[t1].re,
                                              loop,
                                              s->trans[t].re);
                        if (r < 0)
                            goto error;
                    }
                }
            }
        }
    }

    re_unref(eps);

    for_each_trans(t, fa->initial) {
        if (t->to == fin) {
            struct re_str str;
            MEMZERO(&str, 1);
            if (re_as_string(t->re, &str) < 0)
                goto error;
            *regexp = str.rx;
            *regexp_len = str.len;
        }
    }

    list_for_each(s, fa->initial) {
        for_each_trans(t, s) {
            unref(t->re, re);
        }
    }
    fa_free(fa);

    return 0;
 error:
    fa_free(fa);
    re_unref(eps);
    return -1;
}

static int re_restrict_alphabet(struct re *re, uchar from, uchar to) {
    int r1, r2;
    int result = 0;

    switch(re->type) {
    case UNION:
    case CONCAT:
        r1 = re_restrict_alphabet(re->exp1, from, to);
        r2 = re_restrict_alphabet(re->exp2, from, to);
        result = (r1 != 0) ? r1 : r2;
        break;
    case CSET:
        if (re->negate) {
            re->negate = 0;
            bitset_negate(re->cset, UCHAR_NUM);
        }
        for (int i=from; i <= to; i++)
            bitset_clr(re->cset, i);
        break;
    case CHAR:
        if (from <= re->c && re->c <= to)
            result = -1;
        break;
    case ITER:
        result = re_restrict_alphabet(re->exp, from, to);
        break;
    case EPSILON:
        break;
    default:
        assert(0);
        abort();
        break;
    }
    return result;
}

int fa_restrict_alphabet(const char *regexp, size_t regexp_len,
                         char **newregexp, size_t *newregexp_len,
                         char from, char to) {
    int result;
    struct re *re = NULL;
    struct re_parse parse;
    struct re_str str;

    *newregexp = NULL;
    MEMZERO(&parse, 1);
    parse.rx = regexp;
    parse.rend = regexp + regexp_len;
    parse.error = REG_NOERROR;
    re = parse_regexp(&parse);
    if (parse.error != REG_NOERROR)
        return parse.error;

    result = re_restrict_alphabet(re, from, to);
    if (result != 0) {
        result = -2;
        goto done;
    }

    MEMZERO(&str, 1);
    result = re_as_string(re, &str);
    *newregexp = str.rx;
    *newregexp_len = str.len;
 done:
    re_unref(re);
    return result;
}

int fa_expand_char_ranges(const char *regexp, size_t regexp_len,
                          char **newregexp, size_t *newregexp_len) {
    int result;
    struct re *re = NULL;
    struct re_parse parse;
    struct re_str str;

    *newregexp = NULL;
    MEMZERO(&parse, 1);
    parse.rx = regexp;
    parse.rend = regexp + regexp_len;
    parse.error = REG_NOERROR;
    parse.no_ranges = 1;
    re = parse_regexp(&parse);
    if (parse.error != REG_NOERROR)
        return parse.error;

    MEMZERO(&str, 1);
    result = re_as_string(re, &str);
    *newregexp = str.rx;
    *newregexp_len = str.len;
    re_unref(re);
    return result;
}

/* Expand regexp so that it is case-insensitive in a case-sensitive match.
 *
 * Return 1 when a change was made, -1 when an allocation failed, and 0
 * when no change was made.
 */
static int re_case_expand(struct re *re) {
    int result = 0, r1, r2;

    switch(re->type) {
    case UNION:
    case CONCAT:
        r1 = re_case_expand(re->exp1);
        r2 = re_case_expand(re->exp2);
        result = (r1 != 0) ? r1 : r2;
        break;
    case CSET:
        for (int c = 'A'; c <= 'Z'; c++)
            if (bitset_get(re->cset, c)) {
                result = 1;
                bitset_set(re->cset, tolower(c));
            }
        for (int c = 'a'; c <= 'z'; c++)
            if (bitset_get(re->cset, c)) {
                result = 1;
                bitset_set(re->cset, toupper(c));
            }
        break;
    case CHAR:
        if (isalpha(re->c)) {
            int c = re->c;
            re->type = CSET;
            re->negate = false;
            re->no_ranges = 0;
            re->cset = bitset_init(UCHAR_NUM);
            if (re->cset == NULL)
                return -1;
            bitset_set(re->cset, tolower(c));
            bitset_set(re->cset, toupper(c));
            result = 1;
        }
        break;
    case ITER:
        result = re_case_expand(re->exp);
        break;
    case EPSILON:
        break;
    default:
        assert(0);
        abort();
        break;
    }
    return result;
}

int fa_expand_nocase(const char *regexp, size_t regexp_len,
                     char **newregexp, size_t *newregexp_len) {
    int result, r;
    struct re *re = NULL;
    struct re_parse parse;
    struct re_str str;

    *newregexp = NULL;
    MEMZERO(&parse, 1);
    parse.rx = regexp;
    parse.rend = regexp + regexp_len;
    parse.error = REG_NOERROR;
    re = parse_regexp(&parse);
    if (parse.error != REG_NOERROR)
        return parse.error;

    r = re_case_expand(re);
    if (r < 0) {
        re_unref(re);
        return REG_ESPACE;
    }

    if (r == 1) {
        MEMZERO(&str, 1);
        result = re_as_string(re, &str);
        *newregexp = str.rx;
        *newregexp_len = str.len;
    } else {
        *newregexp = strndup(regexp, regexp_len);
        *newregexp_len = regexp_len;
        result = (*newregexp == NULL) ? REG_ESPACE : REG_NOERROR;
    }
    re_unref(re);
    return result;
}

static void print_char(FILE *out, uchar c) {
    /* We escape '/' as '\\/' since dot chokes on bare slashes in labels;
       Also, a space ' ' is shown as '\s' */
    static const char *const escape_from = " \n\t\v\b\r\f\a/\0";
    static const char *const escape_to = "sntvbrfa/0";
    char *p = strchr(escape_from, c);
    if (p != NULL) {
        int i = p - escape_from;
        fprintf(out, "\\\\%c", escape_to[i]);
    } else if (! isprint(c)) {
        fprintf(out, "\\\\0%03o", (unsigned char) c);
    } else if (c == '"') {
        fprintf(out, "\\\"");
    } else {
        fputc(c, out);
    }
}

void fa_dot(FILE *out, struct fa *fa) {
    fprintf(out, "digraph {\n  rankdir=LR;");
    list_for_each(s, fa->initial) {
        if (s->accept) {
            fprintf(out, "\"%p\" [shape=doublecircle];\n", s);
        } else {
            fprintf(out, "\"%p\" [shape=circle];\n", s);
        }
    }
    fprintf(out, "%s -> \"%p\";\n", fa->deterministic ? "dfa" : "nfa",
            fa->initial);

    struct re_str str;
    MEMZERO(&str, 1);
    list_for_each(s, fa->initial) {
        for_each_trans(t, s) {
            fprintf(out, "\"%p\" -> \"%p\" [ label = \"", s, t->to);
            if (fa->trans_re) {
                re_as_string(t->re, &str);
                for (int i=0; i < str.len; i++) {
                    print_char(out, str.rx[i]);
                }
                release_re_str(&str);
            } else {
                print_char(out, t->min);
                if (t->min != t->max) {
                    fputc('-', out);
                    print_char(out, t->max);
                }
            }
            fprintf(out, "\" ];\n");
        }
    }
    fprintf(out, "}\n");
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
