/*
 * augeas.c: the core data structure for storing key/value pairs
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

#include "augeas.h"
#include "internal.h"
#include "config.h"
#include "syntax.h"

#include <fnmatch.h>
#include <argz.h>

/* We always have a toplevel entry for P_ROOT */
#define P_ROOT   "augeas"

struct tree *tree_find(struct tree *tree, const char *path) {
    if (path == NULL)
        return NULL;
    if (*path == SEP)
        path += 1;

    while (tree != NULL) {
        if (tree->label != NULL) {
            if (STREQ(tree->label, path))
                return tree;
            if (tree->children != NULL && pathprefix(tree->label, path)) {
                struct tree *t =
                    tree_find(tree->children, pathstrip(path));
                if (t != NULL)
                    return t;
            }
        }
        tree = tree->next;
    }
    return NULL;
}

/* Return a list of tree nodes matching PATH. The list contains the
 * matching nodes in the CHILDREN pointers and must be freed by the caller
 */
static struct tree *tree_find_all(struct tree *tree, const char *path) {
    struct tree *result = NULL;

    if (path == NULL)
        return NULL;
    if (*path == SEP)
        path += 1;

    while (tree != NULL) {
        if (tree->label != NULL) {
            if (STREQ(tree->label, path)) {
                struct tree *cons;
                CALLOC(cons, 1);
                cons->children = tree;
                list_append(result, cons);
            }
            if (tree->children != NULL && pathprefix(tree->label, path)) {
                struct tree *cdr =
                    tree_find_all(tree->children, pathstrip(path));
                list_append(result, cdr);
            }
        }
        tree = tree->next;
    }
    return result;
}

static struct tree *aug_tree_create(const char *path) {
    struct tree *tree;

    CALLOC(tree, 1);

    const char *end = pathstrip(path);
    if (end == NULL) {
        tree->label = strdup(path);
        char *l = (char *) tree->label + strlen(path) - 1;
        if (*l == SEP)
            *l = '\0';
    } else {
        tree->label = strndup(path, end - path - 1);
        tree->children = aug_tree_create(end);
    }

    return tree;
}

static struct tree *tree_find_or_create(const char *path,
                                            struct tree *tree) {
    struct tree *next = NULL;

    if (*path == SEP) path += 1;

    list_for_each(t, tree) {
        if (streqv(t->label, path))
            return t;
        if (pathprefix(t->label, path)) {
            next = t;
        }
    }

    if (next != NULL) {
        if (next->children == NULL) {
            next->children = aug_tree_create(pathstrip(path));
            while (next->children != NULL)
                next = next->children;
            return next;
        } else {
            return tree_find_or_create(pathstrip(path), next->children);
        }
    } else {
        struct tree *new = aug_tree_create(path);
        list_append(tree, new);
        while (new->children != NULL)
            new = new->children;
        return new;
    }
}

static void aug_tree_free(struct tree *tree) {
    if (tree != NULL) {
        free((void *) tree->label);
        free(tree);
    }
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
        REALLOC(root, strlen(root) + 1);
        strcat(root, "/");
    }
    return root;
}

struct augeas *aug_init(const char *root, const char *loadpath,
                        unsigned int flags) {
    struct augeas *result;

    CALLOC(result, 1);
    CALLOC(result->tree, 1);

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

const char *aug_get(struct augeas *aug, const char *path) {
    struct tree *tree;

    tree = tree_find(aug->tree, path);
    if (tree != NULL)
        return tree->value;

    return NULL;
}

int tree_set(struct tree *root, const char *path, const char *value) {
    struct tree *tree;

    tree = tree_find(root, path);
    if (tree == NULL) {
        tree = tree_find_or_create(path, root);
        if (tree == NULL)
            return -1;
    }
    if (tree->value != NULL) {
        free((char *) tree->value);
        tree->value = NULL;
    }
    if (value != NULL) {
        tree->value = strdup(value);
        if (tree->value == NULL)
            return -1;
    }
    tree->dirty = 1;
    return 0;
}

int aug_set(struct augeas *aug, const char *path, const char *value) {
    return tree_set(aug->tree, path, value);
}

int aug_exists(struct augeas *aug, const char *path) {
    return (tree_find(aug->tree, path) != NULL);
}

int aug_insert(struct augeas *aug, const char *path, const char *sibling) {
    struct tree *parent, *prev = NULL, *new = NULL;
    char *pathdup = NULL, *label;

    if (path == NULL || sibling == NULL || STREQ(path, sibling))
        goto error;

    pathdup = strdup(path);
    if (pathdup == NULL)
        goto error;

    label = strrchr(pathdup, SEP);
    if (label == NULL)
        goto error;
    *label = '\0';

    if (STRNEQLEN(pathdup, sibling, strlen(pathdup)))
        goto error;
    sibling = sibling + strlen(pathdup) + 1;

    parent = tree_find(aug->tree, pathdup);
    *label = SEP;

    if (parent == NULL)
        goto error;

    label += 1;
    list_for_each(t, parent->children) {
        if (streqv(t->label, label))
            goto error;
        if (t->next != NULL && streqv(t->next->label, sibling))
            prev = t;
    }

    if (prev == NULL)
        return -1;

    CALLOC(new, 1);
    new->label = strdup(label);
    new->next = prev->next;
    prev->next = new;
    parent->dirty = 1;

    free(pathdup);
    return 0;
 error:
    free(pathdup);
    free(new);
    return -1;
}

int free_tree(struct tree *tree) {
    int cnt = 0;

    while (tree != NULL) {
        struct tree *del = tree;
        tree = del->next;
        cnt += free_tree(del->children);
        aug_tree_free(del);
        cnt += 1;
    }

    return cnt;
}

char *pathsplit(const char *path) {
    char *ppath = strdup(path);
    char *pend = strrchr(ppath, SEP);

    if (pend == NULL || pend == ppath) {
        free(ppath);
        return NULL;
    }
    *pend = '\0';
    return ppath;
}

int tree_rm(struct tree **htree, const char *path) {
    const char *ppath = pathsplit(path);
    struct tree *tree = *htree;
    struct tree *del = NULL;
    const char *label;
    int cnt = 0;

    if (ppath == NULL) {
        /* Delete one of TREE's siblings */
        if (streqv(path, tree->label)) {
            del = tree;
            *htree = tree->next;
            /* This is not quite right: TREE's parent should be marked
               dirty, but we don't have that. Mark one of its siblings as
               dirty and hope for the best. */
            if (tree != NULL)
                tree->dirty = 1;
        } else {
            label = path;
        }
    } else {
        struct tree *parent = tree_find(tree, ppath);
        label = ppath + strlen(ppath) + 1;
        if (parent == NULL || parent->children == NULL) {
            free((void *) ppath);
            return 0;
        }
        if (streqv(label, parent->children->label)) {
            del = parent->children;
            parent->children = del->next;
        } else {
            tree = parent->children;
        }
        parent->dirty = 1;
    }
    
    if (del == NULL) {
        /* Delete one of TREE's siblings, never TREE itself */
        struct tree *prev;
        for (prev = tree;
             prev->next != NULL && !streqv(label, prev->next->label);
             prev = prev->next);
        if (prev->next != NULL) {
            del = prev->next;
            prev->next = del->next;
        }
    }

    if (del != NULL) {
        cnt = free_tree(del->children);
        aug_tree_free(del);
    }

    free((void *) ppath);
    return cnt + 1;
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
    parent = tree_find_or_create(path, aug->tree);
    if (parent == NULL)
        goto error;

    list_append(parent->children, sub);
    return 0;
 error:
    return -1;
}

int aug_ls(struct augeas *aug, const char *path, const char ***children) {
    struct tree *tl = NULL;
    int cnt = 0;

    // FIXME: Treating / special is a huge kludge
    if (STREQ(path, "/")) {
        CALLOC(tl, 1);
        CALLOC(tl->children, 1);
        tl->children->children = aug->tree;
        path = "";
    } else {
        tl = tree_find_all(aug->tree, path);
    }

    list_for_each(l, tl) {
        list_for_each(t, l->children->children) {
            cnt += (t->label != NULL);
        }
    }

    if (children == NULL)
        goto done;

    *children = calloc(cnt, sizeof(char *));
    if (*children == NULL) {
        cnt = -1;
        goto done;
    }

    int i = 0;
    list_for_each(l, tl) {
        list_for_each(t, l->children->children) {
            while (t != NULL && t->label == NULL) t = t->next;
            if (t == NULL) break;
            int exists = 0;
            for (int j=0; j < i; j++) {
                if (pathendswith((*children)[j], t->label)) {
                    exists = 1;
                    break;
                }
            }
            if (! exists) {
                int len = strlen(path) + 1 + strlen(t->label) + 1;
                char *p = malloc(len);
                snprintf(p, len, "%s/%s", path, t->label);
                (*children)[i++] = p;
            }
        }
    }
    /* Because we skip duplicate nodes, we may actually have fewer than CNT
       elements in CHILDREN */
    if (i < cnt) {
        cnt = i;
        *children = realloc(*children, sizeof(char *)*cnt);
    }
 done:
    /* Listing "/" is special */
    if (strlen(path) == 0) {
        free(tl->children);
    }
    list_free(tl);
    return cnt;
}

static int match_rec(struct tree *tree, const char *pattern,
                     char **path,
                     const char **matches, int size) {

    int cnt = 0;
    int end = strlen(*path);

    for(;tree != NULL; tree = tree->next) {
        if (tree->label == NULL)
            continue;
        *path = realloc(*path, end + 1 + strlen(tree->label) + 1);
        (*path)[end] = SEP;
        strcpy(*path + end + 1, tree->label);

        if (fnmatch(pattern, *path, FNM_NOESCAPE) == 0) {
            if (size > 0) {
                *matches = strdup(*path);
                matches += 1;
                size -= 1;
            }
            cnt += 1;
        }
        int n = match_rec(tree->children, pattern, path, matches, size);
        cnt += n;
        matches += n;
        size -= n;
    }

    (*path)[end] = '\0';
    return cnt;
}

int aug_match(struct augeas *aug, const char *pattern,
              const char **matches, int size) {
    char *path = calloc(10, 1);
    int n = match_rec(aug->tree, pattern, &path, matches, size);
    free(path);
    return n;
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
                transform_save(aug, transform, tpath, t);
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

    list_for_each(t, aug->tree) {
        tree_propagate_dirty(t);
    }
    files = tree_find(aug->tree, AUGEAS_FILES_TREE);
    if (files == NULL)
        return -1;

    if (files->dirty) {
        list_for_each(t, files->children) {
            if (tree_save(aug, t, AUGEAS_FILES_TREE) == -1)
                ret = -1;
        }
    }
    tree_clean(aug->tree);
    return ret;
}

/* FIXME: The usage of PATH is inconsistent between calls from PRINT_REC
 * and PRINT_TREE. In the former case, PATH does not include the label for
 * TREE yet, in the latter it does
 */
static int print_one(FILE *out, struct tree *tree, char **path,
                     int pr_hidden) {
    int end = strlen(*path);
    const char *label;

    if (tree->label == NULL) {
        if (! pr_hidden)
            return -1;
        label = "()";
    } else {
        label = tree->label;
    }

    *path = realloc(*path, strlen(*path) + 1 + strlen(label) + 1);
    if (end > 0) {
        (*path)[end] = SEP;
        strcpy(*path + end + 1, label);
    } else {
        strcpy(*path, label);
    }

    fprintf(out, *path);
    if (tree->value != NULL)
        fprintf(out, " = %s", tree->value);
    fputc('\n', out);

    (*path)[end] = '\0';
    return end;
}

static void print_rec(FILE *out, struct tree *tree, char **path,
                      int pr_hidden) {
    for (;tree != NULL; tree = tree->next) {
        int end = print_one(out, tree, path, pr_hidden);
        if (end == -1)
            continue;
        /* We completely rely on how print_one does its thing here.
           TREE's label is still on *path, but print_one removed it
           by overwriting the / with a 0 */
        (*path)[end] = SEP;
        print_rec(out, tree->children, path, pr_hidden);
        (*path)[end] = '\0';
    }
}

void print_tree(struct tree *tree, FILE *out, const char *path,
                int pr_hidden) {
    if (path == NULL)
        path = "";
    char *pbuf = strdup(path);
    while (tree != NULL) {
        if (tree->children != NULL) {
            print_rec(out, tree->children, &pbuf, pr_hidden);
        } else {
            print_one(out, tree, &pbuf, pr_hidden);
        }
        for (tree = tree->next; tree != NULL; tree = tree->next) {
            if (pathendswith(path, tree->label))
                break;
        }
    }
    free(pbuf);
}

void aug_print(struct augeas *aug, FILE *out, const char *path) {
    struct tree *tree = tree_find(aug->tree, path);
    if (tree != NULL)
        print_tree(tree, out, path, 0);
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

int tree_equal(struct tree *t1, struct tree *t2) {
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
