/*
 * parser.c: parse a configuration file according to a grammar
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

#include <regex.h>
#include <stdarg.h>

#include "regexp.h"
#include "list.h"
#include "internal.h"
#include "memory.h"
#include "info.h"
#include "lens.h"
#include "errcode.h"

/* Our favorite error message */
static const char *const short_iteration =
    "Iterated lens matched less than it should";

struct seq {
    struct seq *next;
    const char *name;
    int value;
};

struct state {
    struct info      *info;
    struct span      *span;
    const char       *text;
    struct seq       *seqs;
    char             *key;
    char             *value;     /* GET_STORE leaves a value here */
    char             *square;    /* last L_DEL from L_SQUARE */
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

/* Used by recursive lenses to stack intermediate results */
struct frame {
    struct lens     *lens;
    char            *key;
    char            *square;
    struct span     *span;
    union {
        struct { /* MGET */
            char        *value;
            struct tree *tree;
        };
        struct { /* M_PARSE */
            struct skel *skel;
            struct dict *dict;
        };
    };
};

/* Used by recursive lenses in get_rec and parse_rec */
enum mode_t { M_GET, M_PARSE };

struct rec_state {
    enum mode_t          mode;
    struct state        *state;
    uint                 fsize;
    uint                 fused;
    struct frame        *frames;
    size_t               start;
    uint                 lvl;  /* Debug only */
};

#define REG_START(state) ((state)->regs->start[(state)->nreg])
#define REG_END(state)   ((state)->regs->end[(state)->nreg])
#define REG_SIZE(state) (REG_END(state) - REG_START(state))
#define REG_POS(state) ((state)->text + REG_START(state))
#define REG_VALID(state) ((state)->regs != NULL &&                      \
                          (state)->nreg < (state)->regs->num_regs)
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

static void vget_error(struct state *state, struct lens *lens,
                       const char *format, va_list ap) {
    int r;

    if (state->error != NULL)
        return;
    CALLOC(state->error, 1);
    state->error->lens = ref(lens);
    if (REG_MATCHED(state))
        state->error->pos  = REG_END(state);
    else
        state->error->pos = 0;
    r = vasprintf(&state->error->message, format, ap);
    if (r == -1)
        state->error->message = NULL;
}

static void get_error(struct state *state, struct lens *lens,
                      const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vget_error(state, lens, format, ap);
    va_end(ap);
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
    if (skel->tag == L_CONCAT || skel->tag == L_STAR || skel->tag == L_MAYBE ||
        skel->tag == L_SQUARE) {
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

    pat = escape(l->ctype->pattern->str, -1, NULL);
    get_error(state, l, "expected %s at '%s'", pat, word);
    free(pat);
}

/*
 * Splitting of the input text
 */

static char *token(struct state *state) {
    ensure0(REG_MATCHED(state), state->info);
    return strndup(REG_POS(state), REG_SIZE(state));
}

static void regexp_match_error(struct state *state, struct lens *lens,
                               int count, struct regexp *r) {
    char *text = NULL;
    char *pat = regexp_escape(r);

    if (state->regs != NULL)
        text = strndup(REG_POS(state), REG_SIZE(state));
    else
        text = strdup("(unknown)");

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
    ensure(lens->tag == L_KEY || lens->tag == L_DEL
           || lens->tag == L_STORE, state->info);
    char *pat = regexp_escape(lens->ctype);
    const char *lname = "(lname)";
    if (lens->tag == L_KEY)
        lname = "key";
    else if (lens->tag == L_DEL)
        lname = "del";
    else if (lens->tag == L_STORE)
        lname = "store";
    get_error(state, lens, "no match for %s /%s/", lname, pat);
    free(pat);
 error:
    return;
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
    ensure0(name != NULL, state->info);
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
    ensure0(lens->tag == L_SEQ, state->info);
    struct seq *seq = find_seq(lens->string->str, state);
    int r;

    r = asprintf((char **) &(state->key), "%d", seq->value);
    ERR_NOMEM(r < 0, state->info);

    seq->value += 1;
 error:
    return NULL;
}

static struct skel *parse_seq(struct lens *lens, struct state *state) {
    get_seq(lens, state);
    return make_skel(lens);
}

static struct tree *get_counter(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_COUNTER, state->info);
    struct seq *seq = find_seq(lens->string->str, state);
    seq->value = 1;
    return NULL;
}

static struct skel *parse_counter(struct lens *lens, struct state *state) {
    get_counter(lens, state);
    return make_skel(lens);
}

static struct tree *get_del(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_DEL, state->info);
    if (! REG_MATCHED(state)) {
        char *pat = regexp_escape(lens->ctype);
        get_error(state, lens, "no match for del /%s/", pat);
        free(pat);
    }
    if (lens->string == NULL) {
        state->square = token(state);
    }
    update_span(state->span, REG_START(state), REG_END(state));
    return NULL;
}

static struct skel *parse_del(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_DEL, state->info);
    struct skel *skel = NULL;

    skel = make_skel(lens);
    if (! REG_MATCHED(state))
        no_match_error(state, lens);
    else
        skel->text = token(state);
    return skel;
}

static struct tree *get_store(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_STORE, state->info);
    ensure0(state->value == NULL, state->info);

    struct tree *tree = NULL;

    if (state->value != NULL)
        get_error(state, lens, "More than one store in a subtree");
    else if (! REG_MATCHED(state))
        no_match_error(state, lens);
    else {
        state->value = token(state);
        if (state->span) {
            state->span->value_start = REG_START(state);
            state->span->value_end = REG_END(state);
            update_span(state->span, REG_START(state), REG_END(state));
        }
    }
    return tree;
}

static struct skel *parse_store(struct lens *lens,
                                ATTRIBUTE_UNUSED struct state *state) {
    ensure0(lens->tag == L_STORE, state->info);
    return make_skel(lens);
}

static struct tree *get_value(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_VALUE, state->info);
    state->value = strdup(lens->string->str);
    return NULL;
}

static struct skel *parse_value(struct lens *lens,
                                ATTRIBUTE_UNUSED struct state *state) {
    ensure0(lens->tag == L_VALUE, state->info);
    return make_skel(lens);
}

static struct tree *get_key(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_KEY, state->info);
    if (! REG_MATCHED(state))
        no_match_error(state, lens);
    else {
        state->key = token(state);
        if (state->span) {
            state->span->label_start = REG_START(state);
            state->span->label_end = REG_END(state);
            update_span(state->span, REG_START(state), REG_END(state));
        }
    }
    return NULL;
}

static struct skel *parse_key(struct lens *lens, struct state *state) {
    get_key(lens, state);
    return make_skel(lens);
}

static struct tree *get_label(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_LABEL, state->info);
    state->key = strdup(lens->string->str);
    return NULL;
}

static struct skel *parse_label(struct lens *lens, struct state *state) {
    get_label(lens, state);
    return make_skel(lens);
}

static struct tree *get_union(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_UNION, state->info);

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
    ensure0(lens->tag == L_UNION, state->info);

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
    ensure0(lens->tag == L_CONCAT, state->info);

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
    ensure0(lens->tag == L_CONCAT, state->info);
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
    ensure0(lens->tag == L_STAR, state->info);
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
    ensure0(lens->tag == L_STAR, state->info);
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
    ensure0(lens->tag == L_MAYBE, state->info);
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
    ensure0(lens->tag == L_MAYBE, state->info);

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
    struct span *span = state->span;

    struct tree *tree = NULL, *children;

    state->key = NULL;
    state->value = NULL;
    if (state->info->flags & AUG_ENABLE_SPAN) {
        state->span = make_span(state->info);
        ERR_NOMEM(state->span == NULL, state->info);
    }

    children = get_lens(lens->child, state);

    tree = make_tree(state->key, state->value, NULL, children);
    tree->span = state->span;

    if (state->span != NULL) {
        update_span(span, state->span->span_start, state->span->span_end);
    }

    state->key = key;
    state->value = value;
    state->span = span;
    return tree;
 error:
    return NULL;
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

static struct tree *get_square(struct lens *lens, struct state *state) {
    ensure0(lens->tag == L_SQUARE, state->info);

    struct tree *tree = NULL;
    char *key = NULL, *square = NULL;

    // get the child lens
    tree = get_concat(lens->child, state);

    key = state->key;
    square = state->square;
    ensure0(key != NULL, state->info);
    ensure0(square != NULL, state->info);

    if (strcmp(key, square) != 0) {
        get_error(state, lens, "%s \"%s\" %s \"%s\"",
                "Parse error: mismatched key in square lens, expecting", key,
                "but got", square);
    }

    FREE(state->square);
    return tree;
}

static struct skel *parse_square(struct lens *lens, struct state *state,
                                 struct dict **dict) {
    ensure0(lens->tag == L_SQUARE, state->info);
    struct skel *skel, *sk;

    skel = parse_concat(lens->child, state, dict);
    sk = make_skel(lens);
    sk->skels = skel;

    return sk;
}

/*
 * Helpers for recursive lenses
 */

ATTRIBUTE_UNUSED
static void print_frames(struct rec_state *state) {
    for (int j = state->fused - 1; j >=0; j--) {
        struct frame *f = state->frames + j;
        for (int i=0; i < state->lvl; i++) fputc(' ', stderr);
        fprintf(stderr, "%2d %s %s %s", j, f->key, f->value, f->square);
        if (f->tree == NULL) {
            fprintf(stderr, " - ");
        } else {
            fprintf(stderr, " { %s = %s } ", f->tree->label, f->tree->value);
        }
        fprintf(stderr, "%s\n", format_lens(f->lens));
    }
}

ATTRIBUTE_PURE
static struct frame *top_frame(struct rec_state *state) {
    ensure0(state->fsize > 0, state->state->info);
    return state->frames + state->fused - 1;
}

/* The nth frame from the top of the stack, where 0th frame is the top */
ATTRIBUTE_PURE
static struct frame *nth_frame(struct rec_state *state, uint n) {
    assert(state->fsize > n);
    return state->frames + state->fused - (n+1);
}

static struct frame *push_frame(struct rec_state *state, struct lens *lens) {
    int r;

    if (state->fused >= state->fsize) {
        uint expand = state->fsize;
        if (expand < 8)
            expand = 8;
        r = REALLOC_N(state->frames, state->fsize + expand);
        ERR_NOMEM(r < 0, state->state->info);
        state->fsize += expand;
    }

    state->fused += 1;

    struct frame *top = top_frame(state);
    MEMZERO(top, 1);
    top->lens = lens;
    return top;
 error:
    return NULL;
}

static struct frame *pop_frame(struct rec_state *state) {
    ensure0(state->fused > 0, state->state->info);

    state->fused -= 1;
    if (state->fused > 0)
        return top_frame(state);
    else
        return NULL;
}

static void dbg_visit(struct lens *lens, char action, size_t start, size_t end,
                      int fused, int lvl) {

    for (int i=0; i < lvl; i++)
        fputc(' ', stderr);
    fprintf(stderr, "%c %zd..%zd %d %s\n", action, start, end,
            fused, format_lens(lens));
}

static void get_terminal(struct frame *top, struct lens *lens,
                         struct state *state) {
    top->tree = get_lens(lens, state);
    top->key = state->key;
    top->value = state->value;
    top->square = state->square;
    state->key = NULL;
    state->value = NULL;
    state->square = NULL;
}

static void parse_terminal(struct frame *top, struct lens *lens,
                           struct state *state) {
    top->dict = NULL;
    top->skel = parse_lens(lens, state, &top->dict);
    top->key = state->key;
    state->key = NULL;
}

static void visit_terminal(struct lens *lens, size_t start, size_t end,
                           void *data) {
    struct rec_state *rec_state = data;
    struct state *state = rec_state->state;
    struct re_registers *old_regs = state->regs;
    uint old_nreg = state->nreg;

    if (state->error != NULL)
        return;

    if (debugging("cf.get"))
        dbg_visit(lens, 'T', start, end, rec_state->fused, rec_state->lvl);
    match(state, lens, lens->ctype, end, start);
    struct frame *top = push_frame(rec_state, lens);
    if (rec_state->mode == M_GET)
        get_terminal(top, lens, state);
    else
        parse_terminal(top, lens, state);
    free_regs(state);
    state->regs = old_regs;
    state->nreg = old_nreg;
}

static void visit_enter(struct lens *lens,
                        ATTRIBUTE_UNUSED size_t start,
                        ATTRIBUTE_UNUSED size_t end,
                        void *data) {
    struct rec_state *rec_state = data;
    struct state *state = rec_state->state;

    if (state->error != NULL)
        return;

    if (debugging("cf.get"))
        dbg_visit(lens, '{', start, end, rec_state->fused, rec_state->lvl);
    rec_state->lvl += 1;
    if (lens->tag == L_SUBTREE) {
        /* Same for parse and get */
        struct frame *f = push_frame(rec_state, lens);
        f->key = state->key;
        f->value = state->value;
        f->span = state->span;
        state->key = NULL;
        state->value = NULL;
    } else if (lens->tag == L_MAYBE) {
        push_frame(rec_state, lens);
        if (state->info->flags & AUG_ENABLE_SPAN) {
            state->span = make_span(state->info);
            ERR_NOMEM(state->span == NULL, state->info);
        }
    }
 error:
    return;
}

static void get_combine(struct rec_state *rec_state,
                        struct lens *lens, uint n) {
    struct tree *tree = NULL, *tail = NULL;
    char *key = NULL, *value = NULL, *square = NULL;
    struct frame *top = NULL;

    if (n > 0)
        top = top_frame(rec_state);

    for (int i=0; i < n; i++, top = pop_frame(rec_state)) {
        list_tail_cons(tree, tail, top->tree);
        /* top->tree might have more than one node, update tail */
        if (tail != NULL)
            while (tail->next != NULL) tail = tail->next;

        if (top->key != NULL) {
            ensure(key == NULL, rec_state->state->info);
            key = top->key;
        }
        if (top->value != NULL) {
            ensure(value == NULL, rec_state->state->info);
            value = top->value;
        }
        if (top->square != NULL) {
            ensure(square == NULL, rec_state->state->info);
            square = top->square;
        }
    }
    top = push_frame(rec_state, lens);
    top->tree = tree;
    top->key = key;
    top->value = value;
    top->square = square;
 error:
    return;
}

static void parse_combine(struct rec_state *rec_state,
                          struct lens *lens, uint n) {
    struct skel *skel = make_skel(lens), *tail = NULL;
    struct dict *dict = NULL;
    char *key = NULL;
    struct frame *top = NULL;

    if (n > 0)
        top = top_frame(rec_state);

    for (int i=0; i < n; i++, top = pop_frame(rec_state)) {
        list_tail_cons(skel->skels, tail, top->skel);
        /* top->skel might have more than one node, update skel */
        if (tail != NULL)
            while (tail->next != NULL) tail = tail->next;
        dict_append(&dict, top->dict);
        if (top->key != NULL) {
            ensure(key == NULL, rec_state->state->info);
            key = top->key;
        }
    }
    top = push_frame(rec_state, lens);
    top->skel = skel;
    top->dict = dict;
    top->key = key;
 error:
    return;
}

static void visit_exit(struct lens *lens,
                       ATTRIBUTE_UNUSED size_t start,
                       ATTRIBUTE_UNUSED size_t end,
                       void *data) {
    struct rec_state *rec_state = data;
    struct state *state = rec_state->state;

    if (state->error != NULL)
        return;

    rec_state->lvl -= 1;
    if (debugging("cf.get"))
        dbg_visit(lens, '}', start, end, rec_state->fused, rec_state->lvl);

    ERR_BAIL(lens->info);

    if (lens->tag == L_SUBTREE) {
        struct frame *top = top_frame(rec_state);
        if (rec_state->mode == M_GET) {
            struct tree *tree;
            // FIXME: tree may leak if pop_frame ensure0 fail
            tree = make_tree(top->key, top->value, NULL, top->tree);
            tree->span = state->span;
            ERR_NOMEM(tree == NULL, lens->info);
            top = pop_frame(rec_state);
            ensure(lens == top->lens, state->info);
            state->key = top->key;
            state->value = top->value;
            state->span = top->span;
            pop_frame(rec_state);
            top = push_frame(rec_state, lens);
            top->tree = tree;
        } else {
            struct skel *skel;
            struct dict *dict;
            skel = make_skel(lens);
            ERR_NOMEM(skel == NULL, lens->info);
            dict = make_dict(top->key, top->skel, top->dict);
            ERR_NOMEM(dict == NULL, lens->info);
            top = pop_frame(rec_state);
            ensure(lens == top->lens, state->info);
            state->key = top->key;
            pop_frame(rec_state);
            top = push_frame(rec_state, lens);
            top->skel = skel;
            top->dict = dict;
        }
    } else if (lens->tag == L_CONCAT) {
        ensure(rec_state->fused >= lens->nchildren, state->info);
        for (int i = 0; i < lens->nchildren; i++) {
            struct frame *fr = nth_frame(rec_state, i);
            BUG_ON(lens->children[i] != fr->lens,
                    lens->info,
             "Unexpected lens in concat %zd..%zd\n  Expected: %s\n  Actual: %s",
                    start, end,
                    format_lens(lens->children[i]),
                    format_lens(fr->lens));
        }
        if (rec_state->mode == M_GET)
            get_combine(rec_state, lens, lens->nchildren);
        else
            parse_combine(rec_state, lens, lens->nchildren);
    } else if (lens->tag == L_STAR) {
        uint n = 0;
        while (n < rec_state->fused &&
               nth_frame(rec_state, n)->lens == lens->child)
            n++;
        if (rec_state->mode == M_GET)
            get_combine(rec_state, lens, n);
        else
            parse_combine(rec_state, lens, n);
    } else if (lens->tag == L_MAYBE) {
        uint n = 1;
        if (rec_state->fused > 0
            && top_frame(rec_state)->lens == lens->child) {
            n = 2;
        }
        if (rec_state->mode == M_GET)
            get_combine(rec_state, lens, n);
        else
            parse_combine(rec_state, lens, n);
    } else if (lens->tag == L_SQUARE) {
        if (rec_state->mode == M_GET) {
            char *key, *square;

            key = top_frame(rec_state)->key;
            square = top_frame(rec_state)->square;

            ensure(key != NULL, state->info);
            ensure(square != NULL, state->info);

            // raise syntax error if they are not equals
            if (strcmp(key, square) != 0){
                get_error(state, lens, "%s \"%s\" %s \"%s\"",
                        "Parse error: mismatched key in square lens, expecting",
                        key, "but got", square);
                state->error->pos = end - strlen(square);
                goto error;
            }

            FREE(square);
            get_combine(rec_state, lens, 1);
        } else {
            parse_combine(rec_state, lens, 1);
        }
    } else {
        top_frame(rec_state)->lens = lens;
    }
 error:
    return;
}

static void visit_error(struct lens *lens, void *data, size_t pos,
                        const char *format, ...) {
    struct rec_state *rec_state = data;
    va_list ap;

    va_start(ap, format);
    vget_error(rec_state->state, lens, format, ap);
    va_end(ap);
    rec_state->state->error->pos = rec_state->start + pos;
}

static struct frame *rec_process(enum mode_t mode, struct lens *lens,
                                 struct state *state) {
    uint end = REG_END(state);
    uint start = REG_START(state);
    size_t len = 0;
    struct re_registers *old_regs = state->regs;
    uint old_nreg = state->nreg;
    int r;
    struct jmt_visitor visitor;
    struct rec_state rec_state;
    int i;
    struct frame *f = NULL;

    MEMZERO(&rec_state, 1);
    MEMZERO(&visitor, 1);

    if (lens->jmt == NULL) {
        lens->jmt = jmt_build(lens);
        ERR_BAIL(lens->info);
    }

    state->regs = NULL;
    state->nreg = 0;

    rec_state.mode  = mode;
    rec_state.state = state;
    rec_state.fused = 0;
    rec_state.lvl   = 0;
    rec_state.start = start;

    visitor.parse = jmt_parse(lens->jmt, state->text + start, end - start);
    ERR_BAIL(lens->info);
    visitor.terminal = visit_terminal;
    visitor.enter = visit_enter;
    visitor.exit = visit_exit;
    visitor.error = visit_error;
    visitor.data = &rec_state;
    r = jmt_visit(&visitor, &len);
    ERR_BAIL(lens->info);
    if (r != 1) {
        get_error(state, lens, "Syntax error");
        state->error->pos = start + len;
    }
    if (rec_state.fused == 0) {
        get_error(state, lens,
                  "Parse did not leave a result on the stack");
        goto error;
    } else if (rec_state.fused > 1) {
        get_error(state, lens,
                  "Parse left additional garbage on the stack");
        goto error;
    }

 done:
    state->regs = old_regs;
    state->nreg = old_nreg;
    jmt_free_parse(visitor.parse);
    return rec_state.frames;
 error:

    for(i = 0; i < rec_state.fused; i++) {
        f = nth_frame(&rec_state, i);
        FREE(f->key);
        FREE(f->square);
        if (mode == M_GET) {
            FREE(f->value);
            free_tree(f->tree);
        } else if (mode == M_PARSE) {
            free_skel(f->skel);
            free_dict(f->dict);
        }
    }
    FREE(rec_state.frames);
    goto done;
}

static struct tree *get_rec(struct lens *lens, struct state *state) {
    struct frame *fr;
    struct tree *tree = NULL;

    fr = rec_process(M_GET, lens, state);
    if (fr != NULL) {
        tree = fr->tree;
        state->key = fr->key;
        state->value = fr->value;
        FREE(fr);
    }
    return tree;
}

static struct skel *parse_rec(struct lens *lens, struct state *state,
                              struct dict **dict) {
    struct skel *skel = NULL;
    struct frame *fr;

    fr = rec_process(M_PARSE, lens, state);
    if (fr != NULL) {
        skel = fr->skel;
        *dict = fr->dict;
        state->key = fr->key;
        FREE(fr);
    }
    return skel;
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
    case L_VALUE:
        tree = get_value(lens, state);
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
    case L_SQUARE:
        tree = get_square(lens, state);
        break;
    default:
        BUG_ON(true, state->info, "illegal lens tag %d", lens->tag);
        break;
    }
 error:
    return tree;
}

/* Initialize registers. Return 0 if the lens matches the entire text, 1 if
 * it does not and -1 on error.
 */
static int init_regs(struct state *state, struct lens *lens, uint size) {
    int r;

    if (lens->tag != L_STAR && ! lens->recursive) {
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
    int partial, r;

    MEMZERO(&state, 1);
    r = ALLOC(state.info);
    ERR_NOMEM(r < 0, info);

    *state.info = *info;
    state.info->ref = UINT_MAX;

    state.text = text;

    /* We are probably being overly cautious here: if the lens can't process
     * all of TEXT, we should really fail somewhere in one of the sublenses.
     * But to be safe, we check that we can process everything anyway, then
     * try to process, hoping we'll get a more specific error, and if that
     * fails, we throw our arms in the air and say 'something went wrong'
     */
    partial = init_regs(&state, lens, size);
    if (partial >= 0) {
        if (lens->recursive)
            tree = get_rec(lens, &state);
        else
            tree = get_lens(lens, &state);
    }

    free_seqs(state.seqs);
    if (state.key != NULL) {
        get_error(&state, lens, "get left unused key %s", state.key);
        free(state.key);
    }
    if (state.value != NULL) {
        get_error(&state, lens, "get left unused value %s", state.value);
        free(state.value);
    }
    if (state.square != NULL) {
        get_error(&state, lens, "get left unused square %s", state.square);
        free(state.square);
    }
    if (partial && state.error == NULL) {
        get_error(&state, lens, "Get did not match entire input");
    }

 error:
    free_regs(&state);
    FREE(state.info);

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
    case L_VALUE:
        skel = parse_value(lens, state);
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
    case L_SQUARE:
        skel = parse_square(lens, state, dict);
        break;
    default:
        BUG_ON(true, state->info, "illegal lens tag %d", lens->tag);
        break;
    }
 error:
    return skel;
}

struct skel *lns_parse(struct lens *lens, const char *text, struct dict **dict,
                       struct lns_error **err) {
    struct state state;
    struct skel *skel = NULL;
    uint size = strlen(text);
    int partial, r;

    MEMZERO(&state, 1);
    r = ALLOC(state.info);
    ERR_NOMEM(r< 0, lens->info);
    state.info->ref = UINT_MAX;
    state.info->error = lens->info->error;
    state.text = text;

    state.text = text;

    partial = init_regs(&state, lens, size);
    if (! partial) {
        *dict = NULL;
        if (lens->recursive)
            skel = parse_rec(lens, &state, dict);
        else
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

 error:
    free_regs(&state);
    FREE(state.info);
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
