/*
 * fa.h: finite automata
 *
 * Copyright (C) 2007-2016 David Lutterkort
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

#ifndef FA_H_
#define FA_H_

#include <stdio.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>

/* The type for a finite automaton. */
struct fa;

/* The type of a state of a finite automaton. The fa_state functions return
 * pointers to this struct. Those pointers are only valid as long as the
 * only fa_* functions that are called are fa_state_* functions. For
 * example, the following code will almost certainly result in a crash (or
 * worse):
 *
 *     struct state *s = fa_state_initial(fa);
 *     fa_minimize(fa);
 *     // Crashes as S will likely have been freed
 *     s = fa_state_next(s)
 */
struct state;

/* Denote some basic automata, used by fa_is_basic and fa_make_basic */
enum fa_basic {
    FA_EMPTY,        /* Accepts the empty language, i.e. no strings */
    FA_EPSILON,      /* Accepts only the empty word */
    FA_TOTAL         /* Accepts all words */
};

/* Choice of minimization algorithm to use; either Hopcroft's O(n log(n))
 * algorithm or Brzozowski's reverse-determinize-reverse-determinize
 * algorithm. While the latter has exponential complexity in theory, it
 * works quite well for some cases.
 */
enum fa_minimization_algorithms {
    FA_MIN_HOPCROFT,
    FA_MIN_BRZOZOWSKI
};

/* Which minimization algorithm to use in FA_MINIMIZE. The library
 * minimizes internally at certain points, too.
 *
 * Defaults to FA_MIN_HOPCROFT
 */
extern int fa_minimization_algorithm;

/* Unless otherwise mentioned, automata passed into routines are never
 * modified. It is the responsibility of the caller to free automata
 * returned by any of these routines when they are no longer needed.
 */

/*
 * Compile the regular expression RE of length SIZE into an automaton. The
 * return value is the same as the return value for the POSIX function
 * regcomp. The syntax for regular expressions is extended POSIX syntax,
 * with the difference that '.' does not match newlines.
 *
 * On success, FA points to the newly allocated automaton constructed for
 * RE, and the function returns REG_NOERROR. Otherwise, FA is NULL, and the
 * return value indicates the error.
 *
 * The FA is case sensitive. Call FA_NOCASE to switch it to
 * case-insensitive.
 */
int fa_compile(const char *re, size_t size, struct fa **fa);

/* Make a new automaton that accepts one of the basic languages defined in
 * the enum FA_BASIC.
 */
struct fa *fa_make_basic(unsigned int basic);

/* Return 1 if FA accepts the basic language BASIC, which must be one of
 * the constants from enum FA_BASIC.
 */
int fa_is_basic(struct fa *fa, unsigned int basic);

/* Minimize FA using the currently-set fa_minimization_algorithm.
 * As a side effect, the automaton will also be deterministic after being
 * minimized. Modifies the automaton in place.
 */
int fa_minimize(struct fa *fa);

/* Return a finite automaton that accepts the concatenation of the
 * languages for FA1 and FA2, i.e. L(FA1).L(FA2)
 */
struct fa *fa_concat(struct fa *fa1, struct fa *fa2);

/* Return a finite automaton that accepts the union of the languages that
 * FA1 and FA2 accept (the '|' operator in regular expressions).
 */
struct fa *fa_union(struct fa *fa1, struct fa *fa2);

/* Return a finite automaton that accepts the intersection of the languages
 * of FA1 and FA2.
 */
struct fa *fa_intersect(struct fa *fa1, struct fa *fa2);

/* Return a finite automaton that accepts the complement of the language of
 * FA, i.e. the set of all words not accepted by FA
 */
struct fa *fa_complement(struct fa *fa);

/* Return a finite automaton that accepts the set difference of the
 * languages of FA1 and FA2, i.e. L(FA1)\L(FA2)
 */
struct fa *fa_minus(struct fa *fa1, struct fa *fa2);

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
struct fa *fa_iter(struct fa *fa, int min, int max);

/* If successful, returns 1 if the language of FA1 is contained in the language
 * of FA2, 0 otherwise. Returns a negative number if an error occurred.
 */
int fa_contains(struct fa *fa1, struct fa *fa2);

/* If successful, returns 1 if the language of FA1 equals the language of FA2,
 * 0 otherwise. Returns a negative number if an error occurred.
 */
int fa_equals(struct fa *fa1, struct fa *fa2);

/* Free all memory used by FA */
void fa_free(struct fa *fa);

/* Print FA to OUT as a graphviz dot file */
void fa_dot(FILE *out, struct fa *fa);

/* Return a finite automaton that accepts the overlap of the languages of
 * FA1 and FA2. The overlap of two languages is the set of strings that can
 * be split in more than one way into a left part accepted by FA1 and a
 * right part accepted by FA2.
 */
struct fa *fa_overlap(struct fa *fa1, struct fa *fa2);

/* Produce an example for the language of FA. The example is not
 * necessarily the shortest possible. The implementation works very hard to
 * have printable characters (preferably alphanumeric) in the example, and
 * to avoid just an empty word.
 *
 * *EXAMPLE will be the example, which may be NULL. If it is non-NULL,
 *  EXAMPLE_LEN will hold the length of the example.
 *
 * Return 0 on success, and a negative numer on error. On error, *EXAMPLE
 * will be NULL.
 *
 * If *EXAMPLE is set, it is the caller's responsibility to free the string
 * by calling free().
 */
int fa_example(struct fa *fa, char **example, size_t *example_len);

/* Produce an example of an ambiguous word for the concatenation of the
 * languages of FA1 and FA2. The return value is such a word (which must be
 * freed by the caller) if it exists. If none exists, NULL is returned.
 *
 * The returned word is of the form UPV and PV and V are set to the first
 * character of P and V in the returned word. The word UPV has the property
 * that U and UP are accepted by FA1 and that PV and V are accepted by FA2.
 *
 * Neither the language of FA1 or of FA2 may contain words with the
 * characters '\001' and '\002', as they are used during construction of
 * the ambiguous word.
 *
 * UPV_LEN will be set to the length of the entire string UPV
 *
 * Returns 0 on success, and a negative number on failure. On failure, UPV,
 * PV, and V will be NULL
 */
int fa_ambig_example(struct fa *fa1, struct fa *fa2,
                     char **upv, size_t *upv_len,
                     char **pv, char **v);

/* Convert the finite automaton FA into a regular expression and set REGEXP
 * to point to that. When REGEXP is compiled into another automaton, it is
 * guaranteed that that automaton and FA accept the same language.
 *
 * The code tries to be semi-clever about keeping the generated regular
 * expression short; to guarantee reasonably short regexps, the automaton
 * should be minimized before passing it to this routine.
 *
 * On success, REGEXP_LEN is set to the length of REGEXP
 *
 * Return 0 on success, and a negative number on failure. The only reason
 * for FA_AS_REGEXP to fail is running out of memory.
 */
int fa_as_regexp(struct fa *fa, char **regexp, size_t *regexp_len);

/* Given the regular expression REGEXP construct a new regular expression
 * NEWREGEXP that does not match strings containing any of the characters
 * in the range FROM to TO, with the endpoints included.
 *
 * The new regular expression is constructed by removing the range FROM to
 * TO from all character sets in REGEXP; if any of the characters [FROM,
 * TO] appear outside a character set in REGEXP, return -2.
 *
 * Return 0 if NEWREGEXP was constructed successfully, -1 if an internal
 * error happened (e.g., an allocation failed) and -2 if NEWREGEXP can not
 * be constructed because any character in the range [FROM, TO] appears
 * outside of a character set.
 *
 * Return a positive value if REGEXP is not syntactically valid; the value
 * returned is one of the REG_ERRCODE_T POSIX error codes. Return 0 on
 * success and -1 if an allocation fails.
 */
int fa_restrict_alphabet(const char *regexp, size_t regexp_len,
                         char **newregexp, size_t *newregexp_len,
                         char from, char to);

/* Convert REGEXP into one that does not use ranges inside character
 * classes.
 *
 * Return a positive value if REGEXP is not syntactically valid; the value
 * returned is one of the REG_ERRCODE_T POSIX error codes. Return 0 on
 * success and -1 if an allocation fails.
 */
int fa_expand_char_ranges(const char *regexp, size_t regexp_len,
                          char **newregexp, size_t *newregexp_len);

/* Modify FA so that it matches ignoring case.
 *
 * Returns 0 on success, and -1 if an allocation fails. On failure, the
 * automaton is not guaranteed to represent anything sensible.
 */
int fa_nocase(struct fa *fa);

/* Return 1 if FA matches ignoring case, 0 if matches are case sensitive */
int fa_is_nocase(struct fa *fa);

/* Assume REGEXP is a case-insensitive regular expression, and convert it
 * to one that matches the same strings when used case sensitively. All
 * occurrences of individual letters c in the regular expression will be
 * replaced by character sets [cC], and lower/upper case characters are
 * added to character sets as needed.
 *
 * Return a positive value if REGEXP is not syntactically valid; the value
 * returned is one of the REG_ERRCODE_T POSIX error codes. Return 0 on
 * success and -1 if an allocation fails.
 */
int fa_expand_nocase(const char *regexp, size_t regexp_len,
                     char **newregexp, size_t *newregexp_len);

/* Generate up to LIMIT words from the language of FA, which is assumed to
 * be finite. The words are returned in WORDS, which is allocated by this
 * function and must be freed by the caller.
 *
 * If FA accepts the empty word, the empty string will be included in
 * WORDS.
 *
 * Return the number of generated words on success, -1 if we run out of
 * memory, and -2 if FA has more than LIMIT words.
 */
int fa_enumerate(struct fa *fa, int limit, char ***words);

/* Print FA to OUT as a JSON file. State 0 is always the initial one.
 * Returns 0 on success, and -1 on failure.
 */
int fa_json(FILE *out, struct fa *fa);

/* Returns true if the FA is deterministic and 0 otherwise */
bool fa_is_deterministic(struct fa *fa);

/* Return the initial state */
struct state *fa_state_initial(struct fa *fa);

/* Return true if this is an accepting state */
bool fa_state_is_accepting(struct state *st);

/* Return the next state; return NULL if there are no more states */
struct state* fa_state_next(struct state *st);

/* Return the number of transitions for a state */
size_t fa_state_num_trans(struct state *st);

/* Produce details about the i-th transition.
 *
 * On success, *to points to the destination state of the transition and
 * the interval [min-max] is the label of the transition.
 *
 * On failure, *to, min and max are not modified.
 *
 * Return 0 on success and -1 when I is larger than the number of
 * transitions of ST.
 */
int fa_state_trans(struct state *st, size_t i,
                   struct state **to, unsigned char *min, unsigned char *max);

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
