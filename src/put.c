/*
 * put.c: 
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

#include "ast.h"

//#define DEBUG_SPEW

// FIXME: Should be in a global header
#define NMATCHES 100

struct state {
    const char  *filename;
    FILE        *out;
    struct ast  *ast;
    struct ast  *dict;
    struct tree *tree;
    const char  *key;
    int         leaf;
};

static void create_match(struct match *match, struct state *state);
static void put_match(struct match *match, struct state *state);

#ifdef DEBUG_SPEW
#define _p(obj) (((obj) == NULL) ? "nil" : (obj)->path)

static const char *_t(struct match *match) {
    static const char *types[] = {
        "action", "[]", "literal", "name", "...", "|", ".",
        "rule", "abbrev", "+", "*", "?"
    };
    return types[match->type];
}

#define debug(fmt, args ...) printf(fmt, ## args)
#else
#define debug(fmt, args ...)
#endif

static struct ast *dict_lookup(struct ast *dict, struct state *state) {
    // Treat AST as a dict keyed on tree->label
    if (dict->match->type != SUBTREE) {
        if (! LEAF_P(dict)) {
            list_for_each(c, dict->children) {
                struct ast *d = dict_lookup(c, state);
                if (d != NULL)
                    return d;
            }
        }
    } else {
        FIXME("********* Dict lookup broken");
        //if (dict->path != NULL && STREQ(dict->path, state->tree->path))
        //    return dict;
    }
    return NULL;
}

/* Return 1 if MATCH->HANDLE is appropriate for TREE */ 
static int handle_match(struct match *match, struct tree *tree) {
    int rc;
    int ovec[NMATCHES];

    list_for_each(lit, match->handle) {
        rc = pcre_exec(lit->literal->re, NULL, tree->label, strlen(tree->label),
                       0, PCRE_ANCHORED, ovec, NMATCHES);
        if (rc >= 1)
            return 1;
    }
    return 0;
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

static void put_action(struct match *match, struct state *state) {
    assert(match->type == ACTION);
    struct action *action = match->xaction;

    switch (action->type) {
    case COUNTER:
    case SEQ:
    case LABEL:
        // Nothing to do
        break;
    case STORE:
        fprintf(state->out, state->tree->value);
        break;
    case KEY:
        fprintf(state->out, state->key);
        break;
    default:
        assert(0);
        break;
    }
}

static void put_subtree(struct match *match, struct state *state) {
    assert(match->type == SUBTREE);
    if (state->tree == NULL) {
        if (! match->epsilon)
            internal_error(NULL, -1, "put_subtree with empty tree");
        return;
    }
    const char *key = state->key;
    state->key = state->tree->label;
    struct ast *dict = dict_lookup(state->dict, state);
    debug("dict_lookup: %s %s %s\n", _p(state->tree),
           _p(state->dict), _p(dict));
    struct tree *tree = state->tree;
    if (tree->children != NULL) {
        state->tree = tree->children;
    } else {
        assert(! state->leaf);
        state->leaf = 1;
    }
    if (dict == NULL)
        create_match(match->matches, state);
    else {
        struct ast *ast = state->ast;
        struct ast *d = state->dict;
        state->ast = dict->children;
        state->dict = state->ast;
        put_match(match->matches, state);
        state->ast = ast;
        state->dict = d;
    }
    state->tree = tree->next;
    state->key = key;
    state->leaf = 0;
}

static void put_literal(struct match *match, struct state *state) {
    assert(match->type == LITERAL || match->type == ANY
           || match->type == ABBREV_REF);
    assert(match == state->ast->match);
    fprintf(state->out, state->ast->token);
}

static void put_alternative(struct match *match, struct state *state) {
    assert(match->type == ALTERNATIVE);
    assert(state->ast->match == match);
    struct ast *ast = state->ast;

    state->ast = ast->children;
    if (state->ast->path == NULL) {
        put_match(state->ast->match, state);
        goto done;
    } else {
        list_for_each(m, match->matches) {
            if (handle_match(m, state->tree)) {
                if (state->ast->match == m)
                    put_match(m, state);
                else
                    create_match(m, state);
                goto done;
            }
        }
    }
    assert(0);
 done:
    state->ast = ast;
}

static void put_sequence(struct match *match, struct state *state) {
    assert(match->type == SEQUENCE);
    assert(state->ast->match == match);
    struct match *m = match->matches;
    struct ast   *ast = state->ast;

    state->ast = ast->children;
    while (m != NULL) {
        put_match(m, state);
        m = m->next;
        //if (state->ast->path != NULL)
        //    state->tree = state->tree->next;
        state->ast = state->ast->next;
    }
    state->ast = ast;
}

static void put_rule_ref(struct match *match, struct state *state) {
    assert(match->type == RULE_REF);
    assert(match == state->ast->match);
    struct ast *ast = state->ast;
    state->ast = ast->children;
    put_match(match->rule->matches, state);
    state->ast = ast;
}

static void put_quant_plus(struct match *match, struct state *state) {
    assert(match->type == QUANT_PLUS);
    assert(state != NULL);
    TODO;
}

static void put_quant_star(struct match *match, struct state *state) {
    assert(match->type == QUANT_STAR);
    struct ast *ast = state->ast;
    struct ast *c = ast->children;

    while (state->tree != NULL && handle_match(match, state->tree)) {
        if (c != NULL) {
            state->ast = c;
            put_match(match->matches, state);
            //if (c->path != NULL && state->tree != NULL)
            //    state->tree = state->tree->next;
            c = c->next;
        } else {
            create_match(match->matches, state);
            //state->tree = state->tree->next;
        }
    }
    state->ast = ast;
}

static void put_quant_maybe(struct match *match, ATTRIBUTE_UNUSED struct state *state) {
    assert(match->type == QUANT_MAYBE);
    TODO;
}

static void put_match(struct match *match, struct state *state) {
    debug("put_match: %s %s %s %s\n", _t(match), _p(state->tree),
          _p(state->ast), (state->ast != NULL) ? _t(state->ast->match) : "x");
    switch(match->type) {
    case LITERAL:
        put_literal(match, state);
        break;
    case ANY:
        put_literal(match, state);
        break;
    case ALTERNATIVE:
        put_alternative(match, state);
        break;
    case SEQUENCE:
        put_sequence(match, state);
        break;
    case RULE_REF:
        put_rule_ref(match, state);
        break;
    case ABBREV_REF:
        put_literal(match, state);
        break;
    case QUANT_PLUS:
        put_quant_plus(match, state);
        break;
    case QUANT_STAR:
        put_quant_star(match, state);
        break;
    case QUANT_MAYBE:
        put_quant_maybe(match, state);
        break;
    case ACTION:
        put_action(match, state);
        break;
    case SUBTREE:
        put_subtree(match, state);
        break;
    default:
        internal_error(NULL, -1,
                       "illegal match type %d", match->type);
        break;
    }
}

static void create_action(struct match *match, struct state *state) {
    assert(match->type == ACTION);
    put_action(match, state);
}

static void create_subtree(struct match *match, struct state *state) {
    put_subtree(match, state);
}

static void create_literal(struct match *match, struct state *state) {
    assert(match->type == LITERAL|| match->type == ANY
           || match->type == ABBREV_REF);

    struct literal *literal = NULL;
    if (match->type == ABBREV_REF) {
        literal = match->abbrev->literal;
    } else if (match->type == LITERAL) {
        literal = match->literal;
    } else {
        internal_error(_FM(match), _L(match),
                       "illegal match type '%d'", match->type);
    }
    if (literal != NULL) {
        print_escaped_chars(state->out, literal->text);
    }
}

static void create_alternative(struct match *match, struct state *state) {
    assert(match->type == ALTERNATIVE);

    list_for_each(m, match->matches) {
        if (handle_match(m, state->tree)) {
            create_match(m, state);
            return;
        }
    }
    assert(0);
}

static void create_sequence(struct match *match, struct state *state) {
    assert(match->type == SEQUENCE);
    list_for_each(m, match->matches) {
        create_match(m, state);
    }
}

static void create_rule_ref(struct match *match, ATTRIBUTE_UNUSED struct state *state) {
    assert(match->type == RULE_REF);
    create_match(match->rule->matches, state);
}

static void create_quant_plus(struct match *match, ATTRIBUTE_UNUSED struct state *state) {
    assert(match->type == QUANT_PLUS);
    TODO;
}

static void create_quant_star(struct match *match, struct state *state) {
    assert(match->type == QUANT_STAR);

    while (state->tree != NULL && handle_match(match, state->tree)) {
        create_match(match->matches, state);
        //state->tree = state->tree->next;
    }
}

static void create_quant_maybe(struct match *match, ATTRIBUTE_UNUSED struct state *state) {
    assert(match->type == QUANT_MAYBE);
    TODO;
}

static void create_match(struct match *match, struct state *state) {
    debug("create_match: %s %s\n", _t(match), _p(state->tree));
    switch(match->type) {
    case LITERAL:
        create_literal(match, state);
        break;
    case ANY:
        create_literal(match, state);
        break;
    case ALTERNATIVE:
        create_alternative(match, state);
        break;
    case SEQUENCE:
        create_sequence(match, state);
        break;
    case RULE_REF:
        create_rule_ref(match, state);
        break;
    case ABBREV_REF:
        create_literal(match, state);
        break;
    case QUANT_PLUS:
        create_quant_plus(match, state);
        break;
    case QUANT_STAR:
        create_quant_star(match, state);
        break;
    case QUANT_MAYBE:
        create_quant_maybe(match, state);
        break;
    case ACTION:
        create_action(match, state);
        break;
    case SUBTREE:
        create_subtree(match, state);
        break;
    default:
        internal_error(NULL, -1,
                       "illegal match type %d", match->type);
        break;
    }
}

void put(FILE *out, struct tree *tree, struct ast *ast) {
    struct grammar *grammar = ast->match->owner->grammar;
    struct state state;

    if (tree == NULL)
        return;

    state.filename = "stderr";
    state.out = out;
    state.ast = ast;
    state.dict = state.ast;
    state.tree = tree->children; // FIXME: Why ?
    state.leaf = 0;
    state.key = tree->label;
    put_match(grammar->rules->matches, &state);
}

/*
  Cleverly created with Elisp:
(defun my-ins-put ()
  (interactive)
  (insert "// BEGIN\n")
  (let
      ((types '("ACTION" "SUBTREE" "LITERAL" "NAME" "ANY" "ALTERNATIVE" 
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
