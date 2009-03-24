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
#include "transform.h"

#include <fnmatch.h>
#include <argz.h>
#include <string.h>

/* Some popular labels that we use in /augeas */
static const char *const s_augeas = "augeas";
static const char *const s_files  = "files";
static const char *const s_load   = "load";
static const char *const s_pathx  = "pathx";
static const char *const s_error  = "error";
static const char *const s_pos    = "pos";

#define TREE_HIDDEN(tree) ((tree)->label == NULL)

static const char *const static_nodes[][2] = {
    { AUGEAS_META_TREE "/version", PACKAGE_VERSION },
    { AUGEAS_META_TREE "/version/save/mode[1]", AUG_SAVE_BACKUP_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[2]", AUG_SAVE_NEWFILE_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[3]", AUG_SAVE_NOOP_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[4]", AUG_SAVE_OVERWRITE_TEXT },
    { AUGEAS_META_TREE "/version/defvar", NULL }
};

static void tree_mark_dirty(struct tree *tree) {
    do {
        tree->dirty = 1;
        tree = tree->parent;
    } while (tree != tree->parent && !tree->dirty);
}

/* Clear the dirty flag in the whole TREE */
static void tree_clean(struct tree *tree) {
    if (tree->dirty) {
        list_for_each(c, tree->children)
            tree_clean(c);
    }
    tree->dirty = 0;
}

static struct tree *tree_child(struct tree *tree, const char *label) {
    if (tree == NULL)
        return NULL;

    list_for_each(child, tree->children) {
        if (streqv(label, child->label))
            return child;
    }
    return NULL;
}

/* Get first existing child or create one */
static struct tree *tree_child_cr(struct tree *tree, const char *label) {
    static struct tree *child = NULL;

    if (tree == NULL)
        return NULL;

    child = tree_child(tree, label);
    if (child == NULL) {
        char *l = strdup(label);
        if (l == NULL)
            return NULL;
        child = tree_append(tree, l, NULL);
    }
    return child;
}

static struct tree *tree_path_cr(struct tree *tree, int n, ...) {
    va_list ap;

    va_start(ap, n);
    for (int i=0; i < n; i++) {
        const char *l = va_arg(ap, const char *);
        tree = tree_child_cr(tree, l);
    }
    va_end(ap);
    return tree;
}

static int tree_set_value(struct tree *tree, const char *value) {
    if (tree->value != NULL) {
        free(tree->value);
        tree->value = NULL;
    }
    if (value != NULL) {
        tree->value = strdup(value);
        if (tree->value == NULL)
            return -1;
    }
    tree_mark_dirty(tree);
    return 0;
}

/* Parse a path expression. Report errors in /augeas/pathx/error */
static struct pathx *parse_user_pathx(const struct augeas *aug,
                                      bool need_nodeset,
                                      const char *path) {
    struct pathx *result;
    int    pos;

    if (pathx_parse(aug->origin, path, need_nodeset, aug->symtab, &result)
        == PATHX_NOERROR)
        return result;

    struct tree *error =
        tree_path_cr(aug->origin, 3, s_augeas, s_pathx, s_error);
    if (error == NULL)
        return NULL;
    tree_set_value(error, pathx_error(result, NULL, &pos));

    struct tree *tpos = tree_child_cr(error, s_pos);
    if (tpos == NULL)
        return NULL;

    free(tpos->value);
    if (ALLOC_N(tpos->value, strlen(path) + 4) >= 0) {
        strncpy(tpos->value, path, pos);
        strcat(tpos->value, "|=|");
        strcat(tpos->value, path + pos);
    }
    return NULL;
}

static const char *init_root(const char *root0) {
    char *root;

    if (root0 == NULL)
        root0 = getenv(AUGEAS_ROOT_ENV);
    if (root0 == NULL)
        root0 = "/";
    root = strdup(root0);
    if (root == NULL)
        return NULL;
    if (root[strlen(root)-1] != SEP) {
        if (REALLOC_N(root, strlen(root) + 2) == -1) {
            FREE(root);
            return NULL;
        }
        strcat(root, "/");
    }
    return root;
}

struct tree *tree_append(struct tree *parent,
                         char *label, char *value) {
    struct tree *result = make_tree(label, value, parent, NULL);
    if (result != NULL)
        list_append(parent->children, result);
    return result;
}

static struct tree *tree_from_transform(struct augeas *aug,
                                        const char *modname,
                                        struct transform *xfm) {
    struct tree *meta = tree_child_cr(aug->origin, s_augeas);
    struct tree *load = NULL, *txfm = NULL;
    char *m = NULL, *q = NULL;

    if (meta == NULL)
        goto error;

    load = tree_child_cr(meta, s_load);
    if (load == NULL)
            goto error;
    if (modname != NULL) {
        m = strdup(modname);
        if (m == NULL)
            goto error;
    } else {
        m = strdup("_");
    }
    txfm = tree_append(load, m, NULL);
    if (txfm == NULL)
        goto error;

    if (asprintf(&q, "@%s", modname) < 0)
        goto error;

    m = strdup("lens");
    tree_append(txfm, m, q);
    list_for_each(f, xfm->filter) {
        char *glob = strdup(f->glob->str);
        char *l = f->include ? strdup("incl") : strdup("excl");
        tree_append(txfm, l, glob);
    }
    return txfm;
 error:
    tree_unlink(txfm);
    return NULL;
}

struct augeas *aug_init(const char *root, const char *loadpath,
                        unsigned int flags) {
    struct augeas *result;
    struct tree *tree_root = make_tree(NULL, NULL, NULL, NULL);

    if (tree_root == NULL)
        return NULL;

    if (ALLOC(result) < 0)
        goto error;
    result->origin = make_tree_origin(tree_root);
    if (result->origin == NULL) {
        free_tree(tree_root);
        goto error;
    }

    result->flags = flags;

    result->root = init_root(root);

    result->origin->children->label = strdup(s_augeas);

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
    /* Make sure we always have /files */
    aug_set(result, AUGEAS_FILES_TREE, NULL);

    if (interpreter_init(result) == -1)
        goto error;

    list_for_each(modl, result->modules) {
        struct transform *xform = modl->autoload;
        if (xform == NULL)
            continue;
        tree_from_transform(result, modl->name, xform);
    }
    if (!(result->flags & AUG_NO_LOAD))
        if (aug_load(result) < 0)
            goto error;

    return result;

 error:
    aug_close(result);
    return NULL;
}

static void tree_unlink_children(struct tree *tree) {
    if (tree == NULL)
        return;

    while (tree->children != NULL)
        tree_unlink(tree->children);
}

int aug_load(struct augeas *aug) {
    struct tree *meta = tree_child_cr(aug->origin, s_augeas);
    struct tree *meta_files = tree_child_cr(meta, s_files);
    struct tree *files = tree_child_cr(aug->origin, s_files);
    struct tree *load = tree_child_cr(meta, s_load);

    if (load == NULL)
        return -1;

    tree_unlink_children(meta_files);
    tree_unlink_children(files);

    list_for_each(xfm, load->children) {
        if (transform_validate(aug, xfm) == 0)
            transform_load(aug, xfm);
    }
    tree_clean(aug->origin);
    return 0;
}

int aug_get(const struct augeas *aug, const char *path, const char **value) {
    struct pathx *p;
    struct tree *match;
    int r;

    p = parse_user_pathx((struct augeas *) aug, true, path);
    if (p == NULL)
        return -1;

    if (value != NULL)
        *value = NULL;

    r = pathx_find_one(p, &match);
    if (r == 1 && value != NULL)
        *value = match->value;
    free_pathx(p);

    return r;
}

int aug_defvar(augeas *aug, const char *name, const char *expr) {
    struct pathx *p;
    int result = -1;

    if (expr == NULL) {
        result = pathx_symtab_undefine(&(aug->symtab), name);
    } else {
        p = parse_user_pathx((struct augeas *) aug, false, expr);
        if (p == NULL)
            goto done;
        result = pathx_symtab_define(&(aug->symtab), name, p);
    }
 done:
    free_pathx(p);
    return result;
}

int aug_defnode(augeas *aug, const char *name, const char *expr,
                const char *value, int *created) {
    struct pathx *p;
    int result = -1;
    int r, cr;
    struct tree *tree;

    if (expr == NULL)
        return -1;
    if (created == NULL)
        created = &cr;

    p = parse_user_pathx((struct augeas *) aug, false, expr);
    if (p == NULL)
        goto done;

    r = pathx_expand_tree(p, &tree);
    if (r < 0)
        goto done;

    *created = r > 0;
    if (*created) {
        r = tree_set_value(tree, value);
        if (r < 0)
            goto done;
    }

    result = pathx_symtab_define(&(aug->symtab), name, p);

 done:
    free_pathx(p);
    return result;
}

struct tree *tree_set(struct pathx *p, const char *value) {
    struct tree *tree;
    int r;

    r = pathx_expand_tree(p, &tree);
    if (r == -1)
        return NULL;

    r = tree_set_value(tree, value);
    if (r < 0)
        return NULL;
    return tree;
}

int aug_set(struct augeas *aug, const char *path, const char *value) {
    struct pathx *p;
    int result;

    p = parse_user_pathx(aug, true, path);
    if (p == NULL)
        return -1;

    result = tree_set(p, value) == NULL ? -1 : 0;
    free_pathx(p);
    return result;
}

int tree_insert(struct pathx *p, const char *label, int before) {
    struct tree *new = NULL, *match;

    if (strchr(label, SEP) != NULL)
        return -1;

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
    return 0;
 error:
    free_tree(new);
    return -1;
}

int aug_insert(struct augeas *aug, const char *path, const char *label,
               int before) {
    struct pathx *p = NULL;
    int result = -1;

    p = parse_user_pathx(aug, true, path);
    if (p == NULL)
        goto done;

    result = tree_insert(p, label, before);
 done:
    free_pathx(p);
    return result;
}

struct tree *make_tree(char *label, char *value, struct tree *parent,
                       struct tree *children) {
    struct tree *tree;
    if (ALLOC(tree) < 0)
        return NULL;

    tree->label = label;
    tree->value = value;
    tree->parent = parent;
    tree->children = children;
    list_for_each(c, tree->children)
        c->parent = tree;
    if (parent != NULL)
        tree_mark_dirty(tree);
    else
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

int tree_unlink(struct tree *tree) {
    int result = 0;

    assert (tree->parent != NULL);
    list_remove(tree, tree->parent->children);
    tree_mark_dirty(tree->parent);
    result = free_tree(tree->children) + 1;
    free_tree_node(tree);
    return result;
}

int tree_rm(struct pathx *p) {
    struct tree *tree, **del;
    int cnt = 0, ndel = 0, i;

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p)) {
        if (! TREE_HIDDEN(tree))
            ndel += 1;
    }

    if (ndel == 0)
        return 0;

    if (ALLOC_N(del, ndel) < 0) {
        free(del);
        return -1;
    }

    for (i = 0, tree = pathx_first(p); tree != NULL; tree = pathx_next(p)) {
        if (TREE_HIDDEN(tree))
            continue;
        pathx_symtab_remove_descendants(p, tree);
        del[i] = tree;
        i += 1;
    }

    for (i = 0; i < ndel; i++)
        cnt += tree_unlink(del[i]);
    free(del);

    return cnt;
}

int aug_rm(struct augeas *aug, const char *path) {
    struct pathx *p = NULL;
    int result;

    p = parse_user_pathx(aug, true, path);
    if (p == NULL)
        return -1;

    result = tree_rm(p);
    free_pathx(p);

    return result;
}

int tree_replace(struct tree *origin, const char *path, struct tree *sub) {
    struct tree *parent;
    struct pathx *p = NULL;
    int r;

    if (pathx_parse(origin, path, true, NULL, &p) != PATHX_NOERROR)
        goto error;

    r = tree_rm(p);
    if (r == -1)
        goto error;

    parent = tree_set(p, NULL);
    if (parent == NULL)
        goto error;

    list_append(parent->children, sub);
    list_for_each(s, sub) {
        s->parent = parent;
    }
    if (sub->dirty)
        tree_mark_dirty(parent);
    free_pathx(p);
    return 0;
 error:
    free_pathx(p);
    return -1;
}

int aug_mv(struct augeas *aug, const char *src, const char *dst) {
    struct pathx *s = NULL, *d = NULL;
    struct tree *ts, *td, *t;
    int r, ret;

    ret = -1;
    s = parse_user_pathx(aug, true, src);
    if (s == NULL)
        goto done;

    d = parse_user_pathx(aug, true, dst);
    if (d == NULL)
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

    tree_unlink(ts);
    tree_mark_dirty(td);

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

    p = parse_user_pathx((struct augeas *) aug, true, pathin);
    if (p == NULL)
        return -1;

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p)) {
        if (! TREE_HIDDEN(tree))
            cnt += 1;
    }

    if (matches == NULL)
        goto done;

    if (ALLOC_N(*matches, cnt) < 0)
        goto error;

    int i = 0;
    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p)) {
        if (TREE_HIDDEN(tree))
            continue;
        (*matches)[i] = path_of_tree(tree);
        if ((*matches)[i] == NULL) {
            goto error;
        }
        i += 1;
    }
 done:
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

static int tree_save(struct augeas *aug, struct tree *tree,
                     const char *path) {
    int result = 0;
    struct tree *meta = tree_child_cr(aug->origin, s_augeas);
    struct tree *load = tree_child_cr(meta, s_load);

    // FIXME: We need to detect subtrees that aren't saved by anything

    if (load == NULL)
        return -1;

    list_for_each(t, tree) {
        if (t->dirty) {
            char *tpath = NULL;
            struct tree *transform = NULL;
            if (asprintf(&tpath, "%s/%s", path, t->label) == -1) {
                result = -1;
                continue;
            }
            list_for_each(xfm, load->children) {
                if (transform_applies(xfm, tpath)) {
                    if (transform == NULL || transform == xfm) {
                        transform = xfm;
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
            } else {
                if (tree_save(aug, t->children, tpath) == -1)
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

static int unlink_removed_files(struct augeas *aug,
                                struct tree *files, struct tree *meta) {
    /* Find all nodes that correspond to a file and might have to be
     * unlinked. A node corresponds to a file if it has a child labelled
     * 'path', and we only consider it if there are no errors associated
     * with it */
    static const char *const file_nodes =
        "descendant-or-self::*[path][count(error) = 0]";

    int result = 0;

    if (! files->dirty)
        return 0;

    for (struct tree *tm = meta->children; tm != NULL;) {
        struct tree *tf = tree_child(files, tm->label);
        struct tree *next = tm->next;
        if (tf == NULL) {
            /* Unlink all files in tm */
            struct pathx *px = NULL;
            if (pathx_parse(tm, file_nodes, true, NULL, &px)
                != PATHX_NOERROR) {
                result = -1;
                continue;
            }
            for (struct tree *t = pathx_first(px);
                 t != NULL;
                 t = pathx_next(px)) {
                remove_file(aug, t);
            }
            free_pathx(px);
        } else if (tf->dirty && ! tree_child(tm, "path")) {
            if (unlink_removed_files(aug, tf, tm) < 0)
                result = -1;
        }
        tm = next;
    }
    return result;
}

int aug_save(struct augeas *aug) {
    int ret = 0;
    struct tree *meta = tree_child_cr(aug->origin, s_augeas);
    struct tree *meta_files = tree_child_cr(meta, s_files);
    struct tree *files = tree_child_cr(aug->origin, s_files);
    struct tree *load = tree_child_cr(meta, s_load);

    if (update_save_flags(aug) < 0)
        return -1;

    if (files == NULL || meta == NULL || load == NULL)
        return -1;

    aug_rm(aug, AUGEAS_EVENTS_SAVED);

    list_for_each(xfm, load->children)
        transform_validate(aug, xfm);

    if (files->dirty) {
        list_for_each(t, files->children) {
            if (tree_save(aug, t, AUGEAS_FILES_TREE) == -1)
                ret = -1;
        }
        /* Remove files whose entire subtree was removed. */
        if (meta_files != NULL) {
            if (unlink_removed_files(aug, files, meta_files) < 0)
                ret = -1;
        }
    }
    if (!(aug->flags & AUG_SAVE_NOOP)) {
        tree_clean(aug->origin);
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

static int print_tree(FILE *out, struct pathx *p, int pr_hidden) {
    char *path = NULL;
    struct tree *tree;
    int r;

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p)) {
        if (TREE_HIDDEN(tree) && ! pr_hidden)
            continue;

        path = path_of_tree(tree);
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
    return 0;
 error:
    free(path);
    return -1;
}

int dump_tree(FILE *out, struct tree *tree) {
    struct pathx *p;
    int result;

    if (pathx_parse(tree, "/*", true, NULL, &p) != PATHX_NOERROR)
        return -1;

    result = print_tree(out, p, 1);
    free_pathx(p);
    return result;
}

int aug_print(const struct augeas *aug, FILE *out, const char *pathin) {
    struct pathx *p;
    int result;

    if (pathin == NULL || strlen(pathin) == 0) {
        pathin = "/*";
    }

    p = parse_user_pathx((struct augeas *) aug, true, pathin);
    if (p == NULL)
        return -1;

    result = print_tree(out, p, 0);
    free_pathx(p);

    return result;
}

void aug_close(struct augeas *aug) {
    if (aug == NULL)
        return;
    free_tree(aug->origin);
    unref(aug->modules, module);
    free((void *) aug->root);
    free(aug->modpathz);
    free_symtab(aug->symtab);
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
