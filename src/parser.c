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
    char       *seq;       /* scratch for count as a string */
    char       *path;      /* current path */
    FILE       *log;
    struct ast *ast;
    struct seq *seqs;
};

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

struct ast *make_ast(struct match *match) {
    struct ast *result;
    
    CALLOC(result, 1);
    result->match = match;

    return result;
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
        state->ast = make_ast(match);
        state->ast->token = strndup(state->pos, len);

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
            print_chars(state->log, state->ast->token, 
                        strlen(state->ast->token));
            fprintf(state->log, ">\n");
        }
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
    struct ast *ast = make_ast(match);

    state->applied = 0;

    list_for_each(p, match->matches) {
        if (applies(p, state)) {
            parse_match(p, state);
            list_append(ast->children, state->ast);
            state->applied = 1;
            break;
        }
    }
    state->ast = ast;
    if (! state->applied) {
        parse_expected_error(state, match);
    }
}

static void parse_sequence(struct match *match, struct state *state) {
    struct ast *ast = make_ast(match);

    state->applied = 1;
    list_for_each(p, match->matches) {
        state->ast = NULL;
        parse_match(p, state);
        if (! state->applied) {
            parse_expected_error(state, p);
            break;
        }
        if (state->ast != NULL)
            list_append(ast->children, state->ast);
    }
    state->ast = ast;
}

static void parse_rule_ref(struct match *match, struct state *state) {
    struct ast *ast = make_ast(match);

    parse_match(match->rule->matches, state);
    ast->children = state->ast;
    state->ast = ast;
}

static void parse_quant_star(struct match *match, struct state *state) {
    struct ast *ast = make_ast(match);
    while (applies(match, state)) {
        state->ast = NULL;
        parse_match(match->matches, state);
        list_append(ast->children, state->ast);
    }
    state->applied = 1;
    state->ast = ast;
}

static void parse_quant_plus(struct match *match, struct state *state) {
    if (! applies(match, state)) {
        grammar_error(state->filename, state->lineno, 
                      "match did not apply");
    } else {
        struct ast *ast = make_ast(match);
        while (applies(match, state)) {
            parse_match(match->matches, state);
            list_append(ast->children, state->ast);
        }
        state->ast = ast;
    }
    state->applied = 1;
}

static void parse_quant_maybe(struct match *match, struct state *state) {
    struct ast *ast = make_ast(match);
    if (applies(match, state)) {
        state->ast = NULL;
        parse_match(match->matches, state);
        list_append(ast->children, state->ast);
    }
    state->ast = ast;
    state->applied = 1;
}

static void parse_subtree(struct match *match, struct state *state) {
    struct ast *ast = make_ast(match);
    parse_match(match->matches, state);
    ast->children = state->ast;
    state->ast = ast;
}

static void parse_action(struct match *match, struct state *state) {
    enum action_type type = match->xaction->type;
    struct ast *ast = make_ast(match);

    if (type == STORE || type == KEY) {
        parse_match(match->xaction->arg, state);
        ast->children = state->ast;
    }
    state->ast = ast;
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

/*
 * Evaluation of actions
 */

static struct seq *get_counter(const char *name, struct state *state) {
    struct seq *seq;

    for (seq=state->seqs; 
         seq != NULL && ! STREQ(seq->name, name); 
         seq = seq->next);

    if (seq == NULL) {
        CALLOC(seq, 1);
        seq->name = name;
        list_append(state->seqs, seq);
    }

    return seq;
}

static const char *seq_next_value(struct match *arg, struct state *state) {
    const char *name = NULL;
    struct seq *seq = NULL;

    if (arg->type == LITERAL && arg->literal->type == QUOTED)
        name = arg->literal->text;
    assert(name != NULL);

    seq = get_counter(name, state);
    
    int len = snprintf(NULL, 0, "%d", seq->value);
    state->seq = realloc(state->seq, len+1);
    snprintf(state->seq, len + 1, "%d", seq->value);
    seq->value += 1;
    
    return state->seq;
}

static const char *token_value(struct ast *ast) {
    assert(! LEAF_P(ast));
    assert(LEAF_P(ast->children));
    assert(ast->children->next == NULL);
    return ast->children->token;
}

static const char *string_value(struct match *arg) {
    if (arg->type == LITERAL && arg->literal->type == QUOTED)
        return arg->literal->text;
    assert(0);
}

static int push(struct ast *ast, struct state *state) {
    struct match *match = ast->match;
    if (match->type == ACTION) {
        const char *value = NULL;

        if (match->xaction->type == SEQ)
            value = seq_next_value(match->xaction->arg, state);
        else if (match->xaction->type == KEY)
            value = token_value(ast);
        else if (match->xaction->type == LABEL)
            value = string_value(match->xaction->arg);

        if (value != NULL) {
            int size = strlen(state->path) + 1 + strlen(value) + 1;

            state->path = realloc(state->path, size);
            strcat(state->path, "/");
            strcat(state->path, value);
            return 1;
        }
        return 0;
    } else if (! LEAF_P(ast)) {
        int cnt = 0;
        list_for_each(c, ast->children) {
            if (c->match->type != SUBTREE)
                cnt += push(c, state);
        }
        assert(cnt <= 1);
        return cnt;
    }
    return 0;
}

static void pop(struct state *state) {
    char *pos = strrchr(state->path, '/');
    if (pos == NULL) {
        internal_error(state->filename, state->lineno,
                       "pop from emtpy path");
    }
    *pos = '\0';
}

const char *longest_prefix(struct ast *ast) {
    const char *result = NULL;

    list_for_each(c, ast) {
        // The path of a node is the longest prefix of the paths
        // of its children
        if (c->path != NULL) {
            if (result == NULL) {
                result = strdup(c->path);
            } else {
                while (! pathprefix(result, c->path)) {
                    char *parent = strrchr(result, SEP);
                    if (parent == NULL) {
                        internal_error(NULL, -1,
                           "Failed to find common prefix for %s and %s",
                                       ast->path, c->path);
                        break;
                    } else {
                        *parent = '\0';
                    }
                }
            }
        }
    }
    if (result != NULL)
        result = realloc((void *) result, strlen(result) + 1);
    return result;
}

static void eval(struct ast *ast, struct state *state) {
    if (ast->match->type == SUBTREE) {
        push(ast, state);
        ast->path = strdup(state->path);
        if (state->flags & PF_ACTION)
            printf("enter  %s\n", state->path);
    }
    if (ACTION_P(ast->match, COUNTER)) {
        const char *name = string_value(ast->match->xaction->arg);
        struct seq *seq = get_counter(name, state);
        seq->value = 0;
    }

    if (! LEAF_P(ast)) {
        /* Evaluate all the child nodes. If this node is a quantifier,
           we mark which child we are going into in state->symtab so
           that lookup_value can bind field references to teh right
           branch in the tree.
           For quantifiers, we also count each iteration so that $seq
           gets a new value in subtrees
        */

        list_for_each(c, ast->children) {
            eval(c, state);
        }
        if (ast->path == NULL) {
            ast->path = longest_prefix(ast->children);
        }
    }
    if (ACTION_P(ast->match, STORE)) {
        assert(ast->path == NULL);
        ast->path = strdup(state->path);
        if (state->flags & PF_ACTION) {
            fprintf(state->log, "assign %s = %s\n", ast->path, 
                    token_value(ast));
        }
    }

    if (ast->match->type == SUBTREE) {
        if (state->flags & PF_ACTION)
            fprintf(state->log, "exit   %s\n", state->path);
        pop(state);
    }
}

/*
 * Printing of AST
 */
static int ast_node(FILE *out, struct ast *ast, int parent, int next) {
    int self = next++;
    const char *name = "???";
    const char *value = LEAF_P(ast) ? "???" : "";

    switch(ast->match->type) {
    case LITERAL:
        name = "literal";
        value = ast->token;
        break;
    case NAME:
        name = "name";
        value = ast->match->name;
        break;
    case ANY:
        name = ast->match->epsilon ? "..?" : "...";
        value = ast->token;
        break;
    case ALTERNATIVE:
        name = "\\|";
        break;
    case SEQUENCE:
        name = ".";
        break;
    case RULE_REF:
        name = ast->match->rule->name;
        break;
    case ABBREV_REF:
        name = ast->match->abbrev->name;
        value = ast->token;
        break;
    case QUANT_PLUS:
        name = "+";
        break;
    case QUANT_STAR:
        name = "*";
        break;
    case QUANT_MAYBE:
        name = "?";
        break;
    case ACTION:
        name = ast->match->xaction->name;
        if (ACTION_P(ast->match, LABEL) || ACTION_P(ast->match, COUNTER)
            || ACTION_P(ast->match, SEQ)) {
            asprintf((char **) &name, "%s %s", ast->match->xaction->name, 
                     string_value(ast->match->xaction->arg));
        }
        break;
    case SUBTREE:
        name = "[]";
        break;
    default:
        name = "???";
        break;
    }
    fprintf(out, "n%d [\n", self);
    fprintf(out, "  label = \" %s | ", name);
    if (value == NULL)
        value = "\\<\\>";
    print_chars(out, value, strlen(value));
    fprintf(out, " | ");
    if (ast->path != NULL)
        fprintf(out, ast->path);
    fprintf(out, " | ");
    print_literal_set(out, ast->match->handle, ast->match->owner->grammar,
                      ' ', ' ');
    fprintf(out, "\"\n  shape = \"record\"\n");
    fprintf(out, "];\n");
    fprintf(out, "n%d -> n%d;\n", parent, self);

    if (! LEAF_P(ast)) {
        list_for_each(c, ast->children) {
            next = ast_node(out, c, self, next);
        }
    }
    return next;
}

void ast_dot(FILE *out, struct ast *ast, int flags) {
    if (!(flags & PF_AST))
        return;
    fprintf(out, "strict digraph ast {\n");
    fprintf(out, "  graph [ rankdir = \"LR\" ];\n");
    if (ast != NULL)
        ast_node(out, ast, 0, 1);
    fprintf(out, "}\n");
}

int parse(struct grammar *grammar, struct aug_file *file, const char *text,
           FILE *log, int flags) {
    struct state state;

    state.filename = file->name;
    state.lineno = 1;
    state.text = text;
    state.pos = text;
    state.applied = 0;
    state.seqs  = NULL;
    CALLOC(state.path, strlen(file->node) + 100);
    strcpy(state.path, file->node);
    CALLOC(state.seq, 10);
    if (flags != PF_NONE && log != NULL) {
        state.flags = flags;
        state.log = log;
    } else {
        state.flags = PF_NONE;
        state.log = stdout;
    }
    parse_match(grammar->rules->matches, &state);
    if (! state.applied || *state.pos != '\0') {
        parse_error(&state, "parse did not read entire file");
        return -1;
    }
    // FIXME: free state->seqs
    file->ast = state.ast;
    free((void *) file->ast->path);
    file->ast->path = strdup(file->node);
    strcpy(state.path, file->node);
    eval(file->ast, &state);
    ast_dot(log, file->ast, flags);
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
