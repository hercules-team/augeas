/*
 * test-xpath.c: check that XPath expressions yield the expected result
 *
 * Copyright (C) 2007-2016 David Lutterkort
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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <augeas.h>
#include <internal.h>
#include <memory.h>

#include "cutest.h"

static const char *abs_top_srcdir;
static char *root;

#define KW_TEST "test"

struct entry {
    struct entry *next;
    char *path;
    char *value;
};

struct test {
    struct test *next;
    char *name;
    char *match;
    struct entry *entries;
};

#define die(msg)                                                    \
    do {                                                            \
        fprintf(stderr, "%d: Fatal error: %s\n", __LINE__, msg);    \
        exit(EXIT_FAILURE);                                         \
    } while(0)

static char *skipws(char *s) {
    while (isspace(*s)) s++;
    return s;
}

static char *findws(char *s) {
    while (*s && ! isspace(*s)) s++;
    return s;
}

static char *token(char *s, char **tok) {
    char *t = skipws(s);
    s = findws(t);
    *tok = strndup(t, s - t);
    return s;
}

static char *token_to_eol(char *s, char **tok) {
    char *t = skipws(s);
    while (*s && *s != '\n') s++;
    *tok = strndup(t, s - t);
    return s;
}

static char *findpath(char *s, char **p) {
    char *t = skipws(s);

    while (*s && *s != '=') s++;
    if (s > t) {
        s -= 1;
        while (*s && isspace(*s)) s -= 1;
        s += 1;
    }
    *p = strndup(t, s - t);
    return s;
}

static void free_tests(struct test *test) {
    while (test != NULL) {
        struct test *del = test;
        test = test->next;
        struct entry *entry = del->entries;
        while (entry != NULL) {
            struct entry *e = entry;
            entry = entry->next;
            free(e->path);
            free(e->value);
            free(e);
        }
        free(del->name);
        free(del->match);
        free(del);
    }
}

static struct test *read_tests(void) {
    char *fname;
    FILE *fp;
    char line[BUFSIZ];
    struct test *result = NULL, *t = NULL;
    int lc = 0;

    if (asprintf(&fname, "%s/tests/xpath.tests", abs_top_srcdir) < 0)
        die("asprintf fname");

    if ((fp = fopen(fname, "r")) == NULL)
        die("fopen xpath.tests");

    while (fgets(line, BUFSIZ, fp) != NULL) {
        lc += 1;
        char *s = skipws(line);
        if (*s == '#' || *s == '\0')
            continue;
        if (STREQLEN(s, KW_TEST, strlen(KW_TEST))) {
            if (ALLOC(t) < 0)
                die("out of memory");
            list_append(result, t);
            s = token(s + strlen(KW_TEST), &(t->name));
            s = token_to_eol(s, &(t->match));
        } else {
            struct entry *e = NULL;
            if (ALLOC(e) < 0)
                die("out of memory");
            list_append(t->entries, e);
            s = findpath(s, &(e->path));
            s = skipws(s);
            if (*s) {
                if (*s != '=') {
                    fprintf(stderr,
                     "line %d: either list only a path or path = value\n", lc);
                    die("xpath.tests has incorrect format");
                }
                s = token_to_eol(s + 1, &(e->value));
            }
        }
        s = skipws(s);
        if (*s != '\0') {
            fprintf(stderr, "line %d: junk at end of line\n", lc);
            die("xpath.tests has incorrect format");
        }
    }
    fclose(fp);
    free(fname);
    return result;
}

static void print_pv(const char *path, const char *value) {
    if (value)
        printf("    %s = %s\n", path, value);
    else
        printf("    %s\n", path);
}

static int has_match(const char *path, char **matches, int nmatches) {
    int found = 0;
    for (int i=0; i < nmatches; i++) {
        if (matches[i] != NULL && STREQ(path, matches[i])) {
            found = 1;
            break;
        }
    }
    return found;
}

static int run_one_test(struct augeas *aug, struct test *t) {
    int nexp = 0, nact;
    char **matches;
    int result = 0;

    printf("%-30s ... ", t->name);
    list_for_each(e, t->entries)
        nexp++;
    nact = aug_match(aug, t->match, &matches);
    if (nact != nexp) {
        result = -1;
    } else {
        struct entry *e;
        const char *val;

        for (e = t->entries; e != NULL; e = e->next) {
            if (! has_match(e->path, matches, nact))
                result = -1;
            if (! streqv(e->value, "...")) {
                aug_get(aug, e->path, &val);
                if (!streqv(e->value, val))
                    result = -1;
            }
        }
    }
    if (result == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");

        printf("  Match: %s\n", t->match);
        printf("  Expected: %d entries\n", nexp);
        list_for_each(e, t->entries) {
            print_pv(e->path, e->value);
        }
        if (nact < 0) {
            printf("  Actual: aug_match failed\n");
            } else {
            printf("  Actual: %d entries\n", nact);
        }
        for (int i=0; i < nact; i++) {
            const char *val;
            aug_get(aug, matches[i], &val);
            print_pv(matches[i], val);
        }
    }
    for (int i=0; i < nact; i++) {
        free(matches[i]);
    }
    free(matches);
    return result;
}

static int test_rm_var(struct augeas *aug) {
    int r;

    printf("%-30s ... ", "rm_var");
    r = aug_defvar(aug, "h", "/files/etc/hosts/2/ipaddr");
    if (r < 0)
        die("aug_defvar failed");

    r = aug_match(aug, "$h", NULL);
    if (r != 1) {
        fprintf(stderr, "expected 1 match, got %d\n", r);
        goto fail;
    }

    r = aug_rm(aug, "/files/etc/hosts/2");
    if (r != 4) {
        fprintf(stderr, "expected 4 nodes removed, got %d\n", r);
        goto fail;
    }

    r = aug_match(aug, "$h", NULL);
    if (r != 0) {
        fprintf(stderr, "expected no match, got %d\n", r);
        goto fail;
    }
    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

static int test_defvar_nonexistent(struct augeas *aug) {
    int r;

    printf("%-30s ... ", "defvar_nonexistent");
    r = aug_defvar(aug, "x", "/foo/bar");
    if (r < 0)
        die("aug_defvar failed");

    r = aug_set(aug, "$x", "baz");
    if (r != -1)
        goto fail;
    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

static int test_defnode_nonexistent(struct augeas *aug) {
    int r, created;

    printf("%-30s ... ", "defnode_nonexistent");
    r = aug_defnode(aug, "x", "/defnode/bar[0 = 1]", "foo", &created);
    if (r != 1)
        die("aug_defnode failed");
    if (created != 1) {
        fprintf(stderr, "defnode did not create a node\n");
        goto fail;
    }
    r = aug_match(aug, "$x", NULL);
    if (r != 1) {
        fprintf(stderr, "$x must have exactly one entry, but has %d\n", r);
        goto fail;
    }

    r = aug_defnode(aug, "x", "/defnode/bar", NULL, &created);
    if (r != 1)
        die("aug_defnode failed");
    if (created != 0) {
        fprintf(stderr, "defnode created node again\n");
        goto fail;
    }

    // FIXME: get values and compare them, too

    r = aug_set(aug, "$x", "baz");
    if (r != 0)
        goto fail;

    r = aug_match(aug, "$x", NULL);
    if (r != 1)
        goto fail;

    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

/* The origin has no siblings (oss-fuzz 42512468) */
static int test_origin_siblings(struct augeas *aug) {
    int r;

    printf("%-30s ... ", "origin_siblings");
    r = aug_match(aug, "/preceding-sibling::*", NULL);
    if (r != 0)
        goto fail;
    r = aug_match(aug, "/following-sibling::*", NULL);
    if (r != 0)
        goto fail;

    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

/* Arithmetic that overflows is an error (oss-fuzz 42519956) */
static int test_arith_overflow(struct augeas *aug) {
    static const char *const exprs[] = {
        "/files[int('9223372036854775807') + 1 = 0]",
        "/files[int('-9223372036854775807') - 2 = 0]",
        "/files[int('224040001001793750') * 111 = 0]"
    };
    int r;

    printf("%-30s ... ", "arith_overflow");
    for (int i=0; i < sizeof(exprs)/sizeof(exprs[0]); i++) {
        r = aug_match(aug, exprs[i], NULL);
        if (r >= 0 || aug_error(aug) != AUG_EPATHX)
            goto fail;
    }

    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

/* Return PREFIX, N copies of OPEN, MIDDLE, N copies of CLOSE */
static char *make_deep_expr(const char *prefix, const char *open,
                            const char *middle, const char *close, int n) {
    size_t len = strlen(prefix) + strlen(middle)
        + n * (strlen(open) + strlen(close)) + 1;
    char *expr = malloc(len);
    if (expr == NULL)
        die("out of memory");
    strcpy(expr, prefix);
    for (int i=0; i < n; i++)
        strcat(expr, open);
    strcat(expr, middle);
    for (int i=0; i < n; i++)
        strcat(expr, close);
    return expr;
}

/* Deeply nested expressions and long chains of operators must be
 * rejected instead of overflowing the stack (oss-fuzz 42512797, 42513857,
 * 511531634, 42512431, 42538268, github #869) */
static int test_depth_limits(struct augeas *aug) {
    static const struct {
        const char *prefix, *open, *middle, *close;
        int n;
        int expected;  /* number of matches, or -1 for an error */
    } cases[] = {
        { "", "(", "/files", ")", 50, 1 },
        { "", "(", "/files", ")", 100000, -1 },
        { "/files", "[*", "", "]", 50, 0 },
        { "/files", "[*", "", "]", 100000, -1 },
        { "/files", "|/files", "", "", 500, 1 },
        { "/files", "|/files", "", "", 100000, -1 },
        { "/files[1", "+1", "]", "", 100000, -1 },
        { "/files[0", " or 0", "]", "", 100000, -1 }
    };
    int r;

    printf("%-30s ... ", "depth_limits");
    for (int i=0; i < sizeof(cases)/sizeof(cases[0]); i++) {
        char *expr = make_deep_expr(cases[i].prefix, cases[i].open,
                                    cases[i].middle, cases[i].close,
                                    cases[i].n);
        r = aug_match(aug, expr, NULL);
        free(expr);
        if (cases[i].expected >= 0 && r != cases[i].expected) {
            fprintf(stderr, "case %d: expected %d matches, got %d\n",
                    i, cases[i].expected, r);
            goto fail;
        }
        if (cases[i].expected < 0 &&
            (r >= 0 || aug_error(aug) != AUG_EPATHX)) {
            fprintf(stderr, "case %d: expected an error, got %d\n", i, r);
            goto fail;
        }
    }

    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

static int test_invalid_regexp(struct augeas *aug) {
    int r;

    printf("%-30s ... ", "invalid_regexp");
    r = aug_match(aug, "/files/*[ * =~ regexp('.*[aeiou')]", NULL);
    if (r >= 0)
        goto fail;

    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

/* Regexps whose expansion by re_compile_pattern would be exponentially
 * large are rejected before they are compiled, as are repetition
 * operators that follow another one. Similar regexps of reasonable size
 * still work */
static int test_regexp_size(struct augeas *aug) {
    static const char *const bad[] = {
        "w++", "w*?", "w?*", "w+{2}", "w{2}*",
        /* 30 levels of ((w)+)+ */
        "((((((((((((((((((((((((((((((w)+)+)+)+)+)+)+)+)+)+)+)+)+)+)+)+)+)+"
        ")+)+)+)+)+)+)+)+)+)+)+)+)+",
        /* 30 levels of ((w){2}){2} */
        "((((((((((((((((((((((((((((((w){2}){2}){2}){2}){2}){2}){2}){2}){2}"
        "){2}){2}){2}){2}){2}){2}){2}){2}){2}){2}){2}){2}){2}){2}){2}){2}){2}"
        "){2}){2}){2}){2}",
        "(a|b{1000}){2000}"
    };
    static const char *const good[] = {
        "w+", "(w+)?", "[+*?]+", "(ab|cd)+x?", "a{1000}", "a{2,}b",
        "((((((((((w)+)+)+)+)+)+)+)+)+)+", "(a|b{100}){100}"
    };
    char *path = NULL;
    int r;

    printf("%-30s ... ", "regexp_size");
    for (int i = 0; i < ARRAY_CARDINALITY(bad); i++) {
        r = asprintf(&path, "/files/*[ * =~ regexp('%s')]", bad[i]);
        if (r < 0)
            goto fail;
        r = aug_match(aug, path, NULL);
        FREE(path);
        if (r >= 0 || aug_error(aug) != AUG_EPATHX) {
            fprintf(stderr, "regexp('%s') was not rejected\n", bad[i]);
            goto fail;
        }
    }
    for (int i = 0; i < ARRAY_CARDINALITY(good); i++) {
        r = asprintf(&path, "/files/*[ * =~ regexp('%s')]", good[i]);
        if (r < 0)
            goto fail;
        r = aug_match(aug, path, NULL);
        FREE(path);
        if (r < 0) {
            fprintf(stderr, "regexp('%s') failed: %s\n", good[i],
                    aug_error_details(aug));
            goto fail;
        }
    }

    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

static int test_wrong_regexp_flag(struct augeas *aug) {
    int r;

    printf("%-30s ... ", "wrong_regexp_flag");
    r = aug_match(aug, "/files/*[ * =~ regexp('abc', 'o')]", NULL);
    if (r >= 0)
        goto fail;

    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

static int test_trailing_ws_in_name(struct augeas *aug) {
    int r;

    printf("%-30s ... ", "trailing_ws_in_name");

    /* We used to incorrectly lop escaped whitespace off the end of a
     * name. Make sure that we really create a tree node with label 'x '
     * with the below set, and look for it in a number of ways to ensure we
     * are not lopping off trailing whitespace. */
    r = aug_set(aug, "/ws\\ ", "1");
    if (r < 0) {
        fprintf(stderr, "failed to set '/ws ': %d\n", r);
        goto fail;
    }
    /* We did not create a node with label 'ws' */
    r = aug_get(aug, "/ws", NULL);
    if (r != 0) {
        fprintf(stderr, "created '/ws' instead: %d\n", r);
        goto fail;
    }

    /* We did not create a node with label 'ws\t' (this also checks that we
     * don't create something like 'ws\\' by dropping the last whitespace
     * character. */
    r = aug_get(aug, "/ws\\\t", NULL);
    if (r != 0) {
        fprintf(stderr, "found '/ws\\t': %d\n", r);
        goto fail;
    }

    /* But we did create 'ws ' */
    r = aug_get(aug, "/ws\\ ", NULL);
    if (r != 1) {
        fprintf(stderr, "could not find '/ws ': %d\n", r);
        goto fail;
    }

    /* If the whitespace is preceded by an even number of '\\' chars,
     * whitespace must be stripped */
    r = aug_set(aug, "/nows\\\\ ", "1");
    if (r < 0) {
        fprintf(stderr, "set of '/nows' failed: %d\n", r);
        goto fail;
    }
    r = aug_get(aug, "/nows\\\\", NULL);
    if (r != 1) {
        fprintf(stderr, "could not get '/nows\\'\n");
        goto fail;
    }
    printf("PASS\n");
    return 0;
 fail:
    printf("FAIL\n");
    return -1;
}

static int run_tests(struct test *tests, int argc, char **argv) {
    char *lensdir;
    struct augeas *aug = NULL;
    int r, result = EXIT_SUCCESS;

    if (asprintf(&lensdir, "%s/lenses", abs_top_srcdir) < 0)
        die("asprintf lensdir failed");

    aug = aug_init(root, lensdir, AUG_NO_STDINC|AUG_SAVE_NEWFILE);
    if (aug == NULL)
        die("aug_init");
    r = aug_defvar(aug, "hosts", "/files/etc/hosts/*");
    if (r != 6)
        die("aug_defvar $hosts");
    r = aug_defvar(aug, "localhost", "'127.0.0.1'");
    if (r != 0)
        die("aug_defvar $localhost");
    r = aug_defvar(aug, "php", "/files/etc/php.ini");
    if (r != 1)
        die("aug_defvar $php");

    list_for_each(t, tests) {
        if (! should_run(t->name, argc, argv))
            continue;
        if (run_one_test(aug, t) < 0)
            result = EXIT_FAILURE;
    }

    if (argc == 0) {
        if (test_rm_var(aug) < 0)
            result = EXIT_FAILURE;

        if (test_defvar_nonexistent(aug) < 0)
            result = EXIT_FAILURE;

        if (test_defnode_nonexistent(aug) < 0)
            result = EXIT_FAILURE;

        if (test_invalid_regexp(aug) < 0)
            result = EXIT_FAILURE;

        if (test_regexp_size(aug) < 0)
            result = EXIT_FAILURE;

        if (test_depth_limits(aug) < 0)
            result = EXIT_FAILURE;

        if (test_origin_siblings(aug) < 0)
            result = EXIT_FAILURE;

        if (test_arith_overflow(aug) < 0)
            result = EXIT_FAILURE;

        if (test_wrong_regexp_flag(aug) < 0)
            result = EXIT_FAILURE;

        if (test_trailing_ws_in_name(aug) < 0)
            result = EXIT_FAILURE;
    }
    aug_close(aug);
    free(lensdir);

    return result;
}

int main(int argc, char **argv) {
    struct test *tests;

    abs_top_srcdir = getenv("abs_top_srcdir");
    if (abs_top_srcdir == NULL)
        die("env var abs_top_srcdir must be set");

    if (asprintf(&root, "%s/tests/root", abs_top_srcdir) < 0) {
        die("failed to set root");
    }

    tests = read_tests();
    int result = run_tests(tests, argc - 1, argv + 1);
    /*
    list_for_each(t, tests) {
        printf("Test %s\n", t->name);
        printf("match %s\n", t->match);
        list_for_each(e, t->entries) {
            if (e->value)
                printf("    %s = %s\n", e->path, e->value);
            else
                printf("    %s\n", e->path);
        }
    }
    */
    free_tests(tests);
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
