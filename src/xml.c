/*
 * xml.c: the implementation of aug_to_xml and supporting functions
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
 * Author: David Lutterkort <lutter@watzmann.net>
 */

#include <config.h>
#include "augeas.h"
#include "internal.h"
#include "memory.h"
#include "info.h"
#include "errcode.h"

#include <libxml/tree.h>

static int to_xml_span(xmlNodePtr elem, const char *pfor, int start, int end) {
    int r;
    char *buf;
    xmlAttrPtr prop;
    xmlNodePtr span_elem;

    span_elem = xmlNewChild(elem, NULL, BAD_CAST "span", NULL);
    if (span_elem == NULL)
        return -1;

    prop = xmlSetProp(span_elem, BAD_CAST "for", BAD_CAST pfor);
    if (prop == NULL)
        return -1;

    /* Format and set the start property */
    r = xasprintf(&buf, "%d", start);
    if (r < 0)
        return -1;

    prop = xmlSetProp(span_elem, BAD_CAST "start", BAD_CAST buf);
    FREE(buf);
    if (prop == NULL)
        return -1;

    /* Format and set the end property */
    r = xasprintf(&buf, "%d", end);
    if (r < 0)
        return -1;

    prop = xmlSetProp(span_elem, BAD_CAST "end", BAD_CAST buf);
    FREE(buf);
    if (prop == NULL)
        return -1;

    return 0;
}

static int to_xml_one(xmlNodePtr elem, const struct tree *tree,
                      const char *pathin) {
    xmlNodePtr value;
    xmlAttrPtr prop;
    int r;

    prop = xmlSetProp(elem, BAD_CAST "label", BAD_CAST tree->label);
    if (prop == NULL)
        goto error;

    if (tree->span) {
        struct span *span = tree->span;

        prop = xmlSetProp(elem, BAD_CAST "file",
                          BAD_CAST span->filename->str);
        if (prop == NULL)
            goto error;

        r = to_xml_span(elem, "label", span->label_start, span->label_end);
        if (r < 0)
            goto error;

        r = to_xml_span(elem, "value", span->value_start, span->value_end);
        if (r < 0)
            goto error;

        r = to_xml_span(elem, "node", span->span_start, span->span_end);
        if (r < 0)
            goto error;
    }

    if (pathin != NULL) {
        prop = xmlSetProp(elem, BAD_CAST "path", BAD_CAST pathin);
        if (prop == NULL)
            goto error;
    }
    if (tree->value != NULL) {
        value = xmlNewTextChild(elem, NULL, BAD_CAST "value",
                                BAD_CAST tree->value);
        if (value == NULL)
            goto error;
    }
    return 0;
 error:
    return -1;
}

static int to_xml_rec(xmlNodePtr pnode, struct tree *start,
                      const char *pathin) {
    int r;
    xmlNodePtr elem;

    elem = xmlNewChild(pnode, NULL, BAD_CAST "node", NULL);
    if (elem == NULL)
        goto error;
    r = to_xml_one(elem, start, pathin);
    if (r < 0)
        goto error;

    list_for_each(tree, start->children) {
        if (TREE_HIDDEN(tree))
            continue;
        r = to_xml_rec(elem, tree, NULL);
        if (r < 0)
            goto error;
    }

    return 0;
 error:
    return -1;
}

static int tree_to_xml(struct pathx *p, xmlNode **xml, const char *pathin) {
    char *path = NULL;
    struct tree *tree;
    xmlAttrPtr expr;
    int r;

    *xml = xmlNewNode(NULL, BAD_CAST "augeas");
    if (*xml == NULL)
        goto error;
    expr = xmlSetProp(*xml, BAD_CAST "match", BAD_CAST pathin);
    if (expr == NULL)
        goto error;

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p)) {
        if (TREE_HIDDEN(tree))
            continue;
        path = path_of_tree(tree);
        if (path == NULL)
            goto error;
        r = to_xml_rec(*xml, tree, path);
        if (r < 0)
            goto error;
        FREE(path);
    }
    return 0;
 error:
    free(path);
    xmlFree(*xml);
    *xml = NULL;
    return -1;
}

int aug_to_xml(const struct augeas *aug, const char *pathin,
               xmlNode **xmldoc, unsigned int flags) {
    struct pathx *p = NULL;
    int result = -1;

    api_entry(aug);

    ARG_CHECK(flags != 0, aug, "aug_to_xml: FLAGS must be 0");
    ARG_CHECK(xmldoc == NULL, aug, "aug_to_xml: XMLDOC must be non-NULL");

    *xmldoc = NULL;

    if (pathin == NULL || strlen(pathin) == 0 || strcmp(pathin, "/") == 0) {
        pathin = "/*";
    }

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), pathin, true);
    ERR_BAIL(aug);
    result = tree_to_xml(p, xmldoc, pathin);
    ERR_THROW(result < 0, aug, AUG_ENOMEM, NULL);
error:
    free_pathx(p);
    api_exit(aug);

    return result;
}
