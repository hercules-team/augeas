/*
 * fatest.c:
 *
 * Copyright (C) 2007-2011 Red Hat Inc.
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

#include "fa.h"
#include "cutest.h"
#include "internal.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define FA_DOT_DIR "FA_DOT_DIR"

struct fa_list {
    struct fa_list *next;
    struct fa *fa;
};

static struct fa_list *fa_list;

static void print_regerror(int err, const char *regexp) {
    size_t size;
    char *errbuf;
    size = regerror(err, NULL, NULL, 0);
    if (ALLOC_N(errbuf, size) < 0)
        die_oom();
    regerror(err, NULL, errbuf, size);
    if (strlen(regexp) > 40) {
        char *s = strndup(regexp, 40);
        fprintf(stderr, "Error building fa from %s...:\n", s);
        free(s);
    } else {
        fprintf(stderr, "Error building fa from %s:\n", regexp);
    }
    fprintf(stderr, "  %s\n", errbuf);
    free(errbuf);
}

static void setup(ATTRIBUTE_UNUSED CuTest *tc) {
    fa_list = NULL;
}

static void teardown(ATTRIBUTE_UNUSED CuTest *tc) {
    list_for_each(fl, fa_list) {
        fa_free(fl->fa);
    }
    list_free(fa_list);
}

static struct fa *mark(struct fa *fa) {
    struct fa_list *fl;

    if (fa != NULL) {
        if (ALLOC(fl) < 0)
            die_oom();
        fl->fa = fa;
        list_cons(fa_list, fl);
    }
    return fa;
}

static void assertAsRegexp(CuTest *tc, struct fa *fa) {
    char *re;
    size_t re_len;
    struct fa *fa1, *fa2;
    struct fa *empty = mark(fa_make_basic(FA_EPSILON));
    int r;

    /* Jump through some hoops to make FA1 a copy of FA */
    fa1 = mark(fa_concat(fa, empty));
    /* Minimize FA1, otherwise the regexp returned is enormous for the */
    /* monster (~ 2MB) and fa_compile becomes incredibly slow          */
    r = fa_minimize(fa1);
    if (r < 0)
        die_oom();

    r = fa_as_regexp(fa1, &re, &re_len);
    CuAssertIntEquals(tc, 0, r);

    r = fa_compile(re, re_len, &fa2);
    if (r != REG_NOERROR) {
        print_regerror(r, re);
    }
    CuAssertIntEquals(tc, REG_NOERROR, r);
    CuAssertTrue(tc, fa_equals(fa, fa2));

    fa_free(fa2);
    free(re);
}

static struct fa *make_fa(CuTest *tc,
                          const char *regexp, size_t reglen,
                          int exp_err) {
    struct fa *fa;
    int r;

    r = fa_compile(regexp, reglen, &fa);
    if (r == REG_ESPACE)
        die_oom();
    if (exp_err == REG_NOERROR) {
        if (r != REG_NOERROR)
            print_regerror(r, regexp);
        CuAssertIntEquals(tc, REG_NOERROR, r);
        CuAssertPtrNotNull(tc, fa);
        mark(fa);
        assertAsRegexp(tc, fa);
    } else {
        CuAssertIntEquals(tc, exp_err, r);
        CuAssertPtrEquals(tc, NULL, fa);
    }
    return fa;
}

static struct fa *make_good_fa(CuTest *tc, const char *regexp) {
    return make_fa(tc, regexp, strlen(regexp), REG_NOERROR);
}

static void dot(struct fa *fa) {
    static int count = 0;
    FILE *fp;
    const char *dot_dir;
    char *fname;
    int r;

    if ((dot_dir = getenv(FA_DOT_DIR)) == NULL)
        return;

    r = asprintf(&fname, "%s/fa_test_%02d.dot", dot_dir, count++);
    if (r == -1)
        return;

    if ((fp = fopen(fname, "w")) == NULL) {
        free(fname);
        return;
    }

    fa_dot(fp, fa);
    fclose(fp);

    free(fname);
}

static void testBadRegexps(CuTest *tc) {
    const char *const re1 = "(x";
    const char *const re2 = "a{5,3}";
    make_fa(tc, re1, strlen(re1), REG_EPAREN);
    make_fa(tc, re2, strlen(re2), REG_BADBR);
}

/* Stress test, mostly good to check that allocation is clean */
static void testMonster(CuTest *tc) {
#define WORD "[a-zA-Z_0-9]+"
#define CWS  "([ \\n\\t]+|\\/\\*([^\\*]|\\*[^\\/])*\\*\\/)*"
#define QUOTED "\"[^\"]*\""
#define ELT    "\\(" WORD "(," CWS WORD "){3}\\)"

    static const char *const monster =
        "(" WORD "|" QUOTED "|" "\\(" CWS ELT  "(," CWS ELT ")*" CWS "\\))";

#undef ELT
#undef QUOTED
#undef CWS
#undef WORD

    struct fa *fa, *fas;
    char *upv, *pv, *v;
    size_t upv_len;

    fa = make_good_fa(tc, monster);

    fa_ambig_example(fa, fa, &upv, &upv_len, &pv, &v);

    /* Monster can't be concatenated with itself */
    CuAssertStrEquals(tc, "AAA", upv);
    CuAssertStrEquals(tc, "AA", pv);
    CuAssertStrEquals(tc, "A", v);
    free(upv);

    /* Monster can also not be starred */
    fas = mark(fa_iter(fa, 0, -1));
    /* Minimize FAS, otherwise the example returned is nondeterministic,
       since example generation depends on the structure of the FA.
       FIXME: Explain why UPV with the unminimized FAS changes to a much
       longer string simply when allocation patterns change (e.g., by
       running the test under valgrind vs. plain glibc malloc. Fishy ?
       FA_EXAMPLE should depend on the structure of the FA, but not
       incidental details like sorting of transitions.
     */
    fa_minimize(fas);
    fa_ambig_example(fas, fa, &upv, &upv_len, &pv, &v);

    CuAssertStrEquals(tc, "AA", upv);
    CuAssertStrEquals(tc, "AA", pv);
    CuAssertStrEquals(tc, "A", v);
    free(upv);
}

static void testChars(CuTest *tc) {
    struct fa *fa1, *fa2, *fa3;

    fa1 = make_good_fa(tc, ".");
    fa2 = make_good_fa(tc, "[a-z]");
    CuAssertTrue(tc, fa_contains(fa2, fa1));

    fa1 = make_good_fa(tc, "(.|\n)");
    CuAssertTrue(tc, fa_contains(fa2, fa1));

    fa1 = mark(fa_intersect(fa1, fa2));
    CuAssertTrue(tc, fa_equals(fa1, fa2));

    fa1 = make_good_fa(tc, "[^b-dxyz]");
    fa2 = make_good_fa(tc, "[a-z]");
    fa3 = mark(fa_intersect(fa1, fa2));
    fa2 = make_good_fa(tc, "[ae-w]");
    CuAssertTrue(tc, fa_equals(fa2, fa3));
}

static void testManualAmbig(CuTest *tc) {
    /* The point of this test is mostly to teach me how Anders Moeller's
       algorithm for finding ambiguous strings works.

       For the two languages a1 = a|ab and a2 = a|ba, a1.a2 has one
       ambiguous word aba, which can be split as a.ba and ab.a.

       This uses X and Y as the markers*/

    struct fa *a1f = make_good_fa(tc, "Xa|XaXb");
    struct fa *a1t = make_good_fa(tc, "(YX)*Xa|(YX)*Xa(YX)*Xb");
    struct fa *a2f = make_good_fa(tc, "Xa|XbXa");
    struct fa *a2t = make_good_fa(tc, "(YX)*Xa|((YX)*Xb(YX)*Xa)");
    struct fa *mp = make_good_fa(tc, "YX(X(.|\n))+");
    struct fa *ms = make_good_fa(tc, "YX(X(.|\n))*");
    struct fa *sp = make_good_fa(tc, "(X(.|\n))+YX");
    struct fa *ss = make_good_fa(tc, "(X(.|\n))*YX");

    struct fa *a1f_mp = mark(fa_concat(a1f, mp));
    struct fa *a1f_mp_a1t = mark(fa_intersect(a1f_mp, a1t));
    struct fa *b1 = mark(fa_concat(a1f_mp_a1t, ms));

    struct fa *sp_a2f = mark(fa_concat(sp, a2f));
    struct fa *sp_a2f_a2t = mark(fa_intersect(sp_a2f, a2t));
    struct fa *b2 = mark(fa_concat(ss, sp_a2f_a2t));

    struct fa *amb = mark(fa_intersect(b1, b2));
    struct fa *exp = make_good_fa(tc, "XaYXXbYXXa");
    CuAssertTrue(tc, fa_equals(exp, amb));
}

static void testContains(CuTest *tc) {
    struct fa *fa1, *fa2, *fa3;

    fa1 = make_good_fa(tc, "ab*");
    fa2 = make_good_fa(tc, "ab+");
    fa3 = make_good_fa(tc, "ab+c*|acc");

    CuAssertTrue(tc, fa_contains(fa1, fa1));
    CuAssertTrue(tc, fa_contains(fa2, fa2));
    CuAssertTrue(tc, fa_contains(fa3, fa3));

    CuAssertTrue(tc, ! fa_contains(fa1, fa2));
    CuAssertTrue(tc, fa_contains(fa2, fa1));
    CuAssertTrue(tc, fa_contains(fa2, fa3));
    CuAssertTrue(tc, ! fa_contains(fa3, fa2));
    CuAssertTrue(tc, ! fa_contains(fa1, fa3));
    CuAssertTrue(tc, ! fa_contains(fa3, fa1));
}

static void testIntersect(CuTest *tc) {
    struct fa *fa1, *fa2, *fa;

    fa1 = make_good_fa(tc, "[a-zA-Z]*[.:=]([0-9]|[^A-Z])*");
    fa2 = make_good_fa(tc, "[a-z][:=][0-9a-z]+");
    fa = mark(fa_intersect(fa1, fa2));
    CuAssertPtrNotNull(tc, fa);
    CuAssertTrue(tc, fa_equals(fa, fa2));
    CuAssertTrue(tc, ! fa_equals(fa, fa1));
}

static void testComplement(CuTest *tc) {
    struct fa *fa1 = make_good_fa(tc, "[b-y]+");
    struct fa *fa2 = mark(fa_complement(fa1));
    /* We use '()' to match the empty word explicitly */
    struct fa *fa3 = make_good_fa(tc, "(()|[b-y]*[^b-y](.|\n)*)");

    CuAssertTrue(tc, fa_equals(fa2, fa3));

    fa2 = mark(fa_complement(fa2));
    CuAssertTrue(tc, fa_equals(fa1, fa2));
}

static void testOverlap(CuTest *tc) {
    struct fa *fa1 = make_good_fa(tc, "a|ab");
    struct fa *fa2 = make_good_fa(tc, "a|ba");
    struct fa *p   = mark(fa_overlap(fa1, fa2));
    struct fa *exp = make_good_fa(tc, "b");

    CuAssertTrue(tc, fa_equals(exp, p));

    fa1 = make_good_fa(tc, "a|b|c|abc");
    fa2 = mark(fa_iter(fa1, 0, -1));
    exp = make_good_fa(tc, "bc");
    p = mark(fa_overlap(fa1, fa2));

    CuAssertTrue(tc, fa_equals(exp, p));
}

static void assertExample(CuTest *tc, const char *regexp, const char *exp) {
    struct fa *fa = make_good_fa(tc, regexp);
    size_t xmpl_len;
    char *xmpl;
    fa_example(fa, &xmpl, &xmpl_len);
    CuAssertStrEquals(tc, exp, xmpl);
    free(xmpl);

    fa_nocase(fa);
    char *s = strdup(exp);
    for (int i = 0; i < strlen(s); i++) s[i] = tolower(s[i]);

    fa_example(fa, &xmpl, &xmpl_len);
    CuAssertStrEquals(tc, s, xmpl);
    free(xmpl);
    free(s);
}

static void testExample(CuTest *tc) {
    assertExample(tc, "(.|\n)", "A");
    assertExample(tc, "(\n|\t|x)", "x");
    assertExample(tc, "[^b-y]", "A");
    assertExample(tc, "x*", "x");
    assertExample(tc, "yx*", "y");
    assertExample(tc, "ab+cx*", "abc");
    assertExample(tc, "ab+cx*|y*", "y");
    assertExample(tc, "u*|[0-9]", "u");
    assertExample(tc, "u+|[0-9]", "u");
    assertExample(tc, "vu+|[0-9]", "0");
    assertExample(tc, "vu{2}|[0-9]", "0");
    assertExample(tc, "\\[", "[");
    assertExample(tc, "[\\]", "\\");
    assertExample(tc, "a{3}", "aaa");
    assertExample(tc, "a{3,}", "aaa");

    assertExample(tc, "\001((\002.)*\001)+\002", "\001\001\002");
    assertExample(tc, "\001((\001.)*\002)+\002", "\001\002\002");

    assertExample(tc, "a[^\001-\004]+b", "aAb");

    /* A strange way to write a? - allowed by POSIX */
    assertExample(tc, "(a|)", "a");
    assertExample(tc, "(|a)", "a");

    struct fa *fa1 = mark(fa_make_basic(FA_EMPTY));
    size_t xmpl_len;
    char *xmpl;
    fa_example(fa1, &xmpl, &xmpl_len);
    CuAssertPtrEquals(tc, NULL, xmpl);

    fa1 = mark(fa_make_basic(FA_EPSILON));
    fa_example(fa1, &xmpl, &xmpl_len);
    CuAssertStrEquals(tc, "", xmpl);
    free(xmpl);
}

static void assertAmbig(CuTest *tc, const char *regexp1, const char *regexp2,
                        const char *exp_upv,
                        const char *exp_pv, const char *exp_v) {

    struct fa *fa1 = make_good_fa(tc, regexp1);
    struct fa *fa2 = make_good_fa(tc, regexp2);
    char *upv, *pv, *v;
    size_t upv_len;
    fa_ambig_example(fa1, fa2, &upv, &upv_len, &pv, &v);
    CuAssertPtrNotNull(tc, upv);
    CuAssertPtrNotNull(tc, pv);
    CuAssertPtrNotNull(tc, v);

    CuAssertStrEquals(tc, exp_upv, upv);
    CuAssertStrEquals(tc, exp_pv, pv);
    CuAssertStrEquals(tc, exp_v, v);
    free(upv);
}

static void assertNotAmbig(CuTest *tc, const char *regexp1,
                           const char *regexp2) {
    struct fa *fa1 = make_good_fa(tc, regexp1);
    struct fa *fa2 = make_good_fa(tc, regexp2);
    char *upv;
    size_t upv_len;
    fa_ambig_example(fa1, fa2, &upv, &upv_len, NULL, NULL);
    CuAssertPtrEquals(tc, NULL, upv);
}

static void testAmbig(CuTest *tc) {
    assertAmbig(tc, "a|ab", "a|ba", "aba", "ba", "a");
    assertAmbig(tc, "(a|ab)*", "a|ba", "aba", "ba", "a");
    assertAmbig(tc, "(a|b|c|d|abcd)", "(a|b|c|d|abcd)*",
                "abcd", "bcd", "");
    assertAmbig(tc, "(a*)*", "a*", "a", "a", "");
    assertAmbig(tc, "(a+)*", "a+", "aa", "aa", "a");

    assertNotAmbig(tc, "a*", "a");
    assertNotAmbig(tc, "(a*b)*", "a*b");
    assertNotAmbig(tc, "(a|b|c|d|abcd)", "(a|b|c|d|abcd)");
}

static void assertFaAsRegexp(CuTest *tc, const char *regexp) {
    char *re;
    size_t re_len;
    struct fa *fa1 = make_good_fa(tc, regexp);
    struct fa *fa2;
    int r;

    r = fa_as_regexp(fa1, &re, &re_len);
    CuAssertIntEquals(tc, 0, r);

    r = fa_compile(re, strlen(re), &fa2);
    CuAssertIntEquals(tc, REG_NOERROR, r);

    CuAssert(tc, regexp, fa_equals(fa1, fa2));

    fa_free(fa2);
    free(re);
}

static void testAsRegexp(CuTest *tc) {
    assertFaAsRegexp(tc, "a*");
    assertFaAsRegexp(tc, "abcd");
    assertFaAsRegexp(tc, "ab|cd");
    assertFaAsRegexp(tc, "[a-z]+");
    assertFaAsRegexp(tc, "[]a-]+");
    assertFaAsRegexp(tc, "[^0-9A-Z]");
    assertFaAsRegexp(tc, "ab|(xy[A-Z0-9])*(uv[^0-9]?)");
    assertFaAsRegexp(tc, "[A-CE-GI-LN-QS-Z]");
}

static void testAsRegexpMinus(CuTest *tc) {
    struct fa *fa1 = make_good_fa(tc, "[A-Za-z]+");
    struct fa *fa2 = make_good_fa(tc, "Deny(Users|Groups|Other)");
    struct fa *fa = mark(fa_minus(fa1, fa2));
    char *re;
    size_t re_len;
    int r;

    r = fa_as_regexp(fa, &re, &re_len);
    CuAssertIntEquals(tc, 0, r);

    struct fa *far = make_good_fa(tc, re);
    CuAssertTrue(tc, fa_equals(fa, far));

    free(re);
}

static void testRangeEnd(CuTest *tc) {
    const char *const re = "[1-0]";
    make_fa(tc, re, strlen(re), REG_ERANGE);
}

static void testNul(CuTest *tc) {
    static const char *const re0 = "a\0b";
    int re0_len = 3;

    struct fa *fa1 = make_fa(tc, "a\0b", re0_len, REG_NOERROR);
    struct fa *fa2 = make_good_fa(tc, "a.b");
    char *re;
    size_t re_len;
    int r;

    CuAssertTrue(tc, fa_contains(fa1, fa2));

    r = fa_as_regexp(fa1, &re, &re_len);
    CuAssertIntEquals(tc, 0, r);
    CuAssertIntEquals(tc, re0_len, re_len);
    CuAssertIntEquals(tc, 0, memcmp(re0, re, re0_len));
}

static void testRestrictAlphabet(CuTest *tc) {
    const char *re = "ab|(xy[B-Z0-9])*(uv[^0-9]?)";
    struct fa *fa_exp = make_good_fa(tc, "((xy[0-9])*)uv[^0-9A-Z]?|ab");
    struct fa *fa_act = NULL;
    size_t nre_len;
    char *nre;
    int r;

    r = fa_restrict_alphabet(re, strlen(re), &nre, &nre_len, 'A', 'Z');
    CuAssertIntEquals(tc, 0, r);
    CuAssertIntEquals(tc, nre_len, strlen(nre));
    fa_act = make_good_fa(tc, nre);
    CuAssertTrue(tc, fa_equals(fa_exp, fa_act));
    free(nre);

    r = fa_restrict_alphabet("HELLO", strlen("HELLO"),
                             &nre, &nre_len, 'A', 'Z');
    CuAssertIntEquals(tc, -2, r);
    CuAssertPtrEquals(tc, NULL, nre);

    r = fa_restrict_alphabet("a{2,", strlen("a{2"), &nre, &nre_len, 'A', 'Z');
    CuAssertIntEquals(tc, REG_EBRACE, r);
}

static void testExpandCharRanges(CuTest *tc) {
    const char *re = "[1-3]*|[a-b]([^\nU-X][^\n])*";
    const char *re2 = "a\\|b";

    char *nre;
    size_t nre_len;
    int r;

    r = fa_expand_char_ranges(re, strlen(re), &nre, &nre_len);
    CuAssertIntEquals(tc, 0, r);
    CuAssertStrEquals(tc, "[123]*|[ab]([^\nUVWX].)*", nre);
    CuAssertIntEquals(tc, strlen(nre), nre_len);
    free(nre);

    r = fa_expand_char_ranges(re2, strlen(re2), &nre, &nre_len);
    CuAssertIntEquals(tc, 0, r);
    CuAssertStrEquals(tc, re2, nre);
    free(nre);
}

static void testNoCase(CuTest *tc) {
    struct fa *fa1 = make_good_fa(tc, "[a-z0-9]");
    struct fa *fa2 = make_good_fa(tc, "B");
    struct fa *fa, *exp;
    int r;

    fa_nocase(fa1);
    fa = fa_intersect(fa1, fa2);
    CuAssertPtrNotNull(tc, fa);
    r = fa_equals(fa, fa2);
    fa_free(fa);
    CuAssertIntEquals(tc, 1, r);

    fa = fa_concat(fa1, fa2);
    exp = make_good_fa(tc, "[a-zA-Z0-9]B");
    r = fa_equals(fa, exp);
    fa_free(fa);
    CuAssertIntEquals(tc, 1, r);
}

static void testExpandNoCase(CuTest *tc) {
    const char *p1 = "aB";
    const char *p2 = "[a-cUV]";
    const char *p3 = "[^a-z]";
    char *s;
    size_t len;
    int r;

    r = fa_expand_nocase(p1, strlen(p1), &s, &len);
    CuAssertIntEquals(tc, 0, r);
    CuAssertStrEquals(tc, "[Aa][Bb]", s);
    free(s);

    r = fa_expand_nocase(p2, strlen(p2), &s, &len);
    CuAssertIntEquals(tc, 0, r);
    CuAssertStrEquals(tc, "[A-CUVa-cuv]", s);
    free(s);

    r = fa_expand_nocase(p3, strlen(p3), &s, &len);
    CuAssertIntEquals(tc, 0, r);
    CuAssertStrEquals(tc, "[^A-Za-z]", s);
    free(s);
}

static void testNoCaseComplement(CuTest *tc) {
    const char *key_s = "keY";
    struct fa *key = make_good_fa(tc, key_s);
    struct fa *isect = NULL;

    fa_nocase(key);

    struct fa *comp = mark(fa_complement(key));

    key = make_good_fa(tc, key_s);

    /* We used to have a bug in totalize that caused the intersection
     * to contain "keY" */
    isect = fa_intersect(key, comp);

    CuAssertIntEquals(tc, 1, fa_is_basic(isect, FA_EMPTY));
}

int main(int argc, char **argv) {
    if (argc == 1) {
        char *output = NULL;
        CuSuite* suite = CuSuiteNew();
        CuSuiteSetup(suite, setup, teardown);

        SUITE_ADD_TEST(suite, testBadRegexps);
        SUITE_ADD_TEST(suite, testMonster);
        SUITE_ADD_TEST(suite, testChars);
        SUITE_ADD_TEST(suite, testManualAmbig);
        SUITE_ADD_TEST(suite, testContains);
        SUITE_ADD_TEST(suite, testIntersect);
        SUITE_ADD_TEST(suite, testComplement);
        SUITE_ADD_TEST(suite, testOverlap);
        SUITE_ADD_TEST(suite, testExample);
        SUITE_ADD_TEST(suite, testAmbig);
        SUITE_ADD_TEST(suite, testAsRegexp);
        SUITE_ADD_TEST(suite, testAsRegexpMinus);
        SUITE_ADD_TEST(suite, testRangeEnd);
        SUITE_ADD_TEST(suite, testNul);
        SUITE_ADD_TEST(suite, testRestrictAlphabet);
        SUITE_ADD_TEST(suite, testExpandCharRanges);
        SUITE_ADD_TEST(suite, testNoCase);
        SUITE_ADD_TEST(suite, testExpandNoCase);
        SUITE_ADD_TEST(suite, testNoCaseComplement);

        CuSuiteRun(suite);
        CuSuiteSummary(suite, &output);
        CuSuiteDetails(suite, &output);
        printf("%s\n", output);
        free(output);
        if (getenv("FAILMALLOC_INTERVAL") != NULL)
            return EXIT_SUCCESS;
        else
            return suite->failCount;
    }

    for (int i=1; i<argc; i++) {
        struct fa *fa;
        int r;
        if ((r = fa_compile(argv[i], strlen(argv[i]), &fa)) != REG_NOERROR) {
            print_regerror(r, argv[i]);
        } else {
            dot(fa);
            size_t s_len;
            char *s;
            fa_example(fa, &s, &s_len);
            printf("Example for %s: %s\n", argv[i], s);
            free(s);
            char *re;
            size_t re_len;
            r = fa_as_regexp(fa, &re, &re_len);
            if (r == 0) {
                printf("/%s/ = /%s/\n", argv[i], re);
                free(re);
            } else {
                printf("/%s/ = ***\n", argv[i]);
            }
            fa_free(fa);
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
