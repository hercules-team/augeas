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

#include "augeas.h"
#include "internal.h"
#include "memory.h"
#include "config.h"
#include "syntax.h"

#include <fnmatch.h>
#include <argz.h>

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
    struct tree *tree;
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

static int sibling_index(struct tree *siblings, struct tree *tree) {
    int indx = 0;
    list_for_each(t, siblings) {
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

/* Return the start of the list of siblings of SEG->TREE */
static struct tree *seg_siblings(struct path *path, struct segment *seg) {
    return (seg == path->segments) ? path->root : (seg-1)->tree->children;
}

/* Return the parent of SEG->TREE or NULL if SEG->TREE is on the toplevel */
static struct tree *seg_parent(struct path *path, struct segment *seg) {
    return (seg == path->segments) ? NULL : (seg-1)->tree;
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
            if (*p) result->nsegments++;
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

static int complete_path(struct path *path, int segnr, struct tree *tree) {
    if (segnr == path->nsegments)
        return 1;

    if (tree == NULL)
        return 0;

    int cur = segnr;
    path->segments[cur].tree = tree;

    while (1) {
        struct segment *seg = path->segments + cur;
        int found = 0;
        struct tree *siblings = seg_siblings(path, seg);
        list_for_each(t, seg->tree) {
            if (seg->any || streqv(t->label, seg->label)) {
                int indx = sibling_index(siblings, t);
                if (!seg->fixed || seg->last || indx == seg->index) {
                    found = 1;
                    seg->tree = t;
                    seg->index = indx;
                    if (!seg->last)
                        break;
                }
            }
        }
        if (found) {
            cur += 1;
            if (cur == path->nsegments)
                break;
            path->segments[cur].tree = seg->tree->children;
        } else {
            cur -= 1;
            if (cur < segnr)
                break;
            path->segments[cur].tree = path->segments[cur].tree->next;
        }
    }
    return (cur == path->nsegments);
}

static struct tree *path_next(struct path *path) {
    for (int i = path->nsegments-1; i >= 0; i--) {
        struct segment *seg = path->segments + i;
        struct tree *siblings = seg_siblings(path, seg);
        if (! seg->fixed) {
            list_for_each(t, seg->tree->next) {
                if (seg->any || streqv(t->label, seg->label)) {
                    seg->index += 1;
                    if (complete_path(path, i+1, t->children)) {
                        seg->tree = t;
                        if (seg->any)
                            seg->index = sibling_index(siblings, t);
                        return last_segment(path)->tree;
                    }
                }
            }
        }
    }
    return NULL;
}

/* Find the first node in TREE matching PATH. Fill in the TREE pointers along
 * the way
 */
static struct tree *path_first(struct path *path) {
    struct tree *tree = path->root;
    struct segment *seg = path->segments;
    int indx = 0;
    int found = 0;

    seg->tree = NULL;
    list_for_each(t, tree) {
        if (seg->any || streqv(t->label, seg->label)) {
            indx += 1;
            if (seg->last || !seg->fixed || indx == seg->index) {
                seg->tree = t;
                seg->index = indx;
                if (seg->any)
                    seg->index = sibling_index(tree, t);
                if (!seg->last && complete_path(path, 1, t->children)) {
                    found = 1;
                    break;
                }
            }
        }
    }
    if (seg->last && seg->tree && complete_path(path, 1, seg->tree->children))
        found = 1;
    return found ? last_segment(path)->tree : NULL;
}

/* Fill the TREE pointer in PATH with a longest prefix that matches in the
 * tree. PATH can not contain any wildcards, or other constructs that would
 * allow more than one path in the tree to match.
 *
 * Return 1 if a node was found that exactly matches PATH, 0 if an incomplete
 * prefix matches, and -1 if more than one node in the tree match
 */
static int path_find_one(struct path *path) {
    struct tree *tree = path->root;

    for_each_segment(seg, path) {
        int indx = 0;
        if (seg->any)
            return -1;

        seg->tree = NULL;
        list_for_each(t, tree) {
            if (streqv(t->label, seg->label)) {
                indx += 1;
                if (!seg->fixed && seg->tree != NULL)
                    return -1;

                if (!seg->fixed || seg->last || indx == seg->index) {
                    seg->tree = t;
                    seg->index = indx;
                }
            }
        }
        if (seg->tree == NULL)
            return 0;
        tree = seg->tree->children;
    }
    return 1;
}

static const char *pretty_label(const struct tree *tree) {
    if (tree == NULL)
        return "(no_tree)";
    else if (tree->label == NULL)
        return "(none)";
    else
        return tree->label;
}

static const char *seg_label(struct segment *seg) {
    if (seg->label != NULL)
        return seg->label;
    return pretty_label(seg->tree);
}

static int seg_needs_qual(struct path *path, struct segment *seg) {
    struct tree *siblings;

    if (seg->index > 1)
        return 1;
    siblings = (seg == path->segments) ? path->root : (seg-1)->tree->children;
    list_for_each(t, siblings) {
        if (t != seg->tree && streqv(t->label, seg->tree->label))
            return 1;
    }
    return 0;
}

static char *format_path(struct path *path) {
    size_t size = 0, used = 0;
    char *result;

    for_each_segment(seg, path) {
        const char *label = seg_label(seg);
        if (seg_needs_qual(path, seg)) {
            size += snprintf(NULL, 0, "/%s[%d]", label, seg->index);
        } else {
            size += strlen(label) + 1;
        }
    }
    size += 1;

    CALLOC(result, size);
    if (result == NULL)
        return NULL;
    for_each_segment(seg, path) {
        const char *label = seg_label(seg);
        if (seg_needs_qual(path, seg)) {
            used += snprintf(result + used, size-used,
                             "/%s[%d]", label, seg->index);
        } else {
            used += snprintf(result + used, size-used, "/%s", label);
        }
    }
    return result;
}

static char *path_expand(struct tree *tree, struct tree *start,
                         const char *ppath) {
    char *path;
    const char *label;
    int cnt = 0, ind = 0, inc = 1, r;

    list_for_each(t, start) {
        if (streqv(t->label, tree->label)) {
            cnt += 1;
            ind += inc;
            if (t == tree)
                inc = 0;
        }
    }
    if (cnt == 1)
        ind = 0;

    label = pretty_label(tree);
    if (ind > 0) {
        r = asprintf(&path, "%s/%s[%d]", ppath, label, ind);
    } else {
        r = asprintf(&path, "%s/%s", ppath, label);
    }
    if (r == -1)
        return NULL;
    return path;
}

static struct segment *aug_tree_create(struct path *path) {
    struct segment *s, *seg;

    for (seg = path->segments;
         seg->tree != NULL && seg <= last_segment(path);
         seg++);
    if (seg->tree != NULL)
        return NULL;

    for (s = seg ; s <= last_segment(path); s++) {
        s->tree = make_tree(strdup(s->label), NULL, NULL);
        if (s->tree == NULL || s->tree->label == NULL)
            goto error;
    }
    for (s = seg ; s < last_segment(path); s++) {
        s->tree->children = (s+1)->tree;
    }

    return seg;

 error:
    for (s = seg; s->tree != NULL && s <= last_segment(path); s++) {
        free_tree(s->tree);
        s->tree = NULL;
    }
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

    CALLOC(result, 1);
    result->tree = make_tree(NULL, NULL, NULL);

    result->flags = flags;

    result->root = init_root(root);

    result->tree->label = strdup(P_ROOT);

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
    argz_add(&result->modpathz, &result->nmodpath, AUGEAS_LENS_DIR);
    /* Clean up trailing slashes */
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
    list_for_each(tree, result->tree) {
        tree_clean(tree);
    }
    return result;

 error:
    aug_close(result);
    return NULL;
}

int aug_get(const struct augeas *aug, const char *path, const char **value) {
    struct path *p = make_path(aug->tree, path);
    int r;

    if (p == NULL)
        return -1;

    if (value != NULL)
        *value = NULL;

    r = path_find_one(p);
    if (r == 1 && value != NULL)
        *value = last_segment(p)->tree->value;
    free_path(p);

    return r;
}

struct tree *tree_set(struct tree *root, const char *path, const char *value) {
    struct tree *tree;
    struct path *p = make_path(root, path);
    int r;

    if (p == NULL)
        goto error;

    r = path_find_one(p);
    if (r == -1)
        goto error;

    if (r == 0) {
        struct segment *seg = aug_tree_create(p);
        if (seg == NULL)
            goto error;
        tree = seg_parent(p, seg);
        if (tree == NULL)
            list_append(root, seg->tree);
        else
            list_append(tree->children, seg->tree);
    }
    tree = last_segment(p)->tree;
    free_path(p);

    if (tree->value != NULL) {
        free((char *) tree->value);
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
    return tree_set(aug->tree, path, value) == NULL ? -1 : 0;
}

int aug_insert(struct augeas *aug, const char *path, const char *label,
               int before) {
    struct path *p = NULL;
    struct tree *new = NULL;

    if (strchr(label, SEP) != NULL)
        return -1;

    p = make_path(aug->tree, path);
    if (p == NULL)
        goto error;

    if (path_find_one(p) != 1)
        goto error;

    new = make_tree(strdup(label), NULL, NULL);
    if (new == NULL || new->label == NULL)
        goto error;

    struct segment *seg = last_segment(p);
    if (before) {
        struct tree *siblings = seg_siblings(p, seg);
        if (siblings == aug->tree) {
            list_insert_before(new, seg->tree, aug->tree);
        } else {
            list_insert_before(new, seg->tree, siblings);
        }
    } else {
        new->next = seg->tree->next;
        seg->tree->next = new;
    }
    return 0;
 error:
    free_tree(new);
    free_path(p);
    return -1;
}

struct tree *make_tree(const char *label, const char *value,
                       struct tree *children) {
    struct tree *tree;
    CALLOC(tree, 1);
    if (tree == NULL)
        return NULL;

    tree->label = label;
    tree->value = value;
    tree->children = children;
    tree->dirty = 1;
    return tree;
}

/* Free one tree node */
static void free_tree_node(struct tree *tree) {
    if (tree == NULL)
        return;

    free((char *) tree->label);
    free((char *) tree->value);
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

int tree_rm(struct tree **htree, const char *path) {
    struct path *p = NULL;
    struct tree *tree, **del, **parents;
    int cnt = 0, ndel = 0, i;

    p = make_path(*htree, path);
    if (p == NULL)
        return -1;

    struct segment *seg = last_segment(p);

    for (tree = path_first(p); tree != NULL; tree = path_next(p)) {
        if (! TREE_HIDDEN(tree))
            ndel += 1;
    }

    if (ndel == 0) {
        free_path(p);
        return 0;
    }

    CALLOC(del, ndel);
    CALLOC(parents, ndel);
    if (del == NULL || parents == NULL) {
        free(del);
        free(parents);
        return -1;
    }

    for (i = 0, tree = path_first(p); tree != NULL; tree = path_next(p)) {
        if (TREE_HIDDEN(tree))
            continue;
        parents[i] = seg_parent(p, seg);
        del[i] = seg->tree;
        i += 1;
    }
    free_path(p);

    for (i = 0; i < ndel; i++) {
        if (parents[i] == NULL) {
            list_remove(del[i], *htree);
            if (*htree)
                (*htree)->dirty = 1;
        } else {
            list_remove(del[i], parents[i]->children);
            parents[i]->dirty = 1;
        }
        cnt += free_tree(del[i]->children) + 1;
        free_tree_node(del[i]);
    }
    free(del);
    free(parents);

    return cnt;
}

int aug_rm(struct augeas *aug, const char *path) {
    return tree_rm(&aug->tree, path);
}

int aug_tree_replace(struct augeas *aug, const char *path, struct tree *sub) {
    struct tree *parent;
    int r;

    r = aug_rm(aug, path);
    if (r == -1)
        goto error;

    parent = tree_set(aug->tree, path, NULL);
    if (parent == NULL)
        goto error;

    list_append(parent->children, sub);
    return 0;
 error:
    return -1;
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

    p = make_path(aug->tree, pathin);
    if (p == NULL)
        return -1;

    for (tree = path_first(p); tree != NULL; tree = path_next(p)) {
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

    p = make_path(aug->tree, pathin);
    int i = 0;
    for (tree = path_first(p); tree != NULL; tree = path_next(p)) {
        if (TREE_HIDDEN(tree))
            continue;
        (*matches)[i] = format_path(p);
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

static int tree_save(struct augeas *aug, struct tree *tree, const char *path) {
    int result = 0;
    // FIXME: We need to detect subtrees that aren't saved by anything
    list_for_each(t, tree) {
        if (t->dirty) {
            char *tpath;
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
                if (transform_save(aug, transform, tpath, t) == -1)
                    result = -1;
            } else {
                if (tree_save(aug, t->children, tpath) == -1)
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
    struct path *p = make_path(aug->tree, AUGEAS_FILES_TREE);

    if (p == NULL || path_find_one(p) != 1) {
        free_path(p);
        return -1;
    }
    files = last_segment(p)->tree;
    free_path(p);

    list_for_each(t, aug->tree) {
        tree_propagate_dirty(t);
    }
    if (files->dirty) {
        list_for_each(t, files->children) {
            if (tree_save(aug, t, AUGEAS_FILES_TREE) == -1)
                ret = -1;
        }
    }
    tree_clean(aug->tree);
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

        path = path_expand(tree, start, ppath);
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

    for (tree = path_first(p); tree != NULL; tree = path_next(p)) {
        if (TREE_HIDDEN(tree) && ! pr_hidden)
            continue;

        path = format_path(p);
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
    return print_tree(aug->tree, out, pathin, 0);
}

void aug_close(struct augeas *aug) {
    if (aug == NULL)
        return;
    free_tree(aug->tree);
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
