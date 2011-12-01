/*
 * regexp.c:
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
#include <regex.h>

#include "internal.h"
#include "syntax.h"
#include "memory.h"
#include "errcode.h"

static const struct string empty_pattern_string = {
    .ref = REF_MAX, .str = (char *) "()"
};

static const struct string *const empty_pattern = &empty_pattern_string;

char *regexp_escape(const struct regexp *r) {
    char *pat = NULL;

    if (r == NULL)
        return strdup("");

#if !HAVE_USELOCALE
    char *nre = NULL;
    int ret;
    size_t nre_len;

    /* Use a range with from > to to force conversion of ranges into
     * short form */
    ret = fa_restrict_alphabet(r->pattern->str, strlen(r->pattern->str),
                               &nre, &nre_len, 2, 1);
    if (ret == 0) {
        pat = escape(nre, nre_len);
        free(nre);
    }
#endif

    if (pat == NULL)
        pat = escape(r->pattern->str, -1, RX_ESCAPES);

    if (pat == NULL)
        return NULL;

    /* Remove unneeded '()' from pat */
    for (int changed = 1; changed;) {
        changed = 0;
        for (char *p = pat; *p != '\0'; p++) {
            if (*p == '(' && p[1] == ')') {
                memmove(p, p+2, strlen(p+2)+1);
                changed = 1;
            }
        }
    }

    if (pat[0] == '(' && pat[strlen(pat)-1] == ')') {
        int level = 1;
        for (int i=1; i < strlen(pat)-1; i++) {
            if (pat[i] == '(')
                level += 1;
            if (pat[i] == ')')
                level -= 1;
            if (level == 0)
                break;
        }
        if (level == 1) {
            memmove(pat, pat+1, strlen(pat+1)+1);
            pat[strlen(pat)-1] = '\0';
        }
    }

    return pat;
}

void print_regexp(FILE *out, struct regexp *r) {
    if (r == NULL) {
        fprintf(out, "<NULL>");
        return;
    }

    fputc('/', out);
    if (r->pattern == NULL)
        fprintf(out, "%p", r);
    else {
        char *rx;
        size_t rx_len;
        fa_restrict_alphabet(r->pattern->str, strlen(r->pattern->str),
                             &rx, &rx_len, 2, 1);
        print_chars(out, rx, rx_len);
        FREE(rx);
    }
    fputc('/', out);
    if (r->nocase)
        fputc('i', out);
}

struct regexp *
make_regexp_unescape(struct info *info, const char *pat, int nocase) {
    char *p = unescape(pat, strlen(pat), NULL);

    if (p == NULL)
        return NULL;
    return make_regexp(info, p, nocase);
}

struct regexp *make_regexp(struct info *info, char *pat, int nocase) {
    struct regexp *regexp;

    make_ref(regexp);
    regexp->info = ref(info);

    make_ref(regexp->pattern);
    regexp->pattern->str = pat;
    regexp->nocase = nocase;
    return regexp;
}

/* Take a POSIX glob and turn it into a regexp. The regexp is constructed
 * by doing the following translations of characters in the string:
 *  * -> [^/]*
 *  ? -> [^/]
 *  leave characters escaped with a backslash alone
 *  escape any of ".|{}()+^$" with a backslash
 *
 * Note that that ignores some of the finer points of globs, like
 * complementation.
 */
struct regexp *make_regexp_from_glob(struct info *info, const char *glob) {
    static const char *const star = "[^/]*";
    static const char *const qmark = "[^/]";
    static const char *const special = ".|{}()+^$";
    int newlen = strlen(glob);
    char *pat = NULL;

    for (const char *s = glob; *s; s++) {
        if (*s == '\\' && *(s+1))
            s += 1;
        else if (*s == '*')
            newlen += strlen(star)-1;
        else if (*s == '?')
            newlen += strlen(qmark)-1;
        else if (strchr(special, *s) != NULL)
            newlen += 1;
    }

    if (ALLOC_N(pat, newlen + 1) < 0)
        return NULL;

    char *t = pat;
    for (const char *s = glob; *s; s++) {
        if (*s == '\\' && *(s+1)) {
            *t++ = *s++;
            *t++ = *s;
        } else if (*s == '*') {
            t = stpcpy(t, star);
        } else if (*s == '?') {
            t = stpcpy(t, qmark);
        } else if (strchr(special, *s) != NULL) {
            *t++ = '\\';
            *t++ = *s;
        } else {
            *t++ = *s;
        }
    }

    return make_regexp(info, pat, 0);
}

void free_regexp(struct regexp *regexp) {
    if (regexp == NULL)
        return;
    assert(regexp->ref == 0);
    unref(regexp->info, info);
    unref(regexp->pattern, string);
    if (regexp->re != NULL) {
        regfree(regexp->re);
        free(regexp->re);
    }
    free(regexp);
}

int regexp_is_empty_pattern(struct regexp *r) {
    for (char *s = r->pattern->str; *s; s++) {
        if (*s != '(' && *s != ')')
            return 0;
    }
    return 1;
}

struct regexp *make_regexp_literal(struct info *info, const char *text) {
    char *pattern, *p;

    /* Escape special characters in text since it should be taken
       literally */
    CALLOC(pattern, 2*strlen(text)+1);
    p = pattern;
    for (const char *t = text; *t != '\0'; t++) {
        if ((*t == '\\') && t[1]) {
            *p++ = *t++;
            *p++ = *t;
        } else if (strchr(".|{}[]()+*?", *t) != NULL) {
            *p++ = '\\';
            *p++ = *t;
        } else {
            *p++ = *t;
        }
    }
    return make_regexp(info, pattern, 0);
}

struct regexp *
regexp_union(struct info *info, struct regexp *r1, struct regexp *r2) {
    struct regexp *r[2];

    r[0] = r1;
    r[1] = r2;
    return regexp_union_n(info, 2, r);
}

char *regexp_expand_nocase(struct regexp *r) {
    const char *p = r->pattern->str;
    char *s = NULL;
    size_t len;
    int ret;

    if (! r->nocase)
        return strdup(p);

    ret = fa_expand_nocase(p, strlen(p), &s, &len);
    ERR_NOMEM(ret == REG_ESPACE, r->info);
    BUG_ON(ret != REG_NOERROR, r->info, NULL);
 error:
    return s;
}

struct regexp *
regexp_union_n(struct info *info, int n, struct regexp **r) {
    size_t len = 0;
    char *pat = NULL, *p, *expanded = NULL;
    int nnocase = 0, npresent = 0;
    int ret;

    for (int i=0; i < n; i++)
        if (r[i] != NULL) {
            len += strlen(r[i]->pattern->str) + strlen("()|");
            npresent += 1;
            if (r[i]->nocase)
                nnocase += 1;
        }

    bool mixedcase = nnocase > 0 && nnocase < npresent;

    if (len == 0)
        return NULL;

    if (ALLOC_N(pat, len) < 0)
        return NULL;

    p = pat;
    int added = 0;
    for (int i=0; i < n; i++) {
        if (r[i] == NULL)
            continue;
        if (added > 0)
            *p++ = '|';
        *p++ = '(';
        if (mixedcase && r[i]->nocase) {
            expanded = regexp_expand_nocase(r[i]);
            ERR_BAIL(r[i]->info);
            len += strlen(expanded) - strlen(r[i]->pattern->str);
            ret = REALLOC_N(pat, len);
            ERR_NOMEM(ret < 0, info);
            p = pat + strlen(pat);
            p = stpcpy(p, expanded);
            FREE(expanded);
        } else {
            p = stpcpy(p, r[i]->pattern->str);
        }
        *p++ = ')';
        added += 1;
    }
    *p = '\0';
    return make_regexp(info, pat, nnocase == npresent);
 error:
    FREE(expanded);
    FREE(pat);
    return NULL;
}

struct regexp *
regexp_concat(struct info *info, struct regexp *r1, struct regexp *r2) {
    struct regexp *r[2];

    r[0] = r1;
    r[1] = r2;
    return regexp_concat_n(info, 2, r);
}

struct regexp *
regexp_concat_n(struct info *info, int n, struct regexp **r) {
    size_t len = 0;
    char *pat = NULL, *p, *expanded = NULL;
    int nnocase = 0, npresent = 0;
    int ret;

    for (int i=0; i < n; i++)
        if (r[i] != NULL) {
            len += strlen(r[i]->pattern->str) + strlen("()");
            npresent += 1;
            if (r[i]->nocase)
                nnocase += 1;
        }

    bool mixedcase = nnocase > 0 && nnocase < npresent;

    if (len == 0)
        return NULL;

    len += 1;
    if (ALLOC_N(pat, len) < 0)
        return NULL;

    p = pat;
    for (int i=0; i < n; i++) {
        if (r[i] == NULL)
            continue;
        *p++ = '(';
        if (mixedcase && r[i]->nocase) {
            expanded = regexp_expand_nocase(r[i]);
            ERR_BAIL(r[i]->info);
            len += strlen(expanded) - strlen(r[i]->pattern->str);
            ret = REALLOC_N(pat, len);
            ERR_NOMEM(ret < 0, info);
            p = pat + strlen(pat);
            p = stpcpy(p, expanded);
            FREE(expanded);
        } else {
            p = stpcpy(p, r[i]->pattern->str);
        }
        *p++ = ')';
    }
    *p = '\0';
    return make_regexp(info, pat, nnocase == npresent);
 error:
    FREE(expanded);
    FREE(pat);
    return NULL;
}

static struct fa *regexp_to_fa(struct regexp *r) {
    const char *p = r->pattern->str;
    int ret;
    struct fa *fa = NULL;

    ret = fa_compile(p, strlen(p), &fa);
    ERR_NOMEM(ret == REG_ESPACE, r->info);
    BUG_ON(ret != REG_NOERROR, r->info, NULL);

    if (r->nocase) {
        ret = fa_nocase(fa);
        ERR_NOMEM(ret < 0, r->info);
    }
    return fa;

 error:
    fa_free(fa);
    return NULL;
}

struct regexp *
regexp_minus(struct info *info, struct regexp *r1, struct regexp *r2) {
    struct regexp *result = NULL;
    struct fa *fa = NULL, *fa1 = NULL, *fa2 = NULL;
    int r;
    char *s = NULL;
    size_t s_len;

    fa1 = regexp_to_fa(r1);
    ERR_BAIL(r1->info);

    fa2 = regexp_to_fa(r2);
    ERR_BAIL(r2->info);

    fa = fa_minus(fa1, fa2);
    if (fa == NULL)
        goto error;

    r = fa_as_regexp(fa, &s, &s_len);
    if (r < 0)
        goto error;

    if (s == NULL) {
        /* FA is the empty set, which we can't represent as a regexp */
        goto error;
    }

    if (regexp_c_locale(&s, NULL) < 0)
        goto error;

    result = make_regexp(info, s, fa_is_nocase(fa));
    s = NULL;

 done:
    fa_free(fa);
    fa_free(fa1);
    fa_free(fa2);
    free(s);
    return result;
 error:
    unref(result, regexp);
    goto done;
}


struct regexp *
regexp_iter(struct info *info, struct regexp *r, int min, int max) {
    const char *p;
    char *s;
    int ret = 0;

    if (r == NULL)
        return NULL;

    p = r->pattern->str;
    if ((min == 0 || min == 1) && max == -1) {
        char q = (min == 0) ? '*' : '+';
        ret = asprintf(&s, "(%s)%c", p, q);
    } else if (min == max) {
        ret = asprintf(&s, "(%s){%d}", p, min);
    } else {
        ret = asprintf(&s, "(%s){%d,%d}", p, min, max);
    }
    return (ret == -1) ? NULL : make_regexp(info, s, r->nocase);
}

struct regexp *
regexp_maybe(struct info *info, struct regexp *r) {
    const char *p;
    char *s;
    int ret;

    if (r == NULL)
        return NULL;
    p = r->pattern->str;
    ret = asprintf(&s, "(%s)?", p);
    return (ret == -1) ? NULL : make_regexp(info, s, r->nocase);
}

struct regexp *regexp_make_empty(struct info *info) {
    struct regexp *regexp;

    make_ref(regexp);
    if (regexp != NULL) {
        regexp->info = ref(info);
        /* Casting away the CONST for EMPTY_PATTERN is ok since it
           is protected against changes because REF == REF_MAX */
        regexp->pattern = (struct string *) empty_pattern;
        regexp->nocase = 0;
    }
    return regexp;
}

static int regexp_compile_internal(struct regexp *r, const char **c) {
    /* See the GNU regex manual or regex.h in gnulib for
     * an explanation of these flags. They are set so that the regex
     * matcher interprets regular expressions the same way that libfa
     * does
     */
    static const reg_syntax_t syntax =
        RE_CONTEXT_INDEP_OPS|RE_CONTEXT_INVALID_OPS|RE_DOT_NOT_NULL
        |RE_INTERVALS|RE_NO_BK_BRACES|RE_NO_BK_PARENS|RE_NO_BK_REFS
        |RE_NO_BK_VBAR|RE_NO_EMPTY_RANGES
        |RE_NO_POSIX_BACKTRACKING|RE_CONTEXT_INVALID_DUP|RE_NO_GNU_OPS;
    reg_syntax_t old_syntax = re_syntax_options;

    *c = NULL;

    if (r->re == NULL)
        CALLOC(r->re, 1);

    re_syntax_options = syntax;
    if (r->nocase)
        re_syntax_options |= RE_ICASE;
    *c = re_compile_pattern(r->pattern->str, strlen(r->pattern->str), r->re);
    re_syntax_options = old_syntax;

    r->re->regs_allocated = REGS_REALLOCATE;
    if (*c != NULL)
        return -1;
    return 0;
}

int regexp_compile(struct regexp *r) {
    const char *c;

    return regexp_compile_internal(r, &c);
}

int regexp_check(struct regexp *r, const char **msg) {
    return regexp_compile_internal(r, msg);
}

int regexp_match(struct regexp *r,
                 const char *string, const int size,
                 const int start, struct re_registers *regs) {
    if (r->re == NULL) {
        if (regexp_compile(r) == -1)
            return -3;
    }
    return re_match(r->re, string, size, start, regs);
}

int regexp_matches_empty(struct regexp *r) {
    return regexp_match(r, "", 0, 0, NULL) == 0;
}

int regexp_nsub(struct regexp *r) {
    if (r->re == NULL)
        if (regexp_compile(r) == -1)
            return -1;
    return r->re->re_nsub;
}

void regexp_release(struct regexp *regexp) {
    if (regexp != NULL && regexp->re != NULL) {
        regfree(regexp->re);
        FREE(regexp->re);
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
