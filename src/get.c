/*
 * parser.c: parse a configuration file according to a grammar
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

#include <regex.h>
#include <stdarg.h>

#include "syntax.h"
#include "list.h"
#include "internal.h"
#include "memory.h"

#define assert_error(state, format, args ...) \
    assert_error_at(__FILE__, __LINE__, &(state->info), format, ## args)

/* A substring in the overall string we are matching. A lens is supposed
   to process the whole string from start[0] ... start[size]
*/
struct split {
    struct split *next;
    const char   *start;    /* The start of the text to process */
    int           size;
};

struct seq {
    struct seq *next;
    const char *name;
    int value;
};

struct state {
    struct info       info;
    struct split     *split;
    const char       *text;
    const char       *pos;
    struct seq       *seqs;
    char             *key;
    char             *value;     /* GET_STORE leaves a value here */
    struct lns_error *error;
};

static void get_error(struct state *state, struct lens *lens,
                      const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 3, 4);

void free_lns_error(struct lns_error *err) {
    if (err == NULL)
        return;
    free(err->message);
    free(err->path);
    unref(err->lens, lens);
    free(err);
}

static void get_error(struct state *state, struct lens *lens,
                      const char *format, ...)
{
    va_list ap;
    int r;

    if (state->error != NULL)
        return;
    CALLOC(state->error, 1);
    state->error->lens = ref(lens);
    state->error->pos  = state->pos - state->text;
    va_start(ap, format);
    r = vasprintf(&state->error->message, format, ap);
    va_end(ap);
    if (r == -1)
        state->error->message = NULL;
}

static struct skel *make_skel(struct lens *lens) {
    struct skel *skel;
    enum lens_tag tag = lens->tag;
    CALLOC(skel, 1);
    skel->tag = tag;
    skel->lens = lens;
    return skel;
}

void free_skel(struct skel *skel) {
    if (skel == NULL)
        return;
    if (skel->tag == L_CONCAT || skel->tag == L_STAR || skel->tag == L_MAYBE) {
        while (skel->skels != NULL) {
            struct skel *del = skel->skels;
            skel->skels = del->next;
            free_skel(del);
        }
    } else if (skel->tag == L_DEL) {
        free(skel->text);
    }
    free(skel);
}

static struct dict *make_dict(char *key,
                              struct skel *skel, struct dict *subdict) {
    struct dict *dict;
    CALLOC(dict, 1);
    CALLOC(dict->entry, 1);
    dict->key = key;
    dict->entry->skel = skel;
    dict->entry->dict = subdict;
    dict->mark = dict->entry;
    return dict;
}

void free_dict(struct dict *dict) {
    while (dict != NULL) {
        struct dict *next = dict->next;
        while (dict->mark != NULL) {
            struct dict_entry *del = dict->mark;
            dict->mark = del->next;
            free_skel(del->skel);
            free_dict(del->dict);
            free(del);
        }
        free(dict->key);
        free(dict);
        dict = next;
    }
}

static void print_skel(struct skel *skel);
static void print_skel_list(struct skel *skels, const char *beg,
                            const char *sep, const char *end) {
    printf("%s", beg);
    list_for_each(s, skels) {
        print_skel(s);
        if (s->next != NULL)
            printf("%s", sep);
    }
    printf("%s", end);
}

static void print_skel(struct skel *skel) {
    switch(skel->tag) {
    case L_DEL:
        if (skel->text == NULL) {
            printf("<>");
        } else {
            fputc('\'', stdout);
            print_chars(stdout, skel->text, -1);
            fputc('\'', stdout);
        }
        break;
    case L_CONCAT:
        print_skel_list(skel->skels, "", " . ", "");
        break;
    case L_STAR:
        print_skel_list(skel->skels, "(", " ", ")*");
        break;
    case L_MAYBE:
        print_skel_list(skel->skels, "(", " ", ")?");
        break;
    case L_SUBTREE:
        print_skel_list(skel->skels, "[", " ", "]");
        break;
    default:
        printf("??");
        break;
    }
}

// DICT_DUMP is only used for debugging
#ifdef DICT_DUMP
static void print_dict(struct dict *dict, int indent) {
    list_for_each(d, dict) {
        printf("%*s%s:\n", indent, "", d->key);
        list_for_each(e, d->entry) {
            printf("%*s", indent+2, "");
            print_skel(e->skel);
            printf("\n");
            print_dict(e->dict, indent+2);
        }
    }
}
#endif

static void dict_append(struct dict **dict, struct dict *d2) {
    struct dict *d1 = *dict;

    if (d1 == NULL) {
        *dict = d2;
        return;
    }

    struct dict *e2 = d2;
#ifdef DICT_DUMP
    printf("DICT_APPEND\n");
    print_dict(d1, 0);
    printf("AND\n");
    print_dict(d2, 0);
#endif
    while (e2 != NULL) {
        struct dict *e1;
        for (e1=d1; e1 != NULL; e1 = e1->next) {
            if (e1->key == NULL) {
                if (e2->key == NULL)
                    break;
            } else {
                if (e2->key != NULL && STREQ(e1->key, e2->key))
                    break;
            }
        }
        if (e1 == NULL) {
            struct dict *last = e2;
            e2 = e2->next;
            last->next = NULL;
            list_append(d1, last);
        } else {
            struct dict *del = e2;
            list_append(e1->entry, e2->entry);
            e2 = e2->next;
            free(del->key);
            free(del);
        }
    }
#ifdef DICT_DUMP
    printf("YIELDS\n");
    print_dict(d1, 0);
    printf("END\n");
#endif
    *dict = d1;
}

static void get_expected_error(struct state *state, struct lens *l) {
    char *word, *p, *pat;

    word = alloca(11);
    strncpy(word, state->split->start, 10);
    word[10] = '\0';
    for (p = word; *p != '\0' && *p != '\n'; p++);
    *p = '\0';

    pat = escape(l->ctype->pattern->str, -1);
    get_error(state, l, "expected %s at '%s'", pat, word);
    free(pat);
}

/*
 * Splitting of the input text
 */

static char *token_from_split(struct split *split) {
    return strndup(split->start, split->size);
}

static struct split *split_append(struct split **split, struct split *tail,
                                  const char *text, int start, int end) {
    struct split *sp;

    if (ALLOC(sp) < 0)
        return NULL;

    sp->start = text + start;
    sp->size = end - start;
    list_tail_cons(*split, tail, sp);
    return tail;
}

static struct split *next_split(struct state *state) {
    if (state->split != NULL) {
        state->split = state->split->next;
        if (state->split != NULL)
            state->pos = state->split->start + state->split->size;
    }
    return state->split;
}

static struct split *set_split(struct state *state, struct split *split) {
    state->split = split;
    if (split != NULL)
        state->pos = split->start + split->size;
    return split;
}

/* Refine a tree split OUTER according to the L_CONCAT lens LENS */
static struct split *split_concat(struct state *state, struct lens *lens) {
    assert(lens->tag == L_CONCAT);

    int count = 0;
    struct split *outer = state->split;
    struct re_registers regs;
    struct split *split = NULL;
    struct regexp *ctype = lens->ctype;

    if (ctype->re != NULL)
        ctype->re->regs_allocated = REGS_UNALLOCATED;
    count = regexp_match(ctype, outer->start, outer->size, 0, &regs);
    if (count == -2) {
        FIXME("Match failed - produce better error");
        abort();
    } else if (count == -1) {
        char *text = strndup(outer->start, outer->size);
        get_error(state, lens,
                  "Failed to match /%s/ with %s", ctype->pattern->str, text);
        free(text);
        return NULL;
    }

    int reg = 1;
    struct split *tail = NULL;
    for (int i=0; i < lens->nchildren; i++) {
        assert(reg < regs.num_regs);
        assert(regs.start[reg] != -1);
        tail = split_append(&split, tail,
                            outer->start, regs.start[reg], regs.end[reg]);
        if (tail == NULL)
            goto error;
        reg += 1 + regexp_nsub(lens->children[i]->ctype);
    }
    assert(reg < regs.num_regs);
 done:
    free(regs.start);
    free(regs.end);
    return split;
 error:
    list_free(split);
    goto done;
}

static struct split *split_iter(struct lens *lens, struct split *outer) {
    assert(lens->tag == L_STAR);

    int count = 0;
    struct re_registers regs;
    struct split *split = NULL;
    struct regexp *ctype = lens->child->ctype;

    if (ctype->re != NULL)
        ctype->re->regs_allocated = REGS_UNALLOCATED;
    count = regexp_match(ctype, outer->start, outer->size, 0, &regs);
    if (count == -2) {
        FIXME("Match failed - produce better error");
        abort();
    } else if (count == -1) {
        return NULL;
    }

    const int reg = 0;
    int pos = 0;
    struct split *tail = NULL;
    while (pos < outer->size) {
        count = regexp_match(ctype, outer->start, outer->size, pos, &regs);
        if (count == -2) {
            FIXME("Match failed - produce better error");
            abort();
        } else if (count == -1) {
            break;
        }
        tail = split_append(&split, tail,
                            outer->start, regs.start[reg], regs.end[reg]);
        if (tail == NULL)
            goto error;
        pos = regs.end[reg];
    }
 done:
    free(regs.start);
    free(regs.end);
    return split;
 error:
    list_free(split);
    goto done;
}

static int applies(struct lens *lens, struct split *split) {
    int count;
    count = regexp_match(lens->ctype, split->start, split->size, 0, NULL);
    if (count == -2) {
        FIXME("Match failed - produce better error");
        abort();
    }
    return (count == split->size);
}

static struct tree *get_lens(struct lens *lens, struct state *state);
static struct skel *parse_lens(struct lens *lens, struct state *state,
                               struct dict **dict);

static void free_seqs(struct seq *seqs) {
    /* Do not free seq->name; it's not owned by the seq, but by some lens */
    list_free(seqs);
}

static struct seq *find_seq(const char *name, struct state *state) {
    assert(name != NULL);
    struct seq *seq;

    for (seq=state->seqs;
         seq != NULL && STRNEQ(seq->name, name);
         seq = seq->next);

    if (seq == NULL) {
        CALLOC(seq, 1);
        seq->name = name;
        seq->value = 1;
        list_append(state->seqs, seq);
    }

    return seq;
}

static struct tree *get_seq(struct lens *lens, struct state *state) {
    assert(lens->tag == L_SEQ);
    struct seq *seq = find_seq(lens->string->str, state);

    if (asprintf((char **) &(state->key), "%d", seq->value) == -1) {
        // FIXME: We are out of memory .. find a way to report that
        abort();
    }
    seq->value += 1;
    return NULL;
}

static struct skel *parse_seq(struct lens *lens, struct state *state) {
    get_seq(lens, state);
    return make_skel(lens);
}

static struct tree *get_counter(struct lens *lens, struct state *state) {
    assert(lens->tag == L_COUNTER);
    struct seq *seq = find_seq(lens->string->str, state);
    seq->value = 1;
    return NULL;
}

static struct skel *parse_counter(struct lens *lens, struct state *state) {
    get_counter(lens, state);
    return make_skel(lens);
}

static struct tree *get_del(struct lens *lens,
                            ATTRIBUTE_UNUSED struct state *state) {
    assert(lens->tag == L_DEL);

    return NULL;
}

static struct skel *parse_del(struct lens *lens, struct state *state) {
    assert(lens->tag == L_DEL);
    struct skel *skel = NULL;

    skel = make_skel(lens);
    skel->text = token_from_split(state->split);
    return skel;
}

static struct tree *get_store(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STORE);
    struct tree *tree = NULL;

    assert(state->value == NULL);
    if (state->value != NULL) {
        get_error(state, lens, "More than one store in a subtree");
    } else {
        state->value = token_from_split(state->split);
    }
    return tree;
}

static struct skel *parse_store(struct lens *lens,
                                ATTRIBUTE_UNUSED struct state *state) {
    assert(lens->tag == L_STORE);
    return make_skel(lens);
}

static struct tree *get_key(struct lens *lens, struct state *state) {
    assert(lens->tag == L_KEY);
    state->key = token_from_split(state->split);
    return NULL;
}

static struct skel *parse_key(struct lens *lens, struct state *state) {
    get_key(lens, state);
    return make_skel(lens);
}

static struct tree *get_label(struct lens *lens, struct state *state) {
    assert(lens->tag == L_LABEL);
    state->key = strdup(lens->string->str);
    return NULL;
}

static struct skel *parse_label(struct lens *lens, struct state *state) {
    get_label(lens, state);
    return make_skel(lens);
}

static struct tree *get_union(struct lens *lens, struct state *state) {
    assert(lens->tag == L_UNION);
    struct tree *tree = NULL;
    int applied = 0;

    for (int i=0; i < lens->nchildren; i++) {
        struct lens *l = lens->children[i];
        if (applies(l, state->split)) {
            tree = get_lens(l, state);
            applied = 1;
            break;
        }
    }
    if (!applied)
        get_expected_error(state, lens);
    return tree;
}

static struct skel *parse_union(struct lens *lens, struct state *state,
                                struct dict **dict) {
    assert(lens->tag == L_UNION);
    struct skel *skel = NULL;
    int applied = 0;

    for (int i=0; i < lens->nchildren; i++) {
        struct lens *l = lens->children[i];
        if (applies(l, state->split)) {
            skel = parse_lens(l, state, dict);
            applied = 1;
            break;
        }
    }
    if (! applied)
        get_expected_error(state, lens);

    return skel;
}

static struct tree *get_concat(struct lens *lens, struct state *state) {
    assert(lens->tag == L_CONCAT);
    struct tree *tree = NULL;
    struct split *oldsplit = state->split;
    struct split *split = split_concat(state, lens);

    set_split(state, split);
    for (int i=0; i < lens->nchildren; i++) {
        struct tree *t = NULL;
        if (state->split == NULL) {
            get_error(state, lens->children[i],
                      "Not enough components in concat");
            list_free(split);
            free_tree(tree);
            return NULL;
        }

        t = get_lens(lens->children[i], state);
        list_append(tree, t);
        next_split(state);
    }

    set_split(state, oldsplit);
    list_free(split);
    return tree;
}

static struct skel *parse_concat(struct lens *lens, struct state *state,
                                 struct dict **dict) {
    assert(lens->tag == L_CONCAT);
    struct skel *skel = make_skel(lens);
    struct split *oldsplit = state->split;
    struct split *split = split_concat(state, lens);

    set_split(state, split);
    for (int i=0; i < lens->nchildren; i++) {
        struct skel *sk = NULL;
        struct dict *di = NULL;
        if (state->split == NULL) {
            get_error(state, lens->children[i],
                      "Not enough components in concat");
            list_free(split);
            free_skel(skel);
            return NULL;
        }

        sk = parse_lens(lens->children[i], state, &di);
        list_append(skel->skels, sk);
        dict_append(dict, di);
        next_split(state);
    }

    set_split(state, oldsplit);
    list_free(split);
    return skel;
}

static struct tree *get_quant_star(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STAR);
    struct tree *tree = NULL, *tail = NULL;
    struct split *oldsplit = state->split;
    struct split *split = split_iter(lens, state->split);

    set_split(state, split);
    while (state->split != NULL) {
        struct tree *t = NULL;
        t = get_lens(lens->child, state);
        list_tail_cons(tree, tail, t);
        next_split(state);
    }
    if (state->pos != oldsplit->start + oldsplit->size) {
        get_error(state, lens, "Short iteration");
    }
    set_split(state, oldsplit);
    list_free(split);
    return tree;
}

static struct skel *parse_quant_star(struct lens *lens, struct state *state,
                                     struct dict **dict) {
    assert(lens->tag == L_STAR);
    struct split *oldsplit = state->split;
    struct split *split = split_iter(lens, state->split);
    struct skel *skel = make_skel(lens), *tail = NULL;

    *dict = NULL;
    set_split(state, split);
    while (state->split != NULL) {
        struct skel *sk;
        struct dict *di = NULL;
        sk = parse_lens(lens->child, state, &di);
        list_tail_cons(skel->skels, tail, sk);
        dict_append(dict, di);
        next_split(state);
    }
    if (state->pos != oldsplit->start + oldsplit->size) {
        get_error(state, lens, "Short iteration");
    }
    set_split(state, oldsplit);
    list_free(split);
    return skel;
}

static struct tree *get_quant_maybe(struct lens *lens, struct state *state) {
    assert(lens->tag == L_MAYBE);
    struct tree *tree = NULL;

    if (applies(lens->child, state->split)) {
        tree = get_lens(lens->child, state);
    }
    return tree;
}

static struct skel *parse_quant_maybe(struct lens *lens, struct state *state,
                                      struct dict **dict) {
    assert(lens->tag == L_MAYBE);

    struct skel *skel = make_skel(lens);
    if (applies(lens->child, state->split)) {
        struct skel *sk;
        sk = parse_lens(lens->child, state, dict);
        list_append(skel->skels, sk);
    }
    return skel;
}

static struct tree *get_subtree(struct lens *lens, struct state *state) {
    char *key = state->key;
    char *value = state->value;
    struct tree *tree = NULL, *children;

    state->key = NULL;
    state->value = NULL;
    children = get_lens(lens->child, state);

    tree = make_tree(state->key, state->value, children);

    state->key = key;
    state->value = value;
    return tree;
}

static struct skel *parse_subtree(struct lens *lens, struct state *state,
                                  struct dict **dict) {
    char *key = state->key;
    struct skel *skel;
    struct dict *di = NULL;

    state->key = NULL;
    skel = parse_lens(lens->child, state, &di);
    *dict = make_dict(state->key, skel, di);
    state->key = key;
    return make_skel(lens);
}

static struct tree *get_lens(struct lens *lens, struct state *state) {
    struct tree *tree = NULL;

    switch(lens->tag) {
    case L_DEL:
        tree = get_del(lens, state);
        break;
    case L_STORE:
        tree = get_store(lens, state);
        break;
    case L_KEY:
        tree = get_key(lens, state);
        break;
    case L_LABEL:
        tree = get_label(lens, state);
        break;
    case L_SEQ:
        tree = get_seq(lens, state);
        break;
    case L_COUNTER:
        tree = get_counter(lens, state);
        break;
    case L_CONCAT:
        tree = get_concat(lens, state);
        break;
    case L_UNION:
        tree = get_union(lens, state);
        break;
    case L_SUBTREE:
        tree = get_subtree(lens, state);
        break;
    case L_STAR:
        tree = get_quant_star(lens, state);
        break;
    case L_MAYBE:
        tree = get_quant_maybe(lens, state);
        break;
    default:
        assert_error(state, "illegal lens tag %d", lens->tag);
        break;
    }
    return tree;
}

struct tree *lns_get(struct info *info, struct lens *lens, const char *text,
                     struct lns_error **err) {
    struct state state;
    struct split split;
    struct tree *tree = NULL;
    int partial = 1;

    MEMZERO(&state, 1);
    MEMZERO(&split, 1);
    state.info = *info;
    state.info.ref = UINT_MAX;
    state.pos = text;

    split.start = text;
    split.size = strlen(text);

    state.split = &split;
    state.text = text;
    state.pos = text;

    /* We are probably being overly cautious here: if the lens can't process
     * all of TEXT, we should really fail somewhere in one of the sublenses.
     * But to be safe, we check that we can process everythign anyway, then
     * try to process, hoping we'll get a more specific error, and if that
     * fails, we throw our arms in the air and say 'something wnet wrong'
     */
    partial = !applies(lens, &split);

    tree = get_lens(lens, &state);

    free_seqs(state.seqs);
    if (state.key != NULL) {
        get_error(&state, lens, "get left unused key %s", state.key);
        free(state.key);
    }
    if (state.value != NULL) {
        get_error(&state, lens, "get left unused value %s", state.value);
        free(state.value);
    }
    if (partial && state.error == NULL) {
        get_error(&state, lens, "Get did not match entire input");
    }

    if (err != NULL) {
        *err = state.error;
    } else {
        if (state.error != NULL) {
            free_tree(tree);
            tree = NULL;
        }
        free_lns_error(state.error);
    }
    return tree;
}

static struct skel *parse_lens(struct lens *lens, struct state *state,
                               struct dict **dict) {
    struct skel *skel = NULL;

    switch(lens->tag) {
    case L_DEL:
        skel = parse_del(lens, state);
        break;
    case L_STORE:
        skel = parse_store(lens, state);
        break;
    case L_KEY:
        skel = parse_key(lens, state);
        break;
    case L_LABEL:
        skel = parse_label(lens, state);
        break;
    case L_SEQ:
        skel = parse_seq(lens, state);
        break;
    case L_COUNTER:
        skel = parse_counter(lens, state);
        break;
    case L_CONCAT:
        skel = parse_concat(lens, state, dict);
        break;
    case L_UNION:
        skel = parse_union(lens, state, dict);
        break;
    case L_SUBTREE:
        skel = parse_subtree(lens, state, dict);
        break;
    case L_STAR:
        skel = parse_quant_star(lens, state, dict);
        break;
    case L_MAYBE:
        skel = parse_quant_maybe(lens, state, dict);
        break;
    default:
        assert_error(state, "illegal lens tag %d", lens->tag);
        break;
    }
    return skel;
}

struct skel *lns_parse(struct lens *lens, const char *text, struct dict **dict,
                       struct lns_error **err) {
    struct state state;
    struct split split;
    struct skel *skel = NULL;

    MEMZERO(&state, 1);
    MEMZERO(&split, 1);
    state.info.ref = UINT_MAX;
    state.text = text;

    split.start = text;
    split.size = strlen(text);

    state.split = &split;
    state.text = text;
    state.pos = text;

    if (applies(lens, &split)) {
        *dict = NULL;
        skel = parse_lens(lens, &state, dict);

        free_seqs(state.seqs);
        if (state.error != NULL) {
            free_skel(skel);
            skel = NULL;
            free_dict(*dict);
            *dict = NULL;
        }
        if (state.key != NULL) {
            get_error(&state, lens, "parse left unused key %s", state.key);
            free(state.key);
        }
        if (state.value != NULL) {
            get_error(&state, lens, "parse left unused value %s", state.value);
            free(state.value);
        }
    } else {
        // This should never happen during lns_parse
        get_error(&state, lens, "parse can not process entire input");
    }

    if (err != NULL) {
        *err = state.error;
    } else {
        free_lns_error(state.error);
    }
    return skel;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
