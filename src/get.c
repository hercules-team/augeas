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

#include "regexp.h"
#include "list.h"
#include "internal.h"
#include "memory.h"
#include "info.h"
#include "lens.h"

/* Our favorite error message */
static const char *const short_iteration =
    "Iterated lens matched less than it should";

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
    struct seq       *seqs;
    char             *key;
    char             *value;     /* GET_STORE leaves a value here */
    struct lns_error *error;
    /* We use the registers from a regular expression match to keep track
     * of the substring we are currently looking at. REGS are the registers
     * from the last regexp match; NREG is the number of the register
     * in REGS that describes the substring we are currently looking at.
     *
     * We adjust NREG as we traverse lenses either because we know the
     * grouping structure of lenses (for L_STAR, the child lens is always
     * in NREG + 1) or by looking at the number of groups in a sublens (as
     * we move from one child of L_CONCAT to the next, we need to add 1 +
     * number of groups of that child to NREG) How NREG is adjusted is
     * closely related to how the REGEXP_* routines build up bigger regexps
     * from smaller ones.
     */
    struct re_registers *regs;
    uint                 nreg;
};

#define REG_START(state) ((state)->regs->start[(state)->nreg])
#define REG_END(state)   ((state)->regs->end[(state)->nreg])
#define REG_SIZE(state) (REG_END(state) - REG_START(state))
#define REG_POS(state) ((state)->text + REG_START(state))
#define REG_VALID(state) ((state)->nreg < (state)->regs->num_regs)
#define REG_MATCHED(state) (REG_VALID(state)                            \
                            && (state)->regs->start[(state)->nreg] >= 0)

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
    if (REG_MATCHED(state))
        state->error->pos  = REG_END(state);
    else
        state->error->pos = 0;
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

static void get_expected_error(struct state *state, struct lens *l) {
    /* Size of the excerpt of the input text we'll show */
    static const int wordlen = 10;
    char word[wordlen+1];
    char *p, *pat;

    if (REG_MATCHED(state))
        strncpy(word, REG_POS(state), wordlen);
    else
        strncpy(word, state->text, wordlen);
    word[wordlen] = '\0';
    for (p = word; *p != '\0' && *p != '\n'; p++);
    *p = '\0';

    pat = escape(l->ctype->pattern->str, -1);
    get_error(state, l, "expected %s at '%s'", pat, word);
    free(pat);
}

/*
 * Splitting of the input text
 */

static char *token(struct state *state) {
    assert(REG_MATCHED(state));
    return strndup(REG_POS(state), REG_SIZE(state));
}

static void regexp_match_error(struct state *state, struct lens *lens,
                               int count, struct regexp *r) {
    char *text = strndup(REG_POS(state), REG_SIZE(state));
    char *pat = regexp_escape(r);

    if (count == -1) {
        get_error(state, lens, "Failed to match /%s/ with %s", pat, text);
    } else if (count == -2) {
        get_error(state, lens, "Internal error matching /%s/ with %s",
                  pat, text);
    } else if (count == -3) {
        /* Should have been caught by the typechecker */
        get_error(state, lens, "Syntax error in regexp /%s/", pat);
    }
    free(pat);
    free(text);
}

static void no_match_error(struct state *state, struct lens *lens) {
    assert(lens->tag == L_KEY || lens->tag == L_DEL
           || lens->tag == L_STORE);
    char *pat = regexp_escape(lens->ctype);
    const char *lname;
    if (lens->tag == L_KEY)
        lname = "key";
    else if (lens->tag == L_DEL)
        lname = "del";
    else if (lens->tag == L_STORE)
        lname = "store";
    else
        assert(0);
    get_error(state, lens, "no match for %s /%s/", lname, pat);
    free(pat);
}

/* Modifies STATE->REGS and STATE->NREG. The caller must save these
 * if they are still needed
 *
 * Return the number of characters matched
 */
static int match(struct state *state, struct lens *lens,
                 struct regexp *re, uint size, uint start) {
    struct re_registers *regs;
    int count;

    if (ALLOC(regs) < 0)
        return -1;

    count = regexp_match(re, state->text, size, start, regs);
    if (count < -1) {
        regexp_match_error(state, lens, count, re);
        FREE(regs);
        return -1;
    }
    state->regs = regs;
    state->nreg = 0;
    return count;
}

static void free_regs(struct state *state) {
    if (state->regs != NULL) {
        free(state->regs->start);
        free(state->regs->end);
        FREE(state->regs);
    }
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

static struct tree *get_del(struct lens *lens, struct state *state) {
    assert(lens->tag == L_DEL);
    if (! REG_MATCHED(state)) {
        char *pat = regexp_escape(lens->ctype);
        get_error(state, lens, "no match for del /%s/", pat);
        free(pat);
    }
    return NULL;
}

static struct skel *parse_del(struct lens *lens, struct state *state) {
    assert(lens->tag == L_DEL);
    struct skel *skel = NULL;

    skel = make_skel(lens);
    if (! REG_MATCHED(state))
        no_match_error(state, lens);
    else
        skel->text = token(state);
    return skel;
}

static struct tree *get_store(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STORE);
    struct tree *tree = NULL;

    assert(state->value == NULL);
    if (state->value != NULL)
        get_error(state, lens, "More than one store in a subtree");
    else if (! REG_MATCHED(state))
        no_match_error(state, lens);
    else
        state->value = token(state);
    return tree;
}

static struct skel *parse_store(struct lens *lens,
                                ATTRIBUTE_UNUSED struct state *state) {
    assert(lens->tag == L_STORE);
    return make_skel(lens);
}

static struct tree *get_key(struct lens *lens, struct state *state) {
    assert(lens->tag == L_KEY);
    if (! REG_MATCHED(state))
        no_match_error(state, lens);
    else
        state->key = token(state);
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
    uint old_nreg = state->nreg;

    state->nreg += 1;
    for (int i=0; i < lens->nchildren; i++) {
        if (REG_MATCHED(state)) {
            tree = get_lens(lens->children[i], state);
            applied = 1;
            break;
        }
        state->nreg += 1 + regexp_nsub(lens->children[i]->ctype);
    }
    state->nreg = old_nreg;
    if (!applied)
        get_expected_error(state, lens);
    return tree;
}

static struct skel *parse_union(struct lens *lens, struct state *state,
                                struct dict **dict) {
    assert(lens->tag == L_UNION);
    struct skel *skel = NULL;
    int applied = 0;
    uint old_nreg = state->nreg;

    state->nreg += 1;
    for (int i=0; i < lens->nchildren; i++) {
        struct lens *l = lens->children[i];
        if (REG_MATCHED(state)) {
            skel = parse_lens(l, state, dict);
            applied = 1;
            break;
        }
        state->nreg += 1 + regexp_nsub(lens->children[i]->ctype);
    }
    state->nreg = old_nreg;
    if (! applied)
        get_expected_error(state, lens);

    return skel;
}

static struct tree *get_concat(struct lens *lens, struct state *state) {
    assert(lens->tag == L_CONCAT);
    struct tree *tree = NULL;
    uint old_nreg = state->nreg;

    state->nreg += 1;
    for (int i=0; i < lens->nchildren; i++) {
        struct tree *t = NULL;
        if (! REG_VALID(state)) {
            get_error(state, lens->children[i],
                      "Not enough components in concat");
            free_tree(tree);
            state->nreg = old_nreg;
            return NULL;
        }

        t = get_lens(lens->children[i], state);
        list_append(tree, t);
        state->nreg += 1 + regexp_nsub(lens->children[i]->ctype);
    }
    state->nreg = old_nreg;

    return tree;
}

static struct skel *parse_concat(struct lens *lens, struct state *state,
                                 struct dict **dict) {
    assert(lens->tag == L_CONCAT);
    struct skel *skel = make_skel(lens);
    uint old_nreg = state->nreg;

    state->nreg += 1;
    for (int i=0; i < lens->nchildren; i++) {
        struct skel *sk = NULL;
        struct dict *di = NULL;
        if (! REG_VALID(state)) {
            get_error(state, lens->children[i],
                      "Not enough components in concat");
            free_skel(skel);
            return NULL;
        }

        sk = parse_lens(lens->children[i], state, &di);
        list_append(skel->skels, sk);
        dict_append(dict, di);
        state->nreg += 1 + regexp_nsub(lens->children[i]->ctype);
    }
    state->nreg = old_nreg;

    return skel;
}

static struct tree *get_quant_star(struct lens *lens, struct state *state) {
    assert(lens->tag == L_STAR);
    struct lens *child = lens->child;
    struct tree *tree = NULL, *tail = NULL;
    struct re_registers *old_regs = state->regs;
    uint old_nreg = state->nreg;
    uint end = REG_END(state);
    uint start = REG_START(state);
    uint size = end - start;

    state->regs = NULL;
    while (size > 0 && match(state, child, child->ctype, end, start) > 0) {
        struct tree *t = NULL;

        t = get_lens(lens->child, state);
        list_tail_cons(tree, tail, t);

        start += REG_SIZE(state);
        size -= REG_SIZE(state);
        free_regs(state);
    }
    free_regs(state);
    state->regs = old_regs;
    state->nreg = old_nreg;
    if (size != 0) {
        get_error(state, lens, "%s", short_iteration);
        state->error->pos = start;
    }
    return tree;
}

static struct skel *parse_quant_star(struct lens *lens, struct state *state,
                                     struct dict **dict) {
    assert(lens->tag == L_STAR);
    struct lens *child = lens->child;
    struct skel *skel = make_skel(lens), *tail = NULL;
    struct re_registers *old_regs = state->regs;
    uint old_nreg = state->nreg;
    uint end = REG_END(state);
    uint start = REG_START(state);
    uint size = end - start;

    *dict = NULL;
    state->regs = NULL;
    while (size > 0 && match(state, child, child->ctype, end, start) > 0) {
        struct skel *sk;
        struct dict *di = NULL;

        sk = parse_lens(lens->child, state, &di);
        list_tail_cons(skel->skels, tail, sk);
        dict_append(dict, di);

        start += REG_SIZE(state);
        size -= REG_SIZE(state);
        free_regs(state);
    }
    free_regs(state);
    state->regs = old_regs;
    state->nreg = old_nreg;
    if (size != 0) {
        get_error(state, lens, "%s", short_iteration);
    }
    return skel;
}

static struct tree *get_quant_maybe(struct lens *lens, struct state *state) {
    assert(lens->tag == L_MAYBE);
    struct tree *tree = NULL;

    /* Check that our child matched. For a construct like (r)?, the
     * matcher will report a zero length match for group 0, even if r
     * does not match at all
     */
    state->nreg += 1;
    if (REG_MATCHED(state)) {
        tree = get_lens(lens->child, state);
    }
    state->nreg -= 1;
    return tree;
}

static struct skel *parse_quant_maybe(struct lens *lens, struct state *state,
                                      struct dict **dict) {
    assert(lens->tag == L_MAYBE);

    struct skel *skel = NULL;

    state->nreg += 1;
    if (REG_MATCHED(state)) {
        skel = parse_lens(lens->child, state, dict);
    }
    state->nreg -= 1;
    if (skel == NULL)
        skel = make_skel(lens);
    return skel;
}

static struct tree *get_subtree(struct lens *lens, struct state *state) {
    char *key = state->key;
    char *value = state->value;
    struct tree *tree = NULL, *children;

    state->key = NULL;
    state->value = NULL;
    children = get_lens(lens->child, state);

    tree = make_tree(state->key, state->value, NULL, children);

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

/* Initialize registers. Return 0 if the lens matches the entire text, 1 if
 * it does not and -1 on error.
 */
static int init_regs(struct state *state, struct lens *lens, uint size) {
    int r;

    if (lens->tag != L_STAR) {
        r = match(state, lens, lens->ctype, size, 0);
        if (r == -1)
            get_error(state, lens, "Input string does not match at all");
        if (r <= -1)
            return -1;
        return r != size;
    }
    /* Special case the very common situation that the lens is (l)*
     * We can avoid matching the entire text in that case - that
     * match can be very expensive
     */
    if (ALLOC(state->regs) < 0)
        return -1;
    state->regs->num_regs = 1;
    if (ALLOC(state->regs->start) < 0 || ALLOC(state->regs->end) < 0)
        return -1;
    state->regs->start[0] = 0;
    state->regs->end[0] = size;
    return 0;
}

struct tree *lns_get(struct info *info, struct lens *lens, const char *text,
                     struct lns_error **err) {
    struct state state;
    struct tree *tree = NULL;
    uint size = strlen(text);
    int partial;

    MEMZERO(&state, 1);
    state.info = *info;
    state.info.ref = UINT_MAX;

    state.text = text;

    /* We are probably being overly cautious here: if the lens can't process
     * all of TEXT, we should really fail somewhere in one of the sublenses.
     * But to be safe, we check that we can process everything anyway, then
     * try to process, hoping we'll get a more specific error, and if that
     * fails, we throw our arms in the air and say 'something went wrong'
     */
    partial = init_regs(&state, lens, size);
    if (partial >= 0)
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

    free_regs(&state);

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
    struct skel *skel = NULL;
    uint size = strlen(text);
    int partial;

    MEMZERO(&state, 1);
    state.info.ref = UINT_MAX;
    state.info.error = lens->info->error;
    state.text = text;

    state.text = text;

    partial = init_regs(&state, lens, size);
    if (! partial) {
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

    free_regs(&state);

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
