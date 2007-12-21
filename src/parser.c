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
    grammar_error(state->filename, state->lineno, format, ## args)

struct state {
    const char *filename;
    int         lineno;
    const char *text;
    const char *pos;
    int         applied;
    int         flags;    /* set of parse_flags */
    FILE       *log;
    struct aug_token *tokens;
};

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

typedef void (*parse_match_func)(struct match *, struct state *);

static void parse_match(struct match *match, struct state *state);

static void parse_literal(struct match *match, struct state *state) {
    struct literal *literal;
    int len;

    if (match->type == ABBREV_REF)
        literal = match->abbrev->literal;
    else if (match->type == LITERAL || match->type == ANY)
        literal = match->literal;
    else {
        internal_error(state->filename, state->lineno,
                       "Illegal match type %d for literal", match->type);
        state->applied = 0;
        return;
    }

    len = lex(literal, state);
    if (len < 0) {
        state->applied = 0;
    } else {
        struct aug_token *token = NULL;
        char *text = strndup(state->pos, len);

        token = aug_make_token(AUG_TOKEN_INERT, text, "/");

        list_append(state->tokens, token);
        advance(state, len);
        state->applied = 1;

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
            print_chars(state->log, token->text, strlen(token->text));
            fprintf(state->log, ">\n");
        }
    }
}

static int applies(struct match *match, struct state *state) {
    list_for_each(f, match->first) {
        if (lex(f->literal, state) > 0)
            return 1;
    }
    // FIXME: Something about follow sets
    return 0;
}

static void parse_alternative(struct match *match, struct state *state) {
    state->applied = 0;
    list_for_each(p, match->matches) {
        if (applies(p, state)) {
            parse_match(p, state);
            /* FIXME: Error if ! state->applied */
            state->applied = 1;
            return;
        }
    }
}

static void parse_sequence(struct match *match, struct state *state) {
    state->applied = 1;
    list_for_each(p, match->matches) {
        parse_match(p, state);
        if (! state->applied)
            return;   /* error */
    }
}

static void parse_field(struct match *match, struct state *state) {
    struct match *field = find_field(match->owner->matches, match->field);
    parse_match(field, state);
}

static void parse_rule(struct rule *rule, struct state *state) {
    if (state->flags & PF_RULE) {
        fprintf(state->log, "R %s:\n", rule->name);
    }
    parse_match(rule->matches, state);
}

static void parse_rule_ref(struct match *match, struct state *state) {
    parse_rule(match->rule, state);
}

static void parse_quant_match(parse_match_func func,
                              struct match *match, struct state *state) {
    switch (match->quant) {
    case Q_ONCE:
        (*func)(match, state);
        break;
    case Q_MAYBE:
        if (applies(match, state)) {
            (*func)(match, state);
        }
        state->applied = 1;
        break;
    case Q_PLUS:
        (*func)(match, state);
        if (! state->applied) {
            grammar_error(state->filename, state->lineno, 
                          "match did not apply");
        }
        while (state->applied)
            (*func)(match, state);
        state->applied = 1;
        break;
    case Q_STAR:
        while (applies(match, state)) {
            (*func)(match, state);
        }
        state->applied = 1;
        break;
    default:
        internal_error(_FM(match), _L(match), 
                       "illegal quant type %d", match->quant);
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
    case FIELD:
        parse_field(match, state);
        break;
    case ALTERNATIVE:
        parse_quant_match(parse_alternative, match, state);
        break;
    case SEQUENCE:
        parse_quant_match(parse_sequence, match, state);
        break;
    case RULE_REF:
        parse_quant_match(parse_rule_ref, match, state);
        break;
    case ABBREV_REF:
        parse_literal(match, state);
        break;
    default:
        internal_error(state->filename, state->lineno,
                       "illegal match type %d", match->type);
        break;
    }
}

void parse(struct grammar *grammar, const char *filename, const char *text,
           FILE *log, int flags) {
    struct state state;

    state.filename = filename;
    state.lineno = 1;
    state.text = text;
    state.pos = text;
    state.applied = 0;
    state.tokens = NULL;
    if (flags != PF_NONE && log != NULL) {
        state.flags = flags;
        state.log = log;
    } else {
        state.flags = PF_NONE;
        state.log = stdout;
    }
    parse_rule(grammar->rules, &state);
    if (! state.applied || *state.pos != '\0')
        fprintf(stderr, "Parse failed\n");
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
