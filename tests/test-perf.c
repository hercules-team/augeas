/*
 * test-perf.c: test performance of API functions
 *
 * Copyright (C) 2009-2016 Red Hat Inc.
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

#include <sys/time.h>
#include <unistd.h>

static const char *abs_top_srcdir;
static char *root;
static char *loadpath;

#define die(msg)                                                    \
    do {                                                            \
        fprintf(stderr, "%d: Fatal error: %s\n", __LINE__, msg);    \
        exit(EXIT_FAILURE);                                         \
    } while(0)

#define time_taken(start, stop) \
    ((stop.tv_sec - start.tv_sec)*1000 + (stop.tv_usec - start.tv_usec)/1000)

/* Test performance of basic predicate [1..n] with many nodes */
static void testPerfPredicate(CuTest *tc) {
    const char *value;
    char *path;
    struct timeval stop, start;
    struct augeas *aug;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);

    gettimeofday(&start, NULL);

    for (int i=1; i <= 5000; i++) {
        if (!asprintf(&path, "/test/service[%i]", i))
            die("failed to generate set path");
        aug_set(aug, path, "test");
    }

    for (int i=1; i <= 5000; i++) {
        if (!asprintf(&path, "/test/service[%i]", i))
            die("failed to generate set path");
        aug_get(aug, path, &value);
        CuAssertStrEquals(tc, "test", value);
    }

    gettimeofday(&stop, NULL);
    printf("testPerfPredicate = %lums\n", time_taken(start, stop));

    aug_close(aug);
}

int main(void) {
    char *output = NULL;
    CuSuite* suite = CuSuiteNew();
    CuSuiteSetup(suite, NULL, NULL);

    SUITE_ADD_TEST(suite, testPerfPredicate);

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
    int result = suite->failCount;
    CuSuiteFree(suite);
    return result;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
