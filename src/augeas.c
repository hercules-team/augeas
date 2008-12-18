/*
 * augeas.c: the core data structure for storing key/value pairs
 *
 * Copyright (C) 2007, 2008 Red Hat Inc.
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
#include "augeas.h"
#include "internal.h"
#include "memory.h"
#include "syntax.h"

#include <fnmatch.h>
#include <argz.h>
#include <string.h>

/* We always have a toplevel entry for P_ROOT */
#define P_ROOT   "augeas"

#define L_BRACK '['
#define R_BRACK ']'

static const char *const last_func = "[last()]";

#define TREE_HIDDEN(tree) ((tree)->label == NULL)

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

static void free_path(struct path *path) {
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
static struct path *make_path(const struct tree *root, const char *path) {
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

static struct tree *path_next(struct path *path, struct tree *cur) {
    int segnr = path->nsegments - 1;

    if (tree_next(&cur, &segnr, -1) < 0)
        return NULL;

    return complete_path(path, segnr, cur);
}

/* Find the first node in TREE matching PATH. */
static struct tree *path_first(struct path *path) {
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
static int path_find_one(struct path *path, struct tree **match, int *segnr) {
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

static const char *pretty_label(const struct tree *tree) {
    if (tree == NULL)
        return "(no_tree)";
    else if (tree->label == NULL)
        return "(none)";
    else
        return tree->label;
}

static char *path_expand(struct tree *tree, const char *ppath) {
    struct tree *siblings = tree->parent->children;

    char *path;
    const char *label;
    int cnt = 0, ind = 0, r;

    list_for_each(t, siblings) {
        if (streqv(t->label, tree->label)) {
            cnt += 1;
            if (t == tree)
                ind = cnt;
        }
    }

    if (ppath == NULL)
        ppath = "";

    label = pretty_label(tree);
    if (cnt > 1) {
        r = asprintf(&path, "%s/%s[%d]", ppath, label, ind);
    } else {
        r = asprintf(&path, "%s/%s", ppath, label);
    }
    if (r == -1)
        return NULL;
    return path;
}

static char *format_path(struct tree *tree) {
    int depth, i;
    struct tree *t, **anc;
    char *path = NULL;

    for (t = tree, depth = 1; ! ROOT_P(t); depth++, t = t->parent);
    if (ALLOC_N(anc, depth) < 0)
        return NULL;

    for (t = tree, i = depth - 1; i >= 0; i--, t = t->parent)
        anc[i] = t;

    for (i = 0; i < depth; i++) {
        char *p = path_expand(anc[i], path);
        free(path);
        path = p;
    }
    return path;
}

/* Expand the tree ROOT so that it contains all components of PATH. PATH
 * must have been initialized against ROOT by a call to PATH_FIND_ONE.
 *
 * Return the first segment that was created by this operation, or NULL on
 * error.
 */
static struct tree *tree_create(struct path *path, struct tree *parent,
                                int segnr) {
    struct segment *s, *seg;
    struct tree *first_child = NULL;

    if (segnr == path->nsegments - 1)
        return 0;

    if (parent == NULL)
        parent = path->root->parent;

    seg = path->segments + segnr + 1;
    for (s = seg ; s <= last_segment(path); s++) {
        struct tree *tree = make_tree(strdup(s->label), NULL, parent, NULL);
        if (first_child == NULL)
            first_child = tree;
        if (tree == NULL || tree->label == NULL)
            goto error;
        list_append(parent->children, tree);
        parent = tree;
    }

    while (first_child->children != NULL)
        first_child = first_child->children;

    return first_child;
 error:
    list_remove(first_child, first_child->parent->children);
    free_tree(first_child);
    return NULL;
}

/* Propagate dirty flags towards the root */
static int tree_propagate_dirty(struct tree *tree) {
    if (tree->dirty)
        return 1;
    list_for_each(c, tree->children) {
        tree->dirty |= tree_propagate_dirty(c);
    }
    return tree->dirty;
}

/* Clear the dirty flag in the whole TREE */
static void tree_clean(struct tree *tree) {
    tree->dirty = 0;
    list_for_each(c, tree->children)
        tree_clean(c);
}

static const char *init_root(const char *root0) {
    char *root;

    if (root0 == NULL)
        root0 = getenv(AUGEAS_ROOT_ENV);
    if (root0 == NULL)
        root0 = "/";
    root = strdup(root0);
    if (root[strlen(root)-1] != SEP) {
        if (REALLOC_N(root, strlen(root) + 2) == -1) {
            FREE(root);
            return NULL;
        }
        strcat(root, "/");
    }
    return root;
}

struct augeas *aug_init(const char *root, const char *loadpath,
                        unsigned int flags) {
    struct augeas *result;
    struct tree *tree_root = make_tree(NULL, NULL, NULL, NULL);

    if (tree_root == NULL)
        return NULL;

    CALLOC(result, 1);
    result->origin = make_tree_origin(tree_root);
    if (result->origin == NULL) {
        free_tree(tree_root);
        goto error;
    }

    result->flags = flags;

    result->root = init_root(root);

    result->origin->children->label = strdup(P_ROOT);

    result->modpathz = NULL;
    result->nmodpath = 0;
    if (loadpath != NULL) {
        argz_add_sep(&result->modpathz, &result->nmodpath,
                     loadpath, PATH_SEP_CHAR);
    }
    char *env = getenv(AUGEAS_LENS_ENV);
    if (env != NULL) {
        argz_add_sep(&result->modpathz, &result->nmodpath, env, PATH_SEP_CHAR);
    }
    if (!(flags & AUG_NO_STDINC)) {
        argz_add(&result->modpathz, &result->nmodpath, AUGEAS_LENS_DIR);
    }
    /* Clean up trailing slashes */
    if (result->nmodpath > 0) {
        argz_stringify(result->modpathz, result->nmodpath, PATH_SEP_CHAR);
        char *s, *t;
        for (s = result->modpathz, t = result->modpathz; *s != '\0'; s++) {
            char *p = s;
            if (*p == '/') {
                while (*p == '/') p += 1;
                if (*p == '\0' || *p == PATH_SEP_CHAR)
                    s = p;
            }
            if (t != s)
                *t++ = *s;
            else
                t += 1;
        }
        if (t != s) {
            *t = '\0';
        }
        s = result->modpathz;
        argz_create_sep(s, PATH_SEP_CHAR, &result->modpathz, &result->nmodpath);
        free(s);
    }

    /* We report the root dir in AUGEAS_META_ROOT, but we only use the
       value we store internally, to avoid any problems with
       AUGEAS_META_ROOT getting changed. */
    aug_set(result, AUGEAS_META_ROOT, result->root);

    if (flags & AUG_SAVE_NEWFILE) {
        aug_set(result, AUGEAS_META_SAVE_MODE, "newfile");
    } else if (flags & AUG_SAVE_BACKUP) {
        aug_set(result, AUGEAS_META_SAVE_MODE, "backup");
    } else {
        aug_set(result, AUGEAS_META_SAVE_MODE, "overwrite");
    }

    if (interpreter_init(result) == -1)
        goto error;

    list_for_each(modl, result->modules) {
        struct transform *xform = modl->autoload;
        if (xform == NULL)
            continue;
        transform_load(result, xform);
    }
    list_for_each(tree, result->origin->children) {
        tree_clean(tree);
    }
    return result;

 error:
    aug_close(result);
    return NULL;
}

int aug_get(const struct augeas *aug, const char *path, const char **value) {
    struct path *p = make_path(aug->origin->children, path);
    struct tree *match;
    int r;

    if (p == NULL)
        return -1;

    if (value != NULL)
        *value = NULL;

    r = path_find_one(p, &match, NULL);
    if (r == 1 && value != NULL)
        *value = match->value;
    free_path(p);

    return r;
}

struct tree *tree_set(struct tree *root, const char *path, const char *value) {
    struct tree *tree;
    struct path *p = make_path(root, path);
    int r, segnr;

    if (p == NULL)
        goto error;

    r = path_find_one(p, &tree, &segnr);
    if (r == -1)
        goto error;

    if (r == 0) {
        if ((tree = tree_create(p, tree, segnr)) == NULL)
            goto error;
    }
    free_path(p);

    if (tree->value != NULL) {
        free(tree->value);
        tree->value = NULL;
    }
    if (value != NULL) {
        tree->value = strdup(value);
        if (tree->value == NULL)
            goto error;
    }
    tree->dirty = 1;
    return tree;
 error:
    free_path(p);
    return NULL;
}

int aug_set(struct augeas *aug, const char *path, const char *value) {
    return tree_set(aug->origin->children, path, value) == NULL ? -1 : 0;
}

int tree_insert(struct tree *origin, const char *path, const char *label,
                int before) {
    assert(origin->parent == origin);

    struct path *p = NULL;
    struct tree *new = NULL, *match;

    if (strchr(label, SEP) != NULL)
        return -1;

    p = make_path(origin->children, path);
    if (p == NULL)
        goto error;

    if (path_find_one(p, &match, NULL) != 1)
        goto error;

    new = make_tree(strdup(label), NULL, match->parent, NULL);
    if (new == NULL || new->label == NULL)
        goto error;

    if (before) {
        list_insert_before(new, match, new->parent->children);
    } else {
        new->next = match->next;
        match->next = new;
    }
    free_path(p);
    return 0;
 error:
    free_tree(new);
    free_path(p);
    return -1;
}

int aug_insert(struct augeas *aug, const char *path, const char *label,
               int before) {
    return tree_insert(aug->origin, path, label, before);
}

struct tree *make_tree(char *label, char *value, struct tree *parent,
                       struct tree *children) {
    struct tree *tree;
    CALLOC(tree, 1);
    if (tree == NULL)
        return NULL;

    tree->label = label;
    tree->value = value;
    tree->parent = parent;
    tree->children = children;
    list_for_each(c, tree->children)
        c->parent = tree;
    tree->dirty = 1;
    return tree;
}

struct tree *make_tree_origin(struct tree *root) {
    struct tree *origin = NULL;

    origin = make_tree(NULL, NULL, NULL, root);
    if (origin == NULL)
        return NULL;

    origin->parent = origin;
    return origin;
}

/* Free one tree node */
static void free_tree_node(struct tree *tree) {
    if (tree == NULL)
        return;

    free(tree->label);
    free(tree->value);
    free(tree);
}

/* Recursively free the whole tree TREE and all its siblings */
int free_tree(struct tree *tree) {
    int cnt = 0;

    while (tree != NULL) {
        struct tree *del = tree;
        tree = del->next;
        cnt += free_tree(del->children);
        free_tree_node(del);
        cnt += 1;
    }

    return cnt;
}

int tree_rm(struct tree *origin, const char *path) {
    assert(origin->parent == origin);
    assert(origin->next == NULL);

    struct path *p = NULL;
    struct tree *root = origin->children;
    struct tree *tree, **del;
    int cnt = 0, ndel = 0, i;

    p = make_path(root, path);
    if (p == NULL)
        return -1;

    for (tree = path_first(p); tree != NULL; tree = path_next(p, tree)) {
        if (! TREE_HIDDEN(tree))
            ndel += 1;
    }

    if (ndel == 0) {
        free_path(p);
        return 0;
    }

    if (ALLOC_N(del, ndel) < 0) {
        free(del);
        return -1;
    }

    for (i = 0, tree = path_first(p); tree != NULL; tree = path_next(p, tree)) {
        if (TREE_HIDDEN(tree))
            continue;
        del[i] = tree;
        i += 1;
    }
    free_path(p);

    for (i = 0; i < ndel; i++) {
        assert (del[i]->parent != NULL);
        list_remove(del[i], del[i]->parent->children);
        del[i]->parent->dirty = 1;
        cnt += free_tree(del[i]->children) + 1;
        free_tree_node(del[i]);
    }
    free(del);

    return cnt;
}

int aug_rm(struct augeas *aug, const char *path) {
    return tree_rm(aug->origin, path);
}

int aug_tree_replace(struct augeas *aug, const char *path, struct tree *sub) {
    struct tree *parent;
    int r;

    r = aug_rm(aug, path);
    if (r == -1)
        goto error;

    parent = tree_set(aug->origin->children, path, NULL);
    if (parent == NULL)
        goto error;

    list_append(parent->children, sub);
    list_for_each(s, sub) {
        s->parent = parent;
    }
    return 0;
 error:
    return -1;
}

int aug_mv(struct augeas *aug, const char *src, const char *dst) {
    struct tree *root = aug->origin->children;
    struct path *s = make_path(root, src);
    struct path *d = make_path(root, dst);
    struct tree *ts, *td, *t;
    int r, ret, segnr;

    ret = -1;
    if (s == NULL || d == NULL)
        goto done;

    r = path_find_one(s, &ts, NULL);
    if (r != 1)
        goto done;

    r = path_find_one(d, &td, &segnr);
    if (r == -1)
        goto done;

    if (r == 0) {
        if ((td = tree_create(d, td, segnr)) == NULL)
            goto done;
    }

    /* Don't move SRC into its own descendent */
    t = td;
    do {
        if (t == ts)
            goto done;
        t = t->parent;
    } while (! ROOT_P(t));

    free_tree(td->children);

    td->children = ts->children;
    list_for_each(c, td->children) {
        c->parent = td;
    }
    free(td->value);
    td->value = ts->value;

    ts->value = NULL;
    ts->children = NULL;


    list_remove(ts, ts->parent->children);

    ts->parent->dirty = 1;
    td->dirty = 1;

    free_tree(ts);

    ret = 0;
 done:
    free_path(s);
    free_path(d);
    return ret;
}

int aug_match(const struct augeas *aug, const char *pathin, char ***matches) {
    struct path *p = NULL;
    struct tree *tree;
    int cnt = 0;

    if (matches != NULL)
        *matches = NULL;

    if (STREQ(pathin, "/")) {
        pathin = "/*";
    }

    p = make_path(aug->origin->children, pathin);
    if (p == NULL)
        return -1;

    for (tree = path_first(p); tree != NULL; tree = path_next(p, tree)) {
        if (! TREE_HIDDEN(tree))
            cnt += 1;
    }
    free_path(p);
    p = NULL;

    if (matches == NULL)
        return cnt;

    CALLOC(*matches, cnt);
    if (*matches == NULL)
        goto error;

    p = make_path(aug->origin->children, pathin);
    int i = 0;
    for (tree = path_first(p); tree != NULL; tree = path_next(p, tree)) {
        if (TREE_HIDDEN(tree))
            continue;
        (*matches)[i] = format_path(tree);
        if ((*matches)[i] == NULL) {
            goto error;
        }
        i += 1;
    }
    free_path(p);
    return cnt;

 error:
    if (matches != NULL) {
        if (*matches != NULL) {
            for (i=0; i < cnt; i++)
                free((*matches)[i]);
            free(*matches);
        }
    }
    free_path(p);
    return -1;
}

static int tree_save(struct augeas *aug, struct tree *tree, const char *path,
                     int *count) {
    int result = 0;
    // FIXME: We need to detect subtrees that aren't saved by anything
    aug_rm(aug, AUGEAS_EVENTS_SAVED);

    list_for_each(t, tree) {
        if (t->dirty) {
            char *tpath = NULL;
            struct transform *transform = NULL;
            if (asprintf(&tpath, "%s/%s", path, t->label) == -1) {
                result = -1;
                continue;
            }
            list_for_each(modl, aug->modules) {
                struct transform *xform = modl->autoload;
                if (xform == NULL)
                    continue;
                if (transform_applies(xform, tpath)) {
                    if (transform == NULL || transform == xform) {
                        transform = xform;
                    } else {
                        FIXME("Multiple transforms for %s", path);
                        result = -1;
                    }
                }
            }
            if (transform != NULL) {
                int r = transform_save(aug, transform, tpath, t);
                if (r == -1)
                    result = -1;
                else if (r > 0) {
                    char *sp;
                    *count += 1;
                    if (asprintf(&sp, AUGEAS_EVENTS_SAVED "[%d]",
                                 *count) < 0) {
                        free(tpath);
                        return -1;
                    }
                    if (aug_set(aug, sp, tpath) < 0) {
                        free(tpath);
                        free(sp);
                        return -1;
                    }
                    free(sp);
                }
            } else {
                if (tree_save(aug, t->children, tpath, count) == -1)
                    result = -1;
            }
            free(tpath);
        }
    }
    return result;
}

int aug_save(struct augeas *aug) {
    int ret = 0;
    struct tree *files;
    struct path *p = make_path(aug->origin->children, AUGEAS_FILES_TREE);

    if (p == NULL || path_find_one(p, &files, NULL) != 1) {
        free_path(p);
        return -1;
    }
    free_path(p);

    list_for_each(t, aug->origin->children) {
        tree_propagate_dirty(t);
    }
    if (files->dirty) {
        int count = 0;
        list_for_each(t, files->children) {
            if (tree_save(aug, t, AUGEAS_FILES_TREE, &count) == -1)
                ret = -1;
        }
    }
    tree_clean(aug->origin->children);
    return ret;
}

static int print_one(FILE *out, const char *path, const char *value) {
    int r;

    r = fprintf(out, "%s", path);
    if (r < 0)
        return -1;
    if (value != NULL) {
        char *val = escape(value, -1);
        r = fprintf(out, " = \"%s\"", val);
        free(val);
        if (r < 0)
            return -1;
    }
    r = fputc('\n', out);
    if (r == EOF)
        return -1;
    return 0;
}

/* PATH is the path up to TREE's parent */
static int print_rec(FILE *out, struct tree *start, const char *ppath,
                     int pr_hidden) {
    int r;
    char *path = NULL;

    list_for_each(tree, start) {
        if (TREE_HIDDEN(tree) && ! pr_hidden)
            continue;

        path = path_expand(tree, ppath);
        if (path == NULL)
            goto error;

        r = print_one(out, path, tree->value);
        if (r < 0)
            goto error;
        r = print_rec(out, tree->children, path, pr_hidden);
        free(path);
        path = NULL;
        if (r < 0)
            goto error;
    }
    return 0;
 error:
    free(path);
    return -1;
}

int print_tree(const struct tree *start, FILE *out, const char *pathin,
               int pr_hidden) {

    struct path *p = make_path(start, pathin);
    char *path = NULL;
    struct tree *tree;
    int r;

    if (p == NULL)
        return -1;

    for (tree = path_first(p); tree != NULL; tree = path_next(p, tree)) {
        if (TREE_HIDDEN(tree) && ! pr_hidden)
            continue;

        path = format_path(tree);
        if (path == NULL)
            goto error;
        r = print_one(out, path, tree->value);
        if (r < 0)
            goto error;
        r = print_rec(out, tree->children, path, pr_hidden);
        if (r < 0)
            goto error;
        free(path);
        path = NULL;
    }
    free_path(p);
    return 0;
 error:
    free(path);
    return -1;
}

int aug_print(const struct augeas *aug, FILE *out, const char *pathin) {
    if (pathin == NULL || strlen(pathin) == 0) {
        pathin = "/*";
    }
    return print_tree(aug->origin->children, out, pathin, 0);
}

void aug_close(struct augeas *aug) {
    if (aug == NULL)
        return;
    free_tree(aug->origin);
    unref(aug->modules, module);
    free((void *) aug->root);
    free(aug->modpathz);
    free(aug);
}

int tree_equal(const struct tree *t1, const struct tree *t2) {
    while (t1 != NULL && t2 != NULL) {
        if (!streqv(t1->label, t2->label))
            return 0;
        if (!streqv(t1->value, t2->value))
            return 0;
        if (! tree_equal(t1->children, t2->children))
            return 0;
        t1 = t1->next;
        t2 = t2->next;
    }
    return t1 == t2;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
