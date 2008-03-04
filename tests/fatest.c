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

#include "fa.h"
#include "cutest.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>

#define FA_DOT_DIR "FA_DOT_DIR"

struct fa_list {
    struct fa_list *next;
    fa_t fa;
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

static fa_t mark(fa_t fa) {
    struct fa_list *fl;

    if (fa != NULL) {
        CALLOC(fl, 1);
        fl->fa = fa;
        list_cons(fa_list, fl);
    }
    return fa;
}

static fa_t make_fa(CuTest *tc, const char *regexp, int exp_err) {
    fa_t fa;
    int r;

    r = fa_compile(regexp, &fa);
    CuAssertIntEquals(tc, exp_err, r);
    if (exp_err == REG_NOERROR) {
        if (r != REG_NOERROR)
            print_regerror(r, regexp);
        CuAssertPtrNotNull(tc, fa);
        mark(fa);
    } else {
        CuAssertPtrEquals(tc, NULL, fa);
    }
    return fa;
}

static fa_t make_good_fa(CuTest *tc, const char *regexp) {
    return make_fa(tc, regexp, REG_NOERROR);
}

static void dot(struct fa *fa, int i) {
    FILE *fp;
    const char *dot_dir;
    char *fname;
    int r;

    if ((dot_dir = getenv(FA_DOT_DIR)) == NULL)
        return;

    r = asprintf(&fname, "%s/fa_test_%02d.dot", dot_dir, i);
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

/* Stress test, mostly good to check that allocation works */
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

    fa_t fa;

    fa = make_good_fa(tc, monster);
}

static void testChars(CuTest *tc) {
    fa_t fa1, fa2;

    // Check that fa_build("[^bc]") does the right thing

    fa1 = make_good_fa(tc, ".");
    fa2 = make_good_fa(tc, "[a-z]");
    CuAssertTrue(tc, fa_contains(fa2, fa1));

    fa1 = make_good_fa(tc, "(.|\n)");
    dot(fa2, 41);
    dot(fa1, 42);
    CuAssertTrue(tc, fa_contains(fa2, fa1));

    fa1 = mark(fa_intersect(fa1, fa2));
    CuAssertTrue(tc, fa_equals(fa1, fa2));
}

static void testContains(CuTest *tc) {
    fa_t fa1, fa2, fa3;

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
    fa_t fa1, fa2, fa;

    fa1 = make_good_fa(tc, "[a-zA-Z]*[.:=]([0-9]|[^A-Z])*");
    fa2 = make_good_fa(tc, "[a-z][:=][0-9a-z]+");
    fa = mark(fa_intersect(fa1, fa2));
    CuAssertPtrNotNull(tc, fa);
    CuAssertTrue(tc, fa_equals(fa, fa2));
    CuAssertTrue(tc, ! fa_equals(fa, fa1));
}

static void testComplement(CuTest *tc) {
    fa_t fa1 = make_good_fa(tc, "[b-y]+");
    fa_t fa2 = mark(fa_complement(fa1));
    /* We use '()' to match the empty word explicitly */
    fa_t fa3 = make_good_fa(tc, "(()|[b-y]*[^b-y](.|\n)*)");

    CuAssertTrue(tc, fa_equals(fa2, fa3));

    fa2 = mark(fa_complement(fa2));
    CuAssertTrue(tc, fa_equals(fa1, fa2));
}

static void testOverlap(CuTest *tc) {
    fa_t fa1 = make_good_fa(tc, "a|ab");
    fa_t fa2 = make_good_fa(tc, "a|ba");
    fa_t p   = mark(fa_overlap(fa1, fa2));
    fa_t exp = make_good_fa(tc, "b");

    CuAssertTrue(tc, fa_equals(exp, p));

    fa1 = make_good_fa(tc, "a|b|c|abc");
    fa2 = mark(fa_iter(fa1, 0, -1));
    exp = make_good_fa(tc, "bc");
    p = mark(fa_overlap(fa1, fa2));

    CuAssertTrue(tc, fa_equals(exp, p));
}

int main(int argc, char **argv) {
    if (argc == 1) {
        char *output = NULL;
        CuSuite* suite = CuSuiteNew();
        CuSuiteSetup(suite, setup, teardown);

        SUITE_ADD_TEST(suite, testBadRegexps);
        SUITE_ADD_TEST(suite, testMonster);
        SUITE_ADD_TEST(suite, testChars);
        SUITE_ADD_TEST(suite, testContains);
        SUITE_ADD_TEST(suite, testIntersect);
        SUITE_ADD_TEST(suite, testComplement);
        SUITE_ADD_TEST(suite, testOverlap);

        CuSuiteRun(suite);
        CuSuiteSummary(suite, &output);
        CuSuiteDetails(suite, &output);
        printf("%s\n", output);
        free(output);
        return suite->failCount;
    }

    for (int i=1; i<argc; i++) {
        fa_t fa;
        int r;
        if ((r = fa_compile(argv[i], &fa)) != REG_NOERROR) {
            print_regerror(r, argv[i]);
        } else {
            dot(fa, i);
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
