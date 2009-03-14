/*
 * fatest.c:
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

#include <config.h>

#include "fa.h"
#include "cutest.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>

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
    errbuf = alloca(size);
    regerror(err, NULL, errbuf, size);
    fprintf(stderr, "Error building fa from %s:\n", regexp);
    fprintf(stderr, "  %s\n", errbuf);
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
        CALLOC(fl, 1);
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
    fa_minimize(fa1);

    r = fa_as_regexp(fa1, &re, &re_len);
    CuAssertIntEquals(tc, 0, r);

    r = fa_compile(re, strlen(re), &fa2);
    CuAssertIntEquals(tc, REG_NOERROR, r);

    CuAssertTrue(tc, fa_equals(fa, fa2));

    fa_free(fa2);
    free(re);
}

static struct fa *make_fa(CuTest *tc, const char *regexp, int exp_err) {
    struct fa *fa;
    int r;

    r = fa_compile(regexp, strlen(regexp), &fa);
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
    return make_fa(tc, regexp, REG_NOERROR);
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
    make_fa(tc, "(x", REG_EPAREN);
    make_fa(tc, "a{5,3}", REG_BADBR);
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
    struct fa *a1f_mp$a1t = mark(fa_intersect(a1f_mp, a1t));
    struct fa *b1 = mark(fa_concat(a1f_mp$a1t, ms));

    struct fa *sp_a2f = mark(fa_concat(sp, a2f));
    struct fa *sp_a2f$a2t = mark(fa_intersect(sp_a2f, a2t));
    struct fa *b2 = mark(fa_concat(ss, sp_a2f$a2t));

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

    assertExample(tc, "\001((\002.)*\001)+\002", "\001\001\002");
    assertExample(tc, "\001((\001.)*\002)+\002", "\001\002\002");

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
    make_fa(tc, "[1-0]", REG_ERANGE);
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

        CuSuiteRun(suite);
        CuSuiteSummary(suite, &output);
        CuSuiteDetails(suite, &output);
        printf("%s\n", output);
        free(output);
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
