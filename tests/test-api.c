/*
 * test-api.c: test public API functions for conformance
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

static const char *abs_top_srcdir;
static char *root;
static char *loadpath;

#define die(msg)                                                    \
    do {                                                            \
        fprintf(stderr, "%d: Fatal error: %s\n", __LINE__, msg);    \
        exit(EXIT_FAILURE);                                         \
    } while(0)

static void testGet(CuTest *tc) {
    int r;
    const char *value;
    struct augeas *aug;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* Make sure we're looking at the right thing */
    r = aug_match(aug, "/augeas/version/save/*", NULL);
    CuAssertTrue(tc, r > 1);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* aug_get returns 1 and the value if exactly one node matches */
    r = aug_get(aug, "/augeas/version/save/*[1]", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* aug_get returns 0 and no value when no node matches */
    r = aug_get(aug, "/augeas/version/save/*[ last() + 1 ]", &value);
    CuAssertIntEquals(tc, 0, r);
    CuAssertPtrEquals(tc, NULL, value);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* aug_get should return an error when multiple nodes match */
    r = aug_get(aug, "/augeas/version/save/*", &value);
    CuAssertIntEquals(tc, -1, r);
    CuAssertPtrEquals(tc, NULL, value);
    CuAssertIntEquals(tc, AUG_EMMATCH, aug_error(aug));

    aug_close(aug);
}

static void testSetM(CuTest *tc) {
    int r;
    struct augeas *aug;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* Change base nodes when SUB is NULL */
    r = aug_setm(aug, "/augeas/version/save/*", NULL, "changed");
    CuAssertIntEquals(tc, 4, r);

    r = aug_match(aug, "/augeas/version/save/*[. = 'changed']", NULL);
    CuAssertIntEquals(tc, 4, r);

    /* Only change existing nodes */
    r = aug_setm(aug, "/augeas/version/save", "mode", "again");
    CuAssertIntEquals(tc, 4, r);

    r = aug_match(aug, "/augeas/version/save/*", NULL);
    CuAssertIntEquals(tc, 4, r);

    r = aug_match(aug, "/augeas/version/save/*[. = 'again']", NULL);
    CuAssertIntEquals(tc, 4, r);

    /* Create a new node */
    r = aug_setm(aug, "/augeas/version/save", "mode[last() + 1]", "newmode");
    CuAssertIntEquals(tc, 1, r);

    r = aug_match(aug, "/augeas/version/save/*", NULL);
    CuAssertIntEquals(tc, 5, r);

    r = aug_match(aug, "/augeas/version/save/*[. = 'again']", NULL);
    CuAssertIntEquals(tc, 4, r);

    r = aug_match(aug, "/augeas/version/save/*[last()][. = 'newmode']", NULL);
    CuAssertIntEquals(tc, 1, r);

    /* Noexistent base */
    r = aug_setm(aug, "/augeas/version/save[last()+1]", "mode", "newmode");
    CuAssertIntEquals(tc, 0, r);

    /* Invalid path expressions */
    r = aug_setm(aug, "/augeas/version/save[]", "mode", "invalid");
    CuAssertIntEquals(tc, -1, r);

    r = aug_setm(aug, "/augeas/version/save/*", "mode[]", "invalid");
    CuAssertIntEquals(tc, -1, r);
}

int main(void) {
    char *output = NULL;
    CuSuite* suite = CuSuiteNew();
    CuSuiteSetup(suite, NULL, NULL);

    SUITE_ADD_TEST(suite, testGet);
    SUITE_ADD_TEST(suite, testSetM);

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
