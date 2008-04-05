/*
 * parser.c: parse a configuration file according to a grammar
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

#include <regex.h>
#include <stdarg.h>

#include "syntax.h"
#include "list.h"
#include "internal.h"

#define assert_error(state, format, args ...) \
    assert_error_at(__FILE__, __LINE__, &(state->info), format, ## args)

struct seq {
    struct seq *next;
    const char *name;
    int value;
};

struct state {
    struct info       info;
    const char       *text;
    const char       *pos;
    int               applied;
    int               flags;     /* set of parse_flags */
    FILE             *log;
    struct seq       *seqs;
    const char       *key;
    int               leaf : 1;  /* Used by get_subtree */
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

    if (state->error != NULL)
        return;
    CALLOC(state->error, 1);
    state->error->lens = ref(lens);
    state->error->pos  = state->pos - state->text;
    va_start(ap, format);
    vasprintf(&state->error->message, format, ap);
    va_end(ap);
}

static struct tree *make_tree(const char *label, const char *value) {
    struct tree *tree;
    CALLOC(tree, 1);
    tree->label = label;
    tree->value = value;
    return tree;
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
        free((char *) skel->text);
    }
    free(skel);
}

static struct dict *make_dict(const char *key,
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
        free((char *) dict->key);
        free(dict);
        dict = next;
    }
}

static void print_skel(struct skel *skel);
static void print_skel_list(struct skel *skels, const char *beg,
                            const char *sep, const char *end) {
    printf(beg);
    list_for_each(s, skels) {
        print_skel(s);
        if (s->next != NULL)
            printf(sep);
    }
    printf(end);
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
    strncpy(word, state->pos, 10);
    word[10] = '\0';
    for (p = word; *p != '\0' && *p != '\n'; p++);
    *p = '\0';

    pat = escape(l->ctype->pattern->str, -1);
    get_error(state, l, "expected %s at '%s'", pat, word);
    free(pat);
}

/*
 * Parsing/construction of the AST
 */
static void advance(struct state *state, int cnt) {
    if (cnt == 0)
        return;

    for (int i=0; i<cnt; i++) {
        assert(state->pos != '\0');
        if (state->pos[i] == '\n')
            state->info.first_line++;
        state->info.last_line = state->info.first_line;
    }
    state->pos += cnt;
    if (state->flags & PF_ADVANCE) {
        fprintf(state->log, "A %3d ", cnt);
        print_pos(state->log, state->text, state->pos - state->text);
    }
}

static int lex(struct lens *lens, struct regexp *regexp, struct state *state) {
    int count;
    int offset = state->pos - state->text;
    count = regexp_match(regexp, state->text, strlen(state->text),
                         offset, NULL);
    if (state->flags & PF_MATCH) {
        fprintf(state->log, "M %d ", offset);
        print_regexp(state->log, regexp);
        fprintf(state->log, " %d..%d\n", offset, offset+count);
    }

    if (count == -2) {
        get_error(state, lens, "Match failed for /%s/ at %d",
                  regexp->pattern->str, offset);
        return -1;
    } else if (count == -1) {
        return 0;
    } else {
        return count;
    }
}

static int match(struct lens *lens, struct state *state,
                 const char **token) {
    assert(lens->tag == L_DEL || lens->tag == L_STORE || lens->tag == L_KEY);

    int len = lex(lens, lens->regexp, state);
    if (len < 0)
        return -1;
    if (token != NULL)
        *token = strndup(state->pos, len);

    if ((state->log != NULL) && (state->flags & PF_TOKEN)) {
        fprintf(state->log, "T ");
        print_regexp(state->log, lens->regexp);
        fprintf(state->log, " = <");
        print_chars(state->log, state->pos, len);
        fprintf(state->log, ">\n");
    }

    advance(state, len);
    return len;
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
        list_append(state->seqs, seq);
    }

    return seq;
}

static struct tree *get_seq(struct lens *lens, struct state *state) {
    assert(lens->tag == L_SEQ);
    struct seq *seq = find_seq(lens->string->str, state);

    asprintf((char **) &(state->key), "%d", seq->value);
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
    seq->value = 0;
    return NULL;
}

static struct skel *parse_counter(struct lens *lens, struct state *state) {
    get_counter(lens, state);
    return make_skel(lens);
}

static struct tree *get_del(struct lens *lens, struct state *state) {
    assert(lens->tag == L_DEL);

    state->applied = match(lens, state, NULL) >= 0;
    return NULL;
}

static struct skel *parse_del(struct lens *lens, struct state *state) {
    assert(lens->tag == L_DEL);
    const char *token = NULL;
    struct skel *skel = NULL;

    state->applied = match(lens, state, &token) >= 0;
    if (state->applied) {
        skel = make_skel(lens);
        skel->text = token;
    }
    return skel;
}

static struct tree *get_store(struct lens *lens, struct state *state) {
    assert(lens->tag = L_STORE);
    const char *token = NULL;
    struct tree *tree = NULL;

    if (match(lens, state, &token) < 0)
        get_expected_error(state, lens);
    else
        tree = make_tree(NULL, token);
    return tree;
}

static struct skel *parse_store(struct lens *lens, struct state *state) {
    assert(lens->tag = L_STORE);
    if (match(lens, state, NULL) < 0)
        get_expected_error(state, lens);
    return make_skel(lens);
}

static struct tree *get_key(struct lens *lens, struct state *state) {
    assert(lens->tag = L_KEY);
    const char *token = NULL;
    if (match(lens, state, &token) < 0)
        get_expected_error(state, lens);
    else
        state->key = token;
    return NULL;
}

static struct skel *parse_key(struct lens *lens, struct state *state) {
    get_key(lens, state);
    return make_skel(lens);
}

static struct tree *get_label(struct lens *lens, struct state *state) {
    assert(lens->tag = L_LABEL);
    state->key = strdup(lens->string->str);
    return NULL;
}

static struct skel *parse_label(struct lens *lens, struct state *state) {
    get_label(lens, state);
    return make_skel(lens);
}

static int applies(struct lens *lens, struct state *state) {
    return lex(lens, lens->ctype, state) > 0;
}

static struct tree *get_union(struct lens *lens, struct state *state) {
    assert(lens->tag == L_UNION);
    struct tree *tree = NULL;

    state->applied = 0;
    for (int i=0; i < lens->nchildren; i++) {
        struct lens *l = lens->children[i];
        if (applies(l, state)) {
            tree = get_lens(l, state);
            state->applied = 1;
            break;
        }
    }
    if (! state->applied)
        get_expected_error(state, lens);
    return tree;
}

static struct skel *parse_union(struct lens *lens, struct state *state,
                                struct dict **dict) {
    assert(lens->tag == L_UNION);
    struct skel *skel = NULL;

    state->applied = 0;
    for (int i=0; i < lens->nchildren; i++) {
        struct lens *l = lens->children[i];
        if (applies(l, state)) {
            skel = parse_lens(l, state, dict);
            state->applied = 1;
            break;
        }
    }
    if (! state->applied)
        get_expected_error(state, lens);

    return skel;
}

static struct tree *get_concat(struct lens *lens, struct state *state) {
    assert(lens->tag == L_CONCAT);

    struct tree *tree = NULL;

    state->applied = 1;
    for (int i=0; i < lens->nchildren; i++) {
        struct tree *t = NULL;
        t = get_lens(lens->children[i], state);
        if (! state->applied) {
            get_expected_error(state, lens->children[i]);
            break;
        }
        list_append(tree, t);
    }

    return tree;
}

static struct skel *parse_concat(struct lens *lens, struct state *state,
                                 struct dict **dict) {
    assert(lens->tag == L_CONCAT);
    struct skel *skel = make_skel(lens);

    state->applied = 1;
    for (int i=0; i < lens->nchildren; i++) {
        struct skel *sk = NULL;
        struct dict *di = NULL;

        sk = parse_lens(lens->children[i], state, &di);
        if (! state->applied) {
            get_expected_error(state, lens->children[i]);
            break;
        }
        list_append(skel->skels, sk);
        dict_append(dict, di);
    }
    return skel;
}

static struct tree *get_quant_star(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STAR);

    struct tree *tree = NULL;
    while (applies(lens->child, state)) {
        struct tree *t = NULL;
        t = get_lens(lens->child, state);
        list_append(tree, t);
    }
    state->applied = 1;
    return tree;
}

static struct skel *parse_quant_star(struct lens *lens, struct state *state, 
                                     struct dict **dict) {
    assert(lens->tag == L_STAR);

    struct skel *skel = make_skel(lens);
    *dict = NULL;
    while (applies(lens->child, state)) {
        struct skel *sk;
        struct dict *di = NULL;
        sk = parse_lens(lens->child, state, &di);
        list_append(skel->skels, sk);
        dict_append(dict, di);
    }
    state->applied = 1;

    return skel;
}

static struct tree *get_quant_maybe(struct lens *lens, struct state *state) {
    assert(lens->tag == L_MAYBE);
    struct tree *tree = NULL;

    if (applies(lens->child, state)) {
        tree = get_lens(lens->child, state);
    }
    state->applied = 1;
    return tree;
}

static struct skel *parse_quant_maybe(struct lens *lens, struct state *state,
                                      struct dict **dict) {
    assert(lens->tag == L_MAYBE);

    struct skel *skel = make_skel(lens);
    if (applies(lens->child, state)) {
        struct skel *sk;
        sk = parse_lens(lens->child, state, dict);
        list_append(skel->skels, sk);
    }
    state->applied = 1;
    return skel;
}

static struct tree *get_subtree(struct lens *lens, struct state *state) {
    const char *key = state->key;
    struct tree *tree = NULL;

    state->key = NULL;
    state->leaf = 1;
    tree = get_lens(lens->child, state);
    if (tree == NULL) {
        tree = make_tree(NULL, NULL);
    }
    if (state->leaf) {
        tree->label = state->key;
    } else {
        struct tree *t = make_tree(state->key, NULL);
        t->children = tree;
        tree = t;
    }
    state->key = key;
    state->leaf = 0;
    return tree;
}

static struct skel *parse_subtree(struct lens *lens, struct state *state,
                                  struct dict **dict) {
    const char *key = state->key;
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
                     FILE *log, int flags, struct lns_error **err) {
    struct state state;
    struct tree *tree;

    MEMZERO(&state, 1);
    state.info = *info;
    state.info.ref = UINT_MAX;

    state.text = text;
    state.pos = text;
    if (flags != PF_NONE && log != NULL) {
        state.flags = flags;
        state.log = log;
    } else {
        state.flags = PF_NONE;
        state.log = stdout;
    }

    tree = get_lens(lens, &state);

    free_seqs(state.seqs);
    if (! state.applied || *state.pos != '\0') {
        get_error(&state, lens, "get did not process entire input");
    }
    if (state.error != NULL) {
        free_tree(tree);
        tree = NULL;
    }
    if (err != NULL) {
        *err = state.error;
    } else {
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
    struct skel *skel;

    MEMZERO(&state, 1);
    state.info.ref = UINT_MAX;
    state.text = text;
    state.pos = text;
    state.flags = PF_NONE;
    state.leaf  = 1;

    *dict = NULL;
    skel = parse_lens(lens, &state, dict);

    free_seqs(state.seqs);
    if (! state.applied || *state.pos != '\0') {
        // This should never happen during lns_parse
        get_error(&state, lens, "parse did not process entire input");
    }
    if (state.error != NULL) {
        free_skel(skel);
        skel = NULL;
        free_dict(*dict);
        *dict = NULL;
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
