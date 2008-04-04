/*
 * builtin.c: builtin primitives
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

#include <stdio.h>
#include <stdarg.h>

#include "syntax.h"

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
    struct value *v = make_value(V_LENS, ref(info));
    v->lens = lns_make_prim(L_DEL, ref(info),
                            ref(rxp->regexp), ref(dflt->string));
    return v;
}

/* V_REGEXP -> V_LENS */
static struct value *lns_store(struct info *info, struct value *rxp) {
    assert(rxp->tag == V_REGEXP);
    struct value *v = make_value(V_LENS, ref(info));
    v->lens = lns_make_prim(L_STORE, ref(info), ref(rxp->regexp), NULL);
    return v;
}

/* V_REGEXP -> V_LENS */
static struct value *lns_key(struct info *info, struct value *rxp) {
    assert(rxp->tag == V_REGEXP);
    struct value *v = make_value(V_LENS, ref(info));
    v->lens = lns_make_prim(L_KEY, ref(info), ref(rxp->regexp), NULL);
    return v;
}

/* V_STRING -> V_LENS */
static struct value *lns_label(struct info *info, struct value *str) {
    assert(str->tag == V_STRING);
    struct value *v = make_value(V_LENS, ref(info));
    v->lens = lns_make_prim(L_LABEL, ref(info), NULL, ref(str->string));
    return v;
}

/* V_STRING -> V_LENS */
static struct value *lns_seq(struct info *info, struct value *str) {
    assert(str->tag == V_STRING);
    struct value *v = make_value(V_LENS, ref(info));
    v->lens = lns_make_prim(L_SEQ, ref(info), NULL, ref(str->string));
    return v;
}

/* V_STRING -> V_LENS */
static struct value *lns_counter(struct info *info, struct value *str) {
    assert(str->tag == V_STRING);
    struct value *v = make_value(V_LENS, ref(info));
    v->lens = lns_make_prim(L_COUNTER, ref(info), NULL, ref(str->string));
    return v;
}

static struct exn *make_exn(struct info *info, const char *message) {
    struct exn *exn;

    CALLOC(exn, 1);
    exn->info = ref(info);
    exn->message = message;
    return exn;
}

static void exn_add_lines(struct exn *exn, int nlines, ...) {
    va_list ap;
    REALLOC(exn->lines, exn->nlines + nlines);
    va_start(ap, nlines);
    for (int i=0; i < nlines; i++) {
        char *line = va_arg(ap, char *);
        exn->lines[exn->nlines + i] = line;
    }
    va_end(ap);
    exn->nlines += nlines;
}

/* V_LENS -> V_STRING -> V_TREE */
static struct value *lens_get(struct info *info, struct value *l,
                              struct value *str) {
    assert(l->tag == V_LENS);
    assert(str->tag == V_STRING);
    struct lns_error *err;
    struct value *v;
    const char *text = str->string->str;

    struct tree *tree = lns_get(info, l->lens, text, NULL, 0, &err);
    if (err == NULL) {
        v = make_value(V_TREE, ref(info));
        v->tree = tree;
    } else {
        struct exn *exn = make_exn(info, err->message);
        char *here;
        asprintf(&here, "Error encountered here (%d characters into string)",
                 err->pos);
        exn_add_lines(exn, 2, here, format_pos(text, err->pos));
        err->message = NULL;
        free_lns_error(err);
        v = make_value(V_EXN, ref(info));
        v->exn = exn;
    }
    return v;
}


/* V_LENS -> V_TREE -> V_STRING -> V_STRING */
static struct value *lens_put(struct info *info, struct value *l,
                              struct value *tree, struct value *str) {
    assert(l->tag == V_LENS);
    assert(tree->tag == V_TREE);
    assert(str->tag == V_STRING);
    
    FILE *stream;
    char *buf;
    size_t size;
    struct value *v;
    struct lns_error *err;

    stream = open_memstream(&buf, &size);
    lns_put(stream, l->lens, tree->tree, str->string->str, &err);
    fclose (stream);

    v = make_value(V_STRING, ref(info));
    v->string = make_string(buf);
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
    if (tree_set(tree->tree, path->string->str, val->string->str) == -1) {
        print_info(stderr, info);
        FIXME("Unexpected error from tree_set");
    }
    return tree;
}

/* V_STRING -> V_TREE -> V_TREE */
static struct value *tree_rm_glue(ATTRIBUTE_UNUSED struct info *info,
                                  struct value *path,
                                  struct value *tree) {
    // FIXME: This only works if TREE is not referenced more than once;
    // otherwise we'll have some pretty weird semantics, and would really
    // need to copy TREE first
    assert(path->tag == V_STRING);
    assert(tree->tag == V_TREE);
    tree_rm(tree->tree, path->string->str);
    return tree;
}

struct module *builtin_init(void) {
    struct module *modl = module_create("Builtin");
    /* Primitive lenses */
    define_native(modl, "del",     2, lns_del, T_REGEXP, T_STRING, T_LENS);
    define_native(modl, "store",   1, lns_store, T_REGEXP, T_LENS);
    define_native(modl, "key",     1, lns_key, T_REGEXP, T_LENS);
    define_native(modl, "label",   1, lns_label, T_STRING, T_LENS);
    define_native(modl, "seq",     1, lns_seq, T_STRING, T_LENS);
    define_native(modl, "counter", 1, lns_counter, T_STRING, T_LENS);
    /* Applying lenses (mostly for tests) */
    define_native(modl, "get",     2, lens_get, T_LENS, T_STRING, T_TREE);
    define_native(modl, "put",     3, lens_put, T_LENS, T_TREE, T_STRING,
                                               T_STRING);
    /* Tree manipulation used by the PUT tests */
    define_native(modl, "set", 3, tree_set_glue, T_STRING, T_STRING, T_TREE, T_TREE);
    define_native(modl, "rm", 2, tree_rm_glue, T_STRING, T_TREE, T_TREE);

    return modl;
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
