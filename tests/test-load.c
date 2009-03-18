/*
 * test-load.c: test the aug_load functionality
 *
 * Copyright (C) 2009 Red Hat Inc.
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

#include <config.h>

#include "augeas.h"

#include "cutest.h"
#include "internal.h"

#define CuAssertPositive(tc, n) CuAssertTrue(tc, (n) > 0)
#define CuAssertZero(tc, n) CuAssertTrue(tc, (n) == 0)
#define CuAssertRetSuccess(tc, n) CuAssertTrue(tc, (n) == 0)

static const char *abs_top_srcdir;
static char *root;
static char *loadpath;

#define die(msg)                                                    \
    do {                                                            \
        fprintf(stderr, "%d: Fatal error: %s\n", __LINE__, msg);    \
        exit(EXIT_FAILURE);                                         \
    } while(0)


static void testDefault(CuTest *tc) {
    augeas *aug = NULL;
    int nmatches, r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC);
    CuAssertPtrNotNull(tc, aug);

    nmatches = aug_match(aug, "/augeas/load/*", NULL);
    CuAssertPositive(tc, nmatches);

    nmatches = aug_match(aug, "/files/etc/hosts/1", NULL);
    CuAssertIntEquals(tc, 1, nmatches);

    r = aug_rm(aug, "/augeas/load/*");
    CuAssertTrue(tc, r >= 0);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    nmatches = aug_match(aug, "/files/*", NULL);
    CuAssertZero(tc, nmatches);

    aug_close(aug);
}

static void testNoLoad(CuTest *tc) {
    augeas *aug = NULL;
    int nmatches, r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);

    nmatches = aug_match(aug, "/augeas/load/*", NULL);
    CuAssertPositive(tc, nmatches);

    nmatches = aug_match(aug, "/files/*", NULL);
    CuAssertZero(tc, nmatches);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    nmatches = aug_match(aug, "/files/*", NULL);
    CuAssertPositive(tc, nmatches);

    /* Now load /etc/hosts only */
    r = aug_rm(aug, "/augeas/load/*[label() != 'Hosts']");
    CuAssertTrue(tc, r >= 0);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    nmatches = aug_match(aug, "/files/etc/*", NULL);
    CuAssertIntEquals(tc, 1, nmatches);

    aug_close(aug);
}

int main(void) {
    char *output = NULL;
    CuSuite* suite = CuSuiteNew();
    CuSuiteSetup(suite, NULL, NULL);

    SUITE_ADD_TEST(suite, testDefault);
    SUITE_ADD_TEST(suite, testNoLoad);

    abs_top_srcdir = getenv("abs_top_srcdir");
    if (abs_top_srcdir == NULL)
        die("env var abs_top_srcdir must be set");

    if (asprintf(&root, "%s/tests/root", abs_top_srcdir) < 0) {
        die("failed to set root");
    }

    if (asprintf(&loadpath, "%s/lenses", abs_top_srcdir) < 0) {
        die("failed to set loadpath");
    }

    CuSuiteRun(suite);
    CuSuiteSummary(suite, &output);
    CuSuiteDetails(suite, &output);
    printf("%s\n", output);
    free(output);
    return suite->failCount;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
