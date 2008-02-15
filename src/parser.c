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

#include <pcre.h>
#include "ast.h"
#include "list.h"
#include "internal.h"

#define NMATCHES 100

#define parse_error(state, format, args ...) \
    grammar_error((state)->filename, (state)->lineno, format, ## args)

struct seq {
    struct seq *next;
    const char *name;
    int value;
};

struct state {
    const char *filename;
    int         lineno;
    const char *text;
    const char *pos;
    int         applied;
    int         flags;     /* set of parse_flags */
    FILE       *log;
    struct seq *seqs;
    const char *key;
    struct tree *tree;
    struct skel *skel;
    struct dict *dict;
};

static struct tree *make_tree(const char *label, const char *value) {
    struct tree *tree;
    CALLOC(tree, 1);
    tree->label = label;
    tree->value = value;
    return tree;
}

static struct skel *make_skel(enum match_type type, struct match *match,
                              int lineno) {
    struct skel *skel;
    assert(type == LITERAL || type == SEQUENCE ||
           type == QUANT_STAR || type == QUANT_PLUS || type == QUANT_MAYBE ||
           type == SUBTREE);
    CALLOC(skel, 1);
    skel->type = type;
    skel->match = match;
    skel->lineno = lineno;
    return skel;
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
    switch(skel->type) {
    case LITERAL:
        if (skel->text == NULL) {
            printf("<>");
        } else {
            fputc('\'', stdout);
            print_chars(stdout, skel->text, -1);
            fputc('\'', stdout);
        }
        break;
    case SEQUENCE:
        print_skel_list(skel->skels, "", " . ", "");
        break;
    case QUANT_STAR:
        print_skel_list(skel->skels, "(", " ", ")*");
        break;
    case QUANT_PLUS:
        print_skel_list(skel->skels, "(", " ", ")+");
        break;
    case QUANT_MAYBE:
        print_skel_list(skel->skels, "(", " ", ")?");
        break;
    case SUBTREE:
        print_skel_list(skel->skels, "[", " ", "]");
        break;
    default:
        printf("??");
        break;
    }
}

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


static struct dict *dict_append(struct dict *d1, struct dict *d2) {
    if (d1 == NULL) {
        return d2;
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
    return d1;
}

static void parse_expected_error(struct state *state, struct match *exp) {
    char *word, *p;
    const char *name = NULL;

    word = alloca(11);
    strncpy(word, state->pos, 10);
    word[10] = '\0';
    for (p = word; *p != '\0' && *p != '\n'; p++);
    *p = '\0';

    while (name == NULL) {
        switch (exp->type) {
        case LITERAL:
            if (exp->literal->type == QUOTED)
                name = exp->literal->text;
            else
                name = exp->literal->pattern;
            break;
        case ANY:
            name = "...";
            break;
        case RULE_REF:
            name = exp->rule->name;
            break;
        case ABBREV_REF:
            name = exp->abbrev->name;
            break;
        case ALTERNATIVE:
        case SEQUENCE:
        case QUANT_PLUS:
        case QUANT_STAR:
        case QUANT_MAYBE:
            exp = exp->matches;
            break;
        default:
            name = "unknown";
            break;
        }
    }
    parse_error(state, "expected %s at '%s'", name, word);
}

/* 
 * Parsing/construction of the AST 
 */
static void advance(struct state *state, int cnt) {
    if (cnt == 0)
        return;

    for (int i=0; i<cnt; i++) {
        if (! state->pos)
            internal_error(NULL, state->lineno, "moved beyond end of input");
        if (state->pos[i] == '\n')
            state->lineno++;
    }
    state->pos += cnt;
    if (state->flags & PF_ADVANCE) {
        static const int window = 28;
        int before = state->pos - state->text;
        int total;
        if (before > window)
            before = window;
        fprintf(state->log, "A %3d ", cnt);
        total = print_chars(NULL, state->pos - before, before);
        if (total < window + 10)
            fprintf(state->log, "%*s", window + 10 - total, "<");
        print_chars(state->log, state->pos - before, before);
        fprintf(state->log, "|=|");
        total = print_chars(state->log, state->pos, 20);
        fprintf(state->log, "%*s\n", window-total, ">");
    }
}

static int lex(struct literal *literal, struct state *state) {
    int rc;
    int matches[NMATCHES];
    int offset = state->pos - state->text;
    rc = pcre_exec(literal->re, NULL, state->text, strlen(state->text),
                   offset, PCRE_ANCHORED, matches, NMATCHES);
    if (state->flags & PF_MATCH) {
        fprintf(state->log, "M %d ", offset);
        print_literal(state->log, literal);
        fprintf(state->log, " %d..%d\n", matches[0], matches[1]);
    }

    if (rc >= 1) {
        if (matches[0] != offset) {
            parse_error(state, "Skipped %d characters", matches[0] - offset);
        }
        return (matches[1] - matches[0]);
    }
    return -1;
}

static const char *re_match(struct match *match, struct state *state) {
    struct literal *literal = NULL;
    const char *result = NULL;

    if (match->type == ABBREV_REF)
        literal = match->abbrev->literal;
    else if (match->type == LITERAL || match->type == ANY)
        literal = match->literal;
    else {
        internal_error(state->filename, state->lineno,
                       "Illegal match type %d for literal", match->type);
        state->applied = 0;
        return NULL;
    }

    int len = lex(literal, state);
    if (len < 0)
        return NULL;
    result = strndup(state->pos, len);
    advance(state, len);

    if (state->flags & PF_TOKEN) {
        struct abbrev *a;
        fprintf(state->log, "T ");
        for (a=match->owner->grammar->abbrevs; a != NULL; a = a->next) {
            if (a->literal == literal) {
                fprintf(state->log, a->name);
                break;
            }
        }
        if (a == NULL) {
            if (match->type == ANY) {
                fprintf(state->log, "..%c", match->epsilon ? '?' : '.');
            } else {
                print_literal(state->log, literal);
            }
        }
        fprintf(state->log, " = <");
        print_chars(state->log, result, strlen(result));
        fprintf(state->log, ">\n");
    }

    return result;
}

static const char *string_value(struct match *arg) {
    if (arg->type == LITERAL && arg->literal->type == QUOTED)
        return strdup(arg->literal->text);
    assert(0);
}

static struct seq *get_seq(struct match *arg, struct state *state) {
    const char *name = NULL;
    struct seq *seq;

    if (arg->type == LITERAL && arg->literal->type == QUOTED)
        name = arg->literal->text;
    assert(name != NULL);

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

static const char *seq_next_value(struct match *arg, struct state *state) {
    struct seq *seq = get_seq(arg, state);
    char *result;

    int len = snprintf(NULL, 0, "%d", seq->value);
    CALLOC(result, len+1);
    snprintf(result, len + 1, "%d", seq->value);
    seq->value += 1;

    return result;
}

static void seq_init(struct match *arg, struct state *state) {
    struct seq *seq = get_seq(arg, state);

    seq->value = 0;
}

static void parse_match(struct match *match, struct state *state);

static void parse_literal(struct match *match, struct state *state) {
    const char *token = NULL;

    token = re_match(match, state);
    if (token == NULL) {
        state->applied = 0;
    } else {
        state->skel = make_skel(LITERAL, match, state->lineno);
        state->skel->text = token;
        state->applied = 1;
    }
}

static int applies(struct match *match, struct state *state) {
    list_for_each(f, match->first) {
        if (lex(f->literal, state) > 0)
            return 1;
    }
    return 0;
}

static void parse_alternative(struct match *match, struct state *state) {
    state->applied = 0;

    list_for_each(p, match->matches) {
        if (applies(p, state)) {
            parse_match(p, state);
            state->applied = 1;
            break;
        }
    }
    if (! state->applied) {
        parse_expected_error(state, match);
    }
}

static void parse_sequence(struct match *match, struct state *state) {
    struct tree *tree = NULL;
    struct skel *skel = make_skel(SEQUENCE, match, state->lineno);
    struct dict *dict = NULL;

    state->applied = 1;
    list_for_each(p, match->matches) {
        state->tree = NULL;
        state->skel = NULL;
        state->dict = NULL;
        parse_match(p, state);
        if (! state->applied) {
            parse_expected_error(state, p);
            break;
        }
        list_append(tree, state->tree);
        list_append(skel->skels, state->skel);
        dict = dict_append(dict, state->dict);
    }
    state->tree = tree;
    state->skel = skel;
    state->dict = dict;
}

static void parse_rule_ref(struct match *match, struct state *state) {
    parse_match(match->rule->matches, state);
}

static void parse_quant_star(struct match *match, struct state *state) {
    struct tree *tree = NULL;
    struct skel *skel = make_skel(QUANT_STAR, match, state->lineno);
    struct dict *dict = NULL;
    while (applies(match, state)) {
        state->tree = NULL;
        state->skel = NULL;
        state->dict = NULL;
        parse_match(match->matches, state);
        list_append(tree, state->tree);
        list_append(skel->skels, state->skel);
        dict = dict_append(dict, state->dict);
    }
    state->applied = 1;
    state->tree = tree;
    state->skel = skel;
    state->dict = dict;
}

static void parse_quant_plus(struct match *match, struct state *state) {
    if (! applies(match, state)) {
        grammar_error(state->filename, state->lineno,
                      "match did not apply");
    } else {
        struct tree *tree = NULL;
        struct skel *skel = make_skel(QUANT_PLUS, match, state->lineno);
        struct dict *dict = NULL;
        while (applies(match, state)) {
            state->tree = NULL;
            state->skel = NULL;
            state->dict = NULL;
            parse_match(match->matches, state);
            list_append(tree, state->tree);
            list_append(skel->skels, state->skel);
            dict = dict_append(dict, state->dict);
        }
        state->tree = tree;
        state->skel = skel;
        state->dict = dict;
        state->applied = 1;
    }
}

static void parse_quant_maybe(struct match *match, struct state *state) {
    struct skel *skel = make_skel(QUANT_MAYBE, match, state->lineno);
    if (applies(match, state)) {
        state->tree = NULL;
        state->skel = NULL;
        state->dict = NULL;
        parse_match(match->matches, state);
        list_append(skel->skels, state->skel);
    }
    state->skel = skel;
    state->applied = 1;
}

static void parse_subtree(struct match *match, struct state *state) {
    const char *key = state->key;

    state->key = NULL;
    state->tree = NULL;
    state->skel = NULL;
    state->dict = NULL;
    parse_match(match->matches, state);
    if (state->tree != NULL) {
        if (state->tree->label == NULL) {
            state->tree->label = state->key;
        } else {
            struct tree *tree = make_tree(state->key, NULL);
            tree->children = state->tree;
            state->tree = tree;
        }
    }
    state->dict = make_dict(state->key, state->skel, state->dict);
    state->key = key;
    state->skel = make_skel(SUBTREE, match, state->lineno);
}

static void parse_action(struct match *match, struct state *state) {
    struct action *action = match->xaction;

    state->skel = make_skel(LITERAL, match, state->lineno);
    switch(action->type) {
    case COUNTER:
        seq_init(action->arg, state);
        break;
    case SEQ:
        state->key = seq_next_value(action->arg, state);
        break;
    case LABEL:
        state->key = string_value(action->arg);
        break;
    case STORE:
        state->tree = make_tree(NULL, re_match(match->xaction->arg, state));
        break;
    case KEY:
        state->key = re_match(match->xaction->arg, state);
        break;
    default:
        internal_error(state->filename, state->lineno,
                       "illegal action type %d", action->type);
        break;
    }
}

static void parse_match(struct match *match, struct state *state) {
    switch(match->type) {
    case LITERAL:
        parse_literal(match, state);
        break;
    case ANY:
        parse_literal(match, state);
        break;
    case ALTERNATIVE:
        parse_alternative(match, state);
        break;
    case SEQUENCE:
        parse_sequence(match, state);
        break;
    case RULE_REF:
        parse_rule_ref(match, state);
        break;
    case ABBREV_REF:
        parse_literal(match, state);
        break;
    case QUANT_PLUS:
        parse_quant_plus(match, state);
        break;
    case QUANT_STAR:
        parse_quant_star(match, state);
        break;
    case QUANT_MAYBE:
        parse_quant_maybe(match, state);
        break;
    case ACTION:
        parse_action(match, state);
        break;
    case SUBTREE:
        parse_subtree(match, state);
        break;
    default:
        internal_error(state->filename, state->lineno,
                       "illegal match type %d", match->type);
        break;
    }
}

struct tree *parse(struct aug_file *file, const char *text,
                   FILE *log, int flags) {
    struct state state;

    state.filename = file->name;
    state.lineno = 1;
    state.text = text;
    state.pos = text;
    state.applied = 0;
    state.seqs  = NULL;
    state.key = NULL;
    state.tree = NULL;
    state.skel = NULL;
    state.dict = NULL;
    if (flags != PF_NONE && log != NULL) {
        state.flags = flags;
        state.log = log;
    } else {
        state.flags = PF_NONE;
        state.log = stdout;
    }
    parse_match(file->grammar->rules->matches, &state);
    if (! state.applied || *state.pos != '\0') {
        parse_error(&state, "parse did not read entire file");
        return NULL;
    }
    file->skel = state.skel;
    file->dict = state.dict;
    // FIXME: free state->seqs
    return state.tree;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
