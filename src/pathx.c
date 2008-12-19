/*
 * pathx.c: handling path expressions
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

#include <config.h>
#include <internal.h>
#include <memory.h>

#define L_BRACK '['
#define R_BRACK ']'

static const char *const last_func = "[last()]";

/* Internal representation of a path in the tree */
struct segment {
    int          index;
    unsigned int fixed : 1; /* Match only one index (either INDEX or LAST) */
    unsigned int last  : 1; /* Match the last node with LABEL */
    unsigned int any   : 1; /* Match any node with any label, implies !FIXED */
    char        *label;
};

struct path {
    size_t      nsegments;
    struct segment *segments;
    struct tree    *root;
};

#define for_each_segment(seg, path)                                 \
    for (typeof(path->segments) (seg) = (path)->segments;           \
         (seg) < (path)->segments + (path)->nsegments;              \
         (seg)++)

#define last_segment(path) (path->segments + path->nsegments - 1)

static int xstrtoul(const char *nptr, char **endptr, int base) {
    unsigned long val;
    char *end;
    int result;

    errno = 0;
    val = strtoul(nptr, &end, base);
    if (errno || (!endptr && *end) || end == nptr || (int) val != val) {
        result = -1;
    } else {
        result = val;
    }
    if (endptr)
        *endptr = end;
    return result;
}

static int sibling_index(struct tree *tree) {
    int indx = 0;

    list_for_each(t, tree->parent->children) {
        if (streqv(t->label, tree->label))
            indx += 1;
        if (t == tree)
            return indx;
    }
    return indx;
}

void free_path(struct path *path) {
    if (path == NULL)
        return;

    if (path->segments != NULL) {
        for (int i=0; i < path->nsegments; i++) {
            free(path->segments[i].label);
        }
        free(path->segments);
    }
    free(path);
}

/* Take a path expression PATH and turn it into a path structure usable for
 * search. The TREE components of the PATH segments will be NULL. Call
 * PATH_FIRST to initialize them to the first match.
 *
 * A path expression follows the grammar
 * PATH = '/' ? SEGMENT ( '/' SEGMENT )* '/' ?
 * SEGMENT = STRING ('[' N ']') ? | '*'
 * where STRING is any string not containing '/' and N is a positive number
 */
struct path *make_path(const struct tree *root, const char *path) {
    struct path *result;

    CALLOC(result, 1);
    if (result == NULL)
        return NULL;
    result->root = (struct tree *) root;

    if (*path != SEP)
        result->nsegments = 1;
    for (const char *p = path; *p != '\0'; p++) {
        if (*p == SEP) {
            while (*p == SEP) p++;
            if (*p == '\0')
                break;
            result->nsegments++;
        }
    }
    if (result->nsegments == 0)
        goto error;

    CALLOC(result->segments, result->nsegments);
    if (result->segments == NULL)
        goto error;

    for_each_segment(seg, result) {
        const char *next, *brack;

        while (*path == SEP)
            path += 1;
        assert(*path);

        brack = strchr(path, L_BRACK);
        next = strchr(path, SEP);
        if (next == NULL)
            next = path + strlen(path);

        if (path[0] == '*') {
            seg->any = 1;
        } else if (brack == NULL || brack >= next) {
            seg->index = 0;
            seg->label = strndup(path, next - path);
        } else {
            seg->label = strndup(path, brack - path);
            if (STREQLEN(brack, last_func, strlen(last_func))) {
                seg->last = 1;
                seg->fixed = 1;
            } else {
                char *end;
                seg->index = xstrtoul(brack + 1, &end, 10);
                seg->fixed = 1;
                if (seg->index <= 0)
                    goto error;
                if (*end != R_BRACK)
                    goto error;
            }
        }
        path = next;
        while (*path == SEP) path += 1;
    }
    return result;

 error:
    free_path(result);
    return NULL;
}

static void calc_last_index(struct segment *seg, struct tree *tree) {
    if (seg->last) {
        seg->index = 0;
        list_for_each(t, tree->parent->children) {
            if (streqv(t->label, seg->label))
                seg->index += 1;
        }
    }
}

/* Assumes that for SEG->LAST == 1, SEG->INDEX has been calculated
   amongst the list of TREE's siblings */
static int seg_match(struct segment *seg, struct tree *tree) {
    if (seg->any || streqv(tree->label, seg->label)) {
        int indx = sibling_index(tree);
        if (!seg->fixed || indx == seg->index) {
            seg->index = indx;
            return 1;
        }
    }
    return 0;
}

static int tree_next(struct tree **tree, int *depth, int mindepth) {
    while ((*tree)->next == NULL && *depth >= mindepth) {
        *depth -= 1;
        *tree = (*tree)->parent;
    }
    if (*depth < mindepth)
        return -1;
    *tree = (*tree)->next;
    return 0;
}

/* Given a node TREE that should match the segment SEGNR in PATH,
   find the next node that matches all of PATH, or return NULL */
static struct tree *complete_path(struct path *path, int segnr,
                                  struct tree *tree) {
    int cur = segnr;

    if (segnr >= path->nsegments || tree == NULL)
        return NULL;

    calc_last_index(path->segments + cur, tree->parent->children);
    while (1) {
        int found = seg_match(path->segments + cur, tree);

        if (found && cur == path->nsegments - 1) {
            return tree;
        } else if (found && cur + 1 < path->nsegments &&
                   tree->children != NULL) {
            cur += 1;
            tree = tree->children;
            calc_last_index(path->segments + cur, tree);
        } else {
            if (tree_next(&tree, &cur, segnr) < 0)
                return NULL;
        }
    }
}

struct tree *path_next(struct path *path, struct tree *cur) {
    int segnr = path->nsegments - 1;

    if (tree_next(&cur, &segnr, -1) < 0)
        return NULL;

    return complete_path(path, segnr, cur);
}

/* Find the first node in TREE matching PATH. */
struct tree *path_first(struct path *path) {
    return complete_path(path, 0, path->root);
}

/* Find a node in the tree that matches the longest prefix of PATH.
 *
 * Return 1 if a node was found that exactly matches PATH, 0 if an incomplete
 * prefix matches, and -1 if more than one node in the tree match.
 *
 * MATCH is set to the tree node that matches, and SEGNR to the
 * number of the segment in PATH where MATCH matched. If no node matches,
 * MATCH will be NULL, and SEGNR -1
 */
static int path_search(struct path *path, struct tree **match, int *segnr) {
    struct tree **matches = NULL;
    int *nmatches = NULL;
    int result;

    if (ALLOC_N(matches, path->nsegments) < 0)
        return -1;

    if (ALLOC_N(nmatches, path->nsegments) < 0) {
        free(matches);
        return -1;
    }

    if (match)
        *match = NULL;
    if (segnr)
        *segnr = -1;

    if (path->root == NULL)
        return -1;

    struct tree *tree = path->root;
    int cur = 0;
    calc_last_index(path->segments, tree);

    while (1) {
        int found = seg_match(path->segments + cur, tree);

        if (found) {
            if (nmatches[cur] == 0)
                matches[cur] = tree;
            nmatches[cur]++;
        }

        if (found && cur + 1 < path->nsegments && tree->children != NULL) {
            cur += 1;
            tree = tree->children;
            calc_last_index(path->segments + cur, tree);
        } else {
            if (tree_next(&tree, &cur, 0) < 0)
                break;
        }
    }

    result = nmatches[path->nsegments - 1] == 1;
    for (cur = path->nsegments-1; cur >=0; cur--) {
        if (nmatches[cur] > 1) {
            result = -1;
            break;
        }
        if (nmatches[cur] == 1) {
            if (match)
                *match = matches[cur];
            if (segnr)
                *segnr = cur;
            break;
        }
    }

    free(matches);
    free(nmatches);
    return result;
}

/* Expand the tree ROOT so that it contains all components of PATH. PATH
 * must have been initialized against ROOT by a call to PATH_FIND_ONE.
 *
 * Return the first segment that was created by this operation, or NULL on
 * error.
 */
int path_expand_tree(struct path *path, struct tree **tree) {
    int r, segnr;

    *tree = path->root;
    r = path_search(path, tree, &segnr);
    if (r == -1)
        return -1;

    if (segnr == path->nsegments - 1)
        return 0;

    struct tree *first_child = NULL;
    struct tree *parent = *tree;
    struct segment *seg = path->segments + segnr + 1;

    if (parent == NULL)
        parent = path->root->parent;

    for (struct segment *s = seg ; s <= last_segment(path); s++) {
        struct tree *t = make_tree(strdup(s->label), NULL, parent, NULL);
        if (first_child == NULL)
            first_child = t;
        if (t == NULL || t->label == NULL)
            goto error;
        list_append(parent->children, t);
        parent = t;
    }

    while (first_child->children != NULL)
        first_child = first_child->children;

    *tree = first_child;
    return 0;

 error:
    list_remove(first_child, first_child->parent->children);
    free_tree(first_child);
    *tree = NULL;
    return -1;
}

int path_find_one(struct path *path, struct tree **tree) {
    *tree = path_first(path);
    if (*tree == NULL)
        return 0;

    if (path_next(path, *tree) != NULL) {
        *tree = NULL;
        return -1;
    }
    return 1;
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
