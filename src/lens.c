/*
 * lens.c: 
 *
 * Copyright (C) 2007 Red Hat Inc.
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

#include "lens.h"

static struct lens *make_lens(enum lens_tag tag, struct info *info) {
    struct lens *lens;
    make_ref(lens);
    lens->tag = tag;
    lens->info = info;

    return lens;
}

struct lens *lns_make_prim(enum lens_tag tag, struct info *info,
                           struct regexp *regexp, struct string *string) {
    struct lens *lens = make_lens(tag, info);
    lens->regexp = regexp;
    lens->string = string;
    return lens;
}

struct lens *lns_make_unop(enum lens_tag tag, struct info *info,
                           struct lens *exp) {
    struct lens *lens = make_lens(tag, info);
    lens->exp = exp;
    return lens;
}

struct lens *lns_make_binop(enum lens_tag tag, struct info *info,
                            struct lens *exp1, struct lens *exp2) {
    struct lens *lens = make_lens(tag, info);
    lens->exp1 = exp1;
    lens->exp2 = exp2;
    return lens;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
