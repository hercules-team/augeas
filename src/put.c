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

struct state {
    const char  *filename;
    FILE        *out;
    struct tree *tree;
    const char  *key;
    int         leaf;
    struct dict *dict;
    struct skel *skel;
};

static void create_match(struct match *match, struct state *state);
static void put_match(struct match *match, struct state *state);

#ifdef DEBUG_SPEW
#define _l(obj) (((obj) == NULL) ? "nil" : (obj)->label)

static const char *_t(struct match *match) {
    static const char *types[] = {
        "action", "[]", "literal", "name", "|", ".",
        "rule", "abbrev", "+", "*", "?"
    };
    return types[match->type];
}

#define debug(fmt, args ...) printf(fmt, ## args)
#else
#define debug(fmt, args ...)
#endif

static struct dict_entry *dict_lookup(const char *key, struct dict *dict) {
    struct dict *d;
    if (key == NULL) {
        for (d=dict; d != NULL && d->key != NULL; d = d->next);
    } else {
        for (d=dict; d != NULL && (d->key == NULL || STRNEQ(key, d->key));
             d = d->next);
    }
    if (d == NULL)
        return NULL;
    struct dict_entry *result = d->entry;
    if (result != NULL)
        d->entry = result->next;
    return result;
}

/* Return 1 if MATCH->HANDLE is appropriate for TREE */
static int handle_match(struct match *match, struct tree *tree) {
    int count;

    if (tree->label == NULL)
        return match->handle == NULL;

    list_for_each(lit, match->handle) {
        count = re_match(lit->literal->re, tree->label, strlen(tree->label),
                         0, NULL);
        if (count >= 1)
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

/*
 * Check whether SKEL has the skeleton type required by MATCH
 */
static int skel_instance_of(struct match *match, struct skel *skel) {
    if (skel == NULL)
        return 0;

    switch (match->type) {
    case ACTION:
        return skel->type == LITERAL;
    case SUBTREE:
        return skel->type == SUBTREE;
    case ABBREV_REF:
    case LITERAL:
        return skel->type == LITERAL;
    case NAME:
        return 0;
    case ALTERNATIVE:
        list_for_each(m, match->matches) {
            if (skel_instance_of(m, skel))
                return 1;
        }
        return 0;
    case SEQUENCE:
        if (skel->type != SEQUENCE)
            return 0;
        skel = skel->skels;
        list_for_each(m, match->matches) {
            if (! skel_instance_of(m, skel))
                return 0;
            skel = skel->next;
        }
        return 1;
    case RULE_REF:
        return skel_instance_of(match->rule->matches, skel);
    case QUANT_PLUS:
    case QUANT_STAR:
    case QUANT_MAYBE:
        if (skel->type != match->type)
            return 0;
        if (match->type == QUANT_MAYBE &&
            skel->skels != NULL && skel->skels->next != NULL)
            return 0;
        list_for_each(s, skel->skels) {
            if (! skel_instance_of(match->matches, s))
                return 0;
        }
        return 1;
    default:
        internal_error(NULL, -1,
                       "illegal match type %d", match->type);
        break;
    }
    return 0;
}

/*
 * put
 */
static void put_action(struct match *match, struct state *state) {
    assert(match->type == ACTION);
    struct action *action = match->action;

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
    // FIXME: Why ?
    if (state->tree == NULL) {
        if (! match->epsilon)
            internal_error(NULL, -1, "put_subtree with empty tree");
        return;
    }
    const char *key = state->key;
    struct dict *dict = state->dict;
    struct skel *skel = state->skel;

    state->key = state->tree->label;
    struct dict_entry *entry = dict_lookup(state->key, dict);
    struct tree *tree = state->tree;
    if (tree->children != NULL) {
        state->tree = tree->children;
    } else {
        // FIXME: state->leaf == 1 means the tree is too flat
        assert(! state->leaf);
        state->leaf = 1;
    }
    if (entry == NULL)
        create_match(match->matches, state);
    else {
        state->skel = entry->skel;
        state->dict = entry->dict;
        put_match(match->matches, state);
    }
    state->key = key;
    state->dict = dict;
    state->skel = skel;
    state->leaf = 0;
}

static void put_literal(struct match *match, struct state *state) {
    assert(match->type == LITERAL || match->type == ABBREV_REF);
    assert(state->skel != NULL);
    assert(state->skel->type == LITERAL);
    fprintf(state->out, state->skel->text);
}

static void put_alternative(struct match *match, struct state *state) {
    assert(match->type == ALTERNATIVE);

    list_for_each(m, match->matches) {
        if (handle_match(m, state->tree)) {
            if (skel_instance_of(m, state->skel))
                put_match(m, state);
            else
                create_match(m, state);
            return;
        }
    }
    assert(0);
}

/* Free the list of trees created by split_tree */
static void free_split(struct tree *tree) {
    while (tree != NULL) {
        struct tree *del = tree;
        tree = del->next;
        free(del);
    }
}

/* Given a tree split SPLIT, return the first entry with non-NULL
   children, and free the rest */
static struct tree *split_tree_car(struct tree *split) {
    if (split == NULL)
        return NULL;
    
    struct tree *result;
    for (result = split;
         result != NULL && result->children == NULL;
         result = result->next);
    if (result == NULL)
        result = split;
    list_remove(result, split);
    result->next = NULL;
    free_split(split);
    return result;
}

/*
 * Split a list of trees into lists of trees to undo the fact that
 * sequence and the quantifiers all concatenate lists of trees.
 */
static struct tree *split_tree_int(struct match *match,
                                   struct state *state) {
    /* The goal is to reorganize the flat list TREES into a list of lists
       of trees, e.g. something like [t1; t2; t3] might get turned into
       [[]; [t1]; []; [t2; t3]] The exact shape of the resulting list
       depends on MATCH.
     */
    struct tree *result = NULL;
    switch (match->type) {
    case ACTION:
        CALLOC(result, 1);
        if (match->action->type == STORE) {
            if (state->tree == NULL)
                goto error;
            result->children = state->tree;
            state->tree = (state->tree)->next;
        }
        return result;
    case SUBTREE:
        if (state->leaf || ! handle_match(match, state->tree))
            return NULL;
        CALLOC(result, 1);
        if (state->tree == NULL)
            goto error;
        result->children = state->tree;
        state->tree = (state->tree)->next;
        return result;
    case ABBREV_REF:
    case LITERAL:
        CALLOC(result, 1);
        return result;
    case ALTERNATIVE:
        list_for_each(m, match->matches) {
            if (((state->tree)->label == NULL) != (m->handle == NULL))
                continue;
            result = split_tree_car(split_tree_int(m, state));
            if (result != NULL)
                return result;
        }
        return NULL;
    case SEQUENCE:
        list_for_each(m, match->matches) {
            struct tree *t = split_tree_car(split_tree_int(m, state));
            if (t == NULL) {
                goto error;
            }
            list_append(result, t);
        }
        return result;
    case RULE_REF:
        return split_tree_int(match->rule->matches, state);
    case QUANT_PLUS:
    case QUANT_STAR:
        // FIXME: THis is greedy in a really stupid way
        // It will preclude us from ever splitting a*.a properly
        while (state->tree != NULL) {
            struct tree *t = split_tree_car(split_tree_int(match->matches, state));
            if (t == NULL) {
                if (result == NULL && match->type != QUANT_PLUS)
                    CALLOC(result, 1);
                return result;
            }
            list_append(result, t);
        }
        if (result == NULL && match->type != QUANT_PLUS)
            CALLOC(result, 1);
        return result;
    case QUANT_MAYBE:
        if (state->tree != NULL) {
            result = split_tree_car(split_tree_int(match->matches, state));
        }
        if (result == NULL)
            CALLOC(result, 1);
        return result;
    default:
        internal_error(NULL, -1,
                       "illegal match type %d", match->type);
        break;
    }
 error:
    free_split(result);
    return NULL;
}

static struct tree *split_tree(struct match *match, struct state *state) {
    struct tree *tree = state->tree;
#ifdef DEBUG_SPEW
    printf("SPLIT_TREE %s:%d %c ", _t(match), match->lineno,
           state->leaf ? 'L' : 'i');
    list_for_each(t, state->tree) {
        printf("%s ", t->label);
    }
    printf("\n");
#endif
    struct tree *split = split_tree_int(match, state);
#ifdef DEBUG_SPEW
    printf("IS ");
    list_for_each(s, split) {
        printf("[");
        list_for_each(t, s->children) {
            if (s->next != NULL && t == s->next->children)
                break;
            printf("%s ", t->label);
        }
        printf("] ");
    }
    printf("\n");
#endif
    state->tree = tree;
    return split;
}

static void put_sequence(struct match *match, struct state *state) {
    assert(match->type == SEQUENCE);
    assert(state->skel->match == match);
    struct tree *tree = state->tree;
    struct tree *split = split_tree(match, state);
    struct tree *smark = split;
    struct skel *skel = state->skel;

    state->skel = skel->skels;
    list_for_each(m, match->matches) {
        if (split == NULL) {
            fprintf(stderr, "Short split on line %d for sequence %d\n",
                    skel->lineno, match->lineno);
            free_split(smark);
            return;
        }
        state->tree = split->children;
        put_match(m, state);
        split = split->next;
        state->skel = state->skel->next;
    }
    state->tree = tree;
    state->skel = skel;
    free_split(smark);
}

static void put_rule_ref(struct match *match, struct state *state) {
    assert(match->type == RULE_REF);
    put_match(match->rule->matches, state);
}

static void put_quant_plus(struct match *match, struct state *state) {
    assert(match->type == QUANT_PLUS);
    assert(state != NULL);
    TODO;
}

static void put_quant_star(struct match *match, struct state *state) {
    assert(match->type == QUANT_STAR);
    assert(state->skel->match == match);
    struct tree *tree = state->tree;
    struct tree *split = split_tree(match, state);
    struct tree *smark = split;
    struct skel *skel = state->skel;

    state->skel = skel->skels;
    while (split != NULL) {
        // FIXME: Why ?
        if (split->children == NULL) {
            split = split->next;
            continue;
        }
        state->tree = split->children;
        if (state->skel == NULL) {
            create_match(match->matches, state);
        } else {
            put_match(match->matches, state);
        }
        split = split->next;
        if (state->skel != NULL)
            state->skel = state->skel->next;
    }
    state->skel = skel;
    state->tree = tree;
    free_split(smark);
}

static void put_quant_maybe(struct match *match, struct state *state) {
    assert(match->type == QUANT_MAYBE);
    assert(state->skel->match == match);
    struct tree *tree = state->tree;
    struct tree *split = split_tree(match, state);
    struct tree *smark = split;
    struct skel *skel = state->skel;

    state->skel = skel->skels;
    if (split != NULL && split->children != NULL) {
        state->tree = split->children;
        if (state->skel == NULL) {
            create_match(match->matches, state);
        } else {
            put_match(match->matches, state);
        }
    }
    state->skel = skel;
    state->tree = tree;
    free_split(smark);
}

static void put_match(struct match *match, struct state *state) {
    debug("put_match: %s:%d %s\n", _t(match), match->lineno, _l(state->tree));
    switch(match->type) {
    case LITERAL:
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
    assert(match->type == LITERAL || match->type == ABBREV_REF);

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
    struct tree *tree = state->tree;
    struct tree *split = split_tree(match, state);
    struct tree *smark = split;

    list_for_each(m, match->matches) {
        if (split == NULL) {
            fprintf(stderr, "Short split for sequence %d\n",
                    match->lineno);
            free_split(smark);
            return;
        }
        state->tree = split->children;
        create_match(m, state);
        split = split->next;
    }
    state->tree = tree;
    free_split(smark);
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
    struct tree *tree = state->tree;
    struct tree *split = split_tree(match, state);
    struct tree *smark = split;

    while (split != NULL) {
        // FIXME: Why ?
        if (split->children == NULL) {
            split = split->next;
            continue;
        }
        state->tree = split->children;
        create_match(match->matches, state);
        split = split->next;
    }
    state->tree = tree;
    free_split(smark);
}

static void create_quant_maybe(struct match *match, struct state *state) {
    assert(match->type == QUANT_MAYBE);
    struct tree *tree = state->tree;
    struct tree *split = split_tree(match, state);
    struct tree *smark = split;

    if (split != NULL && split->children != NULL) {
        state->tree = split->children;
        create_match(match->matches, state);
    }
    state->tree = tree;
    free_split(smark);
}

static void create_match(struct match *match, struct state *state) {
    debug("create_match: %s:%d %s\n", _t(match), match->lineno,
          _l(state->tree));
    switch(match->type) {
    case LITERAL:
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

static void dict_revert(struct dict *dict) {
    if (dict == NULL)
        return;
    dict->entry = dict->mark;
    list_for_each(e, dict->entry) {
        dict_revert(e->dict);
    }
}

void put(FILE *out, struct tree *tree, struct aug_file *file) {
    struct state state;

    if (tree == NULL)
        return;

    dict_revert(file->dict);

    state.filename = "stderr";
    state.out = out;
    state.skel = file->skel;
    state.dict = file->dict;
    state.tree = tree;
    state.leaf = 0;
    state.key = tree->label;
    put_match(file->grammar->rules->matches, &state);
}

/*
  Cleverly created with Elisp:
(defun my-ins-put ()
  (interactive)
  (insert "// BEGIN\n")
  (let
      ((types '("ACTION" "SUBTREE" "LITERAL" "NAME" "ALTERNATIVE" 
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
