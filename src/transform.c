/*
 * transform.c: support for building and running transformers
 *
 * Copyright (C) 2007-2016 David Lutterkort
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

#include <fnmatch.h>
#include <glob.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <selinux/selinux.h>
#include <stdbool.h>

#include "internal.h"
#include "memory.h"
#include "augeas.h"
#include "syntax.h"
#include "transform.h"
#include "errcode.h"

static const int fnm_flags = FNM_PATHNAME;
static const int glob_flags = GLOB_NOSORT;

/* Extension for newly created files */
#define EXT_AUGNEW ".augnew"
/* Extension for backup files */
#define EXT_AUGSAVE ".augsave"

/* Loaded files are tracked underneath METATREE. When a file with name
 * FNAME is loaded, certain entries are made under METATREE / FNAME:
 *   path      : path where tree for FNAME is put
 *   mtime     : time of last modification of the file as reported by stat(2)
 *   lens/info : information about where the applied lens was loaded from
 *   lens/id   : unique hexadecimal id of the lens
 *   error     : indication of errors during processing FNAME, or NULL
 *               if processing succeeded
 *   error/pos : position in file where error occurred (for get errors)
 *   error/path: path to tree node where error occurred (for put errors)
 *   error/message : human-readable error message
 */
static const char *const s_path = "path";
static const char *const s_lens = "lens";
static const char *const s_last = "last_matched";
static const char *const s_next = "next_not_matched";
static const char *const s_info = "info";
static const char *const s_mtime = "mtime";

static const char *const s_error = "error";
/* These are all put underneath "error" */
static const char *const s_pos     = "pos";
static const char *const s_message = "message";
static const char *const s_line    = "line";
static const char *const s_char    = "char";

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

static bool is_excl(struct tree *f) {
    return streqv(f->label, "excl") && f->value != NULL;
}

static bool is_incl(struct tree *f) {
    return streqv(f->label, "incl") && f->value != NULL;
}

static bool is_regular_file(const char *path) {
    int r;
    struct stat st;

    r = stat(path, &st);
    if (r < 0)
        return false;
    return S_ISREG(st.st_mode);
}

static char *mtime_as_string(struct augeas *aug, const char *fname) {
    int r;
    struct stat st;
    char *result = NULL;

    if (fname == NULL) {
        result = strdup("0");
        ERR_NOMEM(result == NULL, aug);
        goto done;
    }

    r = stat(fname, &st);
    if (r < 0) {
        /* If we fail to stat, silently ignore the error
         * and report an impossible mtime */
        result = strdup("0");
        ERR_NOMEM(result == NULL, aug);
    } else {
        r = xasprintf(&result, "%ld", (long) st.st_mtime);
        ERR_NOMEM(r < 0, aug);
    }
 done:
    return result;
 error:
    FREE(result);
    return NULL;
}

/* fnmatch(3) which will match // in a pattern to a path, like glob(3) does */
static int fnmatch_normalize(const char *pattern, const char *string, int flags) {
    int i, j, r;
    char *pattern_norm = NULL;

    r = ALLOC_N(pattern_norm, strlen(pattern) + 1);
    if (r < 0)
        goto error;

    for (i = 0, j = 0; i < strlen(pattern); i++) {
        if (pattern[i] != '/' || pattern[i+1] != '/') {
            pattern_norm[j] = pattern[i];
            j++;
        }
    }
    pattern_norm[j] = 0;

    r = fnmatch(pattern_norm, string, flags);
    FREE(pattern_norm);
    return r;

 error:
    if (pattern_norm != NULL)
        FREE(pattern_norm);
    return -1;
}

static bool file_current(struct augeas *aug, const char *fname,
                         struct tree *finfo) {
    struct tree *mtime = tree_child(finfo, s_mtime);
    struct tree *file = NULL, *path = NULL;
    int r;
    struct stat st;
    int64_t mtime_i;

    if (mtime == NULL || mtime->value == NULL)
        return false;

    r = xstrtoint64(mtime->value, 10, &mtime_i);
    if (r < 0) {
        /* Ignore silently and err on the side of caution */
        return false;
    }

    r = stat(fname, &st);
    if (r < 0)
        return false;

    if (mtime_i != (int64_t) st.st_mtime)
        return false;

    path = tree_child(finfo, s_path);
    if (path == NULL)
        return false;

    file = tree_fpath(aug, path->value);
    return (file != NULL && ! file->dirty);
}

static int filter_generate(struct tree *xfm, const char *root,
                           int *nmatches, char ***matches) {
    glob_t globbuf;
    int gl_flags = glob_flags;
    int r;
    int ret = 0;
    char **pathv = NULL;
    int pathc = 0;
    int root_prefix = strlen(root) - 1;

    *nmatches = 0;
    *matches = NULL;
    MEMZERO(&globbuf, 1);

    list_for_each(f, xfm->children) {
        char *globpat = NULL;
        if (! is_incl(f))
            continue;
        pathjoin(&globpat, 2, root, f->value);
        r = glob(globpat, gl_flags, NULL, &globbuf);
        free(globpat);

        if (r != 0 && r != GLOB_NOMATCH)
            goto error;
        gl_flags |= GLOB_APPEND;
    }

    pathc = globbuf.gl_pathc;
    int pathind = 0;

    if (ALLOC_N(pathv, pathc) < 0)
        goto error;

    for (int i=0; i < pathc; i++) {
        const char *path = globbuf.gl_pathv[i] + root_prefix;
        bool include = true;

        list_for_each(e, xfm->children) {
            if (! is_excl(e))
                continue;

            if (strchr(e->value, SEP) == NULL)
                path = pathbase(path);

            r = fnmatch_normalize(e->value, path, fnm_flags);
            if (r < 0)
                goto error;
            else if (r == 0)
                include = false;
        }

        if (include)
            include = is_regular_file(globbuf.gl_pathv[i]);

        if (include) {
            pathv[pathind] = strdup(globbuf.gl_pathv[i]);
            if (pathv[pathind] == NULL)
                goto error;
            pathind += 1;
        }
    }
    pathc = pathind;

    if (REALLOC_N(pathv, pathc) == -1)
        goto error;

    *matches = pathv;
    *nmatches = pathc;
 done:
    globfree(&globbuf);
    return ret;
 error:
    if (pathv != NULL)
        for (int i=0; i < pathc; i++)
            free(pathv[i]);
    free(pathv);
    ret = -1;
    goto done;
}

int filter_matches(struct tree *xfm, const char *path) {
    int found = 0;
    list_for_each(f, xfm->children) {
        if (is_incl(f) && fnmatch_normalize(f->value, path, fnm_flags) == 0) {
            found = 1;
            break;
        }
    }
    if (! found)
        return 0;
    list_for_each(f, xfm->children) {
        if (is_excl(f) && (fnmatch_normalize(f->value, path, fnm_flags) == 0))
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
    if (filename == NULL)
        pathjoin(&result, 2, AUGEAS_META_FILES, s_error);
    else
        pathjoin(&result, 3, AUGEAS_META_FILES, filename, s_error);
    return result;
}

ATTRIBUTE_FORMAT(printf, 4, 5)
static struct tree *err_set(struct augeas *aug,
                            struct tree *err_info, const char *sub,
                            const char *format, ...) {
    int r;
    va_list ap;
    char *value = NULL;
    struct tree *tree = NULL;

    va_start(ap, format);
    r = vasprintf(&value, format, ap);
    va_end(ap);
    if (r < 0)
        value = NULL;
    ERR_NOMEM(r < 0, aug);

    tree = tree_child_cr(err_info, sub);
    ERR_NOMEM(tree == NULL, aug);

    r = tree_set_value(tree, value);
    ERR_NOMEM(r < 0, aug);

 error:
    free(value);
    return tree;
}

static struct tree *err_lens_entry(struct augeas *aug, struct tree *where,
                           struct lens *lens, const char *label) {
    struct tree *result = NULL;

    if (lens == NULL)
        return NULL;

    char *fi = format_info(lens->info);
    if (fi != NULL) {
        result = err_set(aug, where, label, "%s", fi);
        free(fi);
    }
    return result;
}

/* Record an error in the tree. The error will show up underneath
 * /augeas/FILENAME/error if filename is not NULL, and underneath
 * /augeas/text/PATH otherwise. PATH is the path to the toplevel node in
 * the tree where the lens application happened. When STATUS is NULL, just
 * clear any error associated with FILENAME in the tree.
 */
static int store_error(struct augeas *aug,
                       const char *filename, const char *path,
                       const char *status, int errnum,
                       const struct lns_error *err, const char *text) {
    struct tree *err_info = NULL, *finfo = NULL;
    char *fip = NULL;
    int r;
    int result = -1;

    if (filename != NULL) {
        r = pathjoin(&fip, 2, AUGEAS_META_FILES, filename);
    } else {
        r = pathjoin(&fip, 2, AUGEAS_META_TEXT, path);
    }
    ERR_NOMEM(r < 0, aug);

    finfo = tree_fpath_cr(aug, fip);
    ERR_BAIL(aug);

    if (status != NULL) {
        err_info = tree_child_cr(finfo, s_error);
        ERR_NOMEM(err_info == NULL, aug);

        r = tree_set_value(err_info, status);
        ERR_NOMEM(r < 0, aug);

        /* Errors from err_set are ignored on purpose. We try
         * to report as much as we can */
        if (err != NULL) {
            if (err->pos >= 0) {
                size_t line, ofs;
                err_set(aug, err_info, s_pos, "%d", err->pos);
                if (text != NULL) {
                    calc_line_ofs(text, err->pos, &line, &ofs);
                    err_set(aug, err_info, s_line, "%zd", line);
                    err_set(aug, err_info, s_char, "%zd", ofs);
                }
            }
            if (err->path != NULL) {
                err_set(aug, err_info, s_path, "%s%s", path, err->path);
            }
            struct tree *t = err_lens_entry(aug, err_info, err->lens, s_lens);
            if (t != NULL) {
                err_lens_entry(aug, t, err->last, s_last);
                err_lens_entry(aug, t, err->next, s_next);
            }
            err_set(aug, err_info, s_message, "%s", err->message);
        } else if (errnum != 0) {
            const char *msg = strerror(errnum);
            err_set(aug, err_info, s_message, "%s", msg);
        }
    } else {
        /* No error, nuke the error node if it exists */
        err_info = tree_child(finfo, s_error);
        if (err_info != NULL)
            tree_unlink(aug, err_info);
    }

    tree_clean(finfo);
    result = 0;
 error:
    free(fip);
    return result;
}

/* Set up the file information in the /augeas tree.
 *
 * NODE must be the path to the file contents, and start with /files.
 * LENS is the lens used to transform the file.
 * Create entries under /augeas/NODE with some metadata about the file.
 *
 * Returns 0 on success, -1 on error
 */
static int add_file_info(struct augeas *aug, const char *node,
                         struct lens *lens, const char *lens_name,
                         const char *filename, bool force_reload) {
    struct tree *file, *tree;
    char *tmp = NULL;
    int r;
    char *path = NULL;
    int result = -1;

    if (lens == NULL)
        return -1;

    r = pathjoin(&path, 2, AUGEAS_META_TREE, node);
    ERR_NOMEM(r < 0, aug);

    file = tree_fpath_cr(aug, path);
    file->file = true;
    ERR_BAIL(aug);

    /* Set 'path' */
    tree = tree_child_cr(file, s_path);
    ERR_NOMEM(tree == NULL, aug);
    r = tree_set_value(tree, node);
    ERR_NOMEM(r < 0, aug);

    /* Set 'mtime' */
    if (force_reload) {
        tmp = strdup("0");
        ERR_NOMEM(tmp == NULL, aug);
    } else {
        tmp = mtime_as_string(aug, filename);
        ERR_BAIL(aug);
    }
    tree = tree_child_cr(file, s_mtime);
    ERR_NOMEM(tree == NULL, aug);
    tree_store_value(tree, &tmp);

    /* Set 'lens/info' */
    tmp = format_info(lens->info);
    ERR_NOMEM(tmp == NULL, aug);
    tree = tree_path_cr(file, 2, s_lens, s_info);
    ERR_NOMEM(tree == NULL, aug);
    r = tree_set_value(tree, tmp);
    ERR_NOMEM(r < 0, aug);
    FREE(tmp);

    /* Set 'lens' */
    tree = tree->parent;
    r = tree_set_value(tree, lens_name);
    ERR_NOMEM(r < 0, aug);

    tree_clean(file);

    result = 0;
 error:
    free(path);
    free(tmp);
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

/* Turn the file name FNAME, which starts with aug->root, into
 * a path in the tree underneath /files */
static char *file_name_path(struct augeas *aug, const char *fname) {
    char *path = NULL;

    pathjoin(&path, 2, AUGEAS_FILES_TREE, fname + strlen(aug->root) - 1);
    return path;
}

/* Replace the subtree for FPATH with SUB */
static void tree_freplace(struct augeas *aug, const char *fpath,
                         struct tree *sub) {
    struct tree *parent;

    parent = tree_fpath_cr(aug, fpath);
    ERR_RET(aug);

    parent->file = true;
    tree_unlink_children(aug, parent);
    list_append(parent->children, sub);
    list_for_each(s, sub) {
        s->parent = parent;
    }
}

static struct info*
make_lns_info(struct augeas *aug, const char *filename,
              const char *text, int text_len) {
    struct info *info = NULL;

    make_ref(info);
    ERR_NOMEM(info == NULL, aug);

    if (filename != NULL) {
        make_ref(info->filename);
        ERR_NOMEM(info->filename == NULL, aug);
        info->filename->str = strdup(filename);
    }

    info->first_line = 1;
    info->last_line = 1;
    info->first_column = 1;
    if (text != NULL) {
        info->last_column = text_len;
    }

    info->error = aug->error;

    return info;
 error:
    unref(info, info);
    return NULL;
}

/*
 * Do the bookkeeping around calling lns_get that is common to load_file
 * and text_store, in particular, make sure the tree we read gets put into
 * the right place in AUG and that the span for that tree gets set.
 *
 * Transform TEXT using LENS and put the resulting tree at PATH. Use
 * FILENAME in error messages to indicate where the TEXT came from.
 */
static void lens_get(struct augeas *aug,
                     struct lens *lens,
                     const char *filename,
                     const char *text, int text_len,
                     const char *path,
                     struct lns_error **err) {
    struct info *info = NULL;
    struct span *span = NULL;
    struct tree *tree = NULL;

    info = make_lns_info(aug, filename, text, text_len);
    ERR_BAIL(aug);

    if (aug->flags & AUG_ENABLE_SPAN) {
        /* Allocate the span already to capture a reference to
           info->filename */
        span = make_span(info);
        ERR_NOMEM(span == NULL, info);
    }

    tree = lns_get(info, lens, text, aug->flags & AUG_ENABLE_SPAN, err);

    if (*err == NULL) {
        // Successful get
        tree_freplace(aug, path, tree);
        ERR_BAIL(aug);

        /* top level node span entire file length */
        if (span != NULL && tree != NULL) {
            tree->parent->span = move(span);
            tree->parent->span->span_start = 0;
            tree->parent->span->span_end = text_len;
        }
        tree = NULL;
    }
 error:
    free_span(span);
    unref(info, info);
    free_tree(tree);
}

static int load_file(struct augeas *aug, struct lens *lens,
                     const char *lens_name, char *filename) {
    char *text = NULL;
    const char *err_status = NULL;
    char *path = NULL;
    struct lns_error *err = NULL;
    int result = -1, r, text_len = 0;

    path = file_name_path(aug, filename);
    ERR_NOMEM(path == NULL, aug);

    r = add_file_info(aug, path, lens, lens_name, filename, false);
    if (r < 0)
        goto done;

    text = xread_file(filename);
    if (text == NULL) {
        err_status = "read_failed";
        goto done;
    }
    text_len = strlen(text);
    text = append_newline(text, text_len);

    lens_get(aug, lens, filename, text, text_len, path, &err);
    if (err != NULL) {
        err_status = "parse_failed";
        goto done;
    }
    ERR_BAIL(aug);

    result = 0;
 done:
    store_error(aug, filename + strlen(aug->root) - 1, path, err_status,
                errno, err, text);
 error:
    free_lns_error(err);
    free(path);
    free(text);
    return result;
}

/* The lens for a transform can be referred to in one of two ways:
 * either by a fully qualified name "Module.lens" or by the special
 * syntax "@Module"; the latter means we should take the lens from the
 * autoload transform for Module
 */
static struct lens *lens_from_name(struct augeas *aug, const char *name) {
    struct lens *result = NULL;

    if (name[0] == '@') {
        struct module *modl = NULL;
        for (modl = aug->modules;
             modl != NULL && !streqv(modl->name, name + 1);
             modl = modl->next);
        ERR_THROW(modl == NULL, aug, AUG_ENOLENS,
                  "Could not find module %s", name + 1);
        ERR_THROW(modl->autoload == NULL, aug, AUG_ENOLENS,
                  "No autoloaded lens in module %s", name + 1);
        result = modl->autoload->lens;
    } else {
        result = lens_lookup(aug, name);
    }
    ERR_THROW(result == NULL, aug, AUG_ENOLENS,
              "Can not find lens %s", name);
    return result;
 error:
    return NULL;
}

int text_store(struct augeas *aug, const char *lens_path,
               const char *path, const char *text) {
    struct lns_error *err = NULL;
    int result = -1;
    const char *err_status = NULL;
    struct lens *lens = NULL;

    lens = lens_from_name(aug, lens_path);
    ERR_BAIL(aug);

    lens_get(aug, lens, path, text, strlen(text), path, &err);
    if (err != NULL) {
        err_status = "parse_failed";
        goto error;
    }
    ERR_BAIL(aug);

    result = 0;
 error:
    store_error(aug, NULL, path, err_status, errno, err, text);
    free_lns_error(err);
    return result;
}

const char *xfm_lens_name(struct tree *xfm) {
    struct tree *l = tree_child(xfm, s_lens);

    if (l == NULL)
        return "(unknown)";
    if (l->value == NULL)
        return "(noname)";
    return l->value;
}

struct lens *xfm_lens(struct augeas *aug,
                      struct tree *xfm, const char **lens_name) {
    struct tree *l = NULL;

    if (lens_name != NULL)
        *lens_name = NULL;

    for (l = xfm->children;
         l != NULL && !streqv("lens", l->label);
         l = l->next);

    if (l == NULL || l->value == NULL)
        return NULL;
    if (lens_name != NULL)
        *lens_name = l->value;

    return lens_from_name(aug, l->value);
}

static void xfm_error(struct tree *xfm, const char *msg) {
    char *v = msg ? strdup(msg) : NULL;
    char *l = strdup("error");

    if (l == NULL || v == NULL) {
        free(v);
        free(l);
        return;
    }
    tree_append(xfm, l, v);
}

int transform_validate(struct augeas *aug, struct tree *xfm) {
    struct tree *l = NULL;

    for (struct tree *t = xfm->children; t != NULL; ) {
        if (streqv(t->label, "lens")) {
            l = t;
        } else if ((is_incl(t) || (is_excl(t) && strchr(t->value, SEP) != NULL))
                       && t->value[0] != SEP) {
            /* Normalize relative paths to absolute ones */
            int r;
            r = REALLOC_N(t->value, strlen(t->value) + 2);
            ERR_NOMEM(r < 0, aug);
            memmove(t->value + 1, t->value, strlen(t->value) + 1);
            t->value[0] = SEP;
        }

        if (streqv(t->label, "error")) {
            struct tree *del = t;
            t = del->next;
            tree_unlink(aug, del);
        } else {
            t = t->next;
        }
    }

    if (l == NULL) {
        xfm_error(xfm, "missing a child with label 'lens'");
        return -1;
    }
    if (l->value == NULL) {
        xfm_error(xfm, "the 'lens' node does not contain a lens name");
        return -1;
    }
    lens_from_name(aug, l->value);
    ERR_BAIL(aug);

    return 0;
 error:
    xfm_error(xfm, aug->error->details);
    /* We recorded this error in the tree, clear it so that future
     * operations report this exact same error (against the wrong lens) */
    reset_error(aug->error);
    return -1;
}

void transform_file_error(struct augeas *aug, const char *status,
                          const char *filename, const char *format, ...) {
    char *ep = err_path(filename);
    struct tree *err;
    char *msg;
    va_list ap;
    int r;

    err = tree_fpath_cr(aug, ep);
    FREE(ep);
    if (err == NULL)
        return;

    tree_unlink_children(aug, err);
    tree_set_value(err, status);

    err = tree_child_cr(err, s_message);
    if (err == NULL)
        return;

    va_start(ap, format);
    r = vasprintf(&msg, format, ap);
    va_end(ap);
    if (r < 0)
        return;
    tree_set_value(err, msg);
    free(msg);
}

static struct tree *file_info(struct augeas *aug, const char *fname) {
    char *path = NULL;
    struct tree *result = NULL;
    int r;

    r = pathjoin(&path, 2, AUGEAS_META_FILES, fname);
    ERR_NOMEM(r < 0, aug);

    result = tree_fpath(aug, path);
    ERR_BAIL(aug);
 error:
    free(path);
    return result;
}

int transform_load(struct augeas *aug, struct tree *xfm, const char *file) {
    int nmatches = 0;
    char **matches;
    const char *lens_name;
    struct lens *lens = xfm_lens(aug, xfm, &lens_name);
    int r;

    if (lens == NULL) {
        // FIXME: Record an error and return 0
        return -1;
    }

    r = filter_generate(xfm, aug->root, &nmatches, &matches);
    if (r == -1)
        return -1;
    for (int i=0; i < nmatches; i++) {
        const char *filename = matches[i] + strlen(aug->root) - 1;
        struct tree *finfo = file_info(aug, filename);

        if (file != NULL && STRNEQ(filename, file)) {
            FREE(matches[i]);
            continue;
        }

        if (finfo != NULL && !finfo->dirty &&
            tree_child(finfo, s_lens) != NULL) {
            /* We have a potential conflict: since FINFO is not marked as
               dirty (see aug_load for how the dirty flag on nodes under
               /augeas/files is used during loading), we already processed
               it with another lens. The other lens is recorded in
               FINFO. If it so happens that the lenses are actually the
               same, we silently move on, as this duplication does no
               harm. If they are different we definitely have a problem and
               need to record an error and remove the work the first lens
               did. */
            const char *s = xfm_lens_name(finfo);
            struct lens *other_lens = lens_from_name(aug, s);
            if (lens != other_lens) {
                char *fpath = file_name_path(aug, matches[i]);
                transform_file_error(aug, "mxfm_load", filename,
                  "Lenses %s and %s could be used to load this file",
                                     s, lens_name);
                aug_rm(aug, fpath);
                free(fpath);
            }
        } else if (!file_current(aug, matches[i], finfo)) {
            load_file(aug, lens, lens_name, matches[i]);
        }
        if (finfo != NULL)
            finfo->dirty = 0;
        FREE(matches[i]);
    }
    lens_release(lens);
    free(matches);
    return 0;
}

int transform_applies(struct tree *xfm, const char *path) {
    if (STRNEQLEN(path, AUGEAS_FILES_TREE, strlen(AUGEAS_FILES_TREE))
        || path[strlen(AUGEAS_FILES_TREE)] != SEP)
        return 0;
    return filter_matches(xfm, path + strlen(AUGEAS_FILES_TREE));
}

static int transfer_file_attrs(FILE *from, FILE *to,
                               const char **err_status) {
    struct stat st;
    int ret = 0;
    int selinux_enabled = (is_selinux_enabled() > 0);
    security_context_t con = NULL;

    int from_fd;
    int to_fd = fileno(to);

    if (from == NULL) {
        *err_status = "replace_from_missing";
        return -1;
    }

    from_fd = fileno(from);

    ret = fstat(from_fd, &st);
    if (ret < 0) {
        *err_status = "replace_stat";
        return -1;
    }
    if (selinux_enabled) {
        if (fgetfilecon(from_fd, &con) < 0 && errno != ENOTSUP) {
            *err_status = "replace_getfilecon";
            return -1;
        }
    }

    if (fchown(to_fd, st.st_uid, st.st_gid) < 0) {
        *err_status = "replace_chown";
        return -1;
    }
    if (fchmod(to_fd, st.st_mode) < 0) {
        *err_status = "replace_chmod";
        return -1;
    }
    if (selinux_enabled && con != NULL) {
        if (fsetfilecon(to_fd, con) < 0 && errno != ENOTSUP) {
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
 * If COPY_IF_RENAME_FAILS and UNLINK_IF_RENAME_FAILS are true, and the above
 * copy mechanism is used, it will unlink the TO path and open with O_EXCL
 * to ensure we only copy *from* a bind mount rather than into an attacker's
 * mount placed at TO (e.g. for .augsave).
 *
 * Return 0 on success (either rename succeeded or we copied the contents
 * over successfully), -1 on failure.
 */
static int clone_file(const char *from, const char *to,
                      const char **err_status, int copy_if_rename_fails,
                      int unlink_if_rename_fails) {
    FILE *from_fp = NULL, *to_fp = NULL;
    char buf[BUFSIZ];
    size_t len;
    int to_fd = -1, to_oflags, r;
    int result = -1;

    if (rename(from, to) == 0)
        return 0;
    if ((errno != EXDEV && errno != EBUSY) || !copy_if_rename_fails) {
        *err_status = "rename";
        return -1;
    }

    /* rename not possible, copy file contents */
    if (!(from_fp = fopen(from, "r"))) {
        *err_status = "clone_open_src";
        goto done;
    }

    if (unlink_if_rename_fails) {
        r = unlink(to);
        if (r < 0) {
            *err_status = "clone_unlink_dst";
            goto done;
        }
    }

    to_oflags = unlink_if_rename_fails ? O_EXCL : O_TRUNC;
    if ((to_fd = open(to, O_WRONLY|O_CREAT|to_oflags, S_IRUSR|S_IWUSR)) < 0) {
        *err_status = "clone_open_dst";
        goto done;
    }
    if (!(to_fp = fdopen(to_fd, "w"))) {
        *err_status = "clone_fdopen_dst";
        goto done;
    }

    if (transfer_file_attrs(from_fp, to_fp, err_status) < 0)
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
    if (fflush(to_fp) != 0) {
        *err_status = "clone_flush";
        goto done;
    }
    if (fsync(fileno(to_fp)) < 0) {
        *err_status = "clone_sync";
        goto done;
    }
    result = 0;
 done:
    if (from_fp != NULL)
        fclose(from_fp);
    if (to_fp != NULL) {
        if (fclose(to_fp) != 0) {
            *err_status = "clone_fclose_dst";
            result = -1;
        }
    } else if (to_fd >= 0 && close(to_fd) < 0) {
        *err_status = "clone_close_dst";
        result = -1;
    }
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

static int file_saved_event(struct augeas *aug, const char *path) {
    const char *saved = strrchr(AUGEAS_EVENTS_SAVED, SEP) + 1;
    struct pathx *px;
    struct tree *dummy;
    int r;

    px = pathx_aug_parse(aug, aug->origin, NULL,
                         AUGEAS_EVENTS_SAVED "[last()]", true);
    ERR_BAIL(aug);

    if (pathx_find_one(px, &dummy) == 1) {
        r = tree_insert(px, saved, 0);
        if (r < 0)
            goto error;
    }

    if (! tree_set(px, path))
        goto error;

    free_pathx(px);
    return 0;
 error:
    free_pathx(px);
    return -1;
}

/*
 * Do the bookkeeping around calling LNS_PUT that's needed to update the
 * span after writing a tree to file
 */
static void lens_put(struct augeas *aug, const char *filename,
                     struct lens *lens, const char *text, struct tree *tree,
                     FILE *out, struct lns_error **err) {
    struct info *info = NULL;
    size_t text_len = strlen(text);
    bool with_span = aug->flags & AUG_ENABLE_SPAN;

    info = make_lns_info(aug, filename, text, text_len);
    ERR_BAIL(aug);

    if (with_span) {
        if (tree->span == NULL) {
            tree->span = make_span(info);
            ERR_NOMEM(tree->span == NULL, aug);
        }
        tree->span->span_start = ftell(out);
    }

    lns_put(info, out, lens, tree->children, text,
            aug->flags & AUG_ENABLE_SPAN, err);

    if (with_span) {
        tree->span->span_end = ftell(out);
    }
 error:
    unref(info, info);
}

/*
 * Save TREE->CHILDREN into the file PATH using the lens from XFORM. Errors
 * are noted in the /augeas/files hierarchy in AUG->ORIGIN under
 * PATH/error.
 *
 * Writing the file happens by first writing into a temp file, transferring all
 * file attributes of PATH to the temp file, and then renaming the temp file
 * back to PATH.
 *
 * Temp files are created alongside the destination file to enable the rename,
 * which may be the canonical path (PATH_canon) if PATH is a symlink.
 *
 * If the AUG_SAVE_NEWFILE flag is set, instead rename to PATH.augnew rather
 * than PATH.  If AUG_SAVE_BACKUP is set, move the original to PATH.augsave.
 * (Always PATH.aug{new,save} irrespective of whether PATH is a symlink.)
 *
 * If the rename fails, and the entry AUGEAS_COPY_IF_FAILURE exists in
 * AUG->ORIGIN, PATH is instead overwritten by copying file contents.
 *
 * The table below shows the locations for each permutation.
 *
 * PATH       save flag    temp file           dest file      backup?
 * regular    -            PATH.XXXX           PATH           -
 * regular    BACKUP       PATH.XXXX           PATH           PATH.augsave
 * regular    NEWFILE      PATH.augnew.XXXX    PATH.augnew    -
 * symlink    -            PATH_canon.XXXX     PATH_canon     -
 * symlink    BACKUP       PATH_canon.XXXX     PATH_canon     PATH.augsave
 * symlink    NEWFILE      PATH.augnew.XXXX    PATH.augnew    -
 *
 * Return 0 on success, -1 on failure.
 */
int transform_save(struct augeas *aug, struct tree *xfm,
                   const char *path, struct tree *tree) {
    int   fd;
    FILE *fp = NULL, *augorig_canon_fp = NULL;
    char *augtemp = NULL, *augnew = NULL, *augorig = NULL, *augsave = NULL;
    char *augorig_canon = NULL, *augdest = NULL;
    int   augorig_exists;
    int   copy_if_rename_fails = 0;
    char *text = NULL;
    const char *filename = path + strlen(AUGEAS_FILES_TREE) + 1;
    const char *err_status = NULL;
    char *dyn_err_status = NULL;
    struct lns_error *err = NULL;
    const char *lens_name;
    struct lens *lens = xfm_lens(aug, xfm, &lens_name);
    int result = -1, r;
    bool force_reload;
    struct info *info = NULL;

    errno = 0;

    if (lens == NULL) {
        err_status = "lens_name";
        goto done;
    }

    copy_if_rename_fails =
        aug_get(aug, AUGEAS_COPY_IF_RENAME_FAILS, NULL) == 1;

    if (asprintf(&augorig, "%s%s", aug->root, filename) == -1) {
        augorig = NULL;
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

    if (access(augorig_canon, R_OK) == 0) {
        augorig_canon_fp = fopen(augorig_canon, "r");
        text = xfread_file(augorig_canon_fp);
    } else {
        text = strdup("");
    }

    if (text == NULL) {
        err_status = "put_read";
        goto done;
    }

    text = append_newline(text, strlen(text));

    /* Figure out where to put the .augnew and temp file. If no .augnew file
       then put the temp file next to augorig_canon, else next to .augnew. */
    if (aug->flags & AUG_SAVE_NEWFILE) {
        if (xasprintf(&augnew, "%s" EXT_AUGNEW, augorig) < 0) {
            err_status = "augnew_oom";
            goto done;
        }
        augdest = augnew;
    } else {
        augdest = augorig_canon;
    }

    if (xasprintf(&augtemp, "%s.XXXXXX", augdest) < 0) {
        err_status = "augtemp_oom";
        goto done;
    }

    // FIXME: We might have to create intermediate directories
    // to be able to write augnew, but we have no idea what permissions
    // etc. they should get. Just the process default ?
    fd = mkstemp(augtemp);
    if (fd < 0) {
        err_status = "mk_augtemp";
        goto done;
    }
    fp = fdopen(fd, "w");
    if (fp == NULL) {
        err_status = "open_augtemp";
        goto done;
    }

    if (augorig_exists) {
        if (transfer_file_attrs(augorig_canon_fp, fp, &err_status) != 0) {
            goto done;
        }
    } else {
        /* Since mkstemp is used, the temp file will have secure permissions
         * instead of those implied by umask, so change them for new files */
        mode_t curumsk = umask(022);
        umask(curumsk);

        if (fchmod(fileno(fp), 0666 & ~curumsk) < 0) {
            err_status = "create_chmod";
            goto done;
        }
    }

    if (tree != NULL) {
        lens_put(aug, augorig_canon, lens, text, tree, fp, &err);
        ERR_BAIL(aug);
    }

    if (ferror(fp)) {
        err_status = "error_augtemp";
        goto done;
    }

    if (fflush(fp) != 0) {
        err_status = "flush_augtemp";
        goto done;
    }

    if (fsync(fileno(fp)) < 0) {
        err_status = "sync_augtemp";
        goto done;
    }

    if (fclose(fp) != 0) {
        err_status = "close_augtemp";
        fp = NULL;
        goto done;
    }

    fp = NULL;

    if (err != NULL) {
        err_status = err->pos >= 0 ? "parse_skel_failed" : "put_failed";
        unlink(augtemp);
        goto done;
    }

    {
        char *new_text = xread_file(augtemp);
        int same = 0;
        if (new_text == NULL) {
            err_status = "read_augtemp";
            goto done;
        }
        same = STREQ(text, new_text);
        FREE(new_text);
        if (same) {
            result = 0;
            unlink(augtemp);
            goto done;
        } else if (aug->flags & AUG_SAVE_NOOP) {
            result = 1;
            unlink(augtemp);
            goto done;
        }
    }

    if (!(aug->flags & AUG_SAVE_NEWFILE)) {
        if (augorig_exists && (aug->flags & AUG_SAVE_BACKUP)) {
            r = xasprintf(&augsave, "%s" EXT_AUGSAVE, augorig);
            if (r == -1) {
                augsave = NULL;
                goto done;
            }

            r = clone_file(augorig_canon, augsave, &err_status, 1, 1);
            if (r != 0) {
                dyn_err_status = strappend(err_status, "_augsave");
                goto done;
            }
        }
    }

    r = clone_file(augtemp, augdest, &err_status, copy_if_rename_fails, 0);
    if (r != 0) {
        unlink(augtemp);
        dyn_err_status = strappend(err_status, "_augtemp");
        goto done;
    }

    result = 1;

 done:
    force_reload = aug->flags & AUG_SAVE_NEWFILE;
    r = add_file_info(aug, path, lens, lens_name, augorig, force_reload);
    if (r < 0) {
        err_status = "file_info";
        result = -1;
    }
    if (result > 0) {
        r = file_saved_event(aug, path);
        if (r < 0) {
            err_status = "saved_event";
            result = -1;
        }
    }
    {
        const char *emsg =
            dyn_err_status == NULL ? err_status : dyn_err_status;
        store_error(aug, filename, path, emsg, errno, err, text);
    }
 error:
    free(dyn_err_status);
    lens_release(lens);
    free(text);
    free(augtemp);
    free(augnew);
    if (augorig_canon != augorig)
        free(augorig_canon);
    free(augorig);
    free(augsave);
    free_lns_error(err);
    unref(info, info);

    if (fp != NULL)
        fclose(fp);
    if (augorig_canon_fp != NULL)
        fclose(augorig_canon_fp);
    return result;
}

int text_retrieve(struct augeas *aug, const char *lens_name,
                  const char *path, struct tree *tree,
                  const char *text_in, char **text_out) {
    struct memstream ms;
    bool ms_open = false;
    const char *err_status = NULL;
    struct lns_error *err = NULL;
    struct lens *lens = NULL;
    int result = -1, r;
    struct info *info = NULL;

    MEMZERO(&ms, 1);
    errno = 0;

    lens = lens_from_name(aug, lens_name);
    if (lens == NULL) {
        err_status = "lens_name";
        goto done;
    }

    r = init_memstream(&ms);
    if (r < 0) {
        err_status = "init_memstream";
        goto done;
    }
    ms_open = true;

    if (tree != NULL) {
        lens_put(aug, path, lens, text_in, tree, ms.stream, &err);
        ERR_BAIL(aug);
    }

    r = close_memstream(&ms);
    ms_open = false;
    if (r < 0) {
        err_status = "close_memstream";
        goto done;
    }

    *text_out = ms.buf;
    ms.buf = NULL;

    if (err != NULL) {
        err_status = err->pos >= 0 ? "parse_skel_failed" : "put_failed";
        goto done;
    }

    result = 0;

 done:
    store_error(aug, NULL, path, err_status, errno, err, text_in);
 error:
    lens_release(lens);
    if (result < 0) {
        free(*text_out);
        *text_out = NULL;
    }
    free_lns_error(err);
    unref(info, info);

    if (ms_open)
        close_memstream(&ms);
    return result;
}

int remove_file(struct augeas *aug, struct tree *tree) {
    const char *err_status = NULL;
    char *dyn_err_status = NULL;
    char *augsave = NULL, *augorig = NULL, *augorig_canon = NULL;
    struct tree *path = NULL;
    const char *file_path = NULL;
    char *meta_path = NULL;
    int r;

    path = tree_child(tree, s_path);
    if (path == NULL) {
        err_status = "no child called 'path' for file entry";
        goto error;
    }
    file_path = path->value + strlen(AUGEAS_FILES_TREE);
    path = NULL;

    meta_path = path_of_tree(tree);
    if (meta_path == NULL) {
        err_status = "path_of_tree";
        goto error;
    }

    if ((augorig = strappend(aug->root, file_path)) == NULL) {
        err_status = "root_file";
        goto error;
    }

    augorig_canon = canonicalize_file_name(augorig);
    if (augorig_canon == NULL) {
        if (errno == ENOENT) {
            goto done;
        } else {
            err_status = "canon_augorig";
            goto error;
        }
    }

    r = file_saved_event(aug, meta_path + strlen(AUGEAS_META_TREE));
    if (r < 0) {
        err_status = "saved_event";
        goto error;
    }

    if (aug->flags & AUG_SAVE_NOOP)
        goto done;

    if (aug->flags & AUG_SAVE_BACKUP) {
        /* Move file to one with extension .augsave */
        r = asprintf(&augsave, "%s" EXT_AUGSAVE, augorig_canon);
        if (r == -1) {
            augsave = NULL;
                goto error;
        }

        r = clone_file(augorig_canon, augsave, &err_status, 1, 1);
        if (r != 0) {
            dyn_err_status = strappend(err_status, "_augsave");
            goto error;
        }
    } else {
        /* Unlink file */
        r = unlink(augorig_canon);
        if (r < 0) {
            err_status = "unlink_orig";
            goto error;
        }
    }
    path = NULL;
    tree_unlink(aug, tree);
 done:
    free(meta_path);
    free(augorig);
    free(augorig_canon);
    free(augsave);
    return 0;
 error:
    {
        const char *emsg =
            dyn_err_status == NULL ? err_status : dyn_err_status;
        store_error(aug, file_path, meta_path, emsg, errno, NULL, NULL);
    }
    free(meta_path);
    free(augorig);
    free(augorig_canon);
    free(augsave);
    free(dyn_err_status);
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
