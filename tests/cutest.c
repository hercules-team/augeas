/*
 * This code is based on CuTest by Asim Jalis
 * http://sourceforge.net/projects/cutest/
 *
 * The license for the original code is
 * LICENSE
 *
 * Copyright (c) 2003 Asim Jalis
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in
 * a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <config.h>

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "cutest.h"
#include "memory.h"

#define HUGE_STRING_LEN	8192
#define STRING_MAX		256

#define asprintf_or_die(strp, fmt, args ...)                            \
    if (asprintf(strp, fmt, ## args) == -1) {                           \
        fprintf(stderr, "Fatal error (probably out of memory)\n");      \
        abort();                                                        \
    }

void die_oom(void) {
    printf("Ran out of memory. Send more\n");
    exit(2);
}

/*-------------------------------------------------------------------------*
 * CuTest
 *-------------------------------------------------------------------------*/

void CuTestInit(CuTest* t, const char* name, TestFunction function) {
	t->name = strdup(name);
	t->failed = 0;
	t->ran = 0;
	t->message = NULL;
	t->function = function;
	t->jumpBuf = NULL;
}

CuTest* CuTestNew(const char* name, TestFunction function) {
	CuTest* tc = NULL;
    if (ALLOC(tc) < 0)
        die_oom();
	CuTestInit(tc, name, function);
	return tc;
}

void CuTestRun(CuTest* tc, TestFunction setup, TestFunction teardown) {
	jmp_buf buf;

    if (getenv("CUTEST") && STRNEQ(getenv("CUTEST"), tc->name))
        return;
	tc->jumpBuf = &buf;
	if (setjmp(buf) == 0) {
        if (setup)
            (setup)(tc);
		tc->ran = 1;
		(tc->function)(tc);
	}
    if (teardown && setjmp(buf) == 0) {
        (teardown)(tc);
    }
	tc->jumpBuf = 0;
}

static void CuFailInternal(CuTest* tc, const char* file, int line,
                           const char *string) {
	char *buf = NULL;

	asprintf_or_die(&buf, "%s:%d: %s", file, line, string);

	tc->failed = 1;
	tc->message = buf;
	if (tc->jumpBuf != 0) longjmp(*(tc->jumpBuf), 0);
}

void CuFail_Line(CuTest* tc, const char* file, int line,
                 const char* message2, const char* message) {
    char *string = NULL;

	if (message2 != NULL) {
		asprintf_or_die(&string, "%s:%s", message2, message);
	} else {
        string = strdup(message);
    }
	CuFailInternal(tc, file, line, string);
}

void CuAssert_Line(CuTest* tc, const char* file, int line,
                   const char* message, int condition) {
	if (condition) return;
	CuFail_Line(tc, file, line, NULL, message);
}

void CuAssertStrEquals_LineMsg(CuTest* tc, const char* file, int line,
                               const char* message,
                               const char* expected, const char* actual) {
	char *string = NULL;

	if ((expected == NULL && actual == NULL) ||
	    (expected != NULL && actual != NULL &&
	     strcmp(expected, actual) == 0))
        {
            return;
        }

	if (message != NULL) {
        asprintf_or_die(&string, "%s: expected <%s> but was <%s>", message,
                        expected, actual);
	} else {
        asprintf_or_die(&string, "expected <%s> but was <%s>", expected, actual);
    }
	CuFailInternal(tc, file, line, string);
}

void CuAssertStrNotEqual_LineMsg(CuTest* tc, const char* file, int line,
                                 const char* message,
                                 const char* expected, const char* actual) {
	char *string = NULL;

    if (expected != NULL && actual != NULL && strcmp(expected, actual) != 0)
        return;

	if (message != NULL) {
        asprintf_or_die(&string, "%s: expected <%s> but was <%s>", message,
                        expected, actual);
	} else {
        asprintf_or_die(&string, "expected <%s> but was <%s>", expected, actual);
    }
	CuFailInternal(tc, file, line, string);
}

void CuAssertIntEquals_LineMsg(CuTest* tc, const char* file, int line,
                               const char* message,
                               int expected, int actual) {
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%d> but was <%d>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertDblEquals_LineMsg(CuTest* tc, const char* file, int line,
                               const char* message,
                               double expected, double actual, double delta) {
	char buf[STRING_MAX];
	if (fabs(expected - actual) <= delta) return;
	sprintf(buf, "expected <%lf> but was <%lf>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertPtrEquals_LineMsg(CuTest* tc, const char* file, int line,
                               const char* message,
                               const void* expected, const void* actual) {
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected pointer <0x%p> but was <0x%p>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertPtrNotEqual_LineMsg(CuTest* tc, const char* file, int line,
                                 const char* message,
                                 const void* expected, const void* actual) {
	char buf[STRING_MAX];
	if (expected != actual) return;
	sprintf(buf, "expected pointer <0x%p> to be different from <0x%p>",
            expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


/*-------------------------------------------------------------------------*
 * CuSuite
 *-------------------------------------------------------------------------*/

void CuSuiteInit(CuSuite* testSuite) {
	testSuite->count = 0;
	testSuite->failCount = 0;
}

CuSuite* CuSuiteNew(void) {
	CuSuite* testSuite = NULL;
    if (ALLOC(testSuite) < 0)
        die_oom();
	CuSuiteInit(testSuite);
	return testSuite;
}

void CuSuiteSetup(CuSuite *testSuite,
                  TestFunction setup, TestFunction teardown) {
    testSuite->setup = setup;
    testSuite->teardown = teardown;
}

void CuSuiteAdd(CuSuite* testSuite, CuTest *testCase) {
	assert(testSuite->count < MAX_TEST_CASES);
	testSuite->list[testSuite->count] = testCase;
	testSuite->count++;
}

void CuSuiteAddSuite(CuSuite* testSuite, CuSuite* testSuite2) {
	int i;
	for (i = 0 ; i < testSuite2->count ; ++i)
        {
            CuTest* testCase = testSuite2->list[i];
            CuSuiteAdd(testSuite, testCase);
        }
}

void CuSuiteRun(CuSuite* testSuite) {
	int i;
	for (i = 0 ; i < testSuite->count ; ++i)
        {
            CuTest* testCase = testSuite->list[i];
            CuTestRun(testCase, testSuite->setup, testSuite->teardown);
            if (testCase->failed) { testSuite->failCount += 1; }
        }
}

static void string_append(char **s, const char *p) {
    if (*s == NULL) {
        *s = strdup(p);
    } else {
        int len = strlen(*s) + strlen(p) + 1;
        *s = realloc(*s, len);
        if (*s == NULL)
            die_oom();
        strcat(*s, p);
    }
}

void CuSuiteSummary(CuSuite* testSuite, char **summary) {
	int i;

	for (i = 0 ; i < testSuite->count ; ++i)
        {
            CuTest* testCase = testSuite->list[i];
            string_append(summary, testCase->failed ? "F" : ".");
        }
	string_append(summary, "\n\n");
}

void CuSuiteDetails(CuSuite* testSuite, char **details) {
	int i;
	int failCount = 0;
    char *s = NULL;

	if (testSuite->failCount == 0)
        {
            int passCount = testSuite->count - testSuite->failCount;
            const char* testWord = passCount == 1 ? "test" : "tests";
            asprintf_or_die(&s, "OK (%d %s)\n", passCount, testWord);
            string_append(details, s);
            free(s);
        } else {
		if (testSuite->failCount == 1)
			string_append(details, "There was 1 failure:\n");
		else {
            asprintf_or_die(&s, "There were %d failures:\n",
                            testSuite->failCount);
            string_append(details, s);
            free(s);
        }
		for (i = 0 ; i < testSuite->count ; ++i) {
			CuTest* testCase = testSuite->list[i];
			if (testCase->failed) {
				failCount++;
				asprintf_or_die(&s, "%d) %s:\n%s\n",
                         failCount, testCase->name, testCase->message);
                string_append(details, s);
                free(s);
			}
		}
		string_append(details, "\n!!!FAILURES!!!\n");

		asprintf_or_die(&s, "Runs: %d ",   testSuite->count);
        string_append(details, s);
        free(s);

        asprintf_or_die(&s, "Passes: %d ",
                        testSuite->count - testSuite->failCount);
        string_append(details, s);
        free(s);

		asprintf_or_die(&s, "Fails: %d\n",  testSuite->failCount);
        string_append(details, s);
        free(s);
	}
}

/*
 * Test utilities
 */
void run(CuTest *tc, const char *format, ...) {
    char *command;
    va_list args;
    int r;

    va_start(args, format);
    r = vasprintf(&command, format, args);
    va_end (args);
    if (r < 0)
        CuFail(tc, "Failed to format command (out of memory)");
    r = system(command);
    if (r < 0 || (WIFEXITED(r) && WEXITSTATUS(r) != 0)) {
        char *msg;
        r = asprintf(&msg, "Command %s failed with status %d\n",
                     command, WEXITSTATUS(r));
        CuFail(tc, msg);
        free(msg);
    }
    free(command);
}

int should_run(const char *name, int argc, char **argv) {
    if (argc == 0)
        return 1;
    for (int i=0; i < argc; i++)
        if (STREQ(argv[i], name))
            return 1;
    return 0;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
