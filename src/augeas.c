/*
 * augeas.c: the core data structure for storing key/value pairs
 *
 * Copyright (C) 2007-2011 David Lutterkort
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
#include "errcode.h"

#include <fnmatch.h>
#include <argz.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>
#include <libxml/tree.h>

/* Some popular labels that we use in /augeas */
static const char *const s_augeas = "augeas";
static const char *const s_files  = "files";
static const char *const s_load   = "load";
static const char *const s_pathx  = "pathx";
static const char *const s_error  = "error";
static const char *const s_pos    = "pos";
static const char *const s_vars   = "variables";
static const char *const s_lens   = "lens";
static const char *const s_excl   = "excl";
static const char *const s_incl   = "incl";

#define TREE_HIDDEN(tree) ((tree)->label == NULL)

#define AUGEAS_META_PATHX_FUNC AUGEAS_META_TREE "/version/pathx/functions"

static const char *const static_nodes[][2] = {
    { AUGEAS_FILES_TREE, NULL },
    { AUGEAS_META_TREE "/variables", NULL },
    { AUGEAS_META_TREE "/version", PACKAGE_VERSION },
    { AUGEAS_META_TREE "/version/save/mode[1]", AUG_SAVE_BACKUP_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[2]", AUG_SAVE_NEWFILE_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[3]", AUG_SAVE_NOOP_TEXT },
    { AUGEAS_META_TREE "/version/save/mode[4]", AUG_SAVE_OVERWRITE_TEXT },
    { AUGEAS_META_TREE "/version/defvar/expr", NULL },
    { AUGEAS_META_PATHX_FUNC "/count", NULL },
    { AUGEAS_META_PATHX_FUNC "/glob", NULL },
    { AUGEAS_META_PATHX_FUNC "/label", NULL },
    { AUGEAS_META_PATHX_FUNC "/last", NULL },
    { AUGEAS_META_PATHX_FUNC "/position", NULL },
    { AUGEAS_META_PATHX_FUNC "/regexp", NULL }
};

static const char *const errcodes[] = {
    "No error",                                         /* AUG_NOERROR */
    "Cannot allocate memory",                           /* AUG_ENOMEM */
    "Internal error (please file a bug)",               /* AUG_EINTERNAL */
    "Invalid path expression",                          /* AUG_EPATHX */
    "No match for path expression",                     /* AUG_ENOMATCH */
    "Too many matches for path expression",             /* AUG_EMMATCH */
    "Syntax error in lens definition",                  /* AUG_ESYNTAX */
    "Lens not found",                                   /* AUG_ENOLENS */
    "Multiple transforms",                              /* AUG_EMXFM */
    "Node has no span info",                            /* AUG_ENOSPAN */
    "Cannot move node into its descendant",             /* AUG_EMVDESC */
    "Failed to execute command",                        /* AUG_ECMDRUN */
    "Invalid argument in function call",                /* AUG_EBADARG */
    "Invalid label"                                     /* AUG_ELABEL */
};

static void tree_mark_dirty(struct tree *tree) {
    do {
        tree->dirty = 1;
        tree = tree->parent;
    } while (tree != tree->parent && !tree->dirty);
    tree->dirty = 1;
}

void tree_clean(struct tree *tree) {
    if (tree->dirty) {
        list_for_each(c, tree->children)
            tree_clean(c);
    }
    tree->dirty = 0;
}

struct tree *tree_child(struct tree *tree, const char *label) {
    if (tree == NULL)
        return NULL;

    list_for_each(child, tree->children) {
        if (streqv(label, child->label))
            return child;
    }
    return NULL;
}

struct tree *tree_child_cr(struct tree *tree, const char *label) {
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

struct tree *tree_path_cr(struct tree *tree, int n, ...) {
    va_list ap;

    va_start(ap, n);
    for (int i=0; i < n; i++) {
        const char *l = va_arg(ap, const char *);
        tree = tree_child_cr(tree, l);
    }
    va_end(ap);
    return tree;
}

struct tree *tree_find(struct augeas *aug, const char *path) {
    struct pathx *p = NULL;
    struct tree *result = NULL;
    int r;

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);

    r = pathx_find_one(p, &result);
    BUG_ON(r > 1, aug,
           "Multiple matches for %s when only one was expected",
           path);
 done:
    free_pathx(p);
    return result;
 error:
    result = NULL;
    goto done;
}

struct tree *tree_find_cr(struct augeas *aug, const char *path) {
    struct pathx *p = NULL;
    struct tree *result = NULL;
    int r;

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);

    r = pathx_expand_tree(p, &result);
    ERR_BAIL(aug);
    ERR_THROW(r < 0, aug, AUG_EINTERNAL, "pathx_expand_tree failed");
 error:
    free_pathx(p);
    return result;
}

void tree_store_value(struct tree *tree, char **value) {
    if (streqv(tree->value, *value)) {
        free(*value);
        *value = NULL;
        return;
    }
    if (tree->value != NULL) {
        free(tree->value);
        tree->value = NULL;
    }
    if (*value != NULL) {
        tree->value = *value;
        *value = NULL;
    }
    tree_mark_dirty(tree);
}

int tree_set_value(struct tree *tree, const char *value) {
    char *v = NULL;

    if (streqv(tree->value, value))
        return 0;
    if (value != NULL) {
        v = strdup(value);
        if (v == NULL)
            return -1;
    }
    tree_store_value(tree, &v);
    return 0;
}

static void store_error(const struct augeas *aug, const char *label, const char *value,
                 int nentries, ...) {
    va_list ap;
    struct tree *tree;

    ensure(nentries % 2 == 0, aug);
    tree = tree_path_cr(aug->origin, 3, s_augeas, s_error, label);
    if (tree == NULL)
        return;

    tree_set_value(tree, value);

    va_start(ap, nentries);
    for (int i=0; i < nentries; i += 2) {
        char *l = va_arg(ap, char *);
        char *v = va_arg(ap, char *);
        struct tree *t = tree_child_cr(tree, l);
        if (t != NULL)
            tree_set_value(t, v);
    }
    va_end(ap);
 error:
    return;
}

/* Report pathx errors in /augeas/pathx/error */
static void store_pathx_error(const struct augeas *aug) {
    if (aug->error->code != AUG_EPATHX)
        return;

    store_error(aug, s_pathx, aug->error->minor_details,
                2, s_pos, aug->error->details);
}

struct pathx *pathx_aug_parse(const struct augeas *aug,
                              struct tree *tree,
                              struct tree *root_ctx,
                              const char *path, bool need_nodeset) {
    struct pathx *result;
    struct error *err = err_of_aug(aug);

    if (tree == NULL)
        tree = aug->origin;

    pathx_parse(tree, err, path, need_nodeset, aug->symtab, root_ctx, &result);
    return result;
}

/* Find the tree stored in AUGEAS_CONTEXT */
struct tree *tree_root_ctx(const struct augeas *aug) {
    struct pathx *p = NULL;
    struct tree *match = NULL;
    const char *ctx_path;
    int r;

    p = pathx_aug_parse(aug, aug->origin, NULL, AUGEAS_CONTEXT, true);
    ERR_BAIL(aug);

    r = pathx_find_one(p, &match);
    ERR_THROW(r > 1, aug, AUG_EMMATCH,
              "There are %d nodes matching %s, expecting one",
              r, AUGEAS_CONTEXT);

    if (match == NULL || match->value == NULL || *match->value == '\0')
        goto error;

    /* Clean via augrun's helper to ensure it's valid */
    ctx_path = cleanpath(match->value);
    free_pathx(p);

    p = pathx_aug_parse(aug, aug->origin, NULL, ctx_path, true);
    ERR_BAIL(aug);

    if (pathx_first(p) == NULL) {
        r = pathx_expand_tree(p, &match);
        if (r < 0)
            goto done;
        r = tree_set_value(match, NULL);
        if (r < 0)
            goto done;
    } else {
        r = pathx_find_one(p, &match);
        ERR_THROW(r > 1, aug, AUG_EMMATCH,
                  "There are %d nodes matching the context %s, expecting one",
                  r, ctx_path);
    }

 done:
    free_pathx(p);
    return match;
 error:
    match = NULL;
    goto done;
}

struct tree *tree_append(struct tree *parent,
                         char *label, char *value) {
    struct tree *result = make_tree(label, value, parent, NULL);
    if (result != NULL)
        list_append(parent->children, result);
    return result;
}

static struct tree *tree_append_s(struct tree *parent,
                                  const char *l0, char *v) {
    struct tree *result;
    char *l = strdup(l0);

    if (l == NULL)
        return NULL;
    result = tree_append(parent, l, v);
    if (result == NULL)
        free(l);
    return result;
}

static struct tree *tree_from_transform(struct augeas *aug,
                                        const char *modname,
                                        struct transform *xfm) {
    struct tree *meta = tree_child_cr(aug->origin, s_augeas);
    struct tree *load = NULL, *txfm = NULL, *t;
    char *v = NULL;
    int r;

    ERR_NOMEM(meta == NULL, aug);

    load = tree_child_cr(meta, s_load);
    ERR_NOMEM(load == NULL, aug);

    if (modname == NULL)
        modname = "_";

    txfm = tree_append_s(load, modname, NULL);
    ERR_NOMEM(txfm == NULL, aug);

    r = asprintf(&v, "@%s", modname);
    ERR_NOMEM(r < 0, aug);

    t = tree_append_s(txfm, s_lens, v);
    ERR_NOMEM(t == NULL, aug);
    v = NULL;

    list_for_each(f, xfm->filter) {
        const char *l = f->include ? s_incl : s_excl;
        v = strdup(f->glob->str);
        ERR_NOMEM(v == NULL, aug);
        t = tree_append_s(txfm, l, v);
        ERR_NOMEM(t == NULL, aug);
    }
    return txfm;
 error:
    free(v);
    tree_unlink(txfm);
    return NULL;
}

/* Save user locale and switch to C locale */
#if HAVE_USELOCALE
static void save_locale(struct augeas *aug) {
    if (aug->c_locale == NULL) {
        aug->c_locale = newlocale(LC_ALL_MASK, "C", NULL);
        ERR_NOMEM(aug->c_locale == NULL, aug);
    }

    aug->user_locale = uselocale(aug->c_locale);
 error:
    return;
}
#else
static void save_locale(ATTRIBUTE_UNUSED struct augeas *aug) { }
#endif

#if HAVE_USELOCALE
static void restore_locale(struct augeas *aug) {
    uselocale(aug->user_locale);
    aug->user_locale = NULL;
}
#else
static void restore_locale(ATTRIBUTE_UNUSED struct augeas *aug) { }
#endif

/* Clean up old error messages every time we enter through the public
 * API. Since we make internal calls through the public API, we keep a
 * count of how many times a public API call was made, and only reset when
 * that count is 0. That requires that all public functions enclose their
 * work within a matching pair of api_entry/api_exit calls.
 */
void api_entry(const struct augeas *aug) {
    struct error *err = ((struct augeas *) aug)->error;

    ((struct augeas *) aug)->api_entries += 1;

    if (aug->api_entries > 1)
        return;

    reset_error(err);
    save_locale((struct augeas *) aug);
}

void api_exit(const struct augeas *aug) {
    assert(aug->api_entries > 0);
    ((struct augeas *) aug)->api_entries -= 1;
    if (aug->api_entries == 0) {
        store_pathx_error(aug);
        restore_locale((struct augeas *) aug);
    }
}

static int init_root(struct augeas *aug, const char *root0) {
    if (root0 == NULL)
        root0 = getenv(AUGEAS_ROOT_ENV);
    if (root0 == NULL || root0[0] == '\0')
        root0 = "/";

    aug->root = strdup(root0);
    if (aug->root == NULL)
        return -1;

    if (aug->root[strlen(aug->root)-1] != SEP) {
        if (REALLOC_N(aug->root, strlen(aug->root) + 2) < 0)
            return -1;
        strcat((char *) aug->root, "/");
    }
    return 0;
}

static int init_loadpath(struct augeas *aug, const char *loadpath) {
    int r;

    aug->modpathz = NULL;
    aug->nmodpath = 0;
    if (loadpath != NULL) {
        r = argz_add_sep(&aug->modpathz, &aug->nmodpath,
                         loadpath, PATH_SEP_CHAR);
        if (r != 0)
            return -1;
    }
    char *env = getenv(AUGEAS_LENS_ENV);
    if (env != NULL) {
        r = argz_add_sep(&aug->modpathz, &aug->nmodpath,
                         env, PATH_SEP_CHAR);
        if (r != 0)
            return -1;
    }
    if (!(aug->flags & AUG_NO_STDINC)) {
        r = argz_add(&aug->modpathz, &aug->nmodpath, AUGEAS_LENS_DIR);
        if (r != 0)
            return -1;
        r = argz_add(&aug->modpathz, &aug->nmodpath,
                     AUGEAS_LENS_DIST_DIR);
        if (r != 0)
            return -1;
    }
    /* Clean up trailing slashes */
    if (aug->nmodpath > 0) {
        argz_stringify(aug->modpathz, aug->nmodpath, PATH_SEP_CHAR);
        char *s, *t;
        const char *e = aug->modpathz + strlen(aug->modpathz);
        for (s = aug->modpathz, t = aug->modpathz; s < e; s++) {
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
        s = aug->modpathz;
        aug->modpathz = NULL;
        r = argz_create_sep(s, PATH_SEP_CHAR, &aug->modpathz,
                            &aug->nmodpath);
        free(s);
        if (r != 0)
            return -1;
    }
    return 0;
}

static void init_save_mode(struct augeas *aug) {
    const char *v = AUG_SAVE_OVERWRITE_TEXT;

    if (aug->flags & AUG_SAVE_NEWFILE) {
        v = AUG_SAVE_NEWFILE_TEXT;
    } else if (aug->flags & AUG_SAVE_BACKUP) {
        v = AUG_SAVE_BACKUP_TEXT;
    } else if (aug->flags & AUG_SAVE_NOOP) {
        v = AUG_SAVE_NOOP_TEXT;
    }

    aug_set(aug, AUGEAS_META_SAVE_MODE, v);
}

struct augeas *aug_init(const char *root, const char *loadpath,
                        unsigned int flags) {
    struct augeas *result;
    struct tree *tree_root = make_tree(NULL, NULL, NULL, NULL);
    int r;
    bool close_on_error = true;

    if (tree_root == NULL)
        return NULL;

    if (ALLOC(result) < 0)
        goto error;
    if (ALLOC(result->error) < 0)
        goto error;
    if (make_ref(result->error->info) < 0)
        goto error;
    result->error->info->error = result->error;
    result->error->info->filename = dup_string("(unknown file)");
    if (result->error->info->filename == NULL)
        goto error;
    result->error->aug = result;

    result->origin = make_tree_origin(tree_root);
    if (result->origin == NULL) {
        free_tree(tree_root);
        goto error;
    }

    api_entry(result);

    result->flags = flags;

    r = init_root(result, root);
    ERR_NOMEM(r < 0, result);

    result->origin->children->label = strdup(s_augeas);

    /* We are now initialized enough that we can dare return RESULT even
     * when we encounter errors if the caller so wishes */
    close_on_error = !(flags & AUG_NO_ERR_CLOSE);

    r = init_loadpath(result, loadpath);
    ERR_NOMEM(r < 0, result);

    /* We report the root dir in AUGEAS_META_ROOT, but we only use the
       value we store internally, to avoid any problems with
       AUGEAS_META_ROOT getting changed. */
    aug_set(result, AUGEAS_META_ROOT, result->root);
    ERR_BAIL(result);

    /* Set the default path context */
    aug_set(result, AUGEAS_CONTEXT, AUG_CONTEXT_DEFAULT);
    ERR_BAIL(result);

    for (int i=0; i < ARRAY_CARDINALITY(static_nodes); i++) {
        aug_set(result, static_nodes[i][0], static_nodes[i][1]);
        ERR_BAIL(result);
    }

    init_save_mode(result);
    ERR_BAIL(result);

    const char *v = (flags & AUG_ENABLE_SPAN) ? AUG_ENABLE : AUG_DISABLE;
    aug_set(result, AUGEAS_SPAN_OPTION, v);
    ERR_BAIL(result);

    if (interpreter_init(result) == -1)
        goto error;

    list_for_each(modl, result->modules) {
        struct transform *xform = modl->autoload;
        if (xform == NULL)
            continue;
        tree_from_transform(result, modl->name, xform);
        ERR_BAIL(result);
    }
    if (!(result->flags & AUG_NO_LOAD))
        if (aug_load(result) < 0)
            goto error;

    api_exit(result);
    return result;

 error:
    if (close_on_error) {
        aug_close(result);
        result = NULL;
    }
    if (result != NULL && result->api_entries > 0)
        api_exit(result);
    return result;
}

void tree_unlink_children(struct augeas *aug, struct tree *tree) {
    if (tree == NULL)
        return;

    pathx_symtab_remove_descendants(aug->symtab, tree);

    while (tree->children != NULL)
        tree_unlink(tree->children);
}

static void tree_mark_files(struct tree *tree) {
    if (tree_child(tree, "path") != NULL) {
        tree_mark_dirty(tree);
    } else {
        list_for_each(c, tree->children) {
            tree_mark_files(c);
        }
    }
}

static void tree_rm_dirty_files(struct augeas *aug, struct tree *tree) {
    struct tree *p;

    if (!tree->dirty)
        return;

    if ((p = tree_child(tree, "path")) != NULL) {
        aug_rm(aug, p->value);
        tree_unlink(tree);
    } else {
        struct tree *c = tree->children;
        while (c != NULL) {
            struct tree *next = c->next;
            tree_rm_dirty_files(aug, c);
            c = next;
        }
    }
}

static void tree_rm_dirty_leaves(struct augeas *aug, struct tree *tree,
                                 struct tree *protect) {
    if (! tree->dirty)
        return;

    struct tree *c = tree->children;
    while (c != NULL) {
        struct tree *next = c->next;
        tree_rm_dirty_leaves(aug, c, protect);
        c = next;
    }

    if (tree != protect && tree->children == NULL)
        tree_unlink(tree);
}

int aug_load(struct augeas *aug) {
    const char *option = NULL;
    struct tree *meta = tree_child_cr(aug->origin, s_augeas);
    struct tree *meta_files = tree_child_cr(meta, s_files);
    struct tree *files = tree_child_cr(aug->origin, s_files);
    struct tree *load = tree_child_cr(meta, s_load);
    struct tree *vars = tree_child_cr(meta, s_vars);

    api_entry(aug);

    ERR_NOMEM(load == NULL, aug);

    /* To avoid unnecessary loads of files, we reload an existing file in
     * several steps:
     * (1) mark all file nodes under /augeas/files as dirty (and only those)
     * (2) process all files matched by a lens; we check (in
     *     transform_load) if the file has been modified. If it has, we
     *     reparse it. Either way, we clear the dirty flag. We also need to
     *     reread the file if part or all of it has been modified in the
     *     tree but not been saved yet
     * (3) remove all files from the tree that still have a dirty entry
     *     under /augeas/files. Those files are not processed by any lens
     *     anymore
     * (4) Remove entries from /augeas/files and /files that correspond
     *     to directories without any files of interest
     */

    /* update flags according to option value */
    if (aug_get(aug, AUGEAS_SPAN_OPTION, &option) == 1) {
        if (strcmp(option, AUG_ENABLE) == 0) {
            aug->flags |= AUG_ENABLE_SPAN;
        } else {
            aug->flags &= ~AUG_ENABLE_SPAN;
        }
    }

    tree_clean(meta_files);
    tree_mark_files(meta_files);

    list_for_each(xfm, load->children) {
        if (transform_validate(aug, xfm) == 0)
            transform_load(aug, xfm);
    }

    /* This makes it possible to spot 'directories' that are now empty
     * because we removed their file contents */
    tree_clean(files);

    tree_rm_dirty_files(aug, meta_files);
    tree_rm_dirty_leaves(aug, meta_files, meta_files);
    tree_rm_dirty_leaves(aug, files, files);

    tree_clean(aug->origin);

    list_for_each(v, vars->children) {
        aug_defvar(aug, v->label, v->value);
        ERR_BAIL(aug);
    }

    api_exit(aug);
    return 0;
 error:
    api_exit(aug);
    return -1;
}

static int find_one_node(struct pathx *p, struct tree **match) {
    struct error *err = err_of_pathx(p);
    int r = pathx_find_one(p, match);

    if (r == 1)
        return 0;

    if (r == 0) {
        report_error(err, AUG_ENOMATCH, NULL);
    } else {
        /* r > 1 */
        report_error(err, AUG_EMMATCH, NULL);
    }

    return -1;
}

int aug_get(const struct augeas *aug, const char *path, const char **value) {
    struct pathx *p = NULL;
    struct tree *match;
    int r;

    api_entry(aug);

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);

    if (value != NULL)
        *value = NULL;

    r = pathx_find_one(p, &match);
    ERR_BAIL(aug);
    ERR_THROW(r > 1, aug, AUG_EMMATCH, "There are %d nodes matching %s",
              r, path);

    if (r == 1 && value != NULL)
        *value = match->value;
    free_pathx(p);

    api_exit(aug);
    return r;
 error:
    free_pathx(p);
    api_exit(aug);
    return -1;
}

int aug_label(const struct augeas *aug, const char *path, const char **label) {
    struct pathx *p = NULL;
    struct tree *match;
    int r;

    api_entry(aug);

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);

    if (label != NULL)
        *label = NULL;

    r = pathx_find_one(p, &match);
    ERR_BAIL(aug);
    ERR_THROW(r > 1, aug, AUG_EMMATCH, "There are %d nodes matching %s",
              r, path);

    if (r == 1 && label != NULL)
        *label = match->label;
    free_pathx(p);

    api_exit(aug);
    return r;
 error:
    free_pathx(p);
    api_exit(aug);
    return -1;
}

static void record_var_meta(struct augeas *aug, const char *name,
                            const char *expr) {
    /* Record the definition of the variable */
    struct tree *tree = tree_path_cr(aug->origin, 2, s_augeas, s_vars);
    ERR_NOMEM(tree == NULL, aug);
    if (expr == NULL) {
        tree = tree_child(tree, name);
        if (tree != NULL)
            tree_unlink(tree);
    } else {
        tree = tree_child_cr(tree, name);
        ERR_NOMEM(tree == NULL, aug);
        tree_set_value(tree, expr);
    }
 error:
    return;
}

int aug_defvar(augeas *aug, const char *name, const char *expr) {
    struct pathx *p = NULL;
    int result = -1;

    api_entry(aug);

    if (expr == NULL) {
        result = pathx_symtab_undefine(&(aug->symtab), name);
    } else {
        p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), expr, false);
        ERR_BAIL(aug);
        result = pathx_symtab_define(&(aug->symtab), name, p);
    }
    ERR_BAIL(aug);

    record_var_meta(aug, name, expr);
    ERR_BAIL(aug);
 error:
    free_pathx(p);
    api_exit(aug);
    return result;
}

int aug_defnode(augeas *aug, const char *name, const char *expr,
                const char *value, int *created) {
    struct pathx *p;
    int result = -1;
    int r, cr;
    struct tree *tree;

    api_entry(aug);

    if (expr == NULL)
        goto error;
    if (created == NULL)
        created = &cr;

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), expr, false);
    ERR_BAIL(aug);

    if (pathx_first(p) == NULL) {
        r = pathx_expand_tree(p, &tree);
        if (r < 0)
            goto done;
        *created = 1;
    } else {
        *created = 0;
    }

    if (*created) {
        r = tree_set_value(tree, value);
        if (r < 0)
            goto done;
        result = pathx_symtab_assign_tree(&(aug->symtab), name, tree);
        char *e = path_of_tree(tree);
        ERR_NOMEM(e == NULL, aug)
        record_var_meta(aug, name, e);
        free(e);
        ERR_BAIL(aug);
    } else {
        result = pathx_symtab_define(&(aug->symtab), name, p);
        record_var_meta(aug, name, expr);
        ERR_BAIL(aug);
    }

 done:
    free_pathx(p);
    api_exit(aug);
    return result;
 error:
    api_exit(aug);
    return -1;
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

    api_entry(aug);

    /* Get-out clause, in case context is broken */
    struct tree *root_ctx = NULL;
    if (STRNEQ(path, AUGEAS_CONTEXT))
        root_ctx = tree_root_ctx(aug);

    p = pathx_aug_parse(aug, aug->origin, root_ctx, path, true);
    ERR_BAIL(aug);

    result = tree_set(p, value) == NULL ? -1 : 0;
    free_pathx(p);

    api_exit(aug);
    return result;
 error:
    api_exit(aug);
    return -1;
}

int aug_setm(struct augeas *aug, const char *base,
             const char *sub, const char *value) {
    struct pathx *bx = NULL, *sx = NULL;
    struct tree *bt, *st;
    int result, r;

    api_entry(aug);

    bx = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), base, true);
    ERR_BAIL(aug);

    if (sub != NULL && STREQ(sub, "."))
        sub = NULL;

    result = 0;
    for (bt = pathx_first(bx); bt != NULL; bt = pathx_next(bx)) {
        if (sub != NULL) {
            /* Handle subnodes of BT */
            sx = pathx_aug_parse(aug, bt, NULL, sub, true);
            ERR_BAIL(aug);
            if (pathx_first(sx) != NULL) {
                /* Change existing subnodes matching SUB */
                for (st = pathx_first(sx); st != NULL; st = pathx_next(sx)) {
                    r = tree_set_value(st, value);
                    ERR_NOMEM(r < 0, aug);
                    result += 1;
                }
            } else {
                /* Create a new subnode matching SUB */
                r = pathx_expand_tree(sx, &st);
                if (r == -1)
                    goto error;
                r = tree_set_value(st, value);
                ERR_NOMEM(r < 0, aug);
                result += 1;
            }
            free_pathx(sx);
            sx = NULL;
        } else {
            /* Set nodes matching BT directly */
            r = tree_set_value(bt, value);
            ERR_NOMEM(r < 0, aug);
            result += 1;
        }
    }

 done:
    api_exit(aug);
    return result;
 error:
    result = -1;
    goto done;
}

int tree_insert(struct pathx *p, const char *label, int before) {
    struct tree *new = NULL, *match;

    if (strchr(label, SEP) != NULL)
        return -1;

    if (find_one_node(p, &match) < 0)
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

    api_entry(aug);

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);

    result = tree_insert(p, label, before);
 error:
    free_pathx(p);
    api_exit(aug);
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

    if (tree->span != NULL)
        free_span(tree->span);
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
        pathx_symtab_remove_descendants(pathx_get_symtab(p), tree);
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

    api_entry(aug);

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);

    result = tree_rm(p);
    free_pathx(p);
    ERR_BAIL(aug);

    api_exit(aug);
    return result;
 error:
    api_exit(aug);
    return -1;
}

int aug_span(struct augeas *aug, const char *path, char **filename,
        uint *label_start, uint *label_end, uint *value_start, uint *value_end,
        uint *span_start, uint *span_end) {
    struct pathx *p = NULL;
    int result = -1;
    struct tree *tree = NULL;
    struct span *span;

    api_entry(aug);

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);

    tree = pathx_first(p);
    ERR_BAIL(aug);

    ERR_THROW(tree == NULL, aug, AUG_ENOMATCH, "No node matching %s", path);
    ERR_THROW(tree->span == NULL, aug, AUG_ENOSPAN, "No span info for %s", path);
    ERR_THROW(pathx_next(p) != NULL, aug, AUG_EMMATCH, "Multiple nodes match %s", path);

    span = tree->span;

    if (label_start != NULL)
        *label_start = span->label_start;

    if (label_end != NULL)
        *label_end = span->label_end;

    if (value_start != NULL)
        *value_start = span->value_start;

    if (value_end != NULL)
        *value_end = span->value_end;

    if (span_start != NULL)
        *span_start = span->span_start;

    if (span_end != NULL)
        *span_end = span->span_end;

    /* We are safer here, make sure we have a filename */
    if (filename != NULL) {
        if (span->filename == NULL || span->filename->str == NULL) {
            *filename = strdup("");
        } else {
            *filename = strdup(span->filename->str);
        }
        ERR_NOMEM(*filename == NULL, aug);
    }

    result = 0;
 error:
    free_pathx(p);
    api_exit(aug);
    return result;
}

int tree_replace(struct augeas *aug, const char *path, struct tree *sub) {
    struct tree *parent;
    struct pathx *p = NULL;
    int r;

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);

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

    api_entry(aug);

    ret = -1;
    s = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), src, true);
    ERR_BAIL(aug);

    d = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), dst, true);
    ERR_BAIL(aug);

    r = find_one_node(s, &ts);
    if (r < 0)
        goto error;

    r = pathx_expand_tree(d, &td);
    if (r == -1)
        goto error;

    /* Don't move SRC into its own descendent */
    t = td;
    do {
        ERR_THROW(t == ts, aug, AUG_EMVDESC,
                  "destination %s is a descendant of %s", dst, src);
        t = t->parent;
    } while (t != aug->origin);

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
 error:
    free_pathx(s);
    free_pathx(d);
    api_exit(aug);
    return ret;
}

int aug_rename(struct augeas *aug, const char *src, const char *lbl) {
    struct pathx *s = NULL;
    struct tree *ts;
    int ret;
    int count = 0;

    api_entry(aug);

    ret = -1;
    ERR_THROW(strchr(lbl, '/') != NULL, aug, AUG_ELABEL,
              "Label %s contains a /", lbl);

    s = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), src, true);
    ERR_BAIL(aug);

    for (ts = pathx_first(s); ts != NULL; ts = pathx_next(s)) {
        free(ts->label);
        ts->label = strdup(lbl);
        tree_mark_dirty(ts);
        count ++;
    }

    free_pathx(s);
    api_exit(aug);
    return count;
 error:
    free_pathx(s);
    api_exit(aug);
    return ret;
}

int aug_match(const struct augeas *aug, const char *pathin, char ***matches) {
    struct pathx *p = NULL;
    struct tree *tree;
    int cnt = 0;

    api_entry(aug);

    if (matches != NULL)
        *matches = NULL;

    if (STREQ(pathin, "/")) {
        pathin = "/*";
    }

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), pathin, true);
    ERR_BAIL(aug);

    for (tree = pathx_first(p); tree != NULL; tree = pathx_next(p)) {
        if (! TREE_HIDDEN(tree))
            cnt += 1;
    }
    ERR_BAIL(aug);

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
    ERR_BAIL(aug);
 done:
    free_pathx(p);
    api_exit(aug);
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
    api_exit(aug);
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
                        const char *filename =
                            tpath + strlen(AUGEAS_FILES_TREE) + 1;
                        transform_file_error(aug, "mxfm_save", filename,
                           "Lenses %s and %s could be used to save this file",
                                             xfm_lens_name(transform),
                                             xfm_lens_name(xfm));
                        ERR_REPORT(aug, AUG_EMXFM,
                                   "Path %s transformable by lens %s and %s",
                                   tpath,
                                   xfm_lens_name(transform),
                                   xfm_lens_name(xfm));
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
            if (pathx_parse(tm, err_of_aug(aug), file_nodes, true,
                            aug->symtab, NULL, &px) != PATHX_NOERROR) {
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

    api_entry(aug);

    if (update_save_flags(aug) < 0)
        goto error;

    if (files == NULL || meta == NULL || load == NULL)
        goto error;

    aug_rm(aug, AUGEAS_EVENTS_SAVED);

    list_for_each(xfm, load->children)
        transform_validate(aug, xfm);

    if (files->dirty) {
        if (tree_save(aug, files->children, AUGEAS_FILES_TREE) == -1)
            ret = -1;

        /* Remove files whose entire subtree was removed. */
        if (meta_files != NULL) {
            if (unlink_removed_files(aug, files, meta_files) < 0)
                ret = -1;
        }
    }
    if (!(aug->flags & AUG_SAVE_NOOP)) {
        tree_clean(aug->origin);
    }

    api_exit(aug);
    return ret;
 error:
    api_exit(aug);
    return -1;
}

static int print_one(FILE *out, const char *path, const char *value) {
    int r;

    r = fprintf(out, "%s", path);
    if (r < 0)
        return -1;
    if (value != NULL) {
        char *val = escape(value, -1, STR_ESCAPES);
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

    if (pathx_parse(tree, NULL, "/*", true, NULL, NULL, &p) != PATHX_NOERROR)
        return -1;

    result = print_tree(out, p, 1);
    free_pathx(p);
    return result;
}

static int to_xml_one(xmlNodePtr elem, const struct tree *tree,
                      const char *pathin) {
    xmlNodePtr value;
    xmlAttrPtr prop;

    prop = xmlSetProp(elem, BAD_CAST "label", BAD_CAST tree->label);
    if (prop == NULL)
        goto error;

    if (tree->span) {
        prop = xmlSetProp(elem, BAD_CAST "file",
                          BAD_CAST tree->span->filename->str);
        if (prop == NULL)
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

int aug_text_store(augeas *aug, const char *lens, const char *node,
                   const char *path) {

    struct pathx *p;
    const char *src;
    int result = -1, r;

    api_entry(aug);

    /* Validate PATH is syntactically correct */
    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), path, true);
    ERR_BAIL(aug);
    free_pathx(p);

    r = aug_get(aug, node, &src);
    ERR_BAIL(aug);
    ERR_THROW(r == 0, aug, AUG_ENOMATCH,
              "Source node %s does not exist", node);
    ERR_THROW(src == NULL, aug, AUG_ENOMATCH,
              "Source node %s has a NULL value", node);

    result = text_store(aug, lens, path, src);
 error:
    api_exit(aug);
    return result;
}

int aug_text_retrieve(struct augeas *aug, const char *lens,
                      const char *node_in, const char *path,
                      const char *node_out) {
    struct tree *tree = NULL;
    const char *src;
    char *out = NULL;
    struct tree *tree_out;
    int r;

    api_entry(aug);

    tree = tree_find(aug, path);
    ERR_BAIL(aug);

    r = aug_get(aug, node_in, &src);
    ERR_BAIL(aug);
    ERR_THROW(r == 0, aug, AUG_ENOMATCH,
              "Source node %s does not exist", node_in);
    ERR_THROW(src == NULL, aug, AUG_ENOMATCH,
              "Source node %s has a NULL value", node_in);

    r = text_retrieve(aug, lens, path, tree, src, &out);
    if (r < 0)
        goto error;

    tree_out = tree_find_cr(aug, node_out);
    ERR_BAIL(aug);

    tree_store_value(tree_out, &out);

    api_exit(aug);
    return 0;
 error:
    free(out);
    api_exit(aug);
    return -1;
}

int aug_to_xml(const struct augeas *aug, const char *pathin,
               xmlNode **xmldoc, unsigned int flags) {
    struct pathx *p;
    int result;

    api_entry(aug);

    ARG_CHECK(flags != 0, aug, "aug_to_xml: FLAGS must be 0");
    ARG_CHECK(xmldoc == NULL, aug, "aug_to_xml: XMLDOC must be non-NULL");

    if (pathin == NULL || strlen(pathin) == 0 || strcmp(pathin, "/") == 0) {
        pathin = "/*";
    }

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), pathin, true);
    ERR_BAIL(aug);
    result = tree_to_xml(p, xmldoc, pathin);
    ERR_THROW(result < 0, aug, AUG_ENOMEM, NULL);
    free_pathx(p);
    api_exit(aug);

    return result;
 error:
    if (xmldoc !=NULL)
        *xmldoc = NULL;
    api_exit(aug);
    return -1;
}

int aug_transform(struct augeas *aug, const char *lens,
                  const char *file, int excl) {
    struct tree *meta = tree_child_cr(aug->origin, s_augeas);
    struct tree *load = tree_child_cr(meta, s_load);

    int r = 0, result = -1;
    struct tree *xfm = NULL, *lns = NULL, *t = NULL;
    const char *filter = NULL;
    char *p;
    int exists;
    char *lensname = NULL, *xfmname = NULL;

    api_entry(aug);

    ERR_NOMEM(meta == NULL || load == NULL, aug);

    ARG_CHECK(STREQ("", lens), aug, "aug_transform: LENS must not be empty");
    ARG_CHECK(STREQ("", file), aug, "aug_transform: FILE must not be empty");

    if ((p = strrchr(lens, '.'))) {
        lensname = strdup(lens);
        xfmname = strndup(lens, p - lens);
        ERR_NOMEM(lensname == NULL || xfmname == NULL, aug);
    } else {
        r = xasprintf(&lensname, "%s.lns", lens);
        xfmname = strdup(lens);
        ERR_NOMEM(r < 0 || xfmname == NULL, aug);
    }

    xfm = tree_child_cr(load, xfmname);
    ERR_NOMEM(xfm == NULL, aug);

    lns = tree_child_cr(xfm, s_lens);
    ERR_NOMEM(lns == NULL, aug);

    tree_store_value(lns, &lensname);

    exists = 0;

    filter = excl ? s_excl : s_incl;
    list_for_each(c, xfm->children) {
        if (c->value != NULL && STREQ(c->value, file)
            && streqv(c->label, filter)) {
            exists = 1;
            break;
        }
    }
    if (! exists) {
        t = tree_append_s(xfm, filter, NULL);
        ERR_NOMEM(t == NULL, aug);
        r = tree_set_value(t, file);
        ERR_NOMEM(r < 0, aug);
    }

    result = 0;
 error:
    free(lensname);
    free(xfmname);
    api_exit(aug);
    return result;
}

int aug_print(const struct augeas *aug, FILE *out, const char *pathin) {
    struct pathx *p;
    int result;

    api_entry(aug);

    if (pathin == NULL || strlen(pathin) == 0) {
        pathin = "/*";
    }

    p = pathx_aug_parse(aug, aug->origin, tree_root_ctx(aug), pathin, true);
    ERR_BAIL(aug);

    result = print_tree(out, p, 0);
    free_pathx(p);

    api_exit(aug);
    return result;
 error:
    api_exit(aug);
    return -1;
}

void aug_close(struct augeas *aug) {
    if (aug == NULL)
        return;

    /* There's no point in bothering with api_entry/api_exit here */
    free_tree(aug->origin);
    unref(aug->modules, module);
    if (aug->error->exn != NULL) {
        aug->error->exn->ref = 0;
        free_value(aug->error->exn);
        aug->error->exn = NULL;
    }
    free((void *) aug->root);
    free(aug->modpathz);
    free_symtab(aug->symtab);
    unref(aug->error->info, info);
    free(aug->error->details);
    free(aug->error);
    free(aug);
}

int __aug_load_module_file(struct augeas *aug, const char *filename) {
    api_entry(aug);
    int r = load_module_file(aug, filename);
    api_exit(aug);
    return r;
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
 * Error reporting API
 */
int aug_error(struct augeas *aug) {
    return aug->error->code;
}

const char *aug_error_message(struct augeas *aug) {
    aug_errcode_t errcode = aug->error->code;

    if (errcode >= ARRAY_CARDINALITY(errcodes))
        errcode = AUG_EINTERNAL;
    return errcodes[errcode];
}

const char *aug_error_minor_message(struct augeas *aug) {
    return aug->error->minor_details;
}

const char *aug_error_details(struct augeas *aug) {
    return aug->error->details;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
