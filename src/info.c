/*
 * info.c: filename/linenumber information for parser/interpreter
 *
 * Copyright (C) 2007-2011 David Lutterkort
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
#include "info.h"
#include "internal.h"
#include "memory.h"
#include "ref.h"

/*
 * struct string
 */
struct string *make_string(char *str) {
    struct string *string;
    make_ref(string);
    string->str = str;
    return string;
}

struct string *dup_string(const char *str) {
    struct string *string;
    make_ref(string);
    if (str == NULL)
        string->str = strdup("");
    else
        string->str = strdup(str);
    if (string->str == NULL)
        unref(string, string);
    return string;
}

void free_string(struct string *string) {
    if (string == NULL)
        return;
    assert(string->ref == 0);
    free(string->str);
    free(string);
}

/*
 * struct info
 */
char *format_info(struct info *info) {
    const char *fname;
    char *result = NULL;
    int r = 0;
    int fl = info->first_line, ll = info->last_line;
    int fc = info->first_column, lc = info->last_column;
    fname = (info->filename != NULL) ? info->filename->str : "(unknown file)";

    if (fl > 0) {
        if (fl == ll) {
            if (fc == lc) {
                r = xasprintf(&result, "%s:%d.%d:", fname, fl, fc);
            } else {
                r = xasprintf(&result, "%s:%d.%d-.%d:", fname, fl, fc, lc);
            }
        } else {
            r = xasprintf(&result, "%s:%d.%d-%d.%d:", fname, fl, fc, ll, lc);
        }
    } else {
        r = xasprintf(&result, "%s:", fname);
    }
    return (r == -1) ? NULL : result;
}

void print_info(FILE *out, struct info *info) {
    if (info == NULL) {
        fprintf(out, "(no file info):");
        return;
    }
    fprintf(out, "%s:",
            info->filename != NULL ? info->filename->str : "(unknown file)");
    if (info->first_line > 0) {
        if (info->first_line == info->last_line) {
            if (info->first_column == info->last_column) {
                fprintf(out, "%d.%d:", info->first_line, info->first_column);
            } else {
                fprintf(out, "%d.%d-.%d:", info->first_line,
                        info->first_column, info->last_column);
            }
        } else {
            fprintf(out, "%d.%d-%d.%d:",
                    info->first_line, info->first_column,
                    info->last_line, info->last_column);
        }
    }
}

void free_info(struct info *info) {
    if (info == NULL)
        return;
    assert(info->ref == 0);
    unref(info->filename, string);
    free(info);
}

struct span *make_span(struct info *info) {
    struct span *span = NULL;
    if (ALLOC(span) < 0) {
        return NULL;
    }
    /* UINT_MAX means span is not initialized yet */
    span->span_start = UINT_MAX;
    span->filename = ref(info->filename);
    return span;
}

void free_span(struct span *span) {
    if (span == NULL)
        return;
    unref(span->filename, string);
    free(span);
}

void print_span(struct span *span) {
    if (span == NULL)
        return;
    printf("%s label=(%i:%i) value=(%i:%i) span=(%i,%i)\n",
            span->filename->str,
            span->label_start, span->label_end,
            span->value_start, span->value_end,
            span->span_start, span->span_end);
}

void update_span(struct span *node_info, int x, int y) {
    if (node_info == NULL)
        return;
    if (node_info->span_start == UINT_MAX) {
        node_info->span_start = x;
        node_info->span_end = y;
    } else {
        if (node_info->span_start > x)
            node_info->span_start = x;
        if (node_info->span_end < y)
            node_info->span_end = y;
    }
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
