/*
 * schema.c: support for generating a tree schema from a lens
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

#include "schema.h"

#include "internal.h"
#include "errcode.h"
#include "memory.h"

static struct schema* make_schema(enum schema_tag tag, struct info *info) {
  struct schema *result = NULL;
  int r;

  r = ALLOC(result);
  ERR_NOMEM(r < 0, info);

  result->tag = tag;
  result->info = ref(info);

  return result;
 error:
  return NULL;
}

static struct schema *make_schema_concat_or_union(struct lens *lens) {
  struct schema *result = NULL;
  enum schema_tag tag;

  int r;

  ensure0(lens->tag == L_CONCAT || lens->tag == L_UNION, lens->info);

  tag = lens->tag == L_CONCAT ? S_CONCAT : S_UNION;

  result = make_schema(tag, lens->info);
  ERR_BAIL(lens->info);

  r = ALLOC_N(result->children, lens->nchildren);
  ERR_NOMEM(r < 0, lens->info);
  result->nchildren = lens->nchildren;
  for (int i=0; i < result->nchildren; i++) {
    result->children[i] = schema_from_lens(lens->children[i]);
  }
  return result;
 error:
  free_schema(result);
  return NULL;
}

void simplify_schema(struct schema *schema) {
  if (schema == NULL)
    return;

  switch(schema->tag) {
  case S_REC:
  case S_UNIT:
    break;
  case S_CONCAT:
  case S_UNION:
    {
      int nunit = 0;
      for (int i=0; i < schema->nchildren; i++) {
        simplify_schema(schema->children[i]);
        if (schema->children[i]->tag == S_UNIT)
          nunit += 1;
      }
      if (nunit == schema->nchildren) {
        for (int i=0; i < schema->nchildren; i++) {
          free_schema(schema->children[i]);
        }
        FREE(schema->children);
        schema->tag = S_UNIT;
      } else if (nunit == schema->nchildren - 1) {
        struct schema tmp;
        for (int i=0; i < schema->nchildren; i++) {
          if (schema->children[i]->tag != S_UNIT) {
            tmp = *schema->children[i];
            free(schema->children[i]);
          } else {
            free_schema(schema->children[i]);
          }
        }
        free(schema->children);
        unref(schema->info, info);
        *schema = tmp;
      }
    }
    break;
  case S_SUBTREE:
    simplify_schema(schema->child);
    break;
  case S_STAR:
  case S_MAYBE:
  case S_SQUARE:
    simplify_schema(schema->child);
    if (schema->child->tag == S_UNIT) {
      free_schema(schema->child);
      schema->child = NULL;
      schema->tag = S_UNIT;
    }
    break;
  default:
    BUG_ON(true, schema->info, "Unexpected schema type %d", schema->tag);
    break;
  }
 error:
  return;
}

struct schema *schema_from_lens(struct lens *lens) {
  struct schema *result = NULL;

  if (lens == NULL)
    return NULL;

  switch (lens->tag) {
  case L_DEL:
  case L_STORE:
  case L_VALUE:
  case L_KEY:
  case L_LABEL:
  case L_SEQ:
  case L_COUNTER:
    result = make_schema(S_UNIT, lens->info);
    ERR_BAIL(lens->info);
    break;
  case L_SUBTREE:
    result = make_schema(S_SUBTREE, lens->info);
    ERR_BAIL(lens->info);
    result->ktype = ref(lens->child->ktype);
    result->vtype = ref(lens->child->vtype);
    result->child = schema_from_lens(lens->child);
    break;
  case L_STAR:
    result = make_schema(S_STAR, lens->info);
    ERR_BAIL(lens->info);
    result->child = schema_from_lens(lens->child);
    break;
  case L_MAYBE:
    result = make_schema(S_MAYBE, lens->info);
    ERR_BAIL(lens->info);
    result->child = schema_from_lens(lens->child);
    break;
  case L_SQUARE:
    result = make_schema(S_SQUARE, lens->info);
    ERR_BAIL(lens->info);
    result->child = schema_from_lens(lens->child);
    break;
  case L_CONCAT:
  case L_UNION:
    result = make_schema_concat_or_union(lens);
    ERR_BAIL(lens->info);
    break;
  case L_REC:
    result = make_schema(S_REC, lens->info);
    ERR_BAIL(lens->info);
    break;
  default:
    BUG_ON(true, lens->info, "Unexpected lens tag %d", lens->tag);
    break;
  }

  return result;
 error:
  free_schema(result);
  return NULL;
}

void free_schema(struct schema *schema) {
  if (schema == NULL)
    return;

  switch(schema->tag) {
  case S_REC:
  case S_UNIT:
    break;
  case S_CONCAT:
  case S_UNION:
    for (int i=0; i < schema->nchildren; i++) {
      free_schema(schema->children[i]);
    }
    free(schema->children);
    break;
  case S_SUBTREE:
    unref(schema->ktype, regexp);
    unref(schema->vtype, regexp);
    free_schema(schema->child);
    break;
  case S_STAR:
  case S_MAYBE:
  case S_SQUARE:
    free_schema(schema->child);
    break;
  default:
    BUG_ON(true, schema->info, "Unexpected schema type %d", schema->tag);
    break;
  }
  unref(schema->info, info);
  free(schema);
 error:
  return;
}

static void print_indent(FILE *out, int indent) {
  for (int i=0; i < indent; i++)
    fputc(' ', out);
}

static void dump(FILE *out, struct schema *schema, int indent) {
  if (schema == NULL) {
    print_indent(out, indent);
    fprintf(out, "<<NULL>>\n");
    return;
  }

  switch(schema->tag) {
  case S_REC:
    print_indent(out, indent);
    fprintf(out, "<<REC>>\n");
  case S_UNIT:
    print_indent(out, indent);
    fprintf(out, "()\n");
    break;
  case S_CONCAT:
  case S_UNION:
    print_indent(out, indent);
    if (schema->tag == S_CONCAT) {
      fprintf(out, ".\n");
    } else {
      fprintf(out, "|\n");
    }
    for (int i=0; i < schema->nchildren; i++) {
      dump(out, schema->children[i], indent+2);
    }
    break;
  case S_SUBTREE:
    {
      bool unit = schema->child->tag == S_UNIT;
      print_indent(out, indent);
      fprintf(out, unit ? "{ " : "[ ");
      print_regexp(out, schema->ktype);
      fprintf(out, " = ");
      print_regexp(out, schema->vtype);
      fprintf(out, unit ? " }\n" : " ]\n");
      if (! unit)
        dump(out, schema->child, indent+2);
    }
    break;
  case S_STAR:
  case S_MAYBE:
  case S_SQUARE:
    print_indent(out, indent);
    if (schema->tag == S_STAR) {
      fputc('*', out);
    } else if (schema->tag == S_MAYBE) {
      fputc('?', out);
    } else {
      fputc('#', out);
    }
    fputc('\n', out);
    dump(out, schema->child, indent+2);
    break;
  default:
    BUG_ON(true, schema->info, "Unexpected schema type %d", schema->tag);
    break;
  }
 error:
  return;
}

void dump_schema(FILE *out, struct schema *schema) {
  dump(out, schema, 0);
}
