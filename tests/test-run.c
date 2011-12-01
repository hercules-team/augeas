/*
 * test-run.c: test the aug_srun API function
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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "augeas.h"

#include "cutest.h"
#include "internal.h"
#include <memory.h>

#include <unistd.h>

static const char *abs_top_srcdir;

#define KW_TEST "test"
#define KW_PRINTS "prints"
#define KW_SOMETHING "something"

/* This array needs to be kept in sync with aug_errcode_t, and
 * the entries need to be in the same order as in that enum; used
 * by errtok
 */
static const char *const errtokens[] = {
    "NOERROR", "ENOMEM", "EINTERNAL", "EPATHX", "ENOMATCH",
    "EMMATCH", "ESYNTAX", "ENOLENS", "EMXFM", "ENOSPAN",
    "EMVDESC", "ECMDRUN"
};

struct test {
    struct test *next;
    char *name;
    int  result;
    int  errcode;
    char *cmd;
    char *out;
    bool  out_present;
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

static char *inttok(char *s, int *tok) {
    char *t = skipws(s);
    s = findws(t);
    *tok = strtol(t, NULL, 10);
    return s;
}

static char *errtok(char *s, int *err) {
    char *t = skipws(s);
    s = findws(t);
    if (s == t) {
        *err = AUG_NOERROR;
        return s;
    }
    for (*err = 0; *err < ARRAY_CARDINALITY(errtokens); *err += 1) {
        const char *e = errtokens[*err];
        if (strlen(e) == s - t && STREQLEN(e, t, s - t))
            return s;
    }
    fprintf(stderr, "errtok: '%s'\n", t);
    die("unknown error code");
}

static bool looking_at(const char *s, const char *kw) {
    return STREQLEN(s, kw, strlen(kw));
}

static struct test *read_tests(void) {
    char *fname;
    FILE *fp;
    char line[BUFSIZ];
    struct test *result = NULL, *t = NULL;
    int lc = 0;
    bool append_cmd = true;

    if (asprintf(&fname, "%s/tests/run.tests", abs_top_srcdir) < 0)
        die("asprintf fname");

    if ((fp = fopen(fname, "r")) == NULL)
        die("fopen run.tests");

    while (fgets(line, BUFSIZ, fp) != NULL) {
        lc += 1;
        char *s = skipws(line);
        if (*s == '#' || *s == '\0')
            continue;
        if (*s == ':')
            s += 1;
        char *eos = s + strlen(s) - 2;
        if (eos >= s && *eos == ':') {
            *eos++ = '\n';
            *eos = '\0';
        }

        if (looking_at(s, KW_TEST)) {
            if (ALLOC(t) < 0)
                die_oom();
            list_append(result, t);
            append_cmd = true;
            s = token(s + strlen(KW_TEST), &(t->name));
            s = inttok(s, &t->result);
            s = errtok(s, &t->errcode);
        } else if (looking_at(s, KW_PRINTS)) {
            s = skipws(s + strlen(KW_PRINTS));
            t->out_present = looking_at(s, KW_SOMETHING);
            append_cmd = false;
        } else {
            char **buf = append_cmd ? &(t->cmd) : &(t->out);
            if (*buf == NULL) {
                *buf = strdup(s);
                if (*buf == NULL)
                    die_oom();
            } else {
                if (REALLOC_N(*buf, strlen(*buf) + strlen(s) + 1) < 0)
                    die_oom();
                strcat(*buf, s);
            }
        }
        if (t->out != NULL)
            t->out_present = true;
    }
    return result;
}

#define fail(cond, msg ...)                     \
    if (cond) {                                 \
        printf("FAIL (");                       \
        fprintf(stdout, ## msg);                \
        printf(")\n");                          \
        goto error;                             \
    }

static int run_one_test(struct test *test) {
    int r;
    struct augeas *aug = NULL;
    struct memstream ms;
    int result = 0;

    aug = aug_init("/dev/null", NULL, AUG_NO_STDINC|AUG_NO_LOAD);
    fail(aug == NULL, "aug_init");
    fail(aug_error(aug) != AUG_NOERROR, "aug_init: errcode was %d",
         aug_error(aug));

    printf("%-30s ... ", test->name);

    r = init_memstream(&ms);
    fail(r < 0, "init_memstream");

    r = aug_srun(aug, ms.stream, test->cmd);
    fail(r != test->result, "return value: expected %d, actual %d",
         test->result, r);
    fail(aug_error(aug) != test->errcode, "errcode: expected %s, actual %s",
         errtokens[test->errcode], errtokens[aug_error(aug)]);

    r = close_memstream(&ms);
    fail(r < 0, "close_memstream");
    fail(ms.buf == NULL, "close_memstream left buf NULL");

    if (test->out != NULL) {
        fail(STRNEQ(ms.buf, test->out), "output: expected '%s', actual '%s'",
             test->out, ms.buf);
    } else if (test->out_present) {
        fail(strlen(ms.buf) == 0,
             "output: expected some output");
    } else {
        fail(strlen(ms.buf) > 0,
             "output: expected nothing, actual '%s'", ms.buf);
    }
    printf("PASS\n");

 done:
    free(ms.buf);
    aug_close(aug);
    return result;
 error:
    result = -1;
    goto done;
}

static int run_tests(struct test *tests) {
    int result = EXIT_SUCCESS;

    list_for_each(t, tests) {
        if (run_one_test(t) < 0)
            result = EXIT_FAILURE;
    }
    return result;
}

int main(void) {
    struct test *tests;

    abs_top_srcdir = getenv("abs_top_srcdir");
    if (abs_top_srcdir == NULL)
        die("env var abs_top_srcdir must be set");

    tests = read_tests();
    return run_tests(tests);
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
