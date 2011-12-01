/*
 * put.c:
 *
 * Copyright (C) 2007-2011 David Lutterkort
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
#include "regexp.h"
#include "memory.h"
#include "lens.h"
#include "errcode.h"

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
 * ENC is a string containing the encoding of the current position in the
 * tree.  The encoding is
 *   <label>=<value>/<label>=<value>/.../<label>=<value>/
 * where the label/value pairs come from TREE and its
 * siblings. The encoding uses ENC_EQ instead of the '=' above to avoid
 * clashes with legitimate values, and encodes NULL values as ENC_NULL.
 */
struct split {
    struct split *next;
    struct tree  *tree;
    struct tree  *follow;
    char         *enc;
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
    size_t            pos;
    struct lns_error *error;
};

static void create_lens(struct lens *lens, struct state *state);
static void put_lens(struct lens *lens, struct state *state);

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
    if (strlen(state->path) == 0) {
        state->error->path = strdup("");
    } else {
        state->error->path = strdup(state->path);
    }

    va_start(ap, format);
    r = vasprintf(&state->error->message, format, ap);
    va_end(ap);
    if (r == -1)
        state->error->message = NULL;
}

ATTRIBUTE_PURE
static int enclen(const char *key, const char *value) {
    return ENCLEN(key) + strlen(ENC_EQ) + ENCLEN(value)
        + strlen(ENC_SLASH);
}

static char *encpcpy(char *e, const char *key, const char *value) {
    e = stpcpy(e, ENCSTR(key));
    e = stpcpy(e, ENC_EQ);
    e = stpcpy(e, ENCSTR(value));
    e = stpcpy(e, ENC_SLASH);
    return e;
}

static void regexp_match_error(struct state *state, struct lens *lens,
                               int count, struct split *split) {
    // FIXME: Split the regexp and encoding back
    // into something resembling a tree level
    char *text = NULL;
    char *pat = NULL;

    lns_format_atype(lens, &pat);
    text = enc_format(split->enc + split->start, split->end - split->start);

    if (count == -1) {
        put_error(state, lens,
                  "Failed to match \n    %s\n  with tree\n   %s",
                  pat, text);
    } else if (count == -2) {
        put_error(state, lens,
                  "Internal error matching\n    %s\n  with tree\n   %s",
                  pat, text);
    } else if (count == -3) {
        /* Should have been caught by the typechecker */
        put_error(state, lens, "Syntax error in tree schema\n    %s", pat);
    }
    free(pat);
    free(text);
}

static void free_split(struct split *split) {
    if (split == NULL)
        return;

    free(split->enc);
    free(split);
}

/* Encode the list of TREE's children as a string.
 */
static struct split *make_split(struct tree *tree) {
    struct split *split;

    if (ALLOC(split) < 0)
        return NULL;

    split->tree = tree;
    list_for_each(t, tree) {
        split->end += enclen(t->label, t->value);
    }

    if (ALLOC_N(split->enc, split->end + 1) < 0)
        goto error;

    char *enc = split->enc;
    list_for_each(t, tree) {
        enc = encpcpy(enc, t->label, t->value);
    }
    return split;
 error:
    free_split(split);
    return NULL;
}

static struct split *split_append(struct split **split, struct split *tail,
                                  struct tree *tree, struct tree *follow,
                                  char *enc, size_t start, size_t end) {
    struct split *sp;
    CALLOC(sp, 1);
    sp->tree = tree;
    sp->follow = follow;
    sp->enc = enc;
    sp->start = start;
    sp->end = end;
    list_tail_cons(*split, tail, sp);
    return tail;
}

static struct split *next_split(struct state *state) {
    if (state->split != NULL) {
        state->split = state->split->next;
        if (state->split != NULL)
            state->pos = state->split->end;
    }
    return state->split;
}

static struct split *set_split(struct state *state, struct split *split) {
    state->split = split;
    if (split != NULL)
        state->pos = split->end;
    return split;
}

/* Refine a tree split OUTER according to the L_CONCAT lens LENS */
static struct split *split_concat(struct state *state, struct lens *lens) {
    assert(lens->tag == L_CONCAT);

    int count = 0;
    struct split *outer = state->split;
    struct re_registers regs;
    struct split *split = NULL, *tail = NULL;
    struct regexp *atype = lens->atype;

    /* Fast path for leaf nodes, which will always lead to an empty split */
    // FIXME: This doesn't match the empty encoding
    if (outer->tree == NULL && strlen(outer->enc) == 0
        && regexp_is_empty_pattern(atype)) {
        for (int i=0; i < lens->nchildren; i++) {
            tail = split_append(&split, tail, NULL, NULL,
                                outer->enc, 0, 0);
        }
        return split;
    }

    MEMZERO(&regs, 1);
    count = regexp_match(atype, outer->enc, outer->end,
                         outer->start, &regs);
    if (count >= 0 && count != outer->end - outer->start)
        count = -1;
    if (count < 0) {
        regexp_match_error(state, lens, count, outer);
        goto error;
    }

    struct tree *cur = outer->tree;
    int reg = 1;
    for (int i=0; i < lens->nchildren; i++) {
        assert(reg < regs.num_regs);
        assert(regs.start[reg] != -1);
        struct tree *follow = cur;
        for (int j = regs.start[reg]; j < regs.end[reg]; j++) {
            if (outer->enc[j] == ENC_SLASH_CH)
                follow = follow->next;
        }
        tail = split_append(&split, tail, cur, follow,
                            outer->enc, regs.start[reg], regs.end[reg]);
        cur = follow;
        reg += 1 + regexp_nsub(lens->children[i]->atype);
    }
    assert(reg < regs.num_regs);
 done:
    free(regs.start);
    free(regs.end);
    return split;
 error:
    free_split(split);
    split = NULL;
    goto done;
}

static struct split *split_iter(struct state *state, struct lens *lens) {
    assert(lens->tag == L_STAR);

    int count = 0;
    struct split *outer = state->split;
    struct split *split = NULL;
    struct regexp *atype = lens->child->atype;

    struct tree *cur = outer->tree;
    int pos = outer->start;
    struct split *tail = NULL;
    while (pos < outer->end) {
        count = regexp_match(atype, outer->enc, outer->end, pos, NULL);
        if (count == -1) {
            break;
        } else if (count < -1) {
            regexp_match_error(state, lens->child, count, outer);
            goto error;
        }

        struct tree *follow = cur;
        for (int j = pos; j < pos + count; j++) {
            if (outer->enc[j] == ENC_SLASH_CH)
                follow = follow->next;
        }
        tail = split_append(&split, tail, cur, follow,
                            outer->enc, pos, pos + count);
        cur = follow;
        pos += count;
    }
    return split;
 error:
    free_split(split);
    return NULL;
}

/* Check if LENS applies to the current split in STATE */
static int applies(struct lens *lens, struct state *state) {
    int count;
    struct split *split = state->split;

    count = regexp_match(lens->atype, split->enc, split->end,
                         split->start, NULL);
    if (count < -1) {
        regexp_match_error(state, lens, count, split);
        return 0;
    }

    if (count != split->end - split->start)
        return 0;
    if (count == 0 && lens->value)
        return state->value != NULL;
    return 1;
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
    case L_DEL: {
        int count;
        if (skel->tag != L_DEL)
            return 0;
        count = regexp_match(lens->regexp, skel->text, strlen(skel->text),
                           0, NULL);
        return count == strlen(skel->text);
    }
    case L_STORE:
        return skel->tag == L_STORE;
    case L_KEY:
        return skel->tag == L_KEY;
    case L_LABEL:
        return skel->tag == L_LABEL;
    case L_VALUE:
        return skel->tag == L_VALUE;
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
    case L_MAYBE:
        return skel->tag == L_MAYBE || skel_instance_of(lens->child, skel);
    case L_STAR:
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
    case L_REC:
        return skel_instance_of(lens->body, skel);
    case L_SQUARE:
        return skel->tag == L_SQUARE;
    default:
        BUG_ON(true, lens->info, "illegal lens tag %d", lens->tag);
        break;
    }
 error:
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
    struct split *split = NULL;

    state->key = tree->label;
    state->value = tree->value;
    pathjoin(&state->path, 1, state->key);

    split = make_split(tree->children);
    set_split(state, split);

    dict_lookup(tree->label, state->dict, &state->skel, &state->dict);
    if (state->skel == NULL || ! skel_instance_of(lens->child, state->skel)) {
        create_lens(lens->child, state);
    } else {
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
    if (lens->string != NULL) {
    fprintf(state->out, "%s", state->skel->text);
    } else {
    /* L_DEL with NULL string: replicate the current key */
        fprintf(state->out, "%s", state->key);
    }
}

static void put_union(struct lens *lens, struct state *state) {
    assert(lens->tag == L_UNION);

    for (int i=0; i < lens->nchildren; i++) {
        struct lens *l = lens->children[i];
        if (applies(l, state)) {
            if (skel_instance_of(l, state->skel))
                put_lens(l, state);
            else
                create_lens(l, state);
            return;
        }
    }
    put_error(state, lens, "None of the alternatives in the union match");
}

static void put_concat(struct lens *lens, struct state *state) {
    assert(lens->tag == L_CONCAT);
    struct split *oldsplit = state->split;
    struct skel *oldskel = state->skel;

    struct split *split = split_concat(state, lens);

    state->skel = state->skel->skels;
    set_split(state, split);
    for (int i=0; i < lens->nchildren; i++) {
        if (state->split == NULL) {
            put_error(state, lens,
                      "Not enough components in concat");
            list_free(split);
            return;
        }
        put_lens(lens->children[i], state);
        state->skel = state->skel->next;
        next_split(state);
    }
    list_free(split);
    set_split(state, oldsplit);
    state->skel = oldskel;
}

static void error_quant_star(struct split *last_split, struct lens *lens,
                             struct state *state) {
    struct tree *child = NULL;
    if (last_split != NULL) {
        if (last_split->follow != NULL) {
            child = last_split->follow;
        } else {
            for (child = last_split->tree;
                 child != NULL && child->next != NULL;
                 child = child->next);
        }
    }
    if (child == NULL) {
        put_error(state, lens, "Malformed child node");
    } else {
        put_error(state, lens, "Malformed child node '%s'", child->label);
    }
}

static void put_quant_star(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STAR);
    struct split *oldsplit = state->split;
    struct skel *oldskel = state->skel;
    struct split *last_split = NULL;

    struct split *split = split_iter(state, lens);

    state->skel = state->skel->skels;
    set_split(state, split);
    last_split = state->split;
    while (state->split != NULL && state->skel != NULL) {
        put_lens(lens->child, state);
        state->skel = state->skel->next;
        last_split = state->split;
        next_split(state);
    }
    while (state->split != NULL) {
        create_lens(lens->child, state);
        last_split = state->split;
        next_split(state);
    }
    if (state->pos != oldsplit->end)
        error_quant_star(last_split, lens, state);
    list_free(split);
    set_split(state, oldsplit);
    state->skel = oldskel;
}

static void put_quant_maybe(struct lens *lens, struct state *state) {
    assert(lens->tag == L_MAYBE);
    struct lens *child = lens->child;

    if (applies(child, state)) {
        if (skel_instance_of(child, state->skel))
            put_lens(child, state);
        else
            create_lens(child, state);
    }
}

static void put_store(struct lens *lens, struct state *state) {
    if (state->value == NULL) {
        put_error(state, lens,
                  "Can not store a nonexistent (NULL) value");
    } else if (regexp_match(lens->regexp, state->value, strlen(state->value),
                            0, NULL) != strlen(state->value)) {
        char *pat = regexp_escape(lens->regexp);
        put_error(state, lens,
                  "Value '%s' does not match regexp /%s/ in store lens",
                  state->value, pat);
        free(pat);
    } else {
        fprintf(state->out, "%s", state->value);
    }
}

static void put_rec(struct lens *lens, struct state *state) {
    put_lens(lens->body, state);
}

static void put_square(struct lens *lens, struct state *state) {
    struct skel *oldskel = state->skel;
    state->skel = state->skel->skels;
    put_lens(lens->child, state);
    state->skel = oldskel;
}

static void put_lens(struct lens *lens, struct state *state) {
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
    case L_VALUE:
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
    case L_REC:
        put_rec(lens, state);
        break;
    case L_SQUARE:
        put_square(lens, state);
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
    if (lens->string != NULL) {
        print_escaped_chars(state->out, lens->string->str);
    } else {
        /* L_DEL with NULL string: replicate the current key */
        print_escaped_chars(state->out, state->key);
    }

}

static void create_union(struct lens *lens, struct state *state) {
    assert(lens->tag == L_UNION);

    for (int i=0; i < lens->nchildren; i++) {
        if (applies(lens->children[i], state)) {
            create_lens(lens->children[i], state);
            return;
        }
    }
    put_error(state, lens, "None of the alternatives in the union match");
}

static void create_concat(struct lens *lens, struct state *state) {
    assert(lens->tag == L_CONCAT);
    struct split *oldsplit = state->split;

    struct split *split = split_concat(state, lens);

    set_split(state, split);
    for (int i=0; i < lens->nchildren; i++) {
        if (state->split == NULL) {
            put_error(state, lens,
                      "Not enough components in concat");
            list_free(split);
            return;
        }
        create_lens(lens->children[i], state);
        next_split(state);
    }
    list_free(split);
    set_split(state, oldsplit);
}

static void create_quant_star(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STAR);
    struct split *oldsplit = state->split;
    struct split *last_split = NULL;

    struct split *split = split_iter(state, lens);

    set_split(state, split);
    last_split = state->split;
    while (state->split != NULL) {
        create_lens(lens->child, state);
        last_split = state->split;
        next_split(state);
    }
    if (state->pos != oldsplit->end)
        error_quant_star(last_split, lens, state);
    list_free(split);
    set_split(state, oldsplit);
}

static void create_quant_maybe(struct lens *lens, struct state *state) {
    assert(lens->tag == L_MAYBE);

    if (applies(lens->child, state)) {
        create_lens(lens->child, state);
    }
}

static void create_rec(struct lens *lens, struct state *state) {
    create_lens(lens->body, state);
}

static void create_lens(struct lens *lens, struct state *state) {
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
    case L_VALUE:
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
    case L_REC:
        create_rec(lens, state);
        break;
    case L_SQUARE:
        create_concat(lens->child, state);
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
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
