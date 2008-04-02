/*
 * transform.c: support for building and running transformers
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

#include <fnmatch.h>
#include <glob.h>
#include <stdarg.h>

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
void filter_generate(struct filter *filter, int *nmatches, char ***matches) {
    struct filter *excl, *incl;
    glob_t globbuf;
    int gl_flags = glob_flags;
    int r;

    for (excl = filter; excl != NULL && excl->include; excl = excl->next);
    for (incl = filter; incl != NULL && incl->include; incl = incl->next) {
        // FIXME: Use this to support changing the root dir:
        //  ret = asprintf(&globpat, "%s%s", aug->root,
        //                   filter->glob);
        //    if (ret == -1)
        //        goto exit;
        r = glob(filter->glob, gl_flags, NULL, &globbuf);
        if (r != 0 && r != GLOB_NOMATCH)
            FIXME("Report error");
        gl_flags |= GLOB_APPEND;
    }

    char **pathv = globbuf.gl_pathv;
    int pathc = globbuf.gl_pathc;
    globbuf.gl_pathv = NULL;
    globbuf.gl_pathc = 0;
    globfree(&globbuf);

    list_for_each(e, excl) {
        for (int i=0; i < pathc;) {
            if (fnmatch(e->glob, pathv[i], fnm_flags)) {
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
}

int filter_matches(struct filter *filter, const char *path) {
    int found = 0;
    struct filter *f;
    for (f = filter; f != NULL && f->include; f = f->next)
        found |= fnmatch(f->glob, path, fnm_flags);
    if (! found)
        return 0;
    for (; f != NULL; f = f->next) {
        if (fnmatch(f->glob, path, fnm_flags))
            return 0;
    }
    return 1;
}

/* Join NSEG path components (passed as const char *) into one PATH.
   Allocate as needed. Return 0 on success, -1 on failure */
static int pathjoin(char **path, int nseg, ...) {
    va_list ap;

    va_start(ap, nseg);
    for (int i=0; i < nseg; i++) {
        const char *seg = va_arg(ap, const char *);
        int len = strlen(seg) + 1;

        if (*path != NULL) {
            len += strlen(*path) + 1;
            REALLOC(*path, len);
            if (*path == NULL)
                return -1;
            if (strlen(*path) == 0 || (*path)[strlen(*path)-1] != SEP)
                strcat(*path, "/");
            if (seg[0] == SEP)
                seg += 1;
            strcat(*path, seg);
        } else {
            *path = malloc(len);
            strcpy(*path, seg);
        }
    }
    va_end(ap);
    return 0;
}

/*
 * Transformers
 */
static const char *err_path(const char *filename) {
    char *result = NULL;
    pathjoin(&result, 3, AUGEAS_META_FILES, filename, err_node);
    return result;
}

static char *add_load_info(struct augeas *aug, const char *filename,
                           const char *node, struct lens *lens) {
    char *tmp;

    char *result = NULL;
    int end = 0;
    pathjoin(&result, 2, AUGEAS_META_FILES, filename + strlen(aug->root));
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
    asprintf(&tmp, "%p", lens);
    aug_set(aug, result, tmp);
    free(tmp);
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

    pathjoin(&path, 2, AUGEAS_FILES_TREE, filename);

    errpath = add_load_info(aug, filename, path, lens);
    if (errpath == NULL)
        goto error;

    text = aug_read_file(filename);
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
    if (tree == NULL) {
        err_status = "parse_failed";
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

void transform_load(struct augeas *aug, struct transform *xform) {
    int nmatches;
    char **matches;

    filter_generate(xform->filter, &nmatches, &matches);
    for (int i=0; i < nmatches; i++) {
        load_file(aug, xform->lens, matches[i]);
        free(matches[i]);
    }
    free(matches);
}

int transform_applies(struct transform *xform, const char *path) {
    if (STRNEQLEN(path, AUGEAS_FILES_TREE, strlen(AUGEAS_FILES_TREE))
        || path[strlen(AUGEAS_FILES_TREE)] != SEP)
        return 0;
    return filter_matches(xform->filter, path + strlen(AUGEAS_FILES_TREE));
}

int transform_save(struct augeas *aug, struct transform *xform,
                   const char *path, struct tree *tree) {
    FILE *fp;
    char *augnew = NULL;
    const char *text = NULL;
    const char *filename = path + strlen(AUGEAS_FILES_TREE);
    const char *err_status = NULL;

    asprintf(&augnew, "%s" EXT_AUGNEW, filename);

    text = aug_read_file(filename);
    if (text == NULL) {
        err_status = "put_read";
        goto error;
    }

    fp = fopen(augnew, "w");
    if (fp == NULL)
        goto error;

    if (tree != NULL)
        lns_put(fp, xform->lens, tree, text);
    // FIXME: Delete file if tree == NULL

    if (fclose(fp) != 0)
        goto error;

    if (!(aug->flags & AUG_SAVE_NEWFILE)) {
        if (aug->flags & AUG_SAVE_BACKUP) {
            char *augsave = NULL;
            asprintf(&augsave, "%s" EXT_AUGSAVE, filename);
            if (rename(filename, augsave) != 0) {
                err_status = "rename_augsave";
                goto error;
            }
            if (rename(augnew, filename) != 0) {
                err_status = "rename_augnew";
                goto error;
            }
        }
    }
    return 0;

 error:
    if (err_status != NULL) {
        const char *ep = err_path(filename);
        aug_set(aug, ep, err_status);
        free((char *) ep);
    }
    fclose(fp);
    return -1;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
