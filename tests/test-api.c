/*
 * test-api.c: test public API functions for conformance
 *
 * Copyright (C) 2009-2016 David Lutterkort
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

#include <unistd.h>
#include <libgen.h>

#include <libxml/tree.h>

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
    const char *label;
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

    /* aug_label returns 1 and the label if exactly one node matches */
    r = aug_label(aug, "/augeas/version/save/*[1]", &label);
    CuAssertIntEquals(tc, 1, r);
    CuAssertPtrNotNull(tc, label);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* aug_label returns 0 and no label when no node matches */
    r = aug_label(aug, "/augeas/version/save/*[ last() + 1 ]", &label);
    CuAssertIntEquals(tc, 0, r);
    CuAssertPtrEquals(tc, NULL, label);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* aug_label should return an error when multiple nodes match */
    r = aug_label(aug, "/augeas/version/save/*", &label);
    CuAssertIntEquals(tc, -1, r);
    CuAssertPtrEquals(tc, NULL, label);
    CuAssertIntEquals(tc, AUG_EMMATCH, aug_error(aug));

    /* augeas should prepend context if relative path given */
    r = aug_set(aug, "/augeas/context", "/augeas/version");
    r = aug_get(aug, "save/*[1]", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* augeas should still work with an empty context */
    r = aug_set(aug, "/augeas/context", "");
    r = aug_get(aug, "/augeas/version", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* augeas should ignore trailing slashes in context */
    r = aug_set(aug, "/augeas/context", "/augeas/version/");
    r = aug_get(aug, "save/*[1]", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* augeas should create non-existent context path */
    r = aug_set(aug, "/augeas/context", "/context/foo");
    r = aug_set(aug, "bar", "value");
    r = aug_get(aug, "/context/foo/bar", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* aug_get should set VALUE to NULL even if the path expression is invalid
       Issue #372 */
    value = (const char *) 7;
    r = aug_get(aug, "[invalid path]", &value);
    CuAssertIntEquals(tc, -1, r);
    CuAssertPtrEquals(tc, NULL, value);

    aug_close(aug);
}

static void testSet(CuTest *tc) {
    int r;
    const char *value;
    struct augeas *aug;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* aug_set returns 0 for a simple set */
    r = aug_set(aug, "/augeas/testSet", "foo");
    CuAssertIntEquals(tc, 0, r);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* aug_set returns -1 when cannot set due to multiple nodes */
    r = aug_set(aug, "/augeas/version/save/*", "foo");
    CuAssertIntEquals(tc, -1, r);
    CuAssertIntEquals(tc, AUG_EMMATCH, aug_error(aug));

    /* aug_set is able to set the context, even when currently invalid */
    r = aug_set(aug, "/augeas/context", "( /files | /augeas )");
    CuAssertIntEquals(tc, 0, r);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));
    r = aug_get(aug, "/augeas/version", &value);
    CuAssertIntEquals(tc, -1, r);
    CuAssertIntEquals(tc, AUG_EMMATCH, aug_error(aug));
    r = aug_set(aug, "/augeas/context", "/files");
    CuAssertIntEquals(tc, 0, r);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

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

    aug_close(aug);
}

/* Check that defining a variable leads to a corresponding entry in
 * /augeas/variables and that that entry disappears when the variable is
 * undefined */
static void testDefVarMeta(CuTest *tc) {
    int r;
    struct augeas *aug;
    static const char *const expr = "/augeas/version/save/mode";
    const char *value;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    r = aug_defvar(aug, "var", expr);
    CuAssertIntEquals(tc, 4, r);

    r = aug_match(aug, "/augeas/variables/*", NULL);
    CuAssertIntEquals(tc, 1, r);

    r = aug_get(aug, "/augeas/variables/var", &value);
    CuAssertStrEquals(tc, expr, value);

    r = aug_defvar(aug, "var", NULL);
    CuAssertIntEquals(tc, 0, r);

    r = aug_match(aug, "/augeas/variables/*", NULL);
    CuAssertIntEquals(tc, 0, r);

    aug_close(aug);
}

/* Check that defining a variable with defnode leads to a corresponding
 * entry in /augeas/variables and that that entry disappears when the
 * variable is undefined
 */
static void testDefNodeExistingMeta(CuTest *tc) {
    int r, created;
    struct augeas *aug;
    static const char *const expr = "/augeas/version/save/mode";
    const char *value;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    r = aug_defnode(aug, "var", expr, "other", &created);
    CuAssertIntEquals(tc, 4, r);
    CuAssertIntEquals(tc, 0, created);

    r = aug_match(aug, "/augeas/variables/*", NULL);
    CuAssertIntEquals(tc, 1, r);

    r = aug_get(aug, "/augeas/variables/var", &value);
    CuAssertStrEquals(tc, expr, value);

    r = aug_defvar(aug, "var", NULL);
    CuAssertIntEquals(tc, 0, r);

    r = aug_match(aug, "/augeas/variables/*", NULL);
    CuAssertIntEquals(tc, 0, r);

    aug_close(aug);
}

/* Check that defining a variable with defnode leads to a corresponding
 * entry in /augeas/variables and that that entry disappears when the
 * variable is undefined
 */
static void testDefNodeCreateMeta(CuTest *tc) {
    int r, created;
    struct augeas *aug;
    static const char *const expr = "/augeas/version/save/mode[last()+1]";
    static const char *const expr_can = "/augeas/version/save/mode[5]";
    const char *value;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    r = aug_defnode(aug, "var", expr, "other", &created);
    CuAssertIntEquals(tc, 1, r);
    CuAssertIntEquals(tc, 1, created);

    r = aug_match(aug, "/augeas/variables/*", NULL);
    CuAssertIntEquals(tc, 1, r);

    r = aug_get(aug, "/augeas/variables/var", &value);
    CuAssertStrEquals(tc, expr_can, value);

    r = aug_defvar(aug, "var", NULL);
    CuAssertIntEquals(tc, 0, r);

    r = aug_match(aug, "/augeas/variables/*", NULL);
    CuAssertIntEquals(tc, 0, r);

    aug_close(aug);
}

static void reset_indexes(uint *a, uint *b, uint *c, uint *d, uint *e, uint *f) {
    *a = 0; *b = 0; *c = 0; *d = 0; *e = 0; *f = 0;
}

#define SPAN_TEST_DEF_LAST { .expr = NULL, .ls = 0, .le = 0, \
        .vs = 0, .ve = 0, .ss = 0, .se = 0 }

struct span_test_def {
    const char *expr;
    const char *f;
    int ret;
    int ls;
    int le;
    int vs;
    int ve;
    int ss;
    int se;
};

static const struct span_test_def span_test[] = {
    { .expr = "/files/etc/hosts/1/ipaddr", .f = "hosts", .ret = 0, .ls = 0, .le = 0, .vs = 104, .ve = 113, .ss = 104, .se = 113 },
    { .expr = "/files/etc/hosts/1", .f = "hosts", .ret = 0, .ls = 0, .le = 0, .vs = 0, .ve = 0, .ss = 104, .se = 171 },
    { .expr = "/files/etc/hosts/*[last()]", .f = "hosts", .ret = 0, .ls = 0, .le = 0, .vs = 0, .ve = 0, .ss = 266, .se = 309 },
    { .expr = "/files/etc/hosts/#comment[2]", .f = "hosts", .ret = 0, .ls = 0, .le = 0, .vs = 58, .ve = 103, .ss = 56, .se = 104 },
    { .expr = "/files/etc/hosts", .f = "hosts", .ret = 0, .ls = 0, .le = 0, .vs = 0, .ve = 0, .ss = 0, .se = 309 },
    { .expr = "/files", .f = NULL, .ret = -1, .ls = 0, .le = 0, .vs = 0, .ve = 0, .ss = 0, .se = 0 },
    { .expr = "/random", .f = NULL, .ret = -1, .ls = 0, .le = 0, .vs = 0, .ve = 0, .ss = 0, .se = 0 },
    SPAN_TEST_DEF_LAST
};

static void testNodeInfo(CuTest *tc) {
    int ret;
    int i = 0;
    struct augeas *aug;
    struct span_test_def test;
    char *fbase;
    char msg[1024];
    static const char *const expr = "/files/etc/hosts/1/ipaddr";

    char *filename_ac;
    uint label_start, label_end, value_start, value_end, span_start, span_end;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD|AUG_ENABLE_SPAN);
    ret = aug_load(aug);
    CuAssertRetSuccess(tc, ret);

    while(span_test[i].expr != NULL) {
        test = span_test[i];
        i++;
        ret = aug_span(aug, test.expr, &filename_ac, &label_start, &label_end,
                     &value_start, &value_end, &span_start, &span_end);
        sprintf(msg, "span_test %d ret\n", i);
        CuAssertIntEquals_Msg(tc, msg, test.ret, ret);
        sprintf(msg, "span_test %d label_start\n", i);
        CuAssertIntEquals_Msg(tc, msg, test.ls, label_start);
        sprintf(msg, "span_test %d label_end\n", i);
        CuAssertIntEquals_Msg(tc, msg, test.le, label_end);
        sprintf(msg, "span_test %d value_start\n", i);
        CuAssertIntEquals_Msg(tc, msg, test.vs, value_start);
        sprintf(msg, "span_test %d value_end\n", i);
        CuAssertIntEquals_Msg(tc, msg, test.ve, value_end);
        sprintf(msg, "span_test %d span_start\n", i);
        CuAssertIntEquals_Msg(tc, msg, test.ss, span_start);
        sprintf(msg, "span_test %d span_end\n", i);
        CuAssertIntEquals_Msg(tc, msg, test.se, span_end);
        if (filename_ac != NULL) {
            fbase = basename(filename_ac);
        } else {
            fbase = NULL;
        }
        sprintf(msg, "span_test %d filename\n", i);
        CuAssertStrEquals_Msg(tc, msg, test.f, fbase);
        free(filename_ac);
        filename_ac = NULL;
        reset_indexes(&label_start, &label_end, &value_start, &value_end,
                      &span_start, &span_end);
    }

    /* aug_span returns -1 and when no node matches */
    ret = aug_span(aug, "/files/etc/hosts/*[ last() + 1 ]", &filename_ac,
            &label_start, &label_end, &value_start, &value_end,
            &span_start, &span_end);
    CuAssertIntEquals(tc, -1, ret);
    CuAssertPtrEquals(tc, NULL, filename_ac);
    CuAssertIntEquals(tc, AUG_ENOMATCH, aug_error(aug));

    /* aug_span should return an error when multiple nodes match */
    ret = aug_span(aug, "/files/etc/hosts/*", &filename_ac,
            &label_start, &label_end, &value_start, &value_end,
            &span_start, &span_end);
    CuAssertIntEquals(tc, -1, ret);
    CuAssertPtrEquals(tc, NULL, filename_ac);
    CuAssertIntEquals(tc, AUG_EMMATCH, aug_error(aug));

    /* aug_span returns -1 if nodes span are not loaded */
    aug_close(aug);
    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    ret = aug_load(aug);
    CuAssertRetSuccess(tc, ret);
    ret = aug_span(aug, expr, &filename_ac, &label_start, &label_end,
                 &value_start, &value_end, &span_start, &span_end);
    CuAssertIntEquals(tc, -1, ret);
    CuAssertPtrEquals(tc, NULL, filename_ac);
    CuAssertIntEquals(tc, AUG_ENOSPAN, aug_error(aug));
    reset_indexes(&label_start, &label_end, &value_start, &value_end,
                  &span_start, &span_end);

    aug_close(aug);
}

static void testMv(CuTest *tc) {
    struct augeas *aug;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_set(aug, "/a/b/c", "value");
    CuAssertRetSuccess(tc, r);

    r = aug_mv(aug, "/a/b/c", "/a/b/c/d");
    CuAssertIntEquals(tc, -1, r);
    CuAssertIntEquals(tc, AUG_EMVDESC, aug_error(aug));

    aug_close(aug);
}

static void testCp(CuTest *tc) {
    struct augeas *aug;
    int r;
    const char *value;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_set(aug, "/a/b/c", "value");
    CuAssertRetSuccess(tc, r);

    r = aug_cp(aug, "/a/b/c", "/a/b/c/d");
    CuAssertIntEquals(tc, -1, r);
    CuAssertIntEquals(tc, AUG_ECPDESC, aug_error(aug));

    // Copy recursive tree with empty label
    r = aug_cp(aug, "/files/etc/logrotate.d/rpm/rule/create", "/files/etc/logrotate.d/acpid/rule/create");
    CuAssertRetSuccess(tc, r);

    // Check empty label
    r = aug_get(aug, "/files/etc/logrotate.d/rpm/rule/create", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, NULL, value);

    // Check that copies are well separated
    r = aug_set(aug, "/files/etc/logrotate.d/rpm/rule/create/mode", "1234");
    CuAssertRetSuccess(tc, r);
    r = aug_set(aug, "/files/etc/logrotate.d/acpid/rule/create/mode", "5678");
    CuAssertRetSuccess(tc, r);
    r = aug_get(aug, "/files/etc/logrotate.d/rpm/rule/create/mode", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, "1234", value);
    r = aug_get(aug, "/files/etc/logrotate.d/acpid/rule/create/mode", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, "5678", value);

    aug_close(aug);
}


static void testRename(CuTest *tc) {
    struct augeas *aug;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_set(aug, "/a/b/c", "value");
    CuAssertRetSuccess(tc, r);

    r = aug_rename(aug, "/a/b/c", "d");
    CuAssertIntEquals(tc, 1, r);

    r = aug_set(aug, "/a/e/d", "value2");
    CuAssertRetSuccess(tc, r);

    // Multiple rename
    r = aug_rename(aug, "/a//d", "x");
    CuAssertIntEquals(tc, 2, r);

    // Label with a /
    r = aug_rename(aug, "/a/e/x", "a/b");
    CuAssertIntEquals(tc, -1, r);
    CuAssertIntEquals(tc, AUG_ELABEL, aug_error(aug));

    aug_close(aug);
}

static void testToXml(CuTest *tc) {
    struct augeas *aug;
    int r;
    xmlNodePtr xmldoc, xmlnode;
    xmlChar *value;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    r = aug_to_xml(aug, "/files/etc/passwd", &xmldoc, 0);
    CuAssertRetSuccess(tc, r);

    value = xmlGetProp(xmldoc, BAD_CAST "match");
    CuAssertStrEquals(tc, "/files/etc/passwd", (const char *) value);
    xmlFree(value);

    xmlnode = xmlFirstElementChild(xmldoc);
    value = xmlGetProp(xmlnode, BAD_CAST "label");
    CuAssertStrEquals(tc, "passwd", (const char *) value);
    xmlFree(value);

    value = xmlGetProp(xmlnode, BAD_CAST "path");
    CuAssertStrEquals(tc, "/files/etc/passwd", (const char *) value);
    xmlFree(value);

    xmlnode = xmlFirstElementChild(xmlnode);
    value = xmlGetProp(xmlnode, BAD_CAST "label");
    CuAssertStrEquals(tc, "root", (const char *) value);
    xmlFree(value);
    xmlFreeNode(xmldoc);

    /* Bug #239 */
    r = aug_set(aug, "/augeas/context", "/files/etc/passwd");
    CuAssertRetSuccess(tc, r);
    r = aug_to_xml(aug, ".", &xmldoc, 0);
    CuAssertRetSuccess(tc, r);
    xmlnode = xmlFirstElementChild(xmldoc);
    value = xmlGetProp(xmlnode, BAD_CAST "label");
    CuAssertStrEquals(tc, "passwd", (const char *) value);
    xmlFree(value);

    xmlFreeNode(xmldoc);
    aug_close(aug);
}

static void testTextStore(CuTest *tc) {
    static const char *const hosts = "192.168.0.1 rtr.example.com router\n";
    /* Not acceptable for Hosts.lns - missing canonical and \n */
    static const char *const hosts_bad = "192.168.0.1";
    const char *v;

    struct augeas *aug;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_set(aug, "/raw/hosts", hosts);
    CuAssertRetSuccess(tc, r);

    r = aug_text_store(aug, "Hosts.lns", "/raw/hosts", "/t1");
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/t1/*", NULL);
    CuAssertIntEquals(tc, 1, r);

    /* Test bad lens name */
    r = aug_text_store(aug, "Notthere.lns", "/raw/hosts", "/t2");
    CuAssertIntEquals(tc, -1, r);
    CuAssertIntEquals(tc, AUG_ENOLENS, aug_error(aug));

    r = aug_match(aug, "/t2", NULL);
    CuAssertIntEquals(tc, 0, r);

    /* Test parse error */
    r = aug_set(aug, "/raw/hosts_bad", hosts_bad);
    CuAssertRetSuccess(tc, r);

    r = aug_text_store(aug, "Hosts.lns", "/raw/hosts_bad", "/t3");
    CuAssertIntEquals(tc, -1, r);

    r = aug_match(aug, "/t3", NULL);
    CuAssertIntEquals(tc, 0, r);

    r = aug_get(aug, "/augeas/text/t3/error", &v);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, "parse_failed", v);

    r = aug_text_store(aug, "Hosts.lns", "/raw/hosts", "/t3");
    CuAssertRetSuccess(tc, r);

    r = aug_match(aug, "/augeas/text/t3/error", NULL);
    CuAssertIntEquals(tc, 0, r);

    /* Test invalid PATH */
    r = aug_text_store(aug, "Hosts.lns", "/raw/hosts", "[garbage]");
    CuAssertIntEquals(tc, -1, r);
    CuAssertIntEquals(tc, AUG_EPATHX, aug_error(aug));

    r = aug_match(aug, "/t2", NULL);
    CuAssertIntEquals(tc, 0, r);

    aug_close(aug);
}

static void testTextRetrieve(CuTest *tc) {
    static const char *const hosts = "192.168.0.1 rtr.example.com router\n";
    const char *hosts_out;
    struct augeas *aug;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_set(aug, "/raw/hosts", hosts);
    CuAssertRetSuccess(tc, r);

    r = aug_text_store(aug, "Hosts.lns", "/raw/hosts", "/t1");
    CuAssertRetSuccess(tc, r);

    r = aug_text_retrieve(aug, "Hosts.lns", "/raw/hosts", "/t1", "/out/hosts");
    CuAssertRetSuccess(tc, r);

    r = aug_get(aug, "/out/hosts", &hosts_out);
    CuAssertIntEquals(tc, 1, r);

    CuAssertStrEquals(tc, hosts, hosts_out);

    aug_close(aug);
}

static void testAugEscape(CuTest *tc) {
    static const char *const in  = "a/[]b|=c()!, \td";
    static const char *const exp = "a\\/\\[\\]b\\|\\=c\\(\\)\\!\\,\\ \\\td";
    char *out;
    struct augeas *aug;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_escape_name(aug, in, &out);
    CuAssertRetSuccess(tc, r);

    CuAssertStrEquals(tc, out, exp);
    free(out);

    aug_close(aug);
}

static void testRm(CuTest *tc) {
    struct augeas *aug;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_MODL_AUTOLOAD);
    CuAssertPtrNotNull(tc, aug);

    r = aug_set(aug, "/files/1/2/3/4/5", "1");
    CuAssertRetSuccess(tc, r);

    r = aug_rm(aug, "/files//*");
    CuAssertIntEquals(tc, 5, r);

    aug_close(aug);
}

static void testLoadFile(CuTest *tc) {
    struct augeas *aug;
    const char *value;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    /* augeas should load a single file */
    r = aug_load_file(aug, "/etc/fstab");
    CuAssertRetSuccess(tc, r);
    r = aug_get(aug, "/files/etc/fstab/1/vfstype", &value);
    CuAssertIntEquals(tc, 1, r);
    CuAssertPtrNotNull(tc, value);

    /* Only one file should be loaded */
    r = aug_match(aug, "/files/etc/*", NULL);
    CuAssertIntEquals(tc, 1, r);

    /* augeas should return an error when no lens can be found for a file */
    r = aug_load_file(aug, "/etc/unknown.conf");
    CuAssertIntEquals(tc, -1, r);
    CuAssertIntEquals(tc, AUG_ENOLENS, aug_error(aug));

    /* augeas should return without an error when trying to load a
       nonexistent file that would be handled by a lens */
    r = aug_load_file(aug, "/etc/mtab");
    CuAssertRetSuccess(tc, r);
    r = aug_match(aug, "/files/etc/mtab", NULL);
    CuAssertIntEquals(tc, 0, r);

    aug_close(aug);
}

/* Make sure that if somebody erroneously creates a node
   /augeas/files/path, we do not corrupt the tree. It used to be that
   having such a node would free /augeas/files
*/
static void testLoadBadPath(CuTest *tc) {
    struct augeas *aug;
    int r;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    r = aug_set(aug, "/augeas/files/path", "/files");
    CuAssertRetSuccess(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    aug_close(aug);
}

/* If a lens is set to a partial path which happens to actually resolve to
   a file when appended to the loadpath, we went into an infinite loop of
   loading a module, but then not realizing that it had actually been
   loaded, reloading it over and over again.
   See https://github.com/hercules-team/augeas/issues/522
*/
static void testLoadBadLens(CuTest *tc) {
    struct augeas *aug;
    int r;
    char *lp;

    // This setup depends on the fact that
    //   loadpath == abs_top_srcdir + "/lenses"
    r = asprintf(&lp, "%s:%s", loadpath, abs_top_srcdir);
    CuAssert(tc, "failed to allocate loadpath", (r >= 0));

    aug = aug_init(root, lp, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    r = aug_set(aug, "/augeas/load/Fstab/lens", "lenses/Fstab.lns");
    CuAssertRetSuccess(tc, r);

    r = aug_load(aug);
    CuAssertRetSuccess(tc, r);

    // We used to record the error to load the lens above against every
    // lens that we tried to load after it.
    r = aug_match(aug, "/augeas//error", NULL);
    CuAssertIntEquals(tc, 1, r);

    free(lp);
}

/* Test the aug_ns_* functions */
static void testAugNs(CuTest *tc) {
    struct augeas *aug;
    int r;
    const char *v, *l;
    char *s;

    aug = aug_init(root, loadpath, AUG_NO_STDINC|AUG_NO_LOAD);
    CuAssertPtrNotNull(tc, aug);
    CuAssertIntEquals(tc, AUG_NOERROR, aug_error(aug));

    r = aug_load_file(aug, "/etc/hosts");
    CuAssertIntEquals(tc, 0, r);

    r = aug_defvar(aug, "matches",
                   "/files/etc/hosts/*[label() != '#comment']/ipaddr");
    CuAssertIntEquals(tc, 2, r);

    r = aug_ns_attr(aug, "matches", 0, &v, &l, &s);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, "127.0.0.1", v);
    CuAssertStrEquals(tc, "ipaddr", l);
    CuAssertStrEquals(tc, "/files/etc/hosts", s);
    free(s);
    s = NULL;

    r = aug_ns_attr(aug, "matches", 0, NULL, NULL, NULL);
    CuAssertIntEquals(tc, 1, r);

    r = aug_ns_attr(aug, "matches", 1, &v, NULL, &s);
    CuAssertIntEquals(tc, 1, r);
    CuAssertStrEquals(tc, "172.31.122.14", v);
    CuAssertStrEquals(tc, "/files/etc/hosts", s);
    free(s);
    s = NULL;

    r = aug_ns_attr(aug, "matches", 2, &v, &l, &s);
    CuAssertIntEquals(tc, -1, r);
    CuAssertPtrEquals(tc, NULL, v);
    CuAssertPtrEquals(tc, NULL, l);
    CuAssertPtrEquals(tc, NULL, s);
    free(s);
    s = NULL;

    aug_close(aug);
}

int main(void) {
    char *output = NULL;
    CuSuite* suite = CuSuiteNew();
    CuSuiteSetup(suite, NULL, NULL);

    SUITE_ADD_TEST(suite, testGet);
    SUITE_ADD_TEST(suite, testSet);
    SUITE_ADD_TEST(suite, testSetM);
    SUITE_ADD_TEST(suite, testDefVarMeta);
    SUITE_ADD_TEST(suite, testDefNodeExistingMeta);
    SUITE_ADD_TEST(suite, testDefNodeCreateMeta);
    SUITE_ADD_TEST(suite, testNodeInfo);
    SUITE_ADD_TEST(suite, testMv);
    SUITE_ADD_TEST(suite, testCp);
    SUITE_ADD_TEST(suite, testRename);
    SUITE_ADD_TEST(suite, testToXml);
    SUITE_ADD_TEST(suite, testTextStore);
    SUITE_ADD_TEST(suite, testTextRetrieve);
    SUITE_ADD_TEST(suite, testAugEscape);
    SUITE_ADD_TEST(suite, testRm);
    SUITE_ADD_TEST(suite, testLoadFile);
    SUITE_ADD_TEST(suite, testLoadBadPath);
    SUITE_ADD_TEST(suite, testLoadBadLens);
    SUITE_ADD_TEST(suite, testAugNs);

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
