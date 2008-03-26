/*
 * lens.h: Repreentation of lenses
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

#ifndef __LENS_H
#define __LENS_H

#include "syntax.h"
#include "fa.h"

enum lens_tag {
    L_DEL,
    L_STORE,
    L_KEY,
    L_LABEL,
    L_SEQ,
    L_COUNTER,
    L_CONCAT,
    L_UNION,
    L_SUBTREE,
    L_STAR,
    L_PLUS,
    L_MAYBE
};

struct lens {
    unsigned int              ref;
    enum lens_tag             tag;  /* Same as vt->tag */
    struct info              *info;
    struct regexp            *ctype;
    struct regexp            *atype;
    union {
        /* Primitive lenses */
        struct {                   /* L_DEL uses both */
            struct regexp *regexp; /* L_STORE, L_KEY */
            struct string *string; /* L_LABEL, L_SEQ, L_COUNTER */
        };
        /* Combinators */
        struct lens * exp;         /* L_SUBTREE, L_STAR, L_PLUS, L_MAYBE */
        struct {                   /* L_UNION, L_CONCAT */
            struct lens *exp1;
            struct lens *exp2;
        };
    };
};

/* Constructors for various lens types. Constructor assumes ownership of
 * arguments without incrementing. Caller owns returned lenses.
 */
struct lens *lns_make_prim(enum lens_tag tag, struct info *info,
                           struct regexp *regexp, struct string *string);
struct lens *lns_make_unop(enum lens_tag tag, struct info *info,
                           struct lens *exp);
struct lens *lns_make_binop(enum lens_tag tag, struct info *info,
                            struct lens *exp1, struct lens *exp2);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
