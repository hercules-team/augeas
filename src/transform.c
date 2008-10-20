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

#include <config.h>

#include "canonicalize.h"

#include <fnmatch.h>
#include <glob.h>

#include "internal.h"
#include "memory.h"
#include "augeas.h"
#include "syntax.h"

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
 *   error/pos : position in file where error occured (for get errors)
 *   error/path: path to tree node where error occurred (for put errors)
 *   error/message : human-readable error message
 */
static const char *const path_node = "/path";
static const char *const info_node = "/lens/info";
static const char *const id_node = "/lens/id";

static const char *const err_node = "/error";
/* These are all put underneath "/error" */
static const char *const err_pos_node = "/pos";
static const char *const err_path_node = "/path";
static const char *const err_msg_node = "/message";

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

    *nmatches = 0;
    *matches = NULL;

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
                free(pathv[i]);
                pathc -= 1;
                if (i < pathc) {
                    pathv[i] = pathv[pathc];
                }
            } else {
                i += 1;
            }
        }
    }
    if (REALLOC_N(pathv, pathc) == -1) {
        FREE(pathv);
        pathc = 0;
        ret = -1;
    }
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

static char *err_path(const char *filename) {
    char *result = NULL;
    pathjoin(&result, 3, AUGEAS_META_FILES, filename, err_node);
    return result;
}

/* Record an error in the tree. The error will show up underneath
 * /augeas/FILENAME/error. PATH is the path to the toplevel node in the
 * tree where the lens application happened. When STATUS is NULL, just
 * clear any error associated with FILENAME in the tree.
 */
static int store_error(struct augeas *aug,
                       const char *filename, const char *path,
                       const char *status, int errnum,
                       const struct lns_error *err) {
    char *ep = err_path(filename);
    size_t ep_len;
    int r;
    int result = -1;

    if (ep == NULL)
        return -1;
    ep_len = strlen(ep);

    aug_rm(aug, ep);
    if (status != NULL) {
        r = aug_set(aug, ep, status);
        if (r < 0)
            goto done;

        if (err != NULL) {
            if (err->pos > 0) {
                char *pos;

                r = pathjoin(&ep, 1, err_pos_node);
                if (r < 0)
                    goto done;
                r = asprintf(&pos, "%d", err->pos);
                if (r < 0)
                    goto done;
                r = aug_set(aug, ep, pos);
                FREE(pos);
                if (r < 0)
                    goto done;
            } else {
                char *p;

                r = pathjoin(&ep, 1, err_path_node);
                if (r < 0)
                    goto done;
                r = asprintf(&p, "%s%s", path, err->path);
                if (r < 0)
                    goto done;
                r = aug_set(aug, ep, p);
                FREE(p);
                if (r < 0)
                    goto done;
            }
            ep[ep_len] = '\0';
            r = pathjoin(&ep, 1, err_msg_node);
            if (r < 0)
                goto done;
            r = aug_set(aug, ep, err->message);
            if (r < 0)
                goto done;
        } else if (errnum != 0) {
            const char *msg = strerror(errnum);
            r = pathjoin(&ep, 1, err_msg_node);
            if (r < 0)
                goto done;
            r = aug_set(aug, ep, msg);
            if (r < 0)
                goto done;
        }
    }

    result = 0;
 done:
    free(ep);
    return result;
}

static int add_load_info(struct augeas *aug, const char *filename,
                         const char *node, struct lens *lens) {
    char *tmp = NULL;
    int r;
    char *p = NULL;
    int end = 0;
    int result = -1;

    r = pathjoin(&p, 2, AUGEAS_META_FILES, filename + strlen(aug->root) - 1);
    if (r < 0)
        goto done;
    end = strlen(p);

    r = pathjoin(&p, 1, path_node);
    if (r < 0)
        goto done;

    r = aug_set(aug, p, node);
    if (r < 0)
        goto done;
    p[end] = '\0';

    r = pathjoin(&p, 1, info_node);
    if (r < 0)
        goto done;

    tmp = format_info(lens->info);
    if (tmp == NULL)
        goto done;
    r = aug_set(aug, p, tmp);
    FREE(tmp);
    if (r < 0)
        goto done;
    p[end] = '\0';

    r = pathjoin(&p, 1, id_node);
    if (r < 0)
        goto done;

    r = asprintf(&tmp, "%p", lens);
    if (r >= 0) {
        r = aug_set(aug, p, tmp);
        FREE(tmp);
        if (r < 0)
            goto done;
    }

    result = 0;
 done:
    free(p);
    return result;
}

static char *append_newline(char *text, size_t len) {
    /* Try to append a newline; this is a big hack to work */
    /* around the fact that lenses generally break if the  */
    /* file does not end with a newline. */
    if (len == 0 || text[len-1] != '\n') {
        if (REALLOC_N(text, len+2) == 0) {
            text[len] = '\n';
            text[len+1] = '\0';
        }
    }
    return text;
}

static int load_file(struct augeas *aug, struct lens *lens, char *filename) {
    char *text = NULL;
    const char *err_status = NULL;
    struct aug_file *file = NULL;
    struct tree *tree = NULL;
    char *path = NULL;
    struct lns_error *err = NULL;
    int result = -1, r;

    pathjoin(&path, 2, AUGEAS_FILES_TREE, filename + strlen(aug->root) - 1);

    r = add_load_info(aug, filename, path, lens);
    if (r < 0)
        goto done;

    text = read_file(filename);
    if (text == NULL) {
        err_status = "read_failed";
        goto done;
    }
    text = append_newline(text, strlen(text));

    struct info *info;
    make_ref(info);
    make_ref(info->filename);
    info->filename->str = filename;
    info->first_line = 1;

    tree = lns_get(info, lens, text, &err);

    info->filename->str = NULL;
    unref(info, info);

    if (err != NULL) {
        err_status = "parse_failed";
        goto done;
    }

    aug_tree_replace(aug, path, tree);
    tree = NULL;

    result = 0;
 done:
    store_error(aug, filename + strlen(aug->root) - 1, path, err_status,
                errno, err);
    free_lns_error(err);
    free(path);
    free_tree(tree);
    free(file);
    free(text);
    return result;
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
    lens_release(xform->lens);
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
    char *augorig_canon = NULL;
    char *text = NULL;
    const char *filename = path + strlen(AUGEAS_FILES_TREE) + 1;
    const char *err_status = NULL;
    struct lns_error *err = NULL;
    int result = -1;

    if (asprintf(&augorig, "%s%s", aug->root, filename) == -1) {
        augorig = NULL;
        goto done;
    }

    if (asprintf(&augnew, "%s%s" EXT_AUGNEW, aug->root, filename) == -1) {
        augnew = NULL;
        goto done;
    }

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
    if (fp == NULL) {
        err_status = "open_augnew";
        goto done;
    }

    if (tree != NULL)
        lns_put(fp, xform->lens, tree->children, text, &err);
    // FIXME: Delete file if tree == NULL

    if (ferror (fp) || fclose(fp) != 0)
        goto done;
    fp = NULL;

    if (err != NULL) {
        err_status = "put_failed";
        unlink(augnew);
        goto done;
    }

    {
        char *new_text = read_file(augnew);
        int same = 0;
        if (new_text == NULL) {
            err_status = "read_augnew";
            goto done;
        }
        same = STREQ(text, new_text);
        FREE(new_text);
        if (same) {
            result = 0;
            unlink(augnew);
            goto done;
        }
    }

    if (!(aug->flags & AUG_SAVE_NEWFILE)) {
        augorig_canon = canonicalize_file_name(augorig);
        if (augorig_canon == NULL) {
            if (errno == ENOENT) {
                augorig_canon = augorig;
            } else {
                err_status = "canon_augorig";
                goto done;
            }
        }

        if (aug->flags & AUG_SAVE_BACKUP) {
            int r;
            r = asprintf(&augsave, "%s%s" EXT_AUGSAVE, aug->root, filename);
            if (r == -1) {
                augsave = NULL;
                goto done;
            }

            if (rename(augorig_canon, augsave) != 0) {
                err_status = "rename_augsave";
                goto done;
            }
        }
        if (rename(augnew, augorig_canon) != 0) {
            err_status = "rename_augnew";
            goto done;
        }
    }
    result = 0;

 done:
    store_error(aug, filename, path, err_status, errno, err);
    lens_release(xform->lens);
    free(text);
    free(augnew);
    if (augorig_canon != augorig)
        free(augorig_canon);
    free(augorig);
    free(augsave);
    free_lns_error(err);

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
