/*
 * schema.h: support for generating a tree schema from a lens
 *
 * Copyright (C) 2016 David Lutterkort
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
 * Author: David Lutterkort <lutter@watzmann.net>
 */

#ifndef SCHEMA_H_
#define SCHEMA_H_

#include "stdio.h"

#include "config.h"
#include "lens.h"

/* What we are trying to represent in OCaml notation:

   type schema = Concat schema * schema
            | Union  schema * schema
            | Subtree rx * rx * schema
            | Square schema
            | Star   schema
            | Maybe  schema
            | Rec
            | Unit
*/

enum schema_tag {
  S_UNIT,
  S_CONCAT,
  S_UNION,
  S_SUBTREE,
  S_STAR,
  S_MAYBE,
  S_SQUARE,
  S_REC
};

struct schema {
  enum schema_tag tag;
  struct info *info;
  union {
    struct {                       /* L_CONCAT, L_UNION */
      unsigned int nchildren;
      struct schema **children;
    };
    struct {                       /* L_SUBTREE */
      struct regexp *ktype;
      struct regexp *vtype;
      struct schema *child;        /* L_STAR, L_MAYBE, L_SQUARE */
    };
  };
};

struct schema *schema_from_lens(struct lens *lens);
void simplify_schema(struct schema *);
void dump_schema(FILE *out, struct schema *);
void free_schema(struct schema *);
#endif
