/*
 * fa.c: finite automata
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

/*
 * This implementation follows closely the Java dk.brics.automaton package
 * by Anders Moeller. The project's website is
 * http://www.brics.dk/automaton/.
 *
 * It is by no means a complete reimplementation of that package; only a
 * subset of what Automaton provides is implemented here.
 */

#include <limits.h>
#include <ctype.h>

#include "internal.h"
#include "hash.h"
#include "fa.h"

/* Which algorithm to use in FA_MINIMIZE */
int fa_minimization_algorithm = FA_MIN_HOPCROFT;

/* A finite automaton. INITIAL is both the initial state and the head of
 * the list of all states. Any state that is allocated for this automaton
 * is put on this list. Dead/unreachable states are cleared from the list
 * at opportune times (e.g., during minimization) It's poor man's garbage
 * collection
 */
struct fa {
    struct state *initial;
    int           deterministic : 1;
    int           minimal : 1;
};

/* A state in a finite automaton. Transitions are never shared between
   states so that we can free the list when we need to free the state */
struct state {
    struct state *next;
    unsigned int  accept : 1;
    unsigned int  live : 1;
    unsigned int  reachable : 1;
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
    char          min;
    char          max;
};


#define CHAR_NUM (CHAR_MAX-CHAR_MIN+1)
#define CHAR_IND(i) ((i)-CHAR_MIN)

/*
 * Representation of a parsed regular expression. The regular expression is
 * parsed according to the following grammar by RE_PARSE:
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

static const char * const special_chars = "|()[]{}*+?";

enum re_type {
    UNION,
    CONCAT,
    CSET,
    CHAR,
    ITER,
    EPSILON
};

struct re {
    enum re_type type;
    union {
        struct {                  /* UNION, CONCAT */
            struct re *exp1;
            struct re *exp2;
        };
        struct {                  /* CSET */
            int negate;
            char *cset;
        };
        struct {                  /* CHAR */
            char c;
        };
        struct {                  /* ITER */
            struct re *exp;
            int min;
            int max;
        };
    };
};

/* A map from a set of states to a state. */
typedef hash_t state_set_hash;

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

static struct re *parse_regexp(const char **regexp, int *error);

/* Clean up FA by removing dead transitions and states and reducing
 * transitions. Unreachable states are freed. The return value is the same
 * as FA; returning it is merely a convenience.
 *
 * Only automata in this state should be returned to the user
 */
static struct fa *collect(struct fa *fa);

static void totalize(struct fa *fa);

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
    for (from = CHAR_MIN; from <= CHAR_MAX; from = to+1) {
        while (set->cset[CHAR_IND(from)] == set->negate)
            from += 1;
        if (from > CHAR_MAX)
            break;
        for (to = from;
             to < CHAR_MAX && (set->cset[CHAR_IND(to+1)] == ! set->negate);
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
    CALLOC(s, 1);
    return s;
}

static struct state *add_state(struct fa *fa, int accept) {
    struct state *s = make_state();
    s->accept = accept;
    if (fa->initial == NULL) {
        fa->initial = s;
    } else {
        list_cons(fa->initial->next, s);
    }
    return s;
}

#define for_each_trans(t, s)                                            \
    for (struct trans *t = (s)->trans;                                  \
         (t - (s)->trans) < (s)->tused;                                 \
         t++)

static void add_new_trans(struct state *from, struct state *to,
                          char min, char max) {
    if (from->tused == from->tsize) {
        if (from->tsize == 0)
            from->tsize = array_initial_size;
        else if (from->tsize > array_max_expansion)
            from->tsize += array_max_expansion;
        else
            from->tsize *= 2;
        REALLOC(from->trans, from->tsize);
    }
    from->trans[from->tused].to  = to;
    from->trans[from->tused].min = min;
    from->trans[from->tused].max = max;
    from->tused += 1;
}

static void add_epsilon_trans(struct state *from,
                              struct state *to) {
    from->accept |= to->accept;
    for_each_trans(t, to) {
        add_new_trans(from, t->to, t->min, t->max);
    }
}

static void set_initial(struct fa *fa, struct state *s) {
    list_remove(s, fa->initial);
    list_cons(fa->initial, s);
}

/* Merge automaton FA2 into FA1. This simply adds FA2's states to FA1
   and then frees FA2. It has no influence on the language accepted by FA1
*/
static void fa_merge(struct fa *fa1, struct fa *fa2) {
    list_append(fa1->initial, fa2->initial);
    free(fa2);
}

/*
 * Operations on STATE_SET
 */
static void state_set_init_data(struct state_set *set) {
    set->with_data = 1;
    if (set->data == NULL)
        CALLOC(set->data, set->size);
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
    struct state_set *set;
    CALLOC(set, 1);
    set->sorted = (flags & S_SORTED) ? 1 : 0;
    set->with_data = (flags & S_DATA) ? 1 : 0;
    if (size > 0) {
        set->size = size;
        CALLOC(set->states, set->size);
        if (set->with_data)
            state_set_init_data(set);
    }
    return set;
}

static void state_set_free(struct state_set *set) {
    if (set == NULL)
        return;
    free(set->states);
    free(set->data);
    free(set);
}

static void state_set_expand(struct state_set *set) {
    if (set->size == 0)
        set->size = array_initial_size;
    else if (set->size > array_max_expansion)
        set->size += array_max_expansion;
    else
        set->size *= 2;
    set->states = realloc(set->states, set->size * sizeof(* set->states));
    if (set->with_data)
        set->data = realloc(set->data, set->size * sizeof(* set->data));
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

static int state_set_push(struct state_set *set, struct state *s) {
    if (set->size == set->used)
        state_set_expand(set);
    if (set->sorted) {
        int p = state_set_pos(set, s);
        if (set->size == set->used)
            state_set_expand(set);
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

static int state_set_push_data(struct state_set *set, struct state *s,
                               void *d) {
    int i = state_set_push(set, s);
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
   already in the set. */
static int state_set_add(struct state_set *set, struct state *s) {
    if (set->sorted) {
        int p = state_set_pos(set, s);
        if (p < set->used && set->states[p] == s)
            return 0;
        if (set->size == set->used)
            state_set_expand(set);
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
        state_set_push(set, s);
    }
    return 1;
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
static struct state_set *state_pair_push(struct state_set *set,
                                         struct state *fst,
                                         struct state *snd) {
    if (set == NULL)
        set = state_set_init(-1, S_DATA);
    int i = state_set_push(set, fst);
    set->data[i] = snd;

    return set;
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
    struct state *const *pair = key;
    return ptr_hash(pair[0]) + ptr_hash(pair[1]);
}

static int pair_cmp(const void *key1, const void *key2) {
    struct state *const *pair1 = key1;
    struct state *const *pair2 = key2;

    if (pair1[0] < pair2[0] ||
        (pair1[0] == pair2[0] && pair1[1] < pair2[1]))
        return -1;
    return pair1[1] == pair2[1] ? 0 : 1;
}

static void state_triple_node_free(hnode_t *node, ATTRIBUTE_UNUSED void *ctx) {
    free((void *) hnode_getkey(node));
    free(node);
}

static state_triple_hash *state_triple_init(void) {
    state_triple_hash *hash;

    hash = hash_create(HASHCOUNT_T_MAX, pair_cmp, pair_hash);
    hash_set_allocator(hash, NULL, state_triple_node_free, NULL);
    return hash;
}

static void state_triple_push(state_triple_hash *hash,
                              struct state *s1,
                              struct state *s2,
                              struct state *s3) {
    struct state **pair;
    CALLOC(pair, 2);
    pair[0] = s1;
    pair[1] = s2;
    hash_alloc_insert(hash, pair, s3);
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

static void mark_reachable(struct fa *fa) {
    struct state_set *worklist = state_set_init(-1, S_NONE);

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
                state_set_push(worklist, t->to);
            }
        }
    }
    state_set_free(worklist);
}

/* Return all reachable states. As a sideeffect, all states have their
   REACHABLE flag set appropriately.
 */
static struct state_set *fa_states(struct fa *fa) {
    struct state_set *visited = state_set_init(-1, S_NONE);

    mark_reachable(fa);
    list_for_each(s, fa->initial) {
        if (s->reachable)
            state_set_push(visited, s);
    }
    return visited;
}

/* Return all reachable accepting states. As a sideeffect, all states have
   their REACHABLE flag set appropriately.
 */
static struct state_set *fa_accept_states(struct fa *fa) {
    struct state_set *accept = state_set_init(-1, S_NONE);

    mark_reachable(fa);
    list_for_each(s, fa->initial) {
        if (s->reachable && s->accept)
            state_set_push(accept, s);
    }
    return accept;
}

/* Mark all live states, i.e. states from which an accepting state can be
   reached. All states have their REACHABLE and LIVE flags set
   appropriately.
 */
static void mark_live(struct fa *fa) {
    int changed;

    mark_reachable(fa);
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
}

/*
 * Reverse an automaton in place. Change FA so that it accepts the
 * language that is the reverse of the input automaton.
 *
 * Returns a list of the new initial states of the automaton. The list must
 * be freed by the caller.
 */
static struct state_set *fa_reverse(struct fa *fa) {
    struct state_set *all;
    struct state_set *accept;

    all = fa_states(fa);
    accept = fa_accept_states(fa);

    state_set_init_data(all);

    /* Reverse all transitions */
    int *tused;
    CALLOC(tused, all->used);
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
            add_new_trans(t[j].to, s, t[j].min, t[j].max);
        }
        free(t);
    }
    free(tused);

    /* Make new initial and final states */
    struct state *s = add_state(fa, 0);
    fa->initial->accept = 1;
    set_initial(fa, s);
    for (int i=0; i < accept->used; i++) {
        add_epsilon_trans(s, accept->states[i]);
    }

    fa->deterministic = 0;
    fa->minimal = 0;
    state_set_free(all);
    return accept;
}

/*
 * Return a sorted array of all interval start points in FA. The returned
 * array is a string (null terminated)
 */
static char* start_points(struct fa *fa, int *npoints) {
    char pointset[CHAR_NUM];

    mark_reachable(fa);
    memset(pointset, 0, CHAR_NUM * sizeof(char));
    list_for_each(s, fa->initial) {
        if (! s->reachable)
            continue;
        pointset[CHAR_IND(CHAR_MIN)] = 1;
        for_each_trans(t, s) {
            pointset[CHAR_IND(t->min)] = 1;
            if (t->max < CHAR_MAX)
                pointset[CHAR_IND(t->max+1)] = 1;
        }
    }

    *npoints = 0;
    for(int i=0; i < CHAR_NUM; *npoints += pointset[i], i++);

    char *points;
    CALLOC(points, *npoints+1);
    for (int i=0, n=0; i < CHAR_NUM; i++) {
        if (pointset[i])
            points[n++] = (char) (i + CHAR_MIN);
    }

    return points;
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
        hash += ptr_hash(set->states[i]);
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
}

static state_set_hash *state_set_hash_add(state_set_hash *smap,
                                  struct state_set *set,
                                  struct fa *fa) {
    if (smap == NULL) {
        smap = hash_create(HASHCOUNT_T_MAX, set_cmp, set_hash);
        hash_set_allocator(smap, NULL, set_destroy, NULL);
    }
    hash_alloc_insert(smap, set, add_state(fa, 0));
    return smap;
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

static struct state_set_list *state_set_list_add(struct state_set_list *list,
                                                 struct state_set *set) {
    struct state_set_list *elt;
    CALLOC(elt, 1);
    elt->set = set;
    list_cons(list, elt);
    return list;
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
            s->tsize = s->tused;
            REALLOC(s->trans, s->tsize);
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
static struct fa *collect(struct fa *fa) {

    mark_live(fa);

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
    reduce(fa);
    return fa;
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
static void determinize(struct fa *fa, struct state_set *ini) {
    int npoints;
    int make_ini = (ini == NULL);
    const char *points = NULL;
    state_set_hash *newstate;
    struct state_set_list *worklist;

    if (fa->deterministic)
        return;

    points = start_points(fa, &npoints);
    if (make_ini) {
        ini = state_set_init(-1, S_NONE);
        state_set_push(ini, fa->initial);
    }

    worklist = state_set_list_add(NULL, ini);
    newstate = state_set_hash_add(NULL, ini, fa);
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
            for(int q=0 ; q < sset->used; q++) {
                for_each_trans(t, sset->states[q]) {
                    if (t->min <= points[n] && points[n] <= t->max) {
                        state_set_add(pset, t->to);
                    }
                }
            }
            if (!state_set_hash_contains(newstate, pset)) {
                worklist = state_set_list_add(worklist, pset);
                newstate = state_set_hash_add(newstate, pset, fa);
            }
            pset = state_set_hash_uniq(newstate, pset);

            struct state *q = state_set_hash_get_state(newstate, pset);
            char min = points[n];
            char max = CHAR_MAX;
            if (n+1 < npoints)
                max = points[n+1] - 1;
            add_new_trans(r, q, min, max);
        }
    }
    fa->deterministic = 1;

    state_set_hash_free(newstate, make_ini ? NULL : ini);
    free((void *) points);
    collect(fa);
}

/*
 * Minimization. As a sideeffect of minimization, the transitions are
 * reduced and ordered.
 */

static struct state *step(struct state *s, char c) {
    for_each_trans(t, s) {
        if (t->min <= c && c <= t->max)
            return t->to;
    }
    return NULL;
}

#define UINT_BIT (sizeof(unsigned int) * CHAR_BIT)

typedef unsigned int *bitset_t;

static bitset_t bitset_init(size_t nbits) {
    bitset_t bs;
    CALLOC(bs, (nbits + UINT_BIT) / UINT_BIT);
    return bs;
}

static void bitset_clr(bitset_t bs, unsigned int bit) {
    bs[bit/UINT_BIT] &= ~(1 << (bit % UINT_BIT));
}

static void bitset_set(bitset_t bs, unsigned int bit) {
    bs[bit/UINT_BIT] |= 1 << (bit % UINT_BIT);
}

static int bitset_get(bitset_t bs, unsigned int bit) {
    return bs[bit/UINT_BIT] & (1 << (bit % UINT_BIT));
}

static void bitset_free(bitset_t bs) {
    free(bs);
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

static struct state_list *state_list_init(void) {
    struct state_list *sl;
    CALLOC(sl, 1);
    return sl;
}

static struct state_list_node *state_list_add(struct state_list *sl,
                                              struct state *s) {
    struct state_list_node *n;
    CALLOC(n, 1);
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
    list_free(sl->first);
    free(sl);
}

/* The linear index of element (q,c) in an NSTATES * NSIGMA matrix */
#define INDEX(q, c) (q * nsigma + c)

static void minimize_hopcroft(struct fa *fa) {
    determinize(fa, NULL);

    /* Total automaton, nothing to do */
    if (fa->initial->tused == 1
        && fa->initial->trans[0].to == fa->initial
        && fa->initial->trans[0].min == CHAR_MIN
        && fa->initial->trans[0].max == CHAR_MAX)
        return;

    totalize(fa);

    /* make arrays for numbered states and effective alphabet */
    struct state_set *states = state_set_init(-1, S_NONE);
    list_for_each(s, fa->initial) {
        state_set_push(states, s);
    }
    unsigned int nstates = states->used;

    int nsigma;
    char *sigma = start_points(fa, &nsigma);

    /* initialize data structures */

    /* An ss->used x nsigma matrix of lists of states */
    struct state_set **reverse;
    CALLOC(reverse, nstates * nsigma);
    bitset_t reverse_nonempty = bitset_init(nstates * nsigma);

    struct state_set **partition;
    CALLOC(partition, nstates);

    unsigned int *block;
    CALLOC(block, nstates);

    struct state_list **active;
    CALLOC(active, nstates * nsigma);
    struct state_list_node **active2;
    CALLOC(active2, nstates * nsigma);

    /* PENDING is an array of pairs of ints. The i'th pair is stored in
     * PENDING[2*i] and PENDING[2*i + 1]. There are NPENDING pairs in
     * PENDING at any time. SPENDING is the maximum number of pairs
     * allocated for PENDING.
     */
    int *pending = NULL;
    size_t npending = 0, spending = 0;
    bitset_t pending2 = bitset_init(nstates * nsigma);

    struct state_set *split = state_set_init(-1, S_NONE);
    bitset_t split2 = bitset_init(nstates);

    int *refine;
    CALLOC(refine, nstates);
    bitset_t refine2 = bitset_init(nstates);

    struct state_set **splitblock;
    CALLOC(splitblock, nstates);

    for (int q = 0; q < nstates; q++) {
        splitblock[q] = state_set_init(-1, S_NONE);
        partition[q] = state_set_init(-1, S_NONE);
        for (int x = 0; x < nsigma; x++) {
            reverse[INDEX(q, x)] = state_set_init(-1, S_NONE);
            active[INDEX(q, x)] = state_list_init();
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
        state_set_push(partition[j], qq);
        block[q] = j;
        for (int x = 0; x < nsigma; x++) {
            char y = sigma[x];
            struct state *p = step(qq, y);
            int pn = state_set_index(states, p);
            state_set_push(reverse[INDEX(pn, x)], qq);
            bitset_set(reverse_nonempty, INDEX(pn, x));
        }
    }

    /* initialize active sets */
    for (int j = 0; j <= 1; j++)
        for (int x = 0; x < nsigma; x++)
            for (int q = 0; q < partition[j]->used; q++) {
                struct state *qq = partition[j]->states[q];
                int qn = state_set_index(states, qq);
                if (bitset_get(reverse_nonempty, INDEX(qn, x)))
                    active2[INDEX(qn, x)] =
                        state_list_add(active[INDEX(j, x)], qq);
            }

    /* initialize pending */
    CALLOC(pending, 2*nsigma);
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
                    state_set_push(split, rs);
                    int j = block[s];
                    state_set_push(splitblock[j], rs);
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
                    state_set_push(b2, sp->states[s]);
                    int snum = state_set_index(states, sp->states[s]);
                    block[snum] = k;
                    for (int c = 0; c < nsigma; c++) {
                        struct state_list_node *sn = active2[INDEX(snum, c)];
                        if (sn != NULL && sn->sl == active[INDEX(j,c)]) {
                            state_list_remove(sn);
                            active2[INDEX(snum, c)] =
                                state_list_add(active[INDEX(k, c)],
                                               sp->states[s]);
                        }
                    }
                }
                // update pending
                for (int c = 0; c < nsigma; c++) {
                    int aj = active[INDEX(j, c)]->size;
                    int ak = active[INDEX(k, c)]->size;
                    if (npending + 1 > spending) {
                        spending *= 2;
                        REALLOC(pending, 2 * spending);
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
    struct state_set *newstates = state_set_init(k, S_NONE);
    int *nsnum;
    CALLOC(nsnum, k);
    int *nsind;
    CALLOC(nsind, nstates);

    for (int n = 0; n < k; n++) {
        struct state *s = make_state();
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
            add_new_trans(s, nto, t->min, t->max);
        }
    }
    free(nsind);
    free(nsnum);

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

    /* clean up */
    state_set_free(states);
    free(sigma);
    bitset_free(reverse_nonempty);
    free(block);
    for (int i=0; i < nstates*nsigma; i++) {
        state_set_free(reverse[i]);
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
        state_set_free(splitblock[q]);
        state_set_free(partition[q]);
    }
    free(splitblock);
    free(partition);
    state_set_free(newstates);

    collect(fa);
}

static void minimize_brzozowski(struct fa *fa) {
    struct state_set *set;

    /* Minimize using Brzozowski's algorithm */
    set = fa_reverse(fa);
    determinize(fa, set);
    state_set_free(set);

    set = fa_reverse(fa);
    determinize(fa, set);
    state_set_free(set);
}

void fa_minimize(struct fa *fa) {
    if (fa->minimal)
        return;

    if (fa_minimization_algorithm == FA_MIN_BRZOZOWSKI) {
        minimize_brzozowski(fa);
    } else {
        minimize_hopcroft(fa);
    }

    fa->minimal = 1;
}

/*
 * Construction of fa
 */

static struct fa *fa_make_empty(void) {
    struct fa *fa;

    CALLOC(fa, 1);
    add_state(fa, 0);
    fa->deterministic = 1;
    fa->minimal = 1;
    return fa;
}

static struct fa *fa_make_epsilon(void) {
    struct fa *fa = fa_make_empty();
    fa->initial->accept = 1;
    fa->deterministic= 1;
    fa->minimal = 1;
    return fa;
}

static struct fa *fa_make_char(char c) {
    struct fa *fa = fa_make_empty();
    struct state *s = fa->initial;
    struct state *t = add_state(fa, 1);

    add_new_trans(s, t, c, c);
    fa->deterministic = 1;
    fa->minimal = 1;
    return fa;
}

struct fa *fa_make_basic(unsigned int basic) {
    if (basic == FA_EMPTY) {
        return fa_make_empty();
    } else if (basic == FA_EPSILON) {
        return fa_make_epsilon();
    } else if (basic == FA_TOTAL) {
        struct fa *fa = fa_make_epsilon();
        add_new_trans(fa->initial, fa->initial, CHAR_MIN, CHAR_MAX);
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
        if (! fa->initial->accept || fa->initial->tused != 1)
            return 0;
        struct trans *t = fa->initial->trans;
        return t->to == fa->initial &&
            t->min == CHAR_MIN && t->max == CHAR_MAX;
    }
    return 0;
}

static struct fa *fa_clone(struct fa *fa) {
    struct fa *result = NULL;
    struct state_set *set = state_set_init(-1, S_DATA|S_SORTED);

    CALLOC(result, 1);
    result->deterministic = fa->deterministic;
    result->minimal = fa->minimal;
    list_for_each(s, fa->initial) {
        int i = state_set_push(set, s);
        struct state *q = add_state(result, s->accept);
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
            add_new_trans(sc, toc, t->min, t->max);
        }
    }
    state_set_free(set);
    return result;
}

/* Compute FA1|FA2 and set FA1 to that automaton. FA2 is freed */
static void union_in_place(struct fa *fa1, struct fa *fa2) {
    struct state *s;

    s = add_state(fa1, 0);
    add_epsilon_trans(s, fa1->initial);
    add_epsilon_trans(s, fa2->initial);

    fa1->deterministic = 0;
    fa1->minimal = 0;
    fa_merge(fa1, fa2);

    set_initial(fa1, s);

    collect(fa1);
}

struct fa *fa_union(struct fa *fa1, struct fa *fa2) {
    fa1 = fa_clone(fa1);
    fa2 = fa_clone(fa2);

    union_in_place(fa1, fa2);

    return fa1;
}

/* Concat FA2 onto FA1; frees FA2 and changes FA1 to FA1.FA2 */
static void concat_in_place(struct fa *fa1, struct fa *fa2) {
    list_for_each(s, fa1->initial) {
        if (s->accept) {
            s->accept = 0;
            add_epsilon_trans(s, fa2->initial);
        }
    }

    fa1->deterministic = 0;
    fa1->minimal = 0;
    fa_merge(fa1, fa2);

    collect(fa1);
}

struct fa *fa_concat(struct fa *fa1, struct fa *fa2) {
    fa1 = fa_clone(fa1);
    fa2 = fa_clone(fa2);
    concat_in_place(fa1, fa2);
    return fa1;
}

static struct fa *fa_make_char_set(char *cset, int negate) {
    struct fa *fa = fa_make_empty();
    struct state *s = fa->initial;
    struct state *t = add_state(fa, 1);
    int from = CHAR_MIN;

    while (from <= CHAR_MAX) {
        while (from <= CHAR_MAX && cset[CHAR_IND(from)] == negate)
            from += 1;
        if (from > CHAR_MAX)
            break;
        int to = from;
        while (to < CHAR_MAX && (cset[CHAR_IND(to + 1)] == !negate))
            to += 1;
        add_new_trans(s, t, from, to);
        from = to + 1;
    }

    fa->deterministic = 1;
    fa->minimal = 1;
    return fa;
}

static struct fa *fa_star(struct fa *fa) {
    struct state *s;

    fa = fa_clone(fa);

    s = add_state(fa, 1);
    add_epsilon_trans(s, fa->initial);
    set_initial(fa, s);
    list_for_each(p, fa->initial->next) {
        if (p->accept)
            add_epsilon_trans(p, s);
    }
    fa->deterministic = 0;
    fa->minimal = 0;

    return collect(fa);
}

/* Form the automaton (FA){N}; FA is not modified */
static struct fa *repeat(struct fa *fa, int n) {
    if (n == 0) {
        return fa_make_epsilon();
    } else if (n == 1) {
        return fa_clone(fa);
    } else {
        struct fa *cfa = fa_clone(fa);
        while (n > 1) {
            struct fa *tfa = fa_clone(fa);
            concat_in_place(cfa, tfa);
            n -= 1;
        }
        return cfa;
    }
}

struct fa *fa_iter(struct fa *fa, int min, int max) {
    if (min < 0)
        min = 0;

    if (min > max && max != -1) {
        return fa_make_empty();
    }
    if (max == -1) {
        struct fa *sfa = fa_star(fa);
        if (min == 0)
            return sfa;
        struct fa *cfa = repeat(fa, min);
        concat_in_place(cfa, sfa);
        return cfa;
    } else {
        struct fa *cfa = NULL;

        max -= min;
        cfa = repeat(fa, min);
        if (max > 0) {
            struct fa *cfa2 = fa_clone(fa);
            while (max > 1) {
                struct fa *cfa3 = fa_clone(fa);
                list_for_each(s, cfa3->initial) {
                    if (s->accept) {
                        add_epsilon_trans(s, cfa2->initial);
                    }
                }
                fa_merge(cfa3, cfa2);
                cfa2 = cfa3;
                max -= 1;
            }
            list_for_each(s, cfa->initial) {
                if (s->accept) {
                    add_epsilon_trans(s, cfa2->initial);
                }
            }
            fa_merge(cfa, cfa2);
            cfa->deterministic = 0;
            cfa->minimal = 0;
        }
        return collect(cfa);
    }
}

static void sort_transition_intervals(struct fa *fa) {
    list_for_each(s, fa->initial) {
        qsort(s->trans, s->tused, sizeof(*s->trans), trans_intv_cmp);
    }
}

struct fa *fa_intersect(struct fa *fa1, struct fa *fa2) {
    if (fa1 == fa2)
        return fa_clone(fa1);

    if (fa_is_basic(fa1, FA_EMPTY) || fa_is_basic(fa2, FA_EMPTY))
        return fa_make_empty();

    struct fa *fa = fa_make_empty();
    struct state_set *worklist = state_set_init(-1, S_NONE);
    state_triple_hash *newstates = state_triple_init();

    sort_transition_intervals(fa1);
    sort_transition_intervals(fa2);

    state_set_push(worklist, fa1->initial);
    state_set_push(worklist, fa2->initial);
    state_set_push(worklist, fa->initial);
    state_triple_push(newstates, fa1->initial, fa2->initial, fa->initial);
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
                        state_set_push(worklist, t1[n1].to);
                        state_set_push(worklist, t2[n2].to);
                        state_set_push(worklist, r);
                        state_triple_push(newstates, t1[n1].to, t2[n2].to, r);
                    }
                    char min = t1[n1].min > t2[n2].min 
                        ? t1[n1].min : t2[n2].min;
                    char max = t1[n1].max < t2[n2].max 
                        ? t1[n1].max : t2[n2].max;
                    add_new_trans(s, r, min, max);
                }
            }
        }
    }
    state_set_free(worklist);
    state_triple_free(newstates);
    collect(fa);

    return fa;
}

int fa_contains(fa_t fa1, fa_t fa2) {
    int result = 0;
    struct state_set *worklist;  /* List of pairs of states */
    struct state_set *visited;   /* List of pairs of states */

    if (fa1 == fa2)
        return 1;

    determinize(fa2, NULL);
    sort_transition_intervals(fa1);
    sort_transition_intervals(fa2);

    worklist = state_pair_push(NULL, fa1->initial, fa2->initial);
    visited  = state_pair_push(NULL, fa1->initial, fa2->initial);
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
                    min1 = CHAR_MAX;
                    max1 = CHAR_MIN;
                }
                if (state_pair_find(visited, t1[n1].to, t2[n2].to) == -1) {
                    worklist = state_pair_push(worklist, t1[n1].to, t2[n2].to);
                    visited  = state_pair_push(visited, t1[n1].to, t2[n2].to);
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
}

static void totalize(struct fa *fa) {
    struct state *crash = add_state(fa, 0);

    mark_reachable(fa);
    sort_transition_intervals(fa);

    add_new_trans(crash, crash, CHAR_MIN, CHAR_MAX);
    list_for_each(s, fa->initial) {
        int next = CHAR_MIN;
        int tused = s->tused;
        for (int i=0; i < tused; i++) {
            char min = s->trans[i].min, max = s->trans[i].max;
            if (min > next)
                add_new_trans(s, crash, next, min - 1);
            if (max + 1 > next)
                next = max + 1;
        }
        if (next <= CHAR_MAX)
            add_new_trans(s, crash, next, CHAR_MAX);
    }
}

struct fa *fa_complement(struct fa *fa) {
    fa = fa_clone(fa);
    determinize(fa, NULL);
    totalize(fa);
    list_for_each(s, fa->initial)
        s->accept = ! s->accept;

    return collect(fa);
}

struct fa *fa_minus(struct fa *fa1, struct fa *fa2) {
    if (fa_is_basic(fa1, FA_EMPTY) || fa1 == fa2)
        return fa_make_empty();
    if (fa_is_basic(fa2, FA_EMPTY))
        return fa_clone(fa1);

    struct fa *cfa2 = fa_complement(fa2);
    struct fa *result = fa_intersect(fa1, cfa2);

    fa_free(cfa2);
    return result;
}

static void accept_to_accept(struct fa *fa) {
    struct state *s = add_state(fa, 0);

    mark_reachable(fa);
    list_for_each(a, fa->initial) {
        if (a->accept && a->reachable)
            add_epsilon_trans(s, a);
    }

    set_initial(fa, s);
    fa->deterministic = fa->minimal = 0;
}

struct fa *fa_overlap(struct fa *fa1, struct fa *fa2) {
    struct fa *fa;
    struct state_set *map;

    fa1 = fa_clone(fa1);
    fa2 = fa_clone(fa2);

    accept_to_accept(fa1);

    map = fa_reverse(fa2);
    state_set_free(map);
    determinize(fa2, NULL);
    accept_to_accept(fa2);
    map = fa_reverse(fa2);
    state_set_free(map);
    determinize(fa2, NULL);

    fa = fa_intersect(fa1, fa2);
    fa_free(fa1);
    fa_free(fa2);

    struct fa *eps = fa_make_epsilon();
    struct fa *result = fa_minus(fa, eps);

    fa_free(fa);
    fa_free(eps);
    return result;
}

int fa_equals(fa_t fa1, fa_t fa2) {
    return fa_contains(fa1, fa2) && fa_contains(fa2, fa1);
}

static unsigned int chr_score(char c) {
    if (isalpha(c)) {
        return 2;
    } else if (isalnum(c)) {
        return 3;
    } else if (isprint(c)) {
        return 7;
    } else {
        return 100;
    }
}

static unsigned int str_score(const char *s) {
    unsigned int score = 0;
    for ( ;*s; s++) {
        score += chr_score(*s);
    }
    return score;
}

/* See if we get a better string for DST by appending C to SRC. If DST is
 * NULL or empty, always use SRC + C
 */
static char *string_extend(char *dst, const char *src, char c) {
    if (dst == NULL
        || *dst == '\0'
        || str_score(src) + chr_score(c) < str_score(dst)) {
        int slen = strlen(src);
        dst = realloc(dst, slen + 2);
        strncpy(dst, src, slen);
        dst[slen] = c;
        dst[slen + 1] = '\0';
    }
    return dst;
}

static char pick_char(struct trans *t) {
    for (char c = t->min; c <= t->max; c++)
        if (isalpha(c)) return c;
    for (char c = t->min; c <= t->max; c++)
        if (isalnum(c)) return c;
    for (char c = t->min; c <= t->max; c++)
        if (isprint(c)) return c;
    return t->min;
}

/* Generate an example string for FA. Traverse all transitions and record
 * at each turn the "best" word found for that state.
 */
char *fa_example(fa_t fa) {
    /* Sort to avoid any ambiguity because of reordering of transitions */
    sort_transition_intervals(fa);

    /* Map from state to string */
    struct state_set *path = state_set_init(-1, S_DATA|S_SORTED);
    state_set_push_data(path, fa->initial,(void*) strdup(""));

    /* List of states still to visit */
    struct state_set *worklist = state_set_init(-1, S_NONE);
    state_set_push(worklist, fa->initial);

    while (worklist->used) {
        struct state *s = state_set_pop(worklist);
        char *ps = state_set_find_data(path, s);
        for_each_trans(t, s) {
            char c = pick_char(t);
            int toind = state_set_index(path, t->to);
            if (toind == -1) {
                char *w = string_extend(NULL, ps, c);
                state_set_push(worklist, t->to);
                state_set_push_data(path, t->to, (void *) w);
            } else {
                path->data[toind] = string_extend(path->data[toind], ps, c);
            }
        }
    }

    char *word = NULL;
    for (int i=0; i < path->used; i++) {
        struct state *p = path->states[i];
        char *ps = path->data[i];
        if (p->accept &&
            (word == NULL || *word == '\0'
             || (*ps != '\0' && str_score(word) > str_score(ps)))) {
            free(word);
            word = ps;
        } else {
            free(ps);
        }
    }
    state_set_free(path);
    state_set_free(worklist);
    return word;
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
    fa = fa_clone(fa);

    mark_reachable(fa);
    list_for_each(p, fa->initial) {
        if (! p->reachable)
            continue;

        struct state *r = add_state(fa, 0);
        r->trans = p->trans;
        r->tused = p->tused;
        r->tsize = p->tsize;
        p->trans = NULL;
        p->tused = p->tsize = 0;
        add_new_trans(p, r, X, X);
        if (add_marker) {
            struct state *q = add_state(fa, 0);
            add_new_trans(p, q, Y, Y);
            add_new_trans(q, p, X, X);
        }
    }
    return fa;
}

/* This algorithm is due to Anders Moeller, and can be found in class
 * AutomatonOperations in dk.brics.grammar
 */
char *fa_ambig_example(fa_t fa1, fa_t fa2, char **pv, char **v) {
    static const char X = '\001';
    static const char Y = '\002';

#define Xs "\001"
#define Ys "\002"
    /* These could become static constants */
    fa_t mp, ms, sp, ss;
    fa_compile( Ys Xs "(" Xs "(.|\n))+", &mp);
    fa_compile( Ys Xs "(" Xs "(.|\n))*", &ms);
    fa_compile( "(" Xs "(.|\n))+" Ys Xs, &sp);
    fa_compile("(" Xs "(.|\n))*" Ys Xs, &ss);
#undef Xs
#undef Ys

    fa_t a1f = expand_alphabet(fa1, 0, X, Y);
    fa_t a1t = expand_alphabet(fa1, 1, X, Y);
    fa_t a2f = expand_alphabet(fa2, 0, X, Y);
    fa_t a2t = expand_alphabet(fa2, 1, X, Y);

    /* Compute b1 = ((a1f . mp) & a1t) . ms */
    concat_in_place(a1f, mp);
    fa_t b1 = fa_intersect(a1f, a1t);
    concat_in_place(b1, ms);

    /* Compute b2 = ss . ((sp . a2f) & a2t) */
    concat_in_place(sp, a2f);
    fa_t b2 = fa_intersect(sp, a2t);
    concat_in_place(ss, b2);
    b2 = ss;

    /* The automaton we are really interested in */
    fa_t amb = fa_intersect(b1, b2);

    /* Clean up intermediate automata */
    fa_free(sp);
    fa_free(a1f);
    fa_free(a1t);
    fa_free(a2t);
    fa_free(b1);
    fa_free(b2);

    char *s = fa_example(amb);
    fa_free(amb);

    if (s == NULL)
        return NULL;

    char *result, *t;
    CALLOC(result, (strlen(s)-1)/2 + 1);
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

    free(s);
    return result;
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
            struct fa *fa2 = fa_from_re(re->exp2);
            union_in_place(result, fa2);
        }
        break;
    case CONCAT:
        {
            result = fa_from_re(re->exp1);
            struct fa *fa2 = fa_from_re(re->exp2);
            concat_in_place(result, fa2);
        }
        break;
    case CSET:
        result = fa_make_char_set(re->cset, re->negate);
        break;
    case ITER:
        {
            struct fa *fa = fa_from_re(re->exp);
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
}

static void re_free(struct re *re) {
    if (re == NULL)
        return;
    if (re->type == UNION || re->type == CONCAT) {
        re_free(re->exp1);
        re_free(re->exp2);
    } else if (re->type == ITER) {
        re_free(re->exp);
    } else if (re->type == CSET) {
        free(re->cset);
    }
    free(re);
}

int fa_compile(const char *regexp, struct fa **fa) {
    int ret = REG_NOERROR;
    struct re *re = parse_regexp(&regexp, &ret);
    *fa = NULL;
    if (re == NULL)
        return ret;

    //print_re(re);
    //printf("\n");

    *fa = fa_from_re(re);
    re_free(re);

    // FIXME: it should be enough to clean up here, but that makes
    // various tests fail
    fa_minimize(*fa);
    return ret;
}

/*
 * Regular expression parser
 */

static struct re *make_re(enum re_type type) {
    struct re *re;
    CALLOC(re, 1);
    re->type = type;
    return re;
}

static struct re *make_re_rep(struct re *exp, int min, int max) {
    struct re *re = make_re(ITER);
    re->exp = exp;
    re->min = min;
    re->max = max;
    return re;
}

static struct re *make_re_binop(enum re_type type, struct re *exp1,
                                struct re *exp2) {
    struct re *re = make_re(type);
    re->exp1 = exp1;
    re->exp2 = exp2;
    return re;
}

static struct re *make_re_char(char c) {
    struct re *re = make_re(CHAR);
    re->c = c;
    return re;
}

static struct re *make_re_char_set(int negate) {
    struct re *re = make_re(CSET);
    re->negate = negate;
    CALLOC(re->cset, CHAR_NUM);
    return re;
}

static int more(const char **regexp) {
    return (*regexp) != '\0';
}

static int match(const char **regexp, char m) {
    if (!more(regexp))
        return 0;
    if (**regexp == m) {
        (*regexp) += 1;
        return 1;
    }
    return 0;
}

static int peek(const char **regexp, const char *chars) {
    return strchr(chars, **regexp) != NULL;
}

static char next(const char **regexp) {
    char c = **regexp;
    if (c != '\0')
        *regexp += 1;
    return c;
}

static char parse_char(const char **regexp, const char *special) {
    if (match(regexp, '\\')) {
        char c = next(regexp);
        if (special != NULL) {
            char *f = strchr(special, c);
            if (f != NULL)
                return c;
        }
        if (c == 'n')
            return '\n';
        else if (c == 't')
            return '\t';
        else if (c == 'r')
            return '\r';
        else
            return c;
    } else {
        return next(regexp);
    }
}

static void add_re_char(struct re *re, char from, char to) {
    assert(re->type == CSET);
    for (char c = from; c <= to; c++)
        re->cset[CHAR_IND(c)] = 1;
}

static void parse_char_class(const char **regexp, struct re *re,
                             int *error) {
    if (! more(regexp)) {
        *error = REG_EBRACK;
        goto error;
    }
    char from = parse_char(regexp, NULL);
    char to = from;
    if (match(regexp, '-')) {
        if (! more(regexp)) {
            *error = REG_EBRACK;
            goto error;
        }
        if (peek(regexp, "]")) {
            add_re_char(re, from, to);
            add_re_char(re, '-', '-');
            return;
        } else {
            to = parse_char(regexp, NULL);
        }
    }
    add_re_char(re, from, to);
 error:
    return;
}

static struct re *parse_simple_exp(const char **regexp, int *error) {
    struct re *re = NULL;

    if (match(regexp, '[')) {
        int negate = match(regexp, '^');
        re = make_re_char_set(negate);
        if (re == NULL)
            goto error;
        parse_char_class(regexp, re, error);
        if (*error != REG_NOERROR)
            goto error;
        while (more(regexp) && ! peek(regexp, "]")) {
            parse_char_class(regexp, re, error);
            if (*error != REG_NOERROR)
                goto error;
        }
        if (! match(regexp, ']')) {
            *error = REG_EBRACK;
            goto error;
        }
    } else if (match(regexp, '(')) {
        if (match(regexp, ')')) {
            return make_re(EPSILON);
        }
        re = parse_regexp(regexp, error);
        if (re == NULL)
            goto error;
        if (! match(regexp, ')')) {
            *error = REG_EPAREN;
            goto error;
        }
    } else if (match(regexp, '.')) {
        re = make_re_char_set(1);
        add_re_char(re, '\n', '\n');
    } else {
        if (more(regexp)) {
            char c = parse_char(regexp, special_chars);
            re = make_re_char(c);
        }
    }
    return re;
 error:
    re_free(re);
    return NULL;
}

static int parse_int(const char **regexp, int *error) {
    char *end;
    long l = strtoul(*regexp, &end, 10);
    *regexp = end;
    if ((l<0) || (l > INT_MAX)) {
        *error = REG_BADBR;
        return -1;
    }
    return (int) l;
}

static struct re *parse_repeated_exp(const char **regexp, int *error) {
    struct re *re = parse_simple_exp(regexp, error);
    if (re == NULL)
        goto error;
    if (match(regexp, '?')) {
        re = make_re_rep(re, 0, 1);
    } else if (match(regexp, '*')) {
        re = make_re_rep(re, 0, -1);
    } else if (match(regexp, '+')) {
        re = make_re_rep(re, 1, -1);
    } else if (match(regexp, '{')) {
        int min, max;
        min = parse_int(regexp, error);
        if (min == -1) {
            *error = REG_BADBR;
            goto error;
        }
        if (match(regexp, ',')) {
            max = parse_int(regexp, error);
            if (max == -1)
                goto error;
            if (! match(regexp, '}')) {
                *error = REG_EBRACE;
                goto error;
            }
        } else if (match(regexp, '}')) {
            max = min;
        } else {
            *error = REG_EBRACE;
            goto error;
        }
        if (min > max) {
            *error = REG_BADBR;
            goto error;
        }
        re = make_re_rep(re, min, max);
    }
    return re;
 error:
    re_free(re);
    return NULL;
}

static struct re *parse_concat_exp(const char **regexp, int *error) {
    struct re *re = parse_repeated_exp(regexp, error);
    if (re == NULL)
        goto error;

    if (more(regexp) && ! peek(regexp, ")|")) {
        struct re *re2 = parse_concat_exp(regexp, error);
        if (re2 == NULL)
            goto error;
        return make_re_binop(CONCAT, re, re2);
    }
    return re;

 error:
    re_free(re);
    return NULL;
}

static struct re *parse_regexp(const char **regexp, int *error) {
    struct re *re = parse_concat_exp(regexp, error);
    if (re == NULL)
        goto error;

    if (match(regexp, '|')) {
        struct re *re2 = parse_regexp(regexp, error);
        if (re2 == NULL)
            goto error;
        return make_re_binop(UNION, re, re2);
    }
    return re;

 error:
    re_free(re);
    return NULL;
}

static void print_char(FILE *out, char c) {
    /* We escape '/' as '\\/' since dot chokes on bare slashes in labels;
       Also, a space ' ' is shown as '\s' */
    static const char *const escape_from = " \n\t\v\b\r\f\a/";
    static const char *const escape_to = "sntvbrfa/";
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

    list_for_each(s, fa->initial) {
        for_each_trans(t, s) {
            fprintf(out, "\"%p\" -> \"%p\" [ label = \"", s, t->to);
            print_char(out, t->min);
            if (t->min != t->max) {
                fputc('-', out);
                print_char(out, t->max);
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
