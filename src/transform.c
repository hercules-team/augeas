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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <selinux/selinux.h>

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

static int transfer_file_attrs(const char *from, const char *to,
                               const char **err_status) {
    struct stat st;
    int ret = 0;
    int selinux_enabled = (is_selinux_enabled() > 0);
    security_context_t con = NULL;

    ret = lstat(from, &st);
    if (ret < 0) {
        *err_status = "replace_stat";
        return -1;
    }
    if (selinux_enabled) {
        if (lgetfilecon(from, &con) < 0 && errno != ENOTSUP) {
            *err_status = "replace_getfilecon";
            return -1;
        }
    }

    if (lchown(to, st.st_uid, st.st_gid) < 0) {
        *err_status = "replace_chown";
        return -1;
    }
    if (chmod(to, st.st_mode) < 0) {
        *err_status = "replace_chmod";
        return -1;
    }
    if (selinux_enabled && con != NULL) {
        if (lsetfilecon(to, con) < 0 && errno != ENOTSUP) {
            *err_status = "replace_setfilecon";
            return -1;
        }
        freecon(con);
    }
    return 0;
}

/* Try to rename FROM to TO. If that fails with an error other than EXDEV
 * or EBUSY, return -1. If the failure is EXDEV or EBUSY (which we assume
 * means that FROM or TO is a bindmounted file), and COPY_IF_RENAME_FAILS
 * is true, copy the contents of FROM into TO and delete FROM.
 *
 * Return 0 on success (either rename succeeded or we copied the contents
 * over successfully), -1 on failure.
 */
static int clone_file(const char *from, const char *to,
                      const char **err_status, int copy_if_rename_fails) {
    FILE *from_fp = NULL, *to_fp = NULL;
    char buf[BUFSIZ];
    size_t len;
    int result = -1;

    if (rename(from, to) == 0)
        return 0;
    if ((errno != EXDEV && errno != EBUSY) || !copy_if_rename_fails) {
        *err_status = "rename";
        return -1;
    }

    /* rename not possible, copy file contents */
    from_fp = fopen(from, "r");
    if (!(from_fp = fopen(from, "r"))) {
        *err_status = "clone_open_src";
        goto done;
    }

    if (!(to_fp = fopen(to, "w"))) {
        *err_status = "clone_open_dst";
        goto done;
    }

    if (transfer_file_attrs(from, to, err_status) < 0)
        goto done;

    while ((len = fread(buf, 1, BUFSIZ, from_fp)) > 0) {
        if (fwrite(buf, 1, len, to_fp) != len) {
            *err_status = "clone_write";
            goto done;
        }
    }
    if (ferror(from_fp)) {
        *err_status = "clone_read";
        goto done;
    }

    result = 0;
 done:
    fclose(from_fp);
    fclose(to_fp);
    if (result != 0)
        unlink(to);
    if (result == 0)
        unlink(from);
    return result;
}

static char *strappend(const char *s1, const char *s2) {
    size_t len = strlen(s1) + strlen(s2);
    char *result = NULL, *p;

    if (ALLOC_N(result, len + 1) < 0)
        return NULL;

    p = stpcpy(result, s1);
    stpcpy(p, s2);
    return result;
}

/*
 * Save TREE->CHILDREN into the file PATH using the lens from XFORM. Errors
 * are noted in the /augeas/files hierarchy in AUG->ORIGIN under
 * PATH/error.
 *
 * Writing the file happens by first writing into PATH.augnew, transferring
 * all file attributes of PATH to PATH.augnew, and then renaming
 * PATH.augnew to PATH. If the rename fails, and the entry
 * AUGEAS_COPY_IF_FAILURE exists in AUG->ORIGIN, PATH is overwritten by
 * copying file contents
 *
 * Return 0 on success, -1 on failure.
 */
int transform_save(struct augeas *aug, struct transform *xform,
                   const char *path, struct tree *tree) {
    FILE *fp = NULL;
    char *augnew = NULL, *augorig = NULL, *augsave = NULL;
    char *augorig_canon = NULL;
    int   augorig_exists;
    int   copy_if_rename_fails = 0;
    char *text = NULL;
    const char *filename = path + strlen(AUGEAS_FILES_TREE) + 1;
    const char *err_status = NULL;
    char *dyn_err_status = NULL;
    struct lns_error *err = NULL;
    int result = -1, r;

    copy_if_rename_fails =
        aug_get(aug, AUGEAS_COPY_IF_RENAME_FAILS, NULL) == 1;

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

    augorig_canon = canonicalize_file_name(augorig);
    augorig_exists = 1;
    if (augorig_canon == NULL) {
        if (errno == ENOENT) {
            augorig_canon = augorig;
            augorig_exists = 0;
        } else {
            err_status = "canon_augorig";
            goto done;
        }
    }
    if (augorig_exists) {
        if (transfer_file_attrs(augorig_canon, augnew, &err_status) != 0)
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
        } else if (aug->flags & AUG_SAVE_NOOP) {
            result = 1;
            unlink(augnew);
            goto done;
        }
    }

    if (!(aug->flags & AUG_SAVE_NEWFILE)) {
        if (augorig_exists && (aug->flags & AUG_SAVE_BACKUP)) {
            r = asprintf(&augsave, "%s%s" EXT_AUGSAVE, aug->root, filename);
            if (r == -1) {
                augsave = NULL;
                goto done;
            }

            r = clone_file(augorig_canon, augsave, &err_status, 1);
            if (r != 0) {
                dyn_err_status = strappend(err_status, "_augsave");
                goto done;
            }
        }
        r = clone_file(augnew, augorig_canon, &err_status,
                       copy_if_rename_fails);
        if (r != 0) {
            dyn_err_status = strappend(err_status, "_augnew");
            goto done;
        }
    }
    result = 1;

 done:
    {
        const char *emsg =
            dyn_err_status == NULL ? err_status : dyn_err_status;
        store_error(aug, filename, path, emsg, errno, err);
    }
    free(dyn_err_status);
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
