/*
 * test-load.c: test the aug_load functionality
 *
 * Copyright (C) 2009-2011 Red Hat Inc.
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
static const char *abs_top_builddir;
static char *root = NULL;
static char *loadpath;

#define die(msg)                                                    \
    do {                                                            \
        fprintf(stderr, "%d: Fatal error: %s\n", __LINE__, msg);    \
        exit(EXIT_FAILURE);                                         \
    } while(0)

static char *setup_hosts(CuTest *tc) {
    char *etcdir, *build_root;

    if (asprintf(&build_root, "%s/build/test-load/%s",
                 abs_top_builddir, tc->name) < 0) {
        CuFail(tc, "failed to set build_root");
    }

    if (asprintf(&etcdir, "%s/etc", build_root) < 0)
        CuFail(tc, "asprintf etcdir failed");

    run(tc, "test -d %s && chmod -R u+rw %s || :", build_root, build_root);
    run(tc, "rm -rf %s", build_root);
    run(tc, "mkdir -p %s", etcdir);
    run(tc, "cp -pr %s/etc/hosts %s", root, etcdir);

    free(etcdir);
    return build_root;
}

static struct augeas *setup_hosts_aug(CuTest *tc, char *build_root) {
    struct augeas *aug = NULL;
    int r;

    aug = aug_init(build_root, loadpath, AUG_NO_MODL_AUTOLOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_set(aug, "/augeas/load/Hosts/lens", "Hosts.lns");
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "/augeas/load/Hosts/incl", "/etc/hosts");
    CuAssertRetSuccess(tc, r);

    free(build_root);
    return aug;
}

static struct augeas *setup_writable_hosts(CuTest *tc) {
    char *build_root = setup_hosts(tc);
    run(tc, "chmod -R u+w %s", build_root);
    return setup_hosts_aug(tc, build_root);
}

static struct augeas *setup_unreadable_hosts(CuTest *tc) {
    char *build_root = setup_hosts(tc);
    run(tc, "chmod -R a-r %s/etc/hosts", build_root);
    return setup_hosts_aug(tc, build_root);
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

static void testReloadChanged(CuTest *tc) {
    FILE *fp;
    augeas *aug = NULL;
    const char *build_root, *mtime2, *s;
    char *mtime1;
    char *hosts = NULL;
    int r;

    aug = setup_writable_hosts(tc);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_get(aug, "/augeas/root", &build_root);
    CuAssertIntEquals(tc, 1, r);

    r = aug_get(aug, "/augeas/files/etc/hosts/mtime", &s);
    CuAssertIntEquals(tc, 1, r);
    mtime1 = strdup(s);
    CuAssertPtrNotNull(tc, mtime1);

    /* Tickle /etc/hosts behind augeas' back */
    r = asprintf(&hosts, "%setc/hosts", build_root);
    CuAssertPositive(tc, r);

    fp = fopen(hosts, "a");
    CuAssertPtrNotNull(tc, fp);

    r = fprintf(fp, "192.168.0.1 other.example.com\n");
    CuAssertTrue(tc, r > 0);

    r = fclose(fp);
    CuAssertRetSuccess(tc, r);

    /* Unsaved changes are discarded */
    r = aug_set(aug, "/files/etc/hosts/1/ipaddr", "127.0.0.2");
    CuAssertRetSuccess(tc, r);

    /* Check that we really did load the right file*/
    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_get(aug, "/augeas/files/etc/hosts/mtime", &mtime2);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrNotEqual(tc, mtime1, mtime2);

    r = aug_match(aug, "/files/etc/hosts/*[ipaddr = '192.168.0.1']", NULL);
    CuAssertIntEquals(tc, 1, r);

    r = aug_match(aug, "/files/etc/hosts/1[ipaddr = '127.0.0.1']", NULL);
    CuAssertIntEquals(tc, 1, r);

    free(mtime1);
    free(hosts);
    aug_close(aug);
}

static void testReloadDirty(CuTest *tc) {
    augeas *aug = NULL;
    int r;

    aug = setup_writable_hosts(tc);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    /* Unsaved changes are discarded */
    r = aug_set(aug, "/files/etc/hosts/1/ipaddr", "127.0.0.2");
    CuAssertRetSuccess(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/files/etc/hosts/1[ipaddr = '127.0.0.1']", NULL);
    CuAssertIntEquals(tc, 1, r);

    aug_close(aug);
}

static void testReloadDeleted(CuTest *tc) {
    augeas *aug = NULL;
    int r;

    aug = setup_writable_hosts(tc);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    /* A missing file causes a reload */
    r = aug_rm(aug, "/files/etc/hosts");
    CuAssertPositive(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/files/etc/hosts/1[ipaddr = '127.0.0.1']", NULL);
    CuAssertIntEquals(tc, 1, r);

    /* A missing entry in a file causes a reload */
    r = aug_rm(aug, "/files/etc/hosts/1/ipaddr");
    CuAssertPositive(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/files/etc/hosts/1[ipaddr = '127.0.0.1']", NULL);
    CuAssertIntEquals(tc, 1, r);

    aug_close(aug);
}

static void testReloadDeletedMeta(CuTest *tc) {
    augeas *aug = NULL;
    int r;

    aug = setup_writable_hosts(tc);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    /* Unsaved changes are discarded */
    r = aug_rm(aug, "/augeas/files/etc/hosts");
    CuAssertPositive(tc, r);

    r = aug_set(aug, "/files/etc/hosts/1/ipaddr", "127.0.0.2");
    CuAssertRetSuccess(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/files/etc/hosts/1[ipaddr = '127.0.0.1']", NULL);
    CuAssertIntEquals(tc, 1, r);

    aug_close(aug);
}

/* BZ 613967 - segfault when reloading a file that has been externally
 * modified, and we have a variable pointing into the old tree
 */
static void testReloadExternalMod(CuTest *tc) {
    augeas *aug = NULL;
    int r, created;
    const char *aug_root, *s;
    char *mtime;

    aug = setup_writable_hosts(tc);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_get(aug, "/augeas/files/etc/hosts/mtime", &s);
    CuAssertIntEquals(tc, 1, r);
    mtime = strdup(s);
    CuAssertPtrNotNull(tc, mtime);

    /* Set up a new entry and save */
    r = aug_defnode(aug, "new", "/files/etc/hosts/3", NULL, &created);
    CuAssertIntEquals(tc, 1, r);
    CuAssertIntEquals(tc, 1, created);

    r = aug_set(aug, "$new/ipaddr", "172.31.42.1");
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "$new/canonical", "new.example.com");
    CuAssertRetSuccess(tc, r);

    r = aug_save(aug);
    CuAssertRetSuccess(tc, r);

    /* Fake the mtime to be old */
    r = aug_set(aug, "/augeas/files/etc/hosts/mtime", mtime);
    CuAssertRetSuccess(tc, r);

    /* Now modify the file outside of Augeas */
    r = aug_get(aug, "/augeas/root", &aug_root);
    CuAssertIntEquals(tc, 1, r);

    run(tc, "sed -e '1,2d' %setc/hosts > %setc/hosts.new", aug_root, aug_root);
    run(tc, "mv %setc/hosts.new %setc/hosts", aug_root, aug_root);

    /* Reload and save again */
    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_save(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/files/etc/hosts/#comment", NULL);
    CuAssertIntEquals(tc, 2, r);

    r = aug_match(aug, "/files/etc/hosts/*", NULL);
    CuAssertIntEquals(tc, 5, r);
}

/* Bug #259 - after save with /augeas/save = newfile, make sure we discard
 * changes and reload files.
 */
static void testReloadAfterSaveNewfile(CuTest *tc) {
    augeas *aug = NULL;
    int r;

    aug = setup_writable_hosts(tc);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "/augeas/save", "newfile");
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "/files/etc/hosts/1/ipaddr", "127.0.0.2");
    CuAssertRetSuccess(tc, r);

    r = aug_save(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/files/etc/hosts/1[ipaddr = '127.0.0.1']", NULL);
    CuAssertIntEquals(tc, 1, r);

    aug_close(aug);
}

/* Make sure parse errors from applying a lens to a file that does not
 * match get reported under /augeas//error
 *
 * Tests bug #138
 */
static void testParseErrorReported(CuTest *tc) {
    augeas *aug = NULL;
    int nmatches, r;

    aug = aug_init(root, loadpath, AUG_NO_MODL_AUTOLOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_set(aug, "/augeas/load/Bad/lens", "Yum.lns");
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "/augeas/load/Bad/incl", "/etc/fstab");
    CuAssertRetSuccess(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    nmatches = aug_match(aug, "/augeas/files/etc/fstab/error", NULL);
    CuAssertIntEquals(tc, 1, nmatches);

    aug_close(aug);
}

/* Test failed file opening is reported, e.g. EACCES */
static void testPermsErrorReported(CuTest *tc) {
    augeas *aug = NULL;
    int r;
    const char *s;

    aug = setup_unreadable_hosts(tc);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/files/etc/hosts", NULL);
    CuAssertIntEquals(tc, 0, r);

    r = aug_get(aug, "/augeas/files/etc/hosts/error", &s);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, "read_failed", s);

    r = aug_get(aug, "/augeas/files/etc/hosts/error/message", &s);
    CuAssertIntEquals(tc, 1, r);
}

/* Test bug #252 - excl patterns have no effect when loading with a root */
static void testLoadExclWithRoot(CuTest *tc) {
    augeas *aug = NULL;
    static const char *const cmds =
        "set /augeas/context /augeas/load\n"
        "set Hosts/lens Hosts.lns\n"
        "set Hosts/incl /etc/hosts\n"
        "set Fstab/lens Fstab.lns\n"
        "set Fstab/incl /etc/ho*\n"
        "set Fstab/excl /etc/hosts\n"
        "load";
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_MODL_AUTOLOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_srun(aug, stderr, cmds);
    CuAssertIntEquals(tc, 7, r);

    r = aug_match(aug, "/augeas//error", NULL);
    CuAssertIntEquals(tc, 0, r);
}

/* Test excl patterns matching the end of a filename work, e.g. *.bak */
static void testLoadTrailingExcl(CuTest *tc) {
    augeas *aug = NULL;
    static const char *const cmds =
        "set /augeas/context /augeas/load/Shellvars\n"
        "set lens Shellvars.lns\n"
        "set incl /etc/sysconfig/network-scripts/ifcfg-lo*\n"
        "set excl *.rpmsave\n"
        "load";
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_MODL_AUTOLOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_srun(aug, stderr, cmds);
    CuAssertIntEquals(tc, 5, r);

    r = aug_match(aug, "/augeas/files/etc/sysconfig/network-scripts/ifcfg-lo", NULL);
    CuAssertIntEquals(tc, 1, r);

    r = aug_match(aug, "/augeas/files/etc/sysconfig/network-scripts/ifcfg-lo.rpmsave", NULL);
    CuAssertIntEquals(tc, 0, r);
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
    SUITE_ADD_TEST(suite, testReloadChanged);
    SUITE_ADD_TEST(suite, testReloadDirty);
    SUITE_ADD_TEST(suite, testReloadDeleted);
    SUITE_ADD_TEST(suite, testReloadDeletedMeta);
    SUITE_ADD_TEST(suite, testReloadExternalMod);
    SUITE_ADD_TEST(suite, testReloadAfterSaveNewfile);
    SUITE_ADD_TEST(suite, testParseErrorReported);
    SUITE_ADD_TEST(suite, testPermsErrorReported);
    SUITE_ADD_TEST(suite, testLoadExclWithRoot);
    SUITE_ADD_TEST(suite, testLoadTrailingExcl);

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
