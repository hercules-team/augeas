/*
 * info.h: filename/linenumber information for parser/interpreter
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

#ifndef INFO_H_
#define INFO_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ref.h"

/* Reference-counted strings */
struct string {
    ref_t   ref;
    char   *str;
};

struct string *make_string(char *str);

/* Duplicate a string; if STR is NULL, use the empty string "" */
struct string *dup_string(const char *str);

/* Do not call directly, use UNREF instead */
void free_string(struct string *string);

/* File information */
struct info {
    /* There is only one struct error for each Augeas instance */
    struct error  *error;
    struct string *filename;
    uint16_t first_line;
    uint16_t first_column;
    uint16_t last_line;
    uint16_t last_column;
    ref_t    ref;
    int flags;
};

struct span {
    struct string *filename;
    uint label_start;
    uint label_end;
    uint value_start;
    uint value_end;
    uint span_start;
    uint span_end;
};

char *format_info(struct info *info);

void print_info(FILE *out, struct info *info);

/* Do not call directly, use UNREF instead */
void free_info(struct info *info);

struct span *make_span(struct info *info);
void free_span(struct span *node_info);
void update_span(struct span *node_info, int x, int y);
void print_span(struct span *node_info);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
