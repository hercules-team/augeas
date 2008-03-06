/*
 * fa.h: finite automata
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

#ifndef __FA_H
#define __FA_H

#include <stdio.h>
#include <regex.h>

/* The type for a finite automaton. */
typedef struct fa *fa_t;

/* Denote some basic automata, used by fa_is_basic and fa_make_basic */
enum fa_basic {
    FA_EMPTY,        /* Accepts the empty language, i.e. no strings */
    FA_EPSILON,      /* Accepts only the empty word */
    FA_TOTAL         /* Accepts all words */
};

/* Unless otherwise mentioned, automata passed into routines are never
 * modified. It is the responsibility of the caller to free automata
 * returned by any of these routines when they are no longer needed.
 */

/*
 * Compile the regular expression RE into an automaton. The return value is
 * the same as the return value for the POSIX function regcomp. The syntax
 * for regular expressions is extended POSIX syntax, with the difference
 * that '.' does not match newlines.
 *
 * On success, FA points to the newly allocated automaton constructed for
 * RE, and the function returns REG_NOERROR. Otherwise, FA is NULL, and the
 * return value indicates the error.
 */
int fa_compile(const char *re, fa_t *fa);

/* Make a new automaton that accepts one of the basic languages defined in
 * the enum FA_BASIC.
 */
fa_t fa_make_basic(unsigned int basic);

/* Return 1 if FA accepts the basic language BASIC, which must be one of
 * the constantsfrom enum FA_BASIC.
 */
int fa_is_basic(fa_t fa, unsigned int basic);

/* Minimize FA using Brzozowski's algorithm. As a side-effect, the
 * automaton will also be deterministic after being minimized. Modifies the
 * automaton in place.
 */
void fa_minimize(fa_t fa);

/* Return a finite automaton that accepts the concatenation of the
 * languages for FA1 and FA2, i.e. L(FA1).L(FA2)
 */
fa_t fa_concat(fa_t fa1, fa_t fa2);

/* Return a finite automaton that accepts the union of the languages that
 * FA1 and FA2 accept (the '|' operator in regular expressions).
 */
fa_t fa_union(fa_t fa1, fa_t fa2);

/* Return a finite automaton that accepts the intersection of the languages
 * of FA1 and FA2.
 */
fa_t fa_intersect(fa_t fa1, fa_t fa2);

/* Return a finite automaton that accepts the complement of the language of
 * FA, i.e. the set of all words not accepted by FA
 */
fa_t fa_complement(fa_t fa);

/* Return a finite automaton that accepts the set difference of the
 * languages of FA1 and FA2, i.e. L(FA1)\L(FA2)
 */
fa_t fa_minus(fa_t fa1, fa_t fa2);

/* Return a finite automaton that accepts a repetition of the language that
 * FA accepts. If MAX == -1, the returned automaton accepts arbitrarily
 * long repetitions. MIN must be 0 or bigger, and unless MAX == -1, MIN
 * must be less or equal to MAX. If MIN is greater than 0, the returned
 * automaton accepts only words that have at least MIN repetitions of words
 * from L(FA).
 *
 * The following common regexp repetitios are achieved by the following
 * calls (using a lose notation equating automata and their languages):
 *
 * - FA* = FA_ITER(FA, 0, -1)
 * - FA+ = FA_ITER(FA, 1, -1)
 * - FA? = FA_ITER(FA, 0, 1)
 * - FA{n,m} = FA_ITER(FA, n, m) with 0 <= n and m = -1 or n <= m
 */
fa_t fa_iter(fa_t fa, int min, int max);

/* Return 1 if the language of FA1 is contained in the language of FA2, 0
 * otherwise.
 */
int fa_contains(fa_t fa1, fa_t fa2);

/* Return 1 if the language of FA1 equals the language of FA2 */
int fa_equals(fa_t fa1, fa_t fa2);

/* Free all memory used by FA */
void fa_free(fa_t fa);

/* Print FA to OUT as a graphviz dot file */
void fa_dot(FILE *out, fa_t fa);

/* Return a finite automaton that accepts the overlap of the languages of
 * FA1 and FA2. The overlap of two languages is the set of strings that can
 * be split in more than one way into a left part accepted by FA1 and a
 * right part accepted by FA2.
 */
fa_t fa_overlap(fa_t fa1, fa_t fa2);

/* Produce an example for the language of FA. The example is not
 * necessarily the shortest possible. The implementation works very hard to
 * have printable characters (preferrably alphanumeric) in the example, and
 * to avoid just an empty word.
 */
char *fa_example(fa_t fa);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
