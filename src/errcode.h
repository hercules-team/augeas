/*
 * errcode.h: internal interface for error reporting
 *
 * Copyright (C) 2009-2011 David Lutterkort
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

#ifndef ERRCODE_H_
#define ERRCODE_H_

#include "internal.h"
/* Include augeas.h for the error codes */
#include "augeas.h"

/*
 * Error details in a separate struct that we can pass around
 */
struct error {
    aug_errcode_t  code;
    int            minor;
    char          *details;       /* Human readable explanation */
    const char    *minor_details; /* Human readable version of MINOR */
    /* A dummy info of last resort; this can be used in places where
     * a struct info is needed but none available
     */
    struct info   *info;
    /* Bit of a kludge to get at struct augeas, but since struct error
     * is now available in a lot of places (through struct info), this
     * gives a convenient way to get at the overall state
     */
    const struct augeas *aug;
    /* A preallocated exception so that we can throw something, even
     * under OOM conditions
     */
    struct value *exn;
};

void report_error(struct error *err, aug_errcode_t errcode,
                  const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 3, 4);

void bug_on(struct error *err, const char *srcfile, int srclineno,
            const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 4, 5);

void reset_error(struct error *err);

#define HAS_ERR(obj) ((obj)->error->code != AUG_NOERROR)

#define ERR_BAIL(obj) if ((obj)->error->code != AUG_NOERROR) goto error;

#define ERR_RET(obj) if ((obj)->error->code != AUG_NOERROR) return;

#define ERR_NOMEM(cond, obj)                             \
    if (cond) {                                          \
        report_error((obj)->error, AUG_ENOMEM, NULL);    \
        goto error;                                      \
    }

#define ERR_REPORT(obj, code, fmt ...)          \
    report_error((obj)->error, code, ## fmt)

#define ERR_THROW(cond, obj, code, fmt ...)             \
    do {                                                \
        if (cond) {                                     \
            report_error((obj)->error, code, ## fmt);   \
            goto error;                                 \
        }                                               \
    } while(0)

#define ARG_CHECK(cond, obj, fmt ...)                           \
    do {                                                        \
        if (cond) {                                             \
            report_error((obj)->error, AUG_EBADARG, ## fmt);    \
            goto error;                                         \
        }                                                       \
    } while(0)

/* A variant of assert that uses our error reporting infrastructure
 * instead of aborting
 */
#ifdef NDEBUG
# define ensure(cond, obj) if (0) goto error
# define ensure0(cond, obj) if (0) return NULL
#else
# define ensure(cond, obj)                                           \
    if (!(cond)) {                                                   \
        bug_on((obj)->error, __FILE__, __LINE__, NULL);              \
        goto error;                                                  \
    }
# define ensure0(cond, obj)                                          \
    if (!(cond)) {                                                   \
        bug_on((obj)->error, __FILE__, __LINE__, NULL);              \
        return NULL;                                                 \
    }
#endif

#define BUG_ON(cond, obj, fmt ...)                                  \
    if (cond) {                                                     \
        bug_on((obj)->error, __FILE__, __LINE__, ## fmt);           \
        goto error;                                                 \
    }

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
