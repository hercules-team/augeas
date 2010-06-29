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
static const char *abs_top_builddir;
static char *root = NULL;
static char *loadpath;

#define die(msg)                                                    \
    do {                                                            \
        fprintf(stderr, "%d: Fatal error: %s\n", __LINE__, msg);    \
        exit(EXIT_FAILURE);                                         \
    } while(0)

static struct augeas *setup_writable_hosts(CuTest *tc) {
    char *etcdir, *build_root;
    struct augeas *aug = NULL;
    int r;

    if (asprintf(&build_root, "%s/build/test-load/%s",
                 abs_top_builddir, tc->name) < 0) {
        CuFail(tc, "failed to set build_root");
    }

    if (asprintf(&etcdir, "%s/etc", build_root) < 0)
        CuFail(tc, "asprintf etcdir failed");

    run(tc, "test -d %s && chmod -R u+w %s || :", build_root, build_root);
    run(tc, "rm -rf %s", build_root);
    run(tc, "mkdir -p %s", etcdir);
    run(tc, "cp -pr %s/etc/hosts %s", root, etcdir);
    run(tc, "chmod -R u+w %s", build_root);

    aug = aug_init(build_root, loadpath, AUG_NO_MODL_AUTOLOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_set(aug, "/augeas/load/Hosts/lens", "Hosts.lns");
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "/augeas/load/Hosts/incl", "/etc/hosts");
    CuAssertRetSuccess(tc, r);

    return aug;
}

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

static void testNoAutoload(CuTest *tc) {
    augeas *aug = NULL;
    int nmatches, r;

    aug = aug_init(root, loadpath, AUG_NO_MODL_AUTOLOAD);
    CuAssertPtrNotNull(tc, aug);

    nmatches = aug_match(aug, "/augeas/load/*", NULL);
    CuAssertZero(tc, nmatches);

    r = aug_set(aug, "/augeas/load/Hosts/lens", "Hosts.lns");
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "/augeas/load/Hosts/incl", "/etc/hosts");
    CuAssertRetSuccess(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    nmatches = aug_match(aug, "/files/etc/hosts/*[ipaddr]", NULL);
    CuAssertIntEquals(tc, 2, nmatches);

    aug_close(aug);
}

static void invalidLens(CuTest *tc, augeas *aug, const char *lens) {
    int r, nmatches;

    r = aug_set(aug, "/augeas/load/Junk/lens", lens);
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "/augeas/load/Junk/incl", "/dev/null");
    CuAssertRetSuccess(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    nmatches = aug_match(aug, "/augeas/load/Junk/error", NULL);
    CuAssertIntEquals(tc, 1, nmatches);
}

static void testInvalidLens(CuTest *tc) {
    augeas *aug = NULL;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_rm(aug, "/augeas/load/*");
    CuAssertTrue(tc, r >= 0);

    invalidLens(tc, aug, NULL);
    invalidLens(tc, aug, "@Nomodule");
    invalidLens(tc, aug, "@Util");
    invalidLens(tc, aug, "Nomodule.noelns");

    aug_close(aug);
}

static void testLoadSave(CuTest *tc) {
    augeas *aug = NULL;
    int r;

    aug = setup_writable_hosts(tc);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_save(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/augeas/events/saved", NULL);
    CuAssertIntEquals(tc, 0, r);

    aug_close(aug);
}

/* Tests bug #79 */
static void testLoadDefined(CuTest *tc) {
    augeas *aug = NULL;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC);
    CuAssertPtrNotNull(tc, aug);

    r = aug_defvar(aug, "v", "/files/etc/hosts/*/ipaddr");
    CuAssertIntEquals(tc, 2, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "$v", NULL);
    CuAssertIntEquals(tc, 2, r);

    aug_close(aug);
}

static void testDefvarExpr(CuTest *tc) {
    static const char *const expr = "/files/etc/hosts/*/ipaddr";
    static const char *const expr2 = "/files/etc/hosts/*/canonical";

    augeas *aug = NULL;
    const char *v;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC);
    CuAssertPtrNotNull(tc, aug);

    r = aug_defvar(aug, "v", expr);
    CuAssertIntEquals(tc, 2, r);

    r = aug_get(aug, "/augeas/variables/v", &v);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, expr, v);

    r = aug_defvar(aug, "v", expr2);
    CuAssertIntEquals(tc, 2, r);

    r = aug_get(aug, "/augeas/variables/v", &v);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, expr2, v);

    r = aug_defvar(aug, "v", NULL);
    CuAssertIntEquals(tc, 0, r);

    r = aug_get(aug, "/augeas/variables/v", &v);
    CuAssertIntEquals(tc, 0, r);
    CuAssertStrEquals(tc, NULL, v);

    aug_close(aug);
}

int main(void) {
    char *output = NULL;
    CuSuite* suite = CuSuiteNew();
    CuSuiteSetup(suite, NULL, NULL);

    SUITE_ADD_TEST(suite, testDefault);
    SUITE_ADD_TEST(suite, testNoLoad);
    SUITE_ADD_TEST(suite, testNoAutoload);
    SUITE_ADD_TEST(suite, testInvalidLens);
    SUITE_ADD_TEST(suite, testLoadSave);
    SUITE_ADD_TEST(suite, testLoadDefined);
    SUITE_ADD_TEST(suite, testDefvarExpr);

    abs_top_srcdir = getenv("abs_top_srcdir");
    if (abs_top_srcdir == NULL)
        die("env var abs_top_srcdir must be set");

    abs_top_builddir = getenv("abs_top_builddir");
    if (abs_top_builddir == NULL)
        die("env var abs_top_builddir must be set");

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
