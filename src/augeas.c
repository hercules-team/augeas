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

#define TREE_HIDDEN(tree) ((tree)->label == NULL)

static const char *const static_nodes[][2] = {
    { AUGEAS_META_TREE "/version", PACKAGE_VERSION },
    { AUGEAS_META_TREE "/version/save/mode[1]", AUG_SAVE_BACKUP_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[2]", AUG_SAVE_NEWFILE_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[3]", AUG_SAVE_NOOP_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[4]", AUG_SAVE_OVERWRITE_TEXT }
};

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

    for (int i=0; i < ARRAY_CARDINALITY(static_nodes); i++)
        aug_set(result, static_nodes[i][0], static_nodes[i][1]);

    if (flags & AUG_SAVE_NEWFILE) {
        aug_set(result, AUGEAS_META_SAVE_MODE, AUG_SAVE_NEWFILE_TEXT);
    } else if (flags & AUG_SAVE_BACKUP) {
        aug_set(result, AUGEAS_META_SAVE_MODE, AUG_SAVE_BACKUP_TEXT);
    } else if (flags & AUG_SAVE_NOOP) {
        aug_set(result, AUGEAS_META_SAVE_MODE, AUG_SAVE_NOOP_TEXT);
    } else {
        aug_set(result, AUGEAS_META_SAVE_MODE, AUG_SAVE_OVERWRITE_TEXT);
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
    struct pathx *p;
    struct tree *match;
    int r;

    if (pathx_parse(aug->origin->children, path, &p) != 0)
        return -1;

    if (value != NULL)
        *value = NULL;

    r = pathx_find_one(p, &match);
    if (r == 1 && value != NULL)
        *value = match->value;
    free_pathx(p);

    return r;
}

struct tree *tree_set(struct tree *root, const char *path, const char *value) {
    struct tree *tree;
    struct pathx *p;
    int r;

    if (pathx_parse(root, path, &p) != 0)
        goto error;

    r = pathx_expand_tree(p, &tree);
    if (r == -1)
        goto error;
    free_pathx(p);

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
    free_pathx(p);
    return NULL;
}

int aug_set(struct augeas *aug, const char *path, const char *value) {
    return tree_set(aug->origin->children, path, value) == NULL ? -1 : 0;
}

int tree_insert(struct tree *origin, const char *path, const char *label,
                int before) {
    assert(origin->parent == origin);

    struct pathx *p = NULL;
    struct tree *new = NULL, *match;

    if (strchr(label, SEP) != NULL)
        return -1;

    if (pathx_parse(origin->children, path, &p) != 0)
        goto error;

    if (pathx_find_one(p, &match) != 1)
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
    free_pathx(p);
    return 0;
 error:
    free_tree(new);
    free_pathx(p);
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

    struct pathx *p = NULL;
    struct tree *root = origin->children;
    struct tree *tree, **del;
    int cnt = 0, ndel = 0, i;

    if (pathx_parse(root, path, &p) != 0)
        return -1;

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p, tree)) {
        if (! TREE_HIDDEN(tree))
            ndel += 1;
    }

    if (ndel == 0) {
        free_pathx(p);
        return 0;
    }

    if (ALLOC_N(del, ndel) < 0) {
        free(del);
        return -1;
    }

    for (i = 0, tree = pathx_first(p); tree != NULL; tree = pathx_next(p, tree)) {
        if (TREE_HIDDEN(tree))
            continue;
        del[i] = tree;
        i += 1;
    }
    free_pathx(p);

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
    struct pathx *s, *d;
    struct tree *ts, *td, *t;
    int r, ret;

    ret = -1;
    if (pathx_parse(root, src, &s) != 0 || pathx_parse(root, dst, &d) != 0)
        goto done;

    r = pathx_find_one(s, &ts);
    if (r != 1)
        goto done;

    r = pathx_expand_tree(d, &td);
    if (r == -1)
        goto done;

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
    free_pathx(s);
    free_pathx(d);
    return ret;
}

int aug_match(const struct augeas *aug, const char *pathin, char ***matches) {
    struct pathx *p = NULL;
    struct tree *tree;
    int cnt = 0;

    if (matches != NULL)
        *matches = NULL;

    if (STREQ(pathin, "/")) {
        pathin = "/*";
    }

    if (pathx_parse(aug->origin->children, pathin, &p) != 0)
        return -1;

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p, tree)) {
        if (! TREE_HIDDEN(tree))
            cnt += 1;
    }
    free_pathx(p);
    p = NULL;

    if (matches == NULL)
        return cnt;

    CALLOC(*matches, cnt);
    if (*matches == NULL)
        goto error;

    pathx_parse(aug->origin->children, pathin, &p);

    int i = 0;
    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p, tree)) {
        if (TREE_HIDDEN(tree))
            continue;
        (*matches)[i] = format_path(tree);
        if ((*matches)[i] == NULL) {
            goto error;
        }
        i += 1;
    }
    free_pathx(p);
    return cnt;

 error:
    if (matches != NULL) {
        if (*matches != NULL) {
            for (i=0; i < cnt; i++)
                free((*matches)[i]);
            free(*matches);
        }
    }
    free_pathx(p);
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

/* Reset the flags based on what is set in the tree. */
static int update_save_flags(struct augeas *aug) {
    const char *savemode ;
    int noop = 0 ;

    aug_get(aug, AUGEAS_META_SAVE_MODE, &savemode);
    if (savemode == NULL)
        return -1;

    aug->flags &= ~(AUG_SAVE_BACKUP|AUG_SAVE_NEWFILE|AUG_SAVE_NOOP);
    if (STREQ(savemode, AUG_SAVE_NEWFILE_TEXT)) {
        aug->flags |= AUG_SAVE_NEWFILE;
    } else if (STREQ(savemode, AUG_SAVE_BACKUP_TEXT)) {
        aug->flags |= AUG_SAVE_BACKUP;
    } else if (STREQ(savemode, AUG_SAVE_NOOP_TEXT)) {
        aug->flags |= AUG_SAVE_NOOP ;
        noop = 1 ;
    } else if (STRNEQ(savemode, AUG_SAVE_OVERWRITE_TEXT)) {
        return -1;
    }

    return 0;
}

int aug_save(struct augeas *aug) {
    int ret = 0;
    struct tree *files;
    struct pathx *p = NULL;

    if (update_save_flags(aug) < 0)
        return -1;

    if (pathx_parse(aug->origin->children, AUGEAS_FILES_TREE, &p) != 0)
        return -1;
    if (pathx_find_one(p, &files) != 1) {
        free_pathx(p);
        return -1;
    }
    free_pathx(p);

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
    if (!(aug->flags & AUG_SAVE_NOOP)) {
        tree_clean(aug->origin->children);
    }
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

    struct pathx *p;
    char *path = NULL;
    struct tree *tree;
    int r;

    if (pathx_parse(start, pathin, &p) != 0)
        return -1;

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p, tree)) {
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
    free_pathx(p);
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
