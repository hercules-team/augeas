/*
 * errcode.c: internal interface for error reporting
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

#include <config.h>

#include "errcode.h"
#include "memory.h"
#include <stdarg.h>

static void vreport_error(struct error *err, aug_errcode_t errcode,
                   const char *format, va_list ap) {
    /* We only remember the first error */
    if (err->code != AUG_NOERROR)
        return;
    assert(err->details == NULL);

    err->code = errcode;
    if (format != NULL) {
        if (vasprintf(&err->details, format, ap) < 0)
            err->details = NULL;
    }
}

void report_error(struct error *err, aug_errcode_t errcode,
                  const char *format, ...) {
    va_list ap;

    va_start(ap, format);
    vreport_error(err, errcode, format, ap);
    va_end(ap);
}

void bug_on(struct error *err, const char *srcfile, int srclineno,
            const char *format, ...) {
    char *msg = NULL;
    int r;
    va_list ap;

    if (err->code != AUG_NOERROR)
        return;

    va_start(ap, format);
    vreport_error(err, AUG_EINTERNAL, format, ap);
    va_end(ap);

    if (err->details == NULL) {
        xasprintf(&err->details, "%s:%d:internal error", srcfile, srclineno);
    } else {
        r = xasprintf(&msg, "%s:%d:%s", srcfile, srclineno, err->details);
        if (r >= 0) {
            free(err->details);
            err->details = msg;
        }
    }
}

void reset_error(struct error *err) {
    err->code = AUG_NOERROR;
    err->minor = 0;
    FREE(err->details);
    err->minor_details = NULL;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
