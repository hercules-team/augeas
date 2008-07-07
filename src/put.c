/*
 * put.c:
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

#include <stdarg.h>
#include "syntax.h"

//#define DEBUG_SPEW

/* Data structure to keep track of where we are in the tree. The split
 * describes a sublist of the list of siblings in the current tree. The
 * put_* functions don't operate on the tree directly, instead they operate
 * on a split.
 *
 * The TREE field points to the first tree node for the current invocation
 * of put_*, FOLLOW points to the first sibling following TREE that is not
 * part of the split anymore (NULL if we are talking about all the siblings
 * of TREE)
 *
 * LABELS is a string containing all the labels of the siblings joined with
 * '/' as a separator. We are currently looking at a part of that string,
 * namely the END - START characters starting at LABELS + START.
 */
struct split {
    struct split *next;
    struct tree  *tree;
    struct tree  *follow;
    char         *labels;
    size_t        start;
    size_t        end;
};

struct state {
    FILE             *out;
    struct split     *split;
    const char       *key;
    const char       *value;
    struct dict      *dict;
    struct skel      *skel;
    char             *path;   /* Position in the tree, for errors */
    struct lns_error *error;
};

static void create_lens(struct lens *lens, struct state *state);
static void put_lens(struct lens *lens, struct state *state);

#ifdef DEBUG_SPEW
#define _l(obj) (((obj) == NULL) ? "nil" : (obj)->label)

static const char *_t(struct lens *lens) {
    static const char *types[] = {
        "action", "[]", "literal", "name", "|", ".",
        "rule", "abbrev", "+", "*", "?"
    };
    return types[match->type];
}

#define debug(fmt, args ...) printf(fmt, ## args)
#else
#define debug(fmt, args ...)
#endif

static void put_error(struct state *state, struct lens *lens,
                      const char *format, ...)
{
    va_list ap;
    int r;

    if (state->error != NULL)
        return;

    CALLOC(state->error, 1);
    state->error->lens = ref(lens);
    state->error->pos  = -1;
    state->error->path = strdup(state->path);

    va_start(ap, format);
    r = vasprintf(&state->error->message, format, ap);
    va_end(ap);
    if (r == -1)
        state->error->message = NULL;
}

static struct split *make_split(struct tree *tree) {
    struct split *split;
    CALLOC(split, 1);

    split->tree = tree;
    split->start = 0;
    for (struct tree *t = tree; t != NULL; t = t->next) {
        if (t->label != NULL)
            split->end += strlen(t->label);
        split->end += 1;
    }
    char *labels;
    CALLOC(labels, split->end + 1);
    char *l = labels;
    for (struct tree *t = tree; t != NULL; t = t->next) {
        if (t->label != NULL)
            l = stpcpy(l, t->label);
        l = stpcpy(l, "/");
    }
    split->labels = labels;
    return split;
}

static void free_split(struct split *split) {
    if (split == NULL)
        return;

    free(split->labels);
    free(split);
}

static void split_append(struct split **split,
                         struct tree *tree, struct tree *follow,
                         char *labels, size_t start, size_t end) {
    struct split *sp;
    CALLOC(sp, 1);
    sp->tree = tree;
    sp->follow = follow;
    sp->labels = labels;
    sp->start = start;
    sp->end = end;
    list_append(*split, sp);
}

/* Refine a tree split OUTER according to the L_CONCAT lens LENS */
static struct split *split_concat(struct state *state, struct lens *lens) {
    assert(lens->tag == L_CONCAT);

    int count = 0;
    struct split *outer = state->split;
    struct re_registers regs;
    struct split *split = NULL;
    struct regexp *atype = lens->atype;

    if (atype->re != NULL)
        atype->re->regs_allocated = REGS_UNALLOCATED;
    count = regexp_match(atype, outer->labels, outer->end,
                         outer->start, &regs);
    if (count == -2) {
        FIXME("Match failed - produce better error");
        abort();
    } else if (count == -1) {
        char *labels = strndup(outer->labels + outer->start,
                               outer->end - outer->start);
        put_error(state, lens,
                  "Failed to match /%s/ with %s",
                  atype->pattern->str, labels);
        free(labels);
        return NULL;
    }

    struct tree *cur = outer->tree;
    int reg = 1;
    for (int i=0; i < lens->nchildren; i++) {
        assert(reg < regs.num_regs);
        assert(regs.start[reg] != -1);
        struct tree *follow = cur;
        for (int j = regs.start[reg]; j < regs.end[reg]; j++) {
            if (outer->labels[j] == '/')
                follow = follow->next;
        }
        split_append(&split, cur, follow,
                     outer->labels, regs.start[reg], regs.end[reg]);
        cur = follow;
        reg += 1 + regexp_nsub(lens->children[i]->atype);
    }
    assert(reg < regs.num_regs);
    free(regs.start);
    free(regs.end);
    return split;
}

static struct split *split_iter(struct lens *lens, struct split *outer) {
    assert(lens->tag == L_STAR);

    int count = 0;
    struct re_registers regs;
    struct split *split = NULL;
    struct regexp *atype = lens->child->atype;

    if (atype->re != NULL)
        atype->re->regs_allocated = REGS_UNALLOCATED;
    count = regexp_match(atype, outer->labels, outer->end,
                         outer->start, &regs);
    if (count == -2) {
        FIXME("Match failed - produce better error");
        abort();
    } else if (count == -1) {
        return NULL;
    }

    struct tree *cur = outer->tree;
    const int reg = 0;
    int pos = outer->start;
    while (pos < outer->end) {
        count = regexp_match(atype, outer->labels,
                             outer->end, pos, &regs);
        if (count == -2) {
            FIXME("Match failed - produce better error");
            abort();
        } else if (count == -1) {
            break;
        }
        struct tree *follow = cur;
        for (int j = regs.start[reg]; j < regs.end[reg]; j++) {
            if (outer->labels[j] == '/')
                follow = follow->next;
        }
        split_append(&split, cur, follow,
                     outer->labels, regs.start[reg], regs.end[reg]);
        cur = follow;
        pos = regs.end[reg];
    }
    free(regs.start);
    free(regs.end);
    return split;
}

/* Check if LENS applies to the current split in STATE */
static int applies(struct lens *lens, struct split *split) {
    int count;
    count = regexp_match(lens->atype, split->labels, split->end,
                         split->start, NULL);
    if (count == -2) {
        FIXME("Match failed - produce better error");
        abort();
    }
    return (count == split->end - split->start);
}

static struct dict_entry *dict_lookup(const char *key, struct dict *dict) {
    struct dict *d;
    if (key == NULL) {
        for (d=dict; d != NULL && d->key != NULL; d = d->next);
    } else {
        for (d=dict; d != NULL && (d->key == NULL || STRNEQ(key, d->key));
             d = d->next);
    }
    if (d == NULL)
        return NULL;
    struct dict_entry *result = d->entry;
    if (result != NULL)
        d->entry = result->next;
    return result;
}

/* Print TEXT to OUT, translating common escapes like \n */
static void print_escaped_chars(FILE *out, const char *text) {
    for (const char *c = text; *c != '\0'; c++) {
        if (*c == '\\') {
            char x;
            c += 1;
            if (*c == '\0') {
                fputc(*c, out);
                break;
            }
            switch(*c) {
            case 'a':
                x = '\a';
                break;
            case 'b':
                x = '\b';
                break;
            case 'f':
                x = '\f';
                break;
            case 'n':
                x = '\n';
                break;
            case 'r':
                x = '\r';
                break;
            case 't':
                x = '\t';
                break;
            case 'v':
                x = '\v';
                break;
            default:
                x = *c;
                break;
            }
            fputc(x, out);
        } else {
            fputc(*c, out);
        }
    }
}

/*
 * Check whether SKEL has the skeleton type required by LENS
 */

static int skel_instance_of(struct lens *lens, struct skel *skel) {
    if (skel == NULL)
        return 0;

    switch (lens->tag) {
    case L_DEL:
        if (skel->tag != L_DEL)
            return 0;
        return regexp_match(lens->regexp, skel->text, strlen(skel->text),
                            0, NULL);
    case L_STORE:
        if (skel->tag != L_STORE)
            return 0;
        return regexp_match(lens->regexp, skel->text, strlen(skel->text),
                            0, NULL);
    case L_KEY:
        return skel->tag == L_KEY;
    case L_LABEL:
        return skel->tag == L_LABEL;
    case L_SEQ:
        return skel->tag == L_SEQ;
    case L_COUNTER:
        return skel->tag == L_COUNTER;
    case L_CONCAT:
        {
            struct skel *s = skel->skels;
            for (int i=0; i < lens->nchildren; i++) {
                if (! skel_instance_of(lens->children[i], s))
                    return 0;
                s = s->next;
            }
            return 1;
        }
        break;
    case L_UNION:
        {
            for (int i=0; i < lens->nchildren; i++) {
                if (skel_instance_of(lens->children[i], skel))
                    return 1;
            }
            return 0;
        }
        break;
    case L_SUBTREE:
        return skel->tag == L_SUBTREE;
    case L_STAR:
    case L_MAYBE:
        if (skel->tag != lens->tag)
            return 0;
        if (lens->tag == L_MAYBE &&
            skel->skels != NULL && skel->skels->next != NULL)
            return 0;
        list_for_each(s, skel->skels) {
            if (! skel_instance_of(lens->child, s))
                return 0;
        }
        return 1;
    default:
        assert_error_at(__FILE__, __LINE__, lens->info,
                        "illegal lens tag %d", lens->tag);
        break;
    }
    return 0;
}

/*
 * put
 */
static void put_subtree(struct lens *lens, struct state *state) {
    assert(lens->tag == L_SUBTREE);
    struct state oldstate = *state;
    struct split oldsplit = *state->split;
    size_t oldpathlen = strlen(state->path);

    struct tree *tree = state->split->tree;
    struct dict_entry *entry = dict_lookup(tree->label, state->dict);
    struct split *split = NULL;

    state->key = tree->label;
    state->value = tree->value;
    pathjoin(&state->path, 1, state->key);

    state->split = split = make_split(tree->children);

    if (entry == NULL) {
        state->skel = NULL;
        state->dict = NULL;
        create_lens(lens->child, state);
    } else {
        state->skel = entry->skel;
        state->dict = entry->dict;
        put_lens(lens->child, state);
    }
    assert(state->error != NULL || state->split->next == NULL);

    oldstate.error = state->error;
    oldstate.path = state->path;
    *state = oldstate;
    *state->split= oldsplit;
    free_split(split);
    state->path[oldpathlen] = '\0';
}

static void put_del(ATTRIBUTE_UNUSED struct lens *lens, struct state *state) {
    assert(lens->tag == L_DEL);
    assert(state->skel != NULL);
    assert(state->skel->tag == L_DEL);
    fprintf(state->out, "%s", state->skel->text);
}

static void put_union(struct lens *lens, struct state *state) {
    assert(lens->tag == L_UNION);

    for (int i=0; i < lens->nchildren; i++) {
        struct lens *l = lens->children[i];
        if (applies(l, state->split)) {
            if (skel_instance_of(l, state->skel))
                put_lens(l, state);
            else
                create_lens(l, state);
            return;
        }
    }
    assert(0);
}

static void put_concat(struct lens *lens, struct state *state) {
    assert(lens->tag == L_CONCAT);
    assert(state->skel->lens == lens);
    struct split *oldsplit = state->split;
    struct skel *oldskel = state->skel;

    struct split *split = split_concat(state, lens);

    state->skel = state->skel->skels;
    state->split = split;
    for (int i=0; i < lens->nchildren; i++) {
        if (state->split == NULL) {
            put_error(state, lens,
                      "Not enough components in concat");
            list_free(split);
            return;
        }
        put_lens(lens->children[i], state);
        state->split = state->split->next;
        state->skel = state->skel->next;
    }
    list_free(split);
    state->split = oldsplit;
    state->skel = oldskel;
}

static void put_quant_star(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STAR);
    assert(state->skel->lens == lens);
    struct split *oldsplit = state->split;
    struct skel *oldskel = state->skel;

    struct split *split = split_iter(lens, state->split);
    if (split == NULL)
        return;

    state->skel = state->skel->skels;
    state->split = split;
    while (state->split != NULL && state->skel != NULL) {
        put_lens(lens->child, state);
        state->split = state->split->next;
        state->skel = state->skel->next;
    }
    while (state->split != NULL) {
        create_lens(lens->child, state);
        state->split = state->split->next;
    }
    list_free(split);
    state->split = oldsplit;
    state->skel = oldskel;
}

static void put_quant_maybe(struct lens *lens, struct state *state) {
    assert(lens->tag == L_MAYBE);
    assert(state->skel->lens == lens);

    if (applies(lens->child, state->split)) {
        if (skel_instance_of(lens->child, state->skel))
            put_lens(lens->child, state);
        else
            create_lens(lens->child, state);
    }
}

static void put_store(struct lens *lens, struct state *state) {
    if (state->value == NULL) {
        put_error(state, lens,
                  "Can not store a nonexistent (NULL) value");
    } else if (regexp_match(lens->regexp, state->value, strlen(state->value),
                            0, NULL) != strlen(state->value)) {
        put_error(state, lens,
                  "Value '%s' does not match regexp /%s/ in store lens",
                  state->value, lens->regexp->pattern->str);
    } else {
        fprintf(state->out, "%s", state->value);
    }
}

static void put_lens(struct lens *lens, struct state *state) {
    debug("put_lens: %s:%d %s\n", _t(lens), lens->info->first_line,
          _l(state->tree));
    if (state->error != NULL)
        return;

    switch(lens->tag) {
    case L_DEL:
        put_del(lens, state);
        break;
    case L_STORE:
        put_store(lens, state);
        break;
    case L_KEY:
        fprintf(state->out, "%s", state->key);
        break;
    case L_LABEL:
        /* Nothing to do */
        break;
    case L_SEQ:
        /* Nothing to do */
        break;
    case L_COUNTER:
        /* Nothing to do */
        break;
    case L_CONCAT:
        put_concat(lens, state);
        break;
    case L_UNION:
        put_union(lens, state);
        break;
    case L_SUBTREE:
        put_subtree(lens, state);
        break;
    case L_STAR:
        put_quant_star(lens, state);
        break;
    case L_MAYBE:
        put_quant_maybe(lens, state);
        break;
    default:
        assert(0);
        break;
    }
}

static void create_subtree(struct lens *lens, struct state *state) {
    put_subtree(lens, state);
}

static void create_del(struct lens *lens, struct state *state) {
    assert(lens->tag == L_DEL);

    print_escaped_chars(state->out, lens->string->str);
}

static void create_union(struct lens *lens, struct state *state) {
    assert(lens->tag == L_UNION);

    for (int i=0; i < lens->nchildren; i++) {
        if (applies(lens->children[i], state->split)) {
            create_lens(lens->children[i], state);
            return;
        }
    }
    assert(0);
}

static void create_concat(struct lens *lens, struct state *state) {
    assert(lens->tag == L_CONCAT);
    struct split *oldsplit = state->split;

    struct split *split = split_concat(state, lens);

    state->split = split;
    for (int i=0; i < lens->nchildren; i++) {
        if (state->split == NULL) {
            syntax_error(lens->info, "Short split for concat");
            list_free(split);
            return;
        }
        create_lens(lens->children[i], state);
        state->split = state->split->next;
    }
    list_free(split);
    state->split = oldsplit;
}

static void create_quant_star(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STAR);
    struct split *oldsplit = state->split;

    struct split *split = split_iter(lens, state->split);
    if (split == NULL)
        return;

    state->split = split;
    while (state->split != NULL) {
        create_lens(lens->child, state);
        state->split = state->split->next;
    }
    list_free(split);
    state->split = oldsplit;
}

static void create_quant_maybe(struct lens *lens, struct state *state) {
    assert(lens->tag == L_MAYBE);

    if (applies(lens->child, state->split)) {
        create_lens(lens->child, state);
    }
}

static void create_lens(struct lens *lens, struct state *state) {
    debug("create_lens: %s:%d %s\n", _t(lens), lens->info->first_line,
          _l(state->tree));
    if (state->error != NULL)
        return;
    switch(lens->tag) {
    case L_DEL:
        create_del(lens, state);
        break;
    case L_STORE:
        put_store(lens, state);
        break;
    case L_KEY:
        fprintf(state->out, "%s", state->key);
        break;
    case L_LABEL:
        /* Nothing to do */
        break;
    case L_SEQ:
        /* Nothing to do */
        break;
    case L_COUNTER:
        /* Nothing to do */
        break;
    case L_CONCAT:
        create_concat(lens, state);
        break;
    case L_UNION:
        create_union(lens, state);
        break;
    case L_SUBTREE:
        create_subtree(lens, state);
        break;
    case L_STAR:
        create_quant_star(lens, state);
        break;
    case L_MAYBE:
        create_quant_maybe(lens, state);
        break;
    default:
        assert(0);
        break;
    }
}

void lns_put(FILE *out, struct lens *lens, struct tree *tree,
             const char *text, struct lns_error **err) {
    struct state state;
    struct lns_error *err1;

    if (err != NULL)
        *err = NULL;
    if (tree == NULL)
        return;

    MEMZERO(&state, 1);
    state.path = strdup("");
    state.skel = lns_parse(lens, text, &state.dict, &err1);

    if (err1 != NULL) {
        if (err != NULL)
            *err = err1;
        else
            free_lns_error(err1);
        return;
    }
    state.out = out;
    state.split = make_split(tree);
    state.key = tree->label;
    put_lens(lens, &state);

    free(state.path);
    free_split(state.split);
    free_skel(state.skel);
    free_dict(state.dict);
    if (err != NULL) {
        *err = state.error;
    } else {
        free_lns_error(state.error);
    }
}

/*
  Cleverly created with Elisp:
(defun my-ins-put ()
  (interactive)
  (insert "// BEGIN\n")
  (let
      ((types '("ACTION" "SUBTREE" "LITERAL" "NAME" "ALTERNATIVE"
                "SEQUENCE" "RULE_REF" "ABBREV_REF"
                "QUANT_PLUS" "QUANT_STAR" "QUANT_MAYBE")))
    (mapcar
     (lambda (type)
       (insert "static void put_" (downcase type) "(struct match *match, struct state *state) {
    assert(match->type == " type ");
}

"))
          types)
    (mapcar
     (lambda (type)
       (insert "static void create_" (downcase type) "(struct match *match, struct state *state) {
    assert(match->type == " type ");

}

"))
          types)
  (insert "// END\n")))
 */

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
