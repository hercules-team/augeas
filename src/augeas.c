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

#include <fnmatch.h>

/* Two special entries: they are always on the main list
   so that we don't need to worry about some corner cases in dealing
   with empty lists */

#define P_ROOT   "system"
#define P_SYSTEM_CONFIG "system/config"

/* Hardcoded list of existing providers. Ultimately, they should be created
 * from a metadata description, not in code
 */

/* Provider for parsed files (prov_spec.c) */
extern const struct aug_provider augp_spec;

static const struct aug_provider *providers[] = {
    &augp_spec,
    NULL
};

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

struct augeas *aug_init(const char *root, unsigned int flags) {
    struct augeas *result;

    CALLOC(result, 1);
    CALLOC(result->tree, 1);

    result->flags = flags;

    if (root == NULL)
        root = getenv(AUGEAS_ROOT_ENV);
    if (root == NULL)
        root = "/";
    result->root = strdup(root);

    result->tree->label = strdup(P_ROOT);
    tree_find_or_create(P_SYSTEM_CONFIG, result->tree);

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

    for (int i=0; providers[i] != NULL; i++) {
        const struct aug_provider *prov = providers[i];
        int r;
        r = prov->init(result);
        if (r == -1)
            goto error;
        r = prov->load(result);
        if (r == -1)
            goto error;
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

static int del_rec(struct tree *tree) {
    int cnt = 0;

    while (tree != NULL) {
        struct tree *del = tree;
        tree = del->next;
        cnt += del_rec(del->children);
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

int tree_rm(struct tree *tree, const char *path) {
    const char *ppath = pathsplit(path);
    const char *label = NULL;

    if (ppath == NULL)
        return -1;

    label = ppath + strlen(ppath) + 1;

    struct tree *parent = tree_find(tree, ppath);
    if (parent == NULL || parent->children == NULL) {
        free((void *) ppath);
        return 0;
    }

    struct tree *del;
    if (streqv(label, parent->children->label)) {
        del = parent->children;
        parent->children = del->next;
    } else {
        struct tree *prev;
        for (prev=parent->children;
             prev->next != NULL && !streqv(label, prev->next->label);
             prev = prev->next);
        if (prev->next == NULL) {
            free((void *) ppath);
            return 0;
        }
        del = prev->next;
        prev->next = del->next;
    }

    int cnt = del_rec(del->children);
    aug_tree_free(del);
    parent->dirty = 1;
    free((void *) ppath);
    return cnt + 1;
}

int aug_rm(struct augeas *aug, const char *path) {
    return tree_rm(aug->tree, path);
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

int aug_save(struct augeas *aug) {
    int r;

    for (int i=0; providers[i] != NULL; i++) {
        r = providers[i]->save(aug);
        if (r == -1)
            return -1;
    }
    return 0;
}

static int print_one(FILE *out, struct tree *tree, char **path) {
    int end = strlen(*path);
    if (tree->label == NULL)
        return -1;

    *path = realloc(*path, strlen(*path) + 1 + strlen(tree->label) + 1);
    (*path)[end] = SEP;
    strcpy(*path + end + 1, tree->label);

    fprintf(out, *path);
    if (tree->value != NULL)
        fprintf(out, " = %s", tree->value);
    fputc('\n', out);

    (*path)[end] = '\0';
    return end;
}

static void print_rec(FILE *out, struct tree *tree, char **path) {
    for (;tree != NULL; tree = tree->next) {
        int end = print_one(out, tree, path);
        if (end == -1)
            continue;
        /* We completely rely on how print_one does its thing here.
           TREE's label is still on *path, but print_one removed it
           by overwriting the / with a 0 */
        (*path)[end] = SEP;
        print_rec(out, tree->children, path);
        (*path)[end] = '\0';
    }
}

void aug_print(struct augeas *aug, FILE *out, const char *path) {
    char *pbuf = strdup(path);
    struct tree *tree = tree_find(aug->tree, path);
    while (tree != NULL) {
        if (tree->children != NULL) {
            print_rec(out, tree->children, &pbuf);
        } else {
            print_one(out, tree, &pbuf);
        }
        for (tree = tree->next; tree != NULL; tree = tree->next) {
            if (pathendswith(path, tree->label))
                break;
        }
    }
    free(pbuf);
}

void aug_close(struct augeas *aug) {
    if (aug == NULL)
        return;
    del_rec(aug->tree);
    free((void *) aug->root);
    free(aug);
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
