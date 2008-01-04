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
    int         count;    /* number of iteration during '*'/'+' */
    char       *seq;      /* scratch for count as a string */
    char       *path;     /* current path */
    FILE       *log;
    struct aug_token *tokens;
    struct aug_token **stack;  /* stack of left most token for a rule */
    int        top;            /* top of stack */
};

/* Find the last token we've seen for field FID in rule RULE */
static struct aug_token *lookup_token(struct rule *rule, int fid, 
                                      struct state *state) {
    struct aug_token *token = NULL;

    list_for_each(t, state->stack[state->top]) {
        if (t->match->fid == fid && t->match->owner == rule)
            token = t;
    }
    return token;
}

/* Return value must be duped before putting into other structs */
static const char *lookup_value(struct entry *entry, struct state*state) {
    if (entry->type == E_CONST) {
        return entry->text;
    } else if (entry->type == E_GLOBAL) {
        if (STREQ("seq", entry->text)) {
            int len = snprintf(NULL, 0, "%d", state->count);
            state->seq = realloc(state->seq, len+1);
            snprintf(state->seq, len + 1, "%d", state->count);
            return state->seq;
        } else if (STREQ("basename", entry->text)) {
            const char *basnam = strrchr(state->filename, '/');
            if (basnam == NULL)
                basnam = state->filename;
            else
                basnam += 1;
            return basnam;
        } else {
            // Unknown global, shouldn't have made it to here
            return "global";
        }
    } else if (entry->type == E_FIELD) {
        if (state->top < 0) {
            internal_error(state->filename, state->lineno,
                           "empty rule stack");
        }
        struct aug_token *token = 
            lookup_token(entry->action->rule, entry->field, state);
        if (token == NULL) {
            parse_error(state,
                        "field %d referenced but not found during parsing",
                        entry->field);
            return "failure";
        } else {
            return token->text;
        }
    } else {
        internal_error(state->filename, state->lineno,
                       "illegal entry type %d", entry->type);
        return "illegal";
    }
}

static void push(struct entry *entry, struct state *state) {
    const char *path = lookup_value(entry, state);
    int size = strlen(state->path) + 1 + strlen(path) + 1;

    state->path = realloc(state->path, size);
    strcat(state->path, "/");
    strcat(state->path, path);
}

static void pop(struct state *state) {
    char *pos = strrchr(state->path, '/');
    if (pos == NULL) {
        internal_error(state->filename, state->lineno,
                       "pop from emtpy path");
    }
    *pos = '\0';
}

static void assign(struct entry *value, struct state *state) {
    struct aug_token *token;

    if (value == NULL)
        return;

    token = lookup_token(value->action->rule, value->field, state);
    if (token == NULL) {
        internal_error(state->filename, state->lineno,
                       "tried to assign to nonexistant field %d in rule %s\n",
                       value->field, value->action->rule->name);
        return;
    }
    if (token->node != NULL) {
        parse_error(state, "Token was already assigned to a node");
    }
    
    token->node = strdup(state->path);

    if (state->flags & PF_ACTION)
        fprintf(state->log, "assign %s = %s\n", token->node, token->text);
}

static void action_enter(struct match *match, struct state *state) {
    if (match->action == NULL)
        return;
    list_for_each(e, match->action->path) {
        push(e, state);
        if (state->flags & PF_ACTION)
            printf("enter  %s\n", state->path);
    }
}

static void action_exit(struct match *match, struct state *state) {
    if (match->action == NULL)
        return;

    assign(match->action->value, state);

    list_for_each(e, match->action->path) {
        if (state->flags & PF_ACTION)
            fprintf(state->log, "exit   %s\n", state->path);
        pop(state);
    }
}

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

    action_enter(match, state);

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

        token = aug_make_token(AUG_TOKEN_INERT, text, NULL);
        token->match = match;

        list_append(state->tokens, token);
        advance(state, len);
        state->applied = 1;

        if (state->stack[state->top] == NULL)
            state->stack[state->top] = token;
        
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

    action_exit(match, state);
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

    action_enter(match, state);
    list_for_each(p, match->matches) {
        if (applies(p, state)) {
            parse_match(p, state);
            state->applied = 1;
            break;
        }
    }
    if (! state->applied) {
        parse_error(state, "alternative failed\n");
    }
    action_exit(match, state);
}

static void parse_sequence(struct match *match, struct state *state) {
    state->applied = 1;

    action_enter(match, state);
    list_for_each(p, match->matches) {
        parse_match(p, state);
        if (! state->applied) {
            parse_error(state, "sequence failed\n");
            break;
        }
    }
    action_exit(match, state);
}

static void parse_field(struct match *match, struct state *state) {
    struct match *field = find_field(match->owner->matches, match->field);
    
    action_enter(match, state);
    parse_match(field, state);
    action_exit(match, state);
}

static void parse_rule(struct rule *rule, struct state *state) {
    state->top += 1;
    state->stack = realloc(state->stack, 
                           (state->top + 1) * sizeof(*(state->stack)));
    state->stack[state->top] = NULL;

    parse_match(rule->matches, state);
    if (state->flags & PF_RULE) {
        fprintf(state->log, "R %s:\n", rule->name);
        fprintf(state->log, "  ");
        list_for_each(t, state->stack[state->top]) {
            fprintf(state->log, "<%s[%d]=", t->match->owner->name,
                    t->match->fid);
            print_chars(state->log, t->text, strlen(t->text));
            fputc('>', state->log);
            if (t->next)
                fputc(' ', state->log);
        }
        fputc('\n', state->log);
    }
    state->top -= 1;
}

static void parse_rule_ref(struct match *match, struct state *state) {
    action_enter(match, state);
    parse_rule(match->rule, state);
    action_exit(match, state);
}

static void parse_quant_match(parse_match_func func,
                              struct match *match, struct state *state) {
    int oldcount = state->count;

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
        if (! applies(match, state)) {
            grammar_error(state->filename, state->lineno, 
                          "match did not apply");
        } else {
            state->count = 0;
            while (applies(match, state)) {
                (*func)(match, state);
                state->count++;
            }
            state->count = oldcount;
        }
        state->applied = 1;
        break;
    case Q_STAR:
        state->count = 0;
        while (applies(match, state)) {
            (*func)(match, state);
            state->count++;
        }
        state->count = oldcount;
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

int parse(struct grammar *grammar, struct aug_file *file, const char *text,
           FILE *log, int flags) {
    struct state state;

    state.filename = file->name;
    state.lineno = 1;
    state.text = text;
    state.pos = text;
    state.applied = 0;
    state.tokens = NULL;
    state.count  = 0;
    CALLOC(state.path, strlen(file->node) + 100);
    strcpy(state.path, file->node);
    CALLOC(state.seq, 10);
    CALLOC(state.stack, 10);
    state.top = -1;
    if (flags != PF_NONE && log != NULL) {
        state.flags = flags;
        state.log = log;
    } else {
        state.flags = PF_NONE;
        state.log = stdout;
    }
    parse_rule(grammar->rules, &state);
    file->tokens = state.tokens;
    if (! state.applied || *state.pos != '\0') {
        fprintf(log, "Parse failed\n");
        return -1;
    }
    return 0;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
