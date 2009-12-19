/*
 * builtin.c: builtin primitives
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
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>

#include "syntax.h"
#include "memory.h"
#include "transform.h"
#include "errcode.h"

#define UNIMPL_BODY(name)                       \
    {                                           \
        FIXME(#name " called");                 \
        abort();                                \
    }

/*
 * Lenses
 */

/* V_REGEXP -> V_STRING -> V_LENS */
static struct value *lns_del(struct info *info,
                             struct value *rxp, struct value *dflt) {
    assert(rxp->tag == V_REGEXP);
    assert(dflt->tag == V_STRING);
    return lns_make_prim(L_DEL, ref(info),
                         ref(rxp->regexp), ref(dflt->string));
}

/* V_REGEXP -> V_LENS */
static struct value *lns_store(struct info *info, struct value *rxp) {
    assert(rxp->tag == V_REGEXP);
    return lns_make_prim(L_STORE, ref(info), ref(rxp->regexp), NULL);
}

/* V_REGEXP -> V_LENS */
static struct value *lns_key(struct info *info, struct value *rxp) {
    assert(rxp->tag == V_REGEXP);
    return lns_make_prim(L_KEY, ref(info), ref(rxp->regexp), NULL);
}

/* V_STRING -> V_LENS */
static struct value *lns_label(struct info *info, struct value *str) {
    assert(str->tag == V_STRING);
    return lns_make_prim(L_LABEL, ref(info), NULL, ref(str->string));
}

/* V_STRING -> V_LENS */
static struct value *lns_seq(struct info *info, struct value *str) {
    assert(str->tag == V_STRING);
    return lns_make_prim(L_SEQ, ref(info), NULL, ref(str->string));
}

/* V_STRING -> V_LENS */
static struct value *lns_counter(struct info *info, struct value *str) {
    assert(str->tag == V_STRING);
    return lns_make_prim(L_COUNTER, ref(info), NULL, ref(str->string));
}

static struct value *make_exn_lns_error(struct info *info,
                                        struct lns_error *err,
                                        const char *text) {
    struct value *v;

    if (HAS_ERR(info))
        return exn_error();

    v = make_exn_value(ref(info), "%s", err->message);
    if (err->lens != NULL) {
        char *s = format_info(err->lens->info);
        exn_printf_line(v, "Lens: %s", s);
        free(s);
    }
    if (err->pos >= 0) {
        char *pos = format_pos(text, err->pos);
        size_t line, ofs;
        calc_line_ofs(text, err->pos, &line, &ofs);
        exn_printf_line(v,
                     "Error encountered at %d:%d (%d characters into string)",
                        (int) line, (int) ofs, err->pos);
        if (pos != NULL)
            exn_printf_line(v, "%s", pos);
        free(pos);
    } else {
        exn_printf_line(v, "Error encountered at path %s", err->path);
    }

    return v;
}

static void exn_print_tree(struct value *exn, struct tree *tree) {
    struct memstream ms;

    init_memstream(&ms);
    dump_tree(ms.stream, tree);
    close_memstream(&ms);
    exn_printf_line(exn, "%s", ms.buf);
    FREE(ms.buf);
}

static struct value *make_pathx_exn(struct info *info, struct pathx *p) {
    struct value *v;
    char *msg;
    const char *txt;
    int pos;

    msg = strdup(pathx_error(p, &txt, &pos));
    if (msg == NULL)
        return NULL;

    v = make_exn_value(ref(info), "syntax error in path expression: %s", msg);
    if (ALLOC_N(msg, strlen(txt) + 4) >= 0) {
        strncpy(msg, txt, pos);
        strcat(msg, "|=|");
        strcat(msg, txt + pos);
        exn_add_lines(v, 1, msg);
    }
    return v;
}

/* V_LENS -> V_STRING -> V_TREE */
static struct value *lens_get(struct info *info, struct value *l,
                              struct value *str) {
    assert(l->tag == V_LENS);
    assert(str->tag == V_STRING);
    struct lns_error *err;
    struct value *v;
    const char *text = str->string->str;

    struct tree *tree = lns_get(info, l->lens, text, &err);
    if (err == NULL && ! HAS_ERR(info)) {
        v = make_value(V_TREE, ref(info));
        v->origin = make_tree_origin(tree);
    } else {
        struct tree *t = make_tree_origin(tree);
        if (t == NULL)
            free_tree(tree);
        tree = t;

        v = make_exn_lns_error(info, err, text);
        if (tree != NULL) {
            exn_printf_line(v, "Tree generated so far:");
            exn_print_tree(v, tree);
            free_tree(tree);
        }
        free_lns_error(err);
    }
    return v;
}


/* V_LENS -> V_TREE -> V_STRING -> V_STRING */
static struct value *lens_put(struct info *info, struct value *l,
                              struct value *tree, struct value *str) {
    assert(l->tag == V_LENS);
    assert(tree->tag == V_TREE);
    assert(str->tag == V_STRING);

    struct memstream ms;
    struct value *v;
    struct lns_error *err;

    init_memstream(&ms);
    lns_put(ms.stream, l->lens, tree->origin->children,
            str->string->str, &err);
    close_memstream(&ms);

    if (err == NULL && ! HAS_ERR(info)) {
        v = make_value(V_STRING, ref(info));
        v->string = make_string(ms.buf);
    } else {
        v = make_exn_lns_error(info, err, str->string->str);
        free_lns_error(err);
        FREE(ms.buf);
    }
    return v;
}

/* V_STRING -> V_STRING -> V_TREE -> V_TREE */
static struct value *tree_set_glue(struct info *info, struct value *path,
                                   struct value *val, struct value *tree) {
    // FIXME: This only works if TREE is not referenced more than once;
    // otherwise we'll have some pretty weird semantics, and would really
    // need to copy TREE first
    assert(path->tag == V_STRING);
    assert(val->tag == V_STRING);
    assert(tree->tag == V_TREE);

    struct tree *fake = NULL;
    struct pathx *p = NULL;
    struct value *result = NULL;

    if (tree->origin->children == NULL) {
        tree->origin->children = make_tree(NULL, NULL, tree->origin, NULL);
        fake = tree->origin->children;
    }

    if (pathx_parse(tree->origin, NULL, path->string->str, true, NULL, &p)
        != PATHX_NOERROR) {
        result = make_pathx_exn(ref(info), p);
        goto done;
    }

    if (tree_set(p, val->string->str) == NULL) {
        result = make_exn_value(ref(info),
                                "Tree set of %s to '%s' failed",
                                path->string->str, val->string->str);
        goto done;
    }
    if (fake != NULL) {
        list_remove(fake, tree->origin->children);
        free_tree(fake);
    }
    result = ref(tree);

 done:
    free_pathx(p);
    return result;
}

/* V_STRING -> V_TREE -> V_TREE */
static struct value *tree_clear_glue(struct info *info, struct value *path,
                                     struct value *tree) {
    // FIXME: This only works if TREE is not referenced more than once;
    // otherwise we'll have some pretty weird semantics, and would really
    // need to copy TREE first
    assert(path->tag == V_STRING);
    assert(tree->tag == V_TREE);

    struct tree *fake = NULL;
    struct pathx *p = NULL;
    struct value *result = NULL;

    if (tree->origin->children == NULL) {
        tree->origin->children = make_tree(NULL, NULL, tree->origin, NULL);
        fake = tree->origin->children;
    }

    if (pathx_parse(tree->origin, NULL, path->string->str, true, NULL, &p)
        != PATHX_NOERROR) {
        result = make_pathx_exn(ref(info), p);
        goto done;
    }

    if (tree_set(p, NULL) == NULL) {
        result = make_exn_value(ref(info),
                                "Tree set of %s to NULL failed",
                                path->string->str);
        goto done;
    }
    if (fake != NULL) {
        list_remove(fake, tree->origin->children);
        free_tree(fake);
    }
    result = ref(tree);

 done:
    free_pathx(p);
    return result;
}

static struct value *tree_insert_glue(struct info *info, struct value *label,
                                      struct value *path, struct value *tree,
                                      int before) {
    // FIXME: This only works if TREE is not referenced more than once;
    // otherwise we'll have some pretty weird semantics, and would really
    // need to copy TREE first
    assert(label->tag == V_STRING);
    assert(path->tag == V_STRING);
    assert(tree->tag == V_TREE);

    int r;
    struct pathx *p = NULL;
    struct value *result = NULL;

    if (pathx_parse(tree->origin, NULL, path->string->str, true, NULL, &p)
        != PATHX_NOERROR) {
        result = make_pathx_exn(ref(info), p);
        goto done;
    }

    r = tree_insert(p, label->string->str, before);
    if (r != 0) {
        result = make_exn_value(ref(info),
                                "Tree insert of %s at %s failed",
                                label->string->str, path->string->str);
        goto done;
    }

    result = ref(tree);
 done:
    free_pathx(p);
    return result;
}

/* Insert after */
/* V_STRING -> V_STRING -> V_TREE -> V_TREE */
static struct value *tree_insa_glue(struct info *info, struct value *label,
                                    struct value *path, struct value *tree) {
    return tree_insert_glue(info, label, path, tree, 0);
}

/* Insert before */
/* V_STRING -> V_STRING -> V_TREE -> V_TREE */
static struct value *tree_insb_glue(struct info *info, struct value *label,
                                    struct value *path, struct value *tree) {
    return tree_insert_glue(info, label, path, tree, 1);
}

/* V_STRING -> V_TREE -> V_TREE */
static struct value *tree_rm_glue(struct info *info,
                                  struct value *path,
                                  struct value *tree) {
    // FIXME: This only works if TREE is not referenced more than once;
    // otherwise we'll have some pretty weird semantics, and would really
    // need to copy TREE first
    assert(path->tag == V_STRING);
    assert(tree->tag == V_TREE);

    struct pathx *p = NULL;
    struct value *result = NULL;

    if (pathx_parse(tree->origin, NULL, path->string->str, true, NULL, &p)
        != PATHX_NOERROR) {
        result = make_pathx_exn(ref(info), p);
        goto done;
    }

    if (tree_rm(p) == -1) {
        result = make_exn_value(ref(info), "Tree rm of %s failed",
                                path->string->str);
        goto done;
    }
    result = ref(tree);
 done:
    free_pathx(p);
    return result;
}

/* V_STRING -> V_STRING */
static struct value *gensym(struct info *info, struct value *prefix) {
    assert(prefix->tag == V_STRING);
    static unsigned int count = 0;
    struct value *v;
    char *s;
    int r;

    r = asprintf(&s, "%s%u", prefix->string->str, count);
    if (r == -1)
        return NULL;
    v = make_value(V_STRING, ref(info));
    v->string = make_string(s);
    return v;
}

/* V_STRING -> V_FILTER */
static struct value *xform_incl(struct info *info, struct value *s) {
    assert(s->tag == V_STRING);
    struct value *v = make_value(V_FILTER, ref(info));
    v->filter = make_filter(ref(s->string), 1);
    return v;
}

/* V_STRING -> V_FILTER */
static struct value *xform_excl(struct info *info, struct value *s) {
    assert(s->tag == V_STRING);
    struct value *v = make_value(V_FILTER, ref(info));
    v->filter = make_filter(ref(s->string), 0);
    return v;
}

/* V_LENS -> V_FILTER -> V_TRANSFORM */
static struct value *xform_transform(struct info *info, struct value *l,
                                     struct value *f) {
    assert(l->tag == V_LENS);
    assert(f->tag == V_FILTER);
    if (l->lens->value || l->lens->key) {
        return make_exn_value(ref(info), "Can not build a transform "
                              "from a lens that leaves a %s behind",
                              l->lens->key ? "key" : "value");
    }
    struct value *v = make_value(V_TRANSFORM, ref(info));
    v->transform = make_transform(ref(l->lens), ref(f->filter));
    return v;
}

static struct value *sys_getenv(struct info *info, struct value *n) {
    assert(n->tag == V_STRING);
    struct value *v = make_value(V_STRING, ref(info));
    v->string = dup_string(getenv(n->string->str));
    return v;
}

static struct value *sys_read_file(struct info *info, struct value *n) {
    assert(n->tag == V_STRING);
    char *str = NULL;

    str = read_file(n->string->str);
    if (str == NULL) {
        char error_buf[1024];
        const char *errmsg;
        errmsg = xstrerror(errno, error_buf, sizeof(error_buf));
        struct value *exn = make_exn_value(ref(info),
             "reading file %s failed:", n->string->str);
        exn_printf_line(exn, "%s", errmsg);
        return exn;
    }
    struct value *v = make_value(V_STRING, ref(info));
    v->string = make_string(str);
    return v;
}

struct module *builtin_init(struct error *error) {
    struct module *modl = module_create("Builtin");
    int r;

#define DEFINE_NATIVE(modl, name, nargs, impl, types ...)               \
    r = define_native(error, modl, name, nargs, impl, ##types);         \
    if (r < 0) goto error;

    DEFINE_NATIVE(modl, "gensym", 1, gensym, T_STRING, T_STRING);

    /* Primitive lenses */
    DEFINE_NATIVE(modl, "del",     2, lns_del, T_REGEXP, T_STRING, T_LENS);
    DEFINE_NATIVE(modl, "store",   1, lns_store, T_REGEXP, T_LENS);
    DEFINE_NATIVE(modl, "key",     1, lns_key, T_REGEXP, T_LENS);
    DEFINE_NATIVE(modl, "label",   1, lns_label, T_STRING, T_LENS);
    DEFINE_NATIVE(modl, "seq",     1, lns_seq, T_STRING, T_LENS);
    DEFINE_NATIVE(modl, "counter", 1, lns_counter, T_STRING, T_LENS);
    /* Applying lenses (mostly for tests) */
    DEFINE_NATIVE(modl, "get",     2, lens_get, T_LENS, T_STRING, T_TREE);
    DEFINE_NATIVE(modl, "put",     3, lens_put, T_LENS, T_TREE, T_STRING,
                  T_STRING);
    /* Tree manipulation used by the PUT tests */
    DEFINE_NATIVE(modl, "set", 3, tree_set_glue, T_STRING, T_STRING, T_TREE,
                                                 T_TREE);
    DEFINE_NATIVE(modl, "clear", 2, tree_clear_glue, T_STRING, T_TREE,
                                                 T_TREE);
    DEFINE_NATIVE(modl, "rm", 2, tree_rm_glue, T_STRING, T_TREE, T_TREE);
    DEFINE_NATIVE(modl, "insa", 3, tree_insa_glue, T_STRING, T_STRING, T_TREE,
                                                   T_TREE);
    DEFINE_NATIVE(modl, "insb", 3, tree_insb_glue, T_STRING, T_STRING, T_TREE,
                                                   T_TREE);
    /* Transforms and filters */
    DEFINE_NATIVE(modl, "incl", 1, xform_incl, T_STRING, T_FILTER);
    DEFINE_NATIVE(modl, "excl", 1, xform_excl, T_STRING, T_FILTER);
    DEFINE_NATIVE(modl, "transform", 2, xform_transform, T_LENS, T_FILTER,
                                                         T_TRANSFORM);
    /* System functions */
    struct module *sys = module_create("Sys");
    modl->next = sys;
    DEFINE_NATIVE(sys, "getenv", 1, sys_getenv, T_STRING, T_STRING);
    DEFINE_NATIVE(sys, "read_file", 1, sys_read_file, T_STRING, T_STRING);
    return modl;
 error:
    unref(modl, module);
    return NULL;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
