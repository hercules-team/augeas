/*
 * json.c: the implementation of aug_to_json and supporting functions
 *
 * Copyright (C) 2017 David Lutterkort
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
 * Author: Adam Critchley <opensource@finitestate.io>
 */

#include <config.h>
#include "augeas.h"
#include "internal.h"
#include "memory.h"
#include "info.h"
#include "errcode.h"

static int to_json_span(json_object *elem, const char *pfor, int start, int end)
{
    int r;
    char *buf;
    json_object *jprop;

    json_object *jspan = json_object_new_object();
    if (jspan == NULL)
        return -1;

    json_object_object_add(elem, "span", jspan);

    jprop = json_object_new_string(pfor);
    if (jprop == NULL)
        return -1;

    json_object_object_add(jspan, "for", jprop);

    /* Format and set the start property */
    r = xasprintf(&buf, "%d", start);
    if (r < 0)
        return -1;

    jprop = json_object_new_string(buf);
    FREE(buf);
    if (jprop == NULL)
        return -1;

    json_object_object_add(jspan, "start", jprop);

    /* Format and set the end property */
    r = xasprintf(&buf, "%d", end);
    if (r < 0)
        return -1;

    jprop = json_object_new_string(buf);
    FREE(buf);
    if (jprop == NULL)
        return -1;

    json_object_object_add(jspan, "end", jprop);

    return 0;
}

static int to_json_one(json_object *elem, const struct tree *tree,
                        const char *pathin)
{
    int r;

    if (tree->span)
    {
        struct span *span = tree->span;

        json_object *jpath = json_object_new_string(span->filename->str);
        if (jpath == NULL)
            goto error;

        json_object_object_add(elem, "file", jpath);

        r = to_json_span(elem, "label", span->label_start, span->label_end);
        if (r < 0)
            goto error;

        r = to_json_span(elem, "value", span->value_start, span->value_end);
        if (r < 0)
            goto error;

        r = to_json_span(elem, "node", span->span_start, span->span_end);
        if (r < 0)
            goto error;
    }

    if (pathin != NULL)
    {
        json_object *jpath = json_object_new_string(pathin);
        if (jpath == NULL)
            goto error;

        json_object_object_add(elem, "path", jpath);
    }
    if (tree->value != NULL)
    {
        json_object *jvalue = json_object_new_string(tree->value);
        if (jvalue == NULL)
            goto error;

        json_object_object_add(elem, "value", jvalue);
    }
    return 0;
error:
    return -1;
}

static int to_json_rec(json_object *pnode, struct tree *start,
                        const char *pathin)
{
    int r;

    json_object *elem = json_object_new_object();
    if (elem == NULL)
        goto error;

    r = to_json_one(elem, start, pathin);
    if (r < 0)
        goto error;

    json_object_object_add(pnode, start->label, elem);

    list_for_each(tree, start->children)
    {
        if (TREE_HIDDEN(tree))
            continue;
        r = to_json_rec(elem, tree, NULL);
        if (r < 0)
            goto error;
    }

    return 0;
error:
    return -1;
}

static int tree_to_json(struct pathx *p, json_object **node, const char *pathin)
{
    char *path = NULL;
    struct tree *tree;
    int r;

    *node = json_object_new_object();
    if (*node == NULL)
        goto error;

    json_object *jroot = json_object_new_object();
    if (jroot == NULL)
        goto error;

    json_object_object_add(*node, "augeas", jroot);

    json_object *jmatch = json_object_new_string(pathin);
    if (jmatch == NULL)
        goto error;

    json_object_object_add(jroot, "match", jmatch);

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p))
    {
        if (TREE_HIDDEN(tree))
            continue;
        path = path_of_tree(tree);
        if (path == NULL)
            goto error;
        r = to_json_rec(jroot, tree, path);
        if (r < 0)
            goto error;
        FREE(path);
    }
    return 0;
error:
    free(path);
    json_object_put(*node);
    *node = NULL;
    return -1;
}

int aug_to_json(const struct augeas *aug, const char *pathin,
                json_object **root, unsigned int flags)
{
    struct pathx *p = NULL;
    int result = -1;

    api_entry(aug);

    ARG_CHECK(flags != 0, aug, "aug_to_json: FLAGS must be 0");
    ARG_CHECK(root == NULL, aug, "aug_to_json: json_object must be non-NULL");

    if (pathin == NULL || strlen(pathin) == 0 || strcmp(pathin, "/") == 0)
    {
        pathin = "/*";
    }

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), pathin, true);
    ERR_BAIL(aug);
    result = tree_to_json(p, root, pathin);
    ERR_THROW(result < 0, aug, AUG_ENOMEM, NULL);
error:
    free_pathx(p);
    api_exit(aug);

    return result;
}
