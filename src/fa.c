/*
 * fa.c: finite automata
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

/*
 * This implementation follows closely the Java dk.brics.automaton package
 * by Anders Moeller. The project's website is
 * http://www.brics.dk/automaton/.
 *
 * It is by no means a complete
 * reimplementation of that package; only a subset of what Automaton
 * provides is implemented here.
 */

#include <limits.h>
#include <ctype.h>

#include "internal.h"
#include "fa.h"

/* A finite automaton. INITIAL is both the initial state and the head of
 * the list of all states. Any state that is allocated for this automaton
 * is put on this list. Dead/unreachable states are cleared from the list
 * at opportune times (e.g., during minimization) It's poor man's garbage
 * collection
 */
struct fa {
    struct fa_state *initial;
    int              deterministic;
};

/* A state in a finite automaton. Transitions are never shared between
   states so that we can free the list when we need to free the state */
struct fa_state {
    struct fa_state *next;
    struct fa_trans *transitions;
    int              accept;
};

/* A transition. If the input has a character in the inclusive 
 * range [MIN, MAX], move to TO
 */
struct fa_trans {
    struct fa_trans *next;
    struct fa_state *to;
    char             min;
    char             max;
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
    NSET,
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
        struct {                  /* CSET, NSET */
            char from;
            char to;
        };
        struct {                  /* ITER */
            struct re *exp;
            int min;
            int max;
        };
    };
};

/*
 * Auxiliary data structure used internally in some automata
 * computations. It is used either as a map between states (mapping fst ->
 * snd) or as a map from states to a list of transitions (mapping fst ->
 * trans)
 */
struct fa_map {
    struct fa_map *next;
    struct fa_state *fst;
    struct fa_state *snd;
    struct fa_trans *trans;
};

/* A map from a set of states to a state. A set is represented by an
 * FA_MAP, where only the FST entries of the FA_MAP are used. Used by
 * determinize for the subset construction of turning an NFA into a DFA
 */
struct fa_set_map {
    struct fa_set_map *next;
    struct fa_map     *set;
    struct fa_state   *state;
};

static struct re *parse_regexp(const char **regexp, int *error);


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
        if (re->from == re->to)
            printf("%c", re->from);
        else
            printf("[%c-%c]", re->from, re->to);
        break;
    case NSET:
        if (re->from == re->to)
            printf("[^%c]", re->from);
        else
            printf("[^%c-%c]", re->from, re->to);
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

void fa_free(struct fa *fa) {
    if (fa == NULL)
        return;
    list_for_each(s, fa->initial) {
        list_free(s->transitions);
    }
    list_free(fa->initial);
    free(fa);
}

static struct fa_state *map_get(struct fa_map *map,
                                struct fa_state *s) {
    while (map != NULL && map->fst != s)
        map = map->next;
    return map->snd;
}

static struct fa *fa_clone(struct fa *fa) {
    struct fa *result = NULL;
    struct fa_map *state_map = NULL;

    CALLOC(result, 1);
    result->deterministic = fa->deterministic;
    list_for_each(s, fa->initial) {
        struct fa_map *pair;
        CALLOC(pair, 1);
        pair->fst = s;
        CALLOC(pair->snd, 1);
        pair->snd->accept = s->accept;
        list_append(state_map, pair);
        list_append(result->initial, pair->snd);
    }
    list_for_each(s, fa->initial) {
        struct fa_state *sc;
        sc = map_get(state_map, s);
        assert(sc != NULL);
        list_for_each(t, s->transitions) {
            struct fa_trans *tc;
            CALLOC(tc, 1);
            tc->to = map_get(state_map, t->to);
            tc->min = t->min;
            tc->max = t->max;
            list_append(sc->transitions, tc);
        }
    }
    list_free(state_map);
    return result;
}

static struct fa *make_fa(struct fa_state *initial) {
    struct fa *fa;
    CALLOC(fa, 1);
    fa->initial = initial;
    return fa;
}

static struct fa_state *make_state(int accept) {
    struct fa_state *s;
    CALLOC(s, 1);
    s->accept = accept;
    return s;
}

static struct fa_trans *make_trans(struct fa_state *to,
                                   char min, char max) {
    struct fa_trans *trans;
    CALLOC(trans, 1);
    trans->min = min;
    trans->max = max;
    trans->to = to;
    return trans;
}

static struct fa_trans *add_new_trans(struct fa_state *from,
                                      struct fa_state *to,
                                      char min, char max) {
    struct fa_trans *trans = make_trans(to, min, max);
    list_append(from->transitions, trans);
    return trans;
}

static struct fa_trans *clone_trans(struct fa_trans *t) {
    struct fa_trans *c;
    CALLOC(c, 1);
    c->to = t->to;
    c->min = t->min;
    c->max = t->max;
    return c;
}

static void add_epsilon_trans(struct fa_state *from,
                              struct fa_state *to) {
    from->accept |= to->accept;
    list_for_each(t, to->transitions) {
        list_append(from->transitions, clone_trans(t));
    }
}

static void set_initial(struct fa *fa, struct fa_state *s) {
    s->next = fa->initial;
    fa->initial = s;
}

/* Merge automaton FA2 into FA1. This simply adds FA2's states to FA1
   and then frees FA2. It has no influence on the language accepted by FA1
*/
static void fa_merge(struct fa *fa1, struct fa *fa2) {
    list_append(fa1->initial, fa2->initial);
    free(fa2);
}

/*
 * Operations in an FA_MAP
 */
static struct fa_map *fa_map_append(struct fa_map *map, 
                                    struct fa_state *fst,
                                    struct fa_state *snd,
                                    struct fa_trans *trans) {
    struct fa_map *e;

    CALLOC(e, 1);
    e->fst = fst;
    e->snd = snd;
    e->trans = trans;
    list_append(map, e);

    return map;
}

static struct fa_map *fa_map_pop(struct fa_map *map) {
    struct fa_map *del;

    if (map == NULL)
        return NULL;

    del = map;
    map = map->next;
    free(del);
    return map;
}

static struct fa_map *fa_map_find_fst(struct fa_map *map,
                                      struct fa_state *s) {
    while (map != NULL) {
        if (map->fst == s)
            return map;
        map = map->next;
    }
    return NULL;
}

/*
 * State operations
 */

/* Return all reachable states. The returned FA_MAP has the states in its
 * FST entries.
 */
static struct fa_map *fa_states(struct fa *fa) {
    struct fa_map *visited = NULL;
    struct fa_map *worklist = NULL;

    worklist = fa_map_append(NULL, fa->initial, NULL, NULL);
    visited = fa_map_append(NULL, fa->initial, NULL, NULL);
    while (worklist != NULL) {
        struct fa_state *s = worklist->fst;
        worklist = fa_map_pop(worklist);
        list_for_each(t, s->transitions) {
            if (! fa_map_find_fst(visited, t->to)) {
                visited = fa_map_append(visited, t->to, NULL, NULL);
                worklist = fa_map_append(worklist, t->to, NULL, NULL);
            }
        }
    }

    return visited;
}

/* Return all reachable accepting states. The returned FA_MAP has the
 * accepting states in its FST entries.
 */
static struct fa_map *fa_accept_states(struct fa *fa) {
    struct fa_map *states = fa_states(fa);
    struct fa_map *m;

    while (states != NULL && !states->fst->accept)
        states = fa_map_pop(states);

    if (states == NULL)
        return NULL;

    m = states;
    while (m->next != NULL) {
        if (m->next->fst->accept) {
            m = m->next;
        } else {
            struct fa_map *del = m->next;
            m->next = del->next;
            free(del);
        }
    }

    return states;
}

/* Return all live states, i.e. states from which an accepting state can be
 * reached. The returned FA_MAP has the live states in its FST entries.
 */
static struct fa_map *fa_live_states(struct fa *fa) {
    struct fa_map *states = fa_states(fa);
    struct fa_map *live = NULL;
    int changed;

    for (struct fa_map *s = states; s != NULL; ) {
        struct fa_map *n = s->next;
        if (s->fst->accept) {
            list_remove(s, states);
            list_append(live, s);
        }
        s = n;
    }

    do {
        changed = 0;
        struct fa_map *s = states;
        while (s != NULL) {
            struct fa_map *next = s->next;
            list_for_each(t, s->fst->transitions) {
                if (fa_map_find_fst(live, t->to) != NULL) {
                    changed = 1;
                    list_remove(s, states);
                    list_append(live, s);
                    break;
                }
            }
            s = next;
        }
    } while (changed && states != NULL);

    list_free(states);
    return live;
}

/*
 * Reverse an automaton in place. Change FA so that it accepts the
 * language that is the reverse of the input automaton.
 *
 * Returns a list of the new initial states of the automaton. The list must
 * be freed by the caller.
 */
static struct fa_map *fa_reverse(struct fa *fa) {
    struct fa_map *states;
    struct fa_map *accept;

    states = fa_states(fa);
    accept = fa_accept_states(fa);

    /* Reverse all transitions */
    list_for_each(m, states) {
        m->fst->accept = 0;
        list_for_each(t, m->fst->transitions) {
            struct fa_trans *rev = make_trans(m->fst, t->min, t->max);
            struct fa_map *mrev = fa_map_find_fst(states, t->to);
            list_append(mrev->trans, rev);
        }
    }
    list_for_each(m, states) {
        list_free(m->fst->transitions);
        m->fst->transitions = m->trans;
    }

    /* Make new initial and final states */
    struct fa_state *s = make_state(0);
    fa->initial->accept = 1;
    set_initial(fa, s);
    list_for_each(a, accept) {
        add_epsilon_trans(s, a->fst);
    }

    fa->deterministic = 0;
    list_free(states);

    return accept;
}

/*
 * Return a sorted array of all interval start points in FA. The returned
 * array is a string (null terminated)
 */
static char* start_points(struct fa *fa, int *npoints) {
    char pointset[CHAR_NUM];
    struct fa_map *states = fa_states(fa);

    memset(pointset, 0, CHAR_NUM * sizeof(char));
    list_for_each(m, states) {
        pointset[CHAR_IND(CHAR_MIN)] = 1;
        list_for_each(t, m->fst->transitions) {
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

    list_free(states);
    return points;
}

/*
 * Operations on FA_SET_MAP
 */
static struct fa_set_map *fa_set_map_get(struct fa_set_map *smap,
                                         struct fa_map *set) {
    int setlen;
    list_length(setlen, set);

    list_for_each(sm, smap) {
        int smlen;
        list_length(smlen, sm->set);
        if (smlen == setlen) {
            int found = 1;
            list_for_each(s, set) {
                if (fa_map_find_fst(sm->set, s->fst) == NULL) {
                    found = 0;
                    break;
                }
            }
            if (found)
                return sm;
        }
    }
    return NULL;
}

static int fa_set_map_contains(struct fa_set_map *smap,
                               struct fa_map *set) {
    return fa_set_map_get(smap, set) != NULL;
}

/* 
 * Find the set in SMAP that has the same states as SET. If the two are
 * different, i.e. they point to different memory locations, free SET and
 * return the set found in SMAP
 */
static struct fa_map *fa_set_map_uniq(struct fa_set_map *smap,
                                      struct fa_map *set) {
    struct fa_set_map *sm = fa_set_map_get(smap, set);
    if (sm->set != set) {
        list_free(set);
    }
    return sm->set;
}

static struct fa_state *fa_set_map_get_state(struct fa_set_map *smap,
                                             struct fa_map *set) {
    struct fa_set_map *sm = fa_set_map_get(smap, set);
    return sm->state;
}

static struct fa_set_map *fa_set_map_add(struct fa_set_map *smap,
                                         struct fa_map *set,
                                         struct fa *fa) {
    struct fa_set_map *sm;

    CALLOC(sm, 1);
    sm->set = set;
    sm->state = make_state(0);
    sm->next = smap;
    list_insert_before(sm->state, fa->initial->next, fa->initial);
    return sm;
}

static struct fa_set_map *fa_set_enqueue(struct fa_set_map *smap,
                                         struct fa_map *set) {
    struct fa_set_map *sm;
    CALLOC(sm, 1);
    sm->set = set;
    list_append(smap, sm);
    return smap;
}

static struct fa_map *fa_set_dequeue(struct fa_set_map **smap) {
    struct fa_set_map *sm = *smap;
    struct fa_map *set = sm->set;

    *smap = sm->next;
    free(sm);
    return set;
}

static struct fa_map *fa_set_add(struct fa_map *set, struct fa_state *s) {
    struct fa_map *elt;

    list_for_each(q, set) {
        if (q->fst == s)
            return set;
    }

    CALLOC(elt, 1);
    elt->fst = s;
    list_append(set, elt);
    return set;
}

/* Remove all unreachable states on FA->INITIAL and free their storage */
static void remove_unreachable_states(struct fa *fa) {
    struct fa_map *reach = fa_states(fa);
    struct fa_state *s = fa->initial;

    while (s->next != NULL) {
        if (fa_map_find_fst(reach, s->next) == NULL) {
            struct fa_state *del = s->next;
            s->next = del->next;
            /* Free the state del */
            list_free(del->transitions);
            free(del);
        } else {
            s = s->next;
        }
    }
    list_free(reach);
}

/* Compare transitions lexicographically by (to, min, reverse max) */
static int transcmp(const void *v1, const void *v2) {
    const struct fa_trans *t1 = * (struct fa_trans **) v1;
    const struct fa_trans *t2 = * (struct fa_trans **) v2;

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

/* 
 * Reduce an automaton by combining overlapping and adjacent edge intervals
 * with the same destination. Unreachable states are also removed from the
 * list of states.
 */
static void reduce(struct fa *fa) {
    struct fa_trans **trans = NULL;

    list_for_each(s, fa->initial) {
        int ntrans, i;
        struct fa_trans *t;

        if (s->transitions == NULL)
            continue;

        list_length(ntrans, s->transitions);
        REALLOC(trans, ntrans);

        for (i=0, t = s->transitions; i < ntrans; i++, t = t->next)
            trans[i] = t;

        qsort(trans, ntrans, sizeof(*trans), transcmp);
        t = trans[0];
        t->next = NULL;
        s->transitions = t;
        for (i=1; i < ntrans; i++) {
            if (t->to == trans[i]->to && (trans[i]->min <= t->max + 1)) {
                /* Same target and overlapping/adjacent intervals */
                if (trans[i]->max > t->max)
                    t->max = trans[i]->max;
                free(trans[i]);
                trans[i] = NULL;
            } else {
                t->next = trans[i];
                t = trans[i];
                t->next = NULL;
            }
        }
    }

    free(trans);
}

/*
 * Remove dead transitions from an FA; a transition is dead is it does not
 * lead to a live state. This also removes any states that are not
 * reachable any longer from FA->INITIAL.
 */
static void remove_dead_transitions(struct fa *fa) {
    struct fa_map *live = fa_live_states(fa);

    if (fa_map_find_fst(live, fa->initial) == NULL) {
        /* This automaton accepts nothing, make it the canonical
         * epsilon automaton
         */
        list_for_each(s, fa->initial) {
            list_free(s->transitions);
        }
        list_free(fa->initial->next);
        fa->deterministic = 1;
        return;
    }

    list_for_each(s, fa->initial) {
        if (fa_map_find_fst(live, s) == NULL) {
            list_free(s->transitions);
        } else {
            struct fa_trans *t = s->transitions;
            while (t != NULL) {
                struct fa_trans *n = t->next;
                if (fa_map_find_fst(live, t->to) == NULL) {
                    list_remove(t, s->transitions);
                    free(t);
                }
                t = n;
            }
        }
    }

    list_free(live);

    remove_unreachable_states(fa);
    reduce(fa);
}

static void swap_initial(struct fa *fa) {
    struct fa_state *s = fa->initial;
    if (s->next != NULL) {
        fa->initial = s->next;
        s->next = fa->initial->next;
        fa->initial->next = s;
    }
}

/*
 * Make a finite automaton deterministic using the given set of initial
 * states, using the subset construction
 */
static void determinize(struct fa *fa, struct fa_map *ini) {
    int npoints;
    const char *points = start_points(fa, &npoints);
    struct fa_set_map *newstate;
    struct fa_set_map *worklist;

    /* Data structures are a big headache here since we deal with sets of
     * states. We represent a set of states as a list of FA_MAP, where only
     * the FST entry is used for the state in the list.
     *
     * WORKLIST is a queue of sets of states; only the SET entries
     *          in the FA_SET_MAP is used
     * NEWSTATE is a map from sets of states to states. The VALUE entry
     *          only ever contains one state (in FST)
     *
     * To reduce confusion some, the accessor functions for the various
     * data structures are named to make that a little clearer.
     */
    worklist = fa_set_enqueue(NULL, ini);
    newstate = fa_set_map_add(NULL, ini, fa);
    // Make the new state the initial state
    swap_initial(fa);
    while (worklist != NULL) {
        struct fa_map *sset = fa_set_dequeue(&worklist);
        struct fa_state *r = fa_set_map_get_state(newstate, sset);
        list_for_each(q, sset) {
            if (q->fst->accept)
                r->accept = 1;
        }
        for (int n=0; n < npoints; n++) {
            struct fa_map *pset = NULL;
            list_for_each(q, sset) {
                list_for_each(t, q->fst->transitions) {
                    if (t->min <= points[n] && points[n] <= t->max) {
                        pset = fa_set_add(pset, t->to);
                    }
                }
            }
            if (!fa_set_map_contains(newstate, pset)) {
                worklist = fa_set_enqueue(worklist, pset);
                newstate = fa_set_map_add(newstate, pset, fa);
            }
            pset = fa_set_map_uniq(newstate, pset);

            struct fa_state *q = fa_set_map_get_state(newstate, pset);
            char min = points[n];
            char max = CHAR_MAX;
            if (n+1 < strlen(points))
                max = points[n+1] - 1;
            add_new_trans(r, q, min, max);
        }
    }
    fa->deterministic = 1;

    list_for_each(ns, newstate) {
        if (ns->set != ini)
            list_free(ns->set);
    }
    list_free(newstate);
    free((void *) points);
    remove_dead_transitions(fa);
}

/*
 * Minimization
 */
void fa_minimize(struct fa *fa) {
    struct fa_map *map;

    /* Minimize using Brzozowski's algorithm */
    map = fa_reverse(fa);
    determinize(fa, map);
    list_free(map);

    map = fa_reverse(fa);
    determinize(fa, map);
    list_free(map);
}

/*
 * Construction of fa
 */

static struct fa *fa_make_empty(void) {
    struct fa_state *s = make_state(0);
    struct fa *fa = make_fa(s);
    fa->deterministic = 1;
    return fa;
}

static struct fa *fa_make_epsilon(void) {
    struct fa *fa = fa_make_empty();
    fa->initial->accept = 1;
    return fa;
}

struct fa *fa_union(struct fa *fa1, struct fa *fa2) {
    struct fa_state *s = make_state(0);

    fa1 = fa_clone(fa1);
    fa2 = fa_clone(fa2);

    add_epsilon_trans(s, fa1->initial);
    add_epsilon_trans(s, fa2->initial);

    fa1->deterministic = 0;

    fa_merge(fa1, fa2);

    set_initial(fa1, s);

    fa_minimize(fa1);
    return fa1;
}

struct fa *fa_concat(struct fa *fa1, struct fa *fa2) {
    fa1 = fa_clone(fa1);
    fa2 = fa_clone(fa2);

    list_for_each(s, fa1->initial) {
        if (s->accept) {
            s->accept = 0;
            add_epsilon_trans(s, fa2->initial);
        }
    }

    fa1->deterministic = 0;
    fa_merge(fa1, fa2);

    fa_minimize(fa1);
    return fa1;
}

static struct fa *fa_char_range(char min, char max, int negate) {
    struct fa_state *s = make_state(0);
    struct fa_state *t = make_state(1);
    struct fa *fa = make_fa(s);

    list_append(fa->initial, t);
    if (negate) {
        if (min > CHAR_MIN) {
            add_new_trans(s, t, CHAR_MIN, min-1);
        }
        if (max < CHAR_MAX) {
            add_new_trans(s, t, max+1, CHAR_MAX);
        }
    } else {
        add_new_trans(s, t, min, max);
    }

    fa->deterministic = 1;
    return fa;
}

static struct fa *fa_star(struct fa *fa) {
    struct fa_state *s = make_state(1);

    fa = fa_clone(fa);
    add_epsilon_trans(s, fa->initial);
    list_for_each(p, fa->initial) {
        if (p->accept)
            add_epsilon_trans(p, s);
    }
    set_initial(fa, s);
    fa->deterministic = 0;

    fa_minimize(fa);
    return fa;
}

struct fa *fa_iter(struct fa *fa, int min, int max) {
    if (min < 0)
        min = 0;

    if (min > max && max != -1) {
        return fa_make_empty();
    }
    if (max == -1) {
        struct fa *sfa = fa_star(fa);
        struct fa *cfa = sfa;
        while (min > 0) {
            cfa = fa_concat(fa, cfa);
            min -= 1;
        }
        if (sfa != cfa)
            fa_free(sfa);
        return cfa;
    } else {
        struct fa *cfa = NULL;

        max -= min;
        if (min == 0) {
            cfa = fa_make_epsilon();
        } else if (min == 1) {
            cfa = fa_clone(fa);
        } else {
            cfa = fa_clone(fa);
            while (min > 1) {
                struct fa *cfa2 = cfa;
                cfa = fa_concat(cfa2, fa);
                fa_free(cfa2);
                min -= 1;
            }
        }
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

            fa_minimize(cfa);
        }
        return cfa;
    }
}

/*
 * Construct an fa from a regular expression
 */
static struct fa *fa_from_re(struct re *re) {
    struct fa *result = NULL;

    switch(re->type) {
    case UNION: 
        {
            struct fa *fa1 = fa_from_re(re->exp1);
            struct fa *fa2 = fa_from_re(re->exp2);
            result = fa_union(fa1, fa2);
            fa_free(fa1);
            fa_free(fa2);
        }
        break;
    case CONCAT:
        {
            struct fa *fa1 = fa_from_re(re->exp1);
            struct fa *fa2 = fa_from_re(re->exp2);
            result = fa_concat(fa1, fa2);
            fa_free(fa1);
            fa_free(fa2);
        }
        break;
    case CSET:
    case NSET:
        result = fa_char_range(re->from, re->to, re->type == NSET);
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
    }
    free(re);
}

int fa_compile(const char *regexp, struct fa **fa) {
    int ret = REG_NOERROR;
    struct re *re = parse_regexp(&regexp, &ret);
    *fa = NULL;
    if (re == NULL)
        return ret;

    print_re(re);
    printf("\n");

    *fa = fa_from_re(re);
    re_free(re);

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

static struct re*make_re_char(int negate, char from, char to) {
    struct re *re = make_re(negate ? NSET : CSET);
    re->from = from;
    re->to = to;
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

static struct re *parse_char_class(const char **regexp, int negate,
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
            struct re *re1 = make_re_char(negate, from, from);
            struct re *re2 = make_re_char(negate, '-', '-');
            return make_re_binop(UNION, re1, re2);
        } else {
            to = parse_char(regexp, NULL);
        }
    }
    return make_re_char(negate, from, to);
 error:
    return NULL;
}

static struct re *parse_simple_exp(const char **regexp, int *error) {
    struct re *re = NULL;

    if (match(regexp, '[')) {
        int negate = match(regexp, '^');
        re = parse_char_class(regexp, negate, error);
        while (more(regexp) && ! peek(regexp, "]")) {
            struct re *re2 = parse_char_class(regexp, negate, error);
            re = make_re_binop(UNION, re, re2);
        }
        if (re == NULL)
            goto error;
        if (! match(regexp, ']')) {
            *error = REG_EBRACK;
            goto error;
        }
        return re;
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
        return re;
    } else if (match(regexp, '.')) {
        return make_re_char(0, '\n', '\n');
    } else {
        if (more(regexp)) {
            char c = parse_char(regexp, special_chars);
            re = make_re_char(0, c, c);
        }
        return re;
    }
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
        list_for_each(t, s->transitions) {
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
