/*
 * regexp.h: wrappers for regexp handling
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

#ifndef REGEXP_H_
#define REGEXP_H_

#include <stdio.h>
#include <regex.h>

struct regexp {
    unsigned int              ref;
    struct info              *info;
    struct string            *pattern;
    struct re_pattern_buffer *re;
    unsigned int              nocase : 1;
};

void print_regexp(FILE *out, struct regexp *regexp);

/* Make a regexp with pattern PAT, which is not copied. Ownership
 * of INFO is taken.
 */
struct regexp *make_regexp(struct info *info, char *pat, int nocase);

/* Make a regexp with pattern PAT, which is copied. Ownership of INFO is
 * taken. Escape sequences like \n in PAT are interpreted.
 */
struct regexp *make_regexp_unescape(struct info *info, const char *pat,
                                    int nocase);

/* Return 1 if R is an empty pattern, i.e. one consisting of nothing but
   '(' and ')' characters, 0 otherwise */
int regexp_is_empty_pattern(struct regexp *r);

/* Make a regexp that matches TEXT literally; the string TEXT
 * is not used by the returned rgexp and must be freed by the caller
 */
struct regexp *make_regexp_literal(struct info *info, const char *text);

/* Make a regexp from a glob pattern */
struct regexp *make_regexp_from_glob(struct info *info, const char *glob);

/* Do not call directly, use UNREF instead */
void free_regexp(struct regexp *regexp);

/* Compile R->PATTERN into R->RE; return -1 and print an error
 * if compilation fails. Return 0 otherwise
 */
int regexp_compile(struct regexp *r);

/* Check the syntax of R->PATTERN; return -1 if the pattern has a syntax
 * error, and a string indicating the error in *C. Return 0 if the pattern
 * is a valid regular expression.
 */
int regexp_check(struct regexp *r, const char **msg);

/* Call RE_MATCH on R->RE and return its result; if R hasn't been compiled
 * yet, compile it. Return -3 if compilation fails
 */
int regexp_match(struct regexp *r, const char *string, const int size,
                 const int start, struct re_registers *regs);

/* Return 1 if R matches the empty string, 0 otherwise */
int regexp_matches_empty(struct regexp *r);

/* Return the number of subexpressions (parentheses) inside R. May cause
 * compilation of R; return -1 if compilation fails.
 */
int regexp_nsub(struct regexp *r);

struct regexp *
regexp_union(struct info *, struct regexp *r1, struct regexp *r2);

struct regexp *
regexp_concat(struct info *, struct regexp *r1, struct regexp *r2);

struct regexp *
regexp_union_n(struct info *, int n, struct regexp **r);

struct regexp *
regexp_concat_n(struct info *, int n, struct regexp **r);

struct regexp *
regexp_iter(struct info *info, struct regexp *r, int min, int max);

/* Return a new REGEXP that matches all the words matched by R1 but
 * not by R2
 */
struct regexp *
regexp_minus(struct info *info, struct regexp *r1, struct regexp *r2);

struct regexp *
regexp_maybe(struct info *info, struct regexp *r);

struct regexp *regexp_make_empty(struct info *);

/* Free up temporary data structures, most importantly compiled
   regular expressions */
void regexp_release(struct regexp *regexp);

/* Produce a printable representation of R */
char *regexp_escape(const struct regexp *r);

/* If R is case-insensitive, expand its pattern so that it matches the same
 * string even when used in a case-sensitive match. */
char *regexp_expand_nocase(struct regexp *r);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
