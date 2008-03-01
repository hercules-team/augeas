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
#include <stdio.h>
#include <stdlib.h>

#define FA_DOT_DIR "FA_DOT_DIR"

static fa_t make_fa(CuTest *tc, const char *regexp, int exp_err) {
    fa_t fa;
    int r;

    r = fa_compile(regexp, &fa);
    CuAssertIntEquals(tc, exp_err, r);
    if (exp_err == REG_NOERROR) {
        CuAssertPtrNotNull(tc, fa);
    } else {
        CuAssertPtrEquals(tc, NULL, fa);
    }
    return fa;
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

    fa = make_fa(tc, monster, REG_NOERROR);
    fa_free(fa);
}

static void testContains(CuTest *tc) {
    fa_t fa1, fa2, fa3;

    fa1 = make_fa(tc, "ab*", REG_NOERROR);
    fa2 = make_fa(tc, "ab+", REG_NOERROR);
    fa3 = make_fa(tc, "ab+c*|acc", REG_NOERROR);

    CuAssertTrue(tc, fa_contains(fa1, fa1));
    CuAssertTrue(tc, fa_contains(fa2, fa2));
    CuAssertTrue(tc, fa_contains(fa3, fa3));

    CuAssertTrue(tc, ! fa_contains(fa1, fa2));
    CuAssertTrue(tc, fa_contains(fa2, fa1));
    CuAssertTrue(tc, fa_contains(fa2, fa3));
    CuAssertTrue(tc, ! fa_contains(fa3, fa2));
    CuAssertTrue(tc, ! fa_contains(fa1, fa3));
    CuAssertTrue(tc, ! fa_contains(fa3, fa1));

    fa_free(fa1);
    fa_free(fa2);
    fa_free(fa3);
}

// Check that fa_build("[^-x-]") does the right thing

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

int main(int argc, char **argv) {
    if (argc == 1) {
        char *output = NULL;
        CuSuite* suite = CuSuiteNew();
        CuSuiteSetup(suite, NULL, NULL);

        SUITE_ADD_TEST(suite, testBadRegexps);
        SUITE_ADD_TEST(suite, testMonster);
        SUITE_ADD_TEST(suite, testContains);

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
            size_t size;
            char *errbuf;
            size = regerror(r, NULL, NULL, 0);
            errbuf = alloca(size);
            regerror(r, NULL, errbuf, size);
            fprintf(stderr, "Error building fa from %s:\n", argv[i]);
            fprintf(stderr, "  %s\n", errbuf);
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
