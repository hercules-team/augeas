/*
 * transform.c: support for building and running transformers
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

#include <fnmatch.h>
#include <glob.h>

#include "internal.h"
#include "augeas.h"
#include "syntax.h"
#include "config.h"

static const int fnm_flags = FNM_FILE_NAME;
static const int glob_flags = GLOB_NOSORT;

/* Extension for newly created files */
#define EXT_AUGNEW ".augnew"
/* Extension for backup files */
#define EXT_AUGSAVE ".augsave"

/* Loaded files are tracked underneath METATREE. When a file with name
 * FNAME is loaded, certain entries are made under METATREE / FNAME: 
 *   path      : path where tree for FNAME is put
 *   lens/info : information about where the applied lens was loaded from
 *   lens/id   : unique hexadecimal id of the lens
 *   error     : indication of errors during processing FNAME, or NULL
 *               if processing succeeded
 */
static const char *const path_node = "/path";
static const char *const info_node = "/lens/info";
static const char *const id_node = "/lens/id";
static const char *const err_node = "/error";

/*
 * Filters
 */
struct filter *make_filter(struct string *glb, unsigned int include) {
    struct filter *f;
    make_ref(f);
    f->glob = glb;
    f->include = include;
    return f;
}

void free_filter(struct filter *f) {
    if (f == NULL)
        return;
    assert(f->ref == 0);
    unref(f->next, filter);
    unref(f->glob, string);
    free(f);
}

static const char *pathbase(const char *path) {
    const char *p = strrchr(path, SEP);
    return (p == NULL) ? path : p + 1;
}

static int filter_generate(struct filter *filter, const char *root,
                           int *nmatches, char ***matches) {
    glob_t globbuf;
    int gl_flags = glob_flags;
    int r;
    int ret = 0;

    list_for_each(f, filter) {
        char *globpat = NULL;
        if (! f->include)
            continue;
        pathjoin(&globpat, 2, root, f->glob->str);
        r = glob(globpat, gl_flags, NULL, &globbuf);
        free(globpat);

        if (r != 0 && r != GLOB_NOMATCH) {
            ret = -1;
            goto done;
        }
        gl_flags |= GLOB_APPEND;
    }

    char **pathv = globbuf.gl_pathv;
    int pathc = globbuf.gl_pathc;
    globbuf.gl_pathv = NULL;
    globbuf.gl_pathc = 0;

    list_for_each(e, filter) {
        if (e->include)
            continue;
        for (int i=0; i < pathc;) {
            const char *path = pathv[i];
            if (strchr(e->glob->str, SEP) == NULL)
                path = pathbase(path);
            if (fnmatch(e->glob->str, path, fnm_flags) == 0) {
                pathc -= 1;
                if (i < pathc) {
                    pathv[i] = pathv[pathc];
                }
            } else {
                i += 1;
            }
        }
    }
    REALLOC(pathv, pathc);
    *matches = pathv;
    *nmatches = pathc;
 done:
    globfree(&globbuf);
    return ret;
}

static int filter_matches(struct filter *filter, const char *path) {
    int found = 0;
    list_for_each(f, filter) {
        if (f->include)
            found |= (fnmatch(f->glob->str, path, fnm_flags) == 0);
    }
    if (! found)
        return 0;
    list_for_each(f, filter) {
        if (!f->include && (fnmatch(f->glob->str, path, fnm_flags) == 0))
            return 0;
    }
    return 1;
}

/*
 * Transformers
 */
struct transform *make_transform(struct lens *lens, struct filter *filter) {
    struct transform *xform;
    make_ref(xform);
    xform->lens = lens;
    xform->filter = filter;
    return xform;
}

void free_transform(struct transform *xform) {
    if (xform == NULL)
        return;
    assert(xform->ref == 0);
    unref(xform->lens, lens);
    unref(xform->filter, filter);
    free(xform);
}

static const char *err_path(const char *filename) {
    char *result = NULL;
    pathjoin(&result, 3, AUGEAS_META_FILES, filename, err_node);
    return result;
}

static char *add_load_info(struct augeas *aug, const char *filename,
                           const char *node, struct lens *lens) {
    char *tmp;
    int r;
    char *result = NULL;
    int end = 0;

    pathjoin(&result, 2, AUGEAS_META_FILES, filename + strlen(aug->root) - 1);
    end = strlen(result);

    pathjoin(&result, 1, path_node);
    aug_set(aug, result, node);
    result[end] = '\0';

    pathjoin(&result, 1, info_node);
    tmp = format_info(lens->info);
    aug_set(aug, result, tmp);
    free(tmp);
    result[end] = '\0';

    pathjoin(&result, 1, id_node);
    r = asprintf(&tmp, "%p", lens);
    if (r >= 0) {
        aug_set(aug, result, tmp);
        free(tmp);
    }
    result[end] = '\0';

    pathjoin(&result, 1, err_node);
    return result;
}

static int load_file(struct augeas *aug, struct lens *lens,
                     const char *filename) {
    const char *text = NULL;
    char *errpath = NULL;
    const char *err_status = NULL;
    struct aug_file *file = NULL;
    struct tree *tree;
    char *path = NULL;
    struct lns_error *err;

    pathjoin(&path, 2, AUGEAS_FILES_TREE, filename + strlen(aug->root) - 1);

    errpath = add_load_info(aug, filename, path, lens);
    if (errpath == NULL)
        goto error;

    text = read_file(filename);
    if (text == NULL) {
        err_status = "read_failed";
        goto error;
    }

    struct info *info;
    make_ref(info);
    make_ref(info->filename);
    info->filename->str = filename;
    info->first_line = 1;
    tree = lns_get(info, lens, text, stdout, PF_NONE, &err);
    if (err != NULL) {
        err_status = "parse_failed";
        free_tree(tree);
        goto error;
    }
    free((void *) text);

    aug_tree_replace(aug, path, tree);

    aug_set(aug, errpath, NULL);
    free(errpath);
    return 0;
 error:
    if (errpath != NULL)
        aug_set(aug, errpath, err_status);
    free(errpath);
    free(file);
    free((void *) text);
    return -1;
}

int transform_load(struct augeas *aug, struct transform *xform) {
    int nmatches;
    char **matches;
    int r;

    r = filter_generate(xform->filter, aug->root, &nmatches, &matches);
    if (r == -1)
        return -1;
    for (int i=0; i < nmatches; i++) {
        load_file(aug, xform->lens, matches[i]);
        free(matches[i]);
    }
    free(matches);
    return 0;
}

int transform_applies(struct transform *xform, const char *path) {
    if (STRNEQLEN(path, AUGEAS_FILES_TREE, strlen(AUGEAS_FILES_TREE))
        || path[strlen(AUGEAS_FILES_TREE)] != SEP)
        return 0;
    return filter_matches(xform->filter, path + strlen(AUGEAS_FILES_TREE));
}

int transform_save(struct augeas *aug, struct transform *xform,
                   const char *path, struct tree *tree) {
    FILE *fp = NULL;
    char *augnew = NULL, *augorig = NULL, *augsave = NULL;
    const char *text = NULL;
    const char *filename = path + strlen(AUGEAS_FILES_TREE) + 1;
    const char *err_status = NULL;
    struct lns_error *err;
    int result = -1;

    if (asprintf(&augorig, "%s%s", aug->root, filename) == -1)
        goto done;

    if (asprintf(&augnew, "%s%s" EXT_AUGNEW, aug->root, filename) == -1)
        goto done;

    if (access(augorig, R_OK) == 0) {
        text = read_file(augorig);
    } else {
        text = strdup("");
    }

    if (text == NULL) {
        err_status = "put_read";
        goto done;
    }

    // FIXME: We might have to create intermediary directories
    // to be able to write augnew, but we have no idea what permissions
    // etc. they should get. Just the process default ?
    fp = fopen(augnew, "w");
    if (fp == NULL)
        goto done;

    if (tree != NULL)
        lns_put(fp, xform->lens, tree->children, text, &err);
    // FIXME: Delete file if tree == NULL

    if (fclose(fp) != 0)
        goto done;
    fp = NULL;

    if (!(aug->flags & AUG_SAVE_NEWFILE)) {
        if (aug->flags & AUG_SAVE_BACKUP) {
            int r;
            r = asprintf(&augsave, "%s%s" EXT_AUGSAVE, aug->root, filename);
            if (r == -1)
                goto done;
            if (rename(augorig, augsave) != 0) {
                err_status = "rename_augsave";
                goto done;
            }
        }
        if (rename(augnew, augorig) != 0) {
            err_status = "rename_augnew";
            goto done;
        }
    }
    result = 0;

 done:
    free(augnew);
    free(augorig);
    free(augsave);
    if (err_status != NULL) {
        const char *ep = err_path(filename);
        aug_set(aug, ep, err_status);
        free((char *) ep);
    }
    if (fp != NULL)
        fclose(fp);
    return result;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
