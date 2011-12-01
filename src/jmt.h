/*
 * jmt.h: Earley parser for lenses based on Jim/Mandelbaum transducers
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

#ifndef EARLEY_H_
#define EARLEY_H_

#include <stdlib.h>
#include <stdint.h>
#include "lens.h"

struct jmt;
struct jmt_parse;

typedef uint32_t ind_t;

struct lens;

typedef void (*jmt_traverser)(struct lens *l, size_t start, size_t end,
                              void *data);

typedef void (*jmt_error)(struct lens *lens, void *data, size_t pos,
                          const char *format, ...);

struct jmt_visitor {
    struct jmt_parse *parse;
    jmt_traverser    terminal;
    jmt_traverser    enter;
    jmt_traverser    exit;
    jmt_error        error;
    void             *data;
};

struct jmt *jmt_build(struct lens *l);

struct jmt_parse *jmt_parse(struct jmt *jmt, const char *text, size_t text_len);

void jmt_free_parse(struct jmt_parse *);

/* Returns -1 on internal error, 0 on syntax error, 1 on a successful
 * parse.
 */
int jmt_visit(struct jmt_visitor *visitor, size_t *len);

void jmt_free(struct jmt *jmt);

void jmt_dot(struct jmt *jmt, const char *fname);
#endif

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
