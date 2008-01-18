/*
 * emit.c: output a tree back into a config file
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


/*
 * Turning (a part of) the tree back into a config file needs to make sure
 * that we only produce config files that are syntactically correct. That
 * means that the grammar has to be used as a guide to how output should be
 * produced.
 *
 * When the time comes to store tree changes, the AST is first changed to
 * reflect those tree changes. First, deletions in the config tree are
 * spotted by making a complete pass over the AST and deleting anything
 * from the AST that is not in the config tree anymore. Second, a pass is
 * made over the config tree to find any entries that have been added or
 * changed. For each added entry, the AST is expanded until we have a leaf
 * corresponding to that entry in the config tree.
 */

#include "ast.h"
#include "augeas.h"
#include "list.h"

// FIXME: Should be in a global header
#define NMATCHES 100

/* Free an entire AST */
static void ast_delete(struct ast *ast) {
    if (ast == NULL)
        return;

    if (! LEAF_P(ast)) {
        while (ast->children != NULL) {
            struct ast *del = ast->children;
            ast->children = del->next;
            ast_delete(del);
        }
    } else {
        safe_free((void *) ast->token);
    }
    safe_free((void *) ast->path);
    safe_free(ast);
}

/* Delete AST and its subtree if ast->path does not exist in the
   config tree.
 */
static int do_deletion(struct ast **astp) {
    int changed = 0;
    struct ast *ast = *astp;

    if (ast->path != NULL && ! aug_exists(ast->path)) {
        ast_delete(ast);
        *astp = NULL;
        return 1;
    }

    if (!LEAF_P(ast)) {
        struct ast *next = ast->children->next;
        while (ast->children && do_deletion(&(ast->children))) {
            changed = 1;
            if (ast->children == NULL) {
                ast->children = next;
                next = ast->children->next;
            }
        }
        if (ast->children != NULL) {
            struct ast *c = ast->children;
            while (c->next != NULL) {
                next = c->next->next;
                if (do_deletion(&(c->next))) {
                    changed = 1;
                    if (c->next == NULL)
                        c->next = next;
                } else {
                    c = c->next;
                }
            }
        }
    }
    
    if (ast->children == NULL) {
        ast_delete(ast);
        *astp = NULL;
        changed = 1;
    }

    return changed;
}

/* Find the node in AST that has an instruction to enter PATH. */
static struct ast *ast_find(struct ast *ast, const char *path) {
    if (ast->match->action != NULL && ast->match->action->path != NULL
        && STREQ(ast->path, path))
        return ast;
    if (LEAF_P(ast))
        return NULL;
    list_for_each(c, ast->children) {
        if (c->path != NULL && pathprefix(c->path, path))
            return ast_find(c, path);
    }
    return NULL;
}

static int store_value(struct ast *ast, const char *path) {
    int changed = 0;

    // FIXME: The connection between the ast node receiving the value and 
    // the node to wich the action->value is attached is not right. When
    // we see an action->value, we need to find the AST node corresponding
    // to that field and assign to that
    // Alternatively, we could transform the grammar so that action->value
    // is always attached to the targetted match; that would be closer
    // to a store(RE) in the language
    if (LEAF_P(ast)) {
        if (ast->match->action != NULL && 
            ast->match->action->value != NULL
            && ast->path != NULL && STREQ(ast->path, path)) {
            const char *value = aug_get(path);
            if (value == NULL) {
                if (ast->token != NULL) {
                    safe_free((void *) ast->token);
                    ast->token = NULL;
                    changed = 1;
                }
            } else {
                if (ast->token == NULL || STRNEQ(ast->token, value)) {
                    safe_free((void *) ast->token);
                    ast->token = strdup(value);
                    changed = 1;
                }
            }
        }
    } else {
        // FIXME: This could be shortcircuited; once the value has been
        // stored, there's no need to go over the rest of the tree
        list_for_each(c, ast->children) {
            if (store_value(c, path))
                changed = 1;
        }
    }

    return changed;
}

// Find the most appropriate match amongst MATCHES that leads to
// REST
static struct match *find_handle_match(struct match *matches, 
                                       const char *rest) {
    int rc;
    int ovec[NMATCHES];
    
    if (rest == NULL)
        return NULL;

    list_for_each(m, matches) {
        list_for_each(lit, m->handle) {
            rc = pcre_exec(lit->literal->re, NULL, rest, strlen(rest),
                           0, PCRE_ANCHORED, ovec, NMATCHES);
            if (rc >= 1)
                return m;
        }
    }
    return NULL;
}

static struct ast *ast_expand(struct match *match, const char *path, 
                              const char *rest);

/* Generate an AST for REST iff one of match->matches indicates that
   that is needed. SUBMTAHC_P(match) must be true
 */
static struct ast *ast_expand_maybe(struct match *match, const char *path, 
                                    const char *rest) {
    struct ast *result = NULL;
    struct match *m = find_handle_match(match->matches, rest);
    /* m could be NULL, either because the tree doesn't generate
       syntactically correct output, or because we will later insert
       more nodes from the tree that will make the AST syntactically 
       correct.
    */
    if (m != NULL) {
        struct ast *c = ast_expand(m, path, rest);
        if (c != NULL) {
            result = make_ast(match);
            result->children = c;
        }
    }
    
    return result;
}

/* Generate an AST starting at MATCH that leads to the leaf for PATH. The
   returned AST may not be syntactically correct, but future inserts into
   the AST may make it so. REST is the suffix of PATH that lead to MATCH
   being chosen as the match to expand.
 */
static struct ast *ast_expand(struct match *match, const char *path, 
                              const char *rest) {
    struct ast *result = NULL;

    // For each e in match->action->path: remove one prefix from rest
    if (rest != NULL && rest[0] == SEP)
        rest += 1;

    if (match->action != NULL && rest != NULL) {
        list_for_each(e, match->action->path) {
            char *s = strchr(rest, SEP);
            if (s == NULL) {
                rest = NULL;
                break;
            }
            rest = s + 1;
        }
    }
    if (rest != NULL && rest[0] == '\0')
        rest = NULL;

    switch(match->type) {
    case ALTERNATIVE:
        result = ast_expand_maybe(match, path, rest);
        break;
    case SEQUENCE:
        {
            struct match *mrest = find_handle_match(match->matches, rest);
            list_for_each(m, match->matches) {
                /* Only generate an AST for children that either
                   do not belong to their own subtree or for the
                   child that we are currently going to */
                if (m != mrest && m->handle != NULL)
                    continue;
                struct ast *c = ast_expand(m, path, rest);
                if (c != NULL) {
                    if (result == NULL)
                        result = make_ast(match);
                    list_append(result->children, c);
                }
            }
        }
        break;
    case RULE_REF:
        {
            struct ast *c = ast_expand(match->rule->matches, path, rest);
            if (c != NULL) {
                result = make_ast(match);
                result->children = c;
            }
        }
        break;
    case QUANT_PLUS:
    case QUANT_STAR:
    case QUANT_MAYBE:
        result = ast_expand_maybe(match, path, rest);
        break;
    case LITERAL:
    case ANY:
    case FIELD:
    case ABBREV_REF:
        result = make_ast(match);
        break;
    default:
        internal_error(_FM(match), _L(match),
                       "Unexpected item %d\n", match->type);
        break;
    }

    if (result != NULL) {
        if (match->action != NULL && match->action->path != NULL) {
            int len = strlen(path);
            if (rest != NULL)
                len -= strlen(rest);
            result->path = calloc(len+1, sizeof(char));
            strncpy((char *) result->path, path, len);
        } else {
            if (! LEAF_P(result))
                result->path = longest_prefix(result->children);
        }
    }
    return result;
}

static void ast_insert(struct ast *ast, const char *path) {
    int moved;

    do {
        moved = 0;
        if (! LEAF_P(ast)) {
            list_for_each(c, ast->children) {
                if (c->path != NULL && pathprefix(c->path, path)) {
                    ast = c;
                    moved = 1;
                    break;
                }
            }
        }
    } while (moved);

    if (LEAF_P(ast)) {
        /* Should never get here */
        internal_error(NULL, -1, "Unexpected leaf");
        return;
    }
    
    
    struct match *match = ast->match;
    struct ast *sub = NULL;
    struct match *child;
    const char *rest = path + strlen(ast->path);
    if (*rest == SEP)
        rest += 1;
    if (*rest == '\0') {
        // FIXME: This really means that PATH can't be generated
        // from the grammar. And since we only try to insert
        // paths that have values assigned to them, the user put
        // something in the tree that will vanish when the tree is
        // saved
        internal_error(NULL, -1, "Exhausted path %s", path);
        return;
    }

    switch(match->type) {
    case SEQUENCE:
        child = find_handle_match(match->matches, rest);
        // FIXME: child == NULL can indicate that we don't find a handle
        // for rest either because match->matches all need a handle that
        // is longer than rest (i.e. children of rest) or because rest
        // is genuinely invalid
        if (child != NULL) {
            sub = ast_expand(child, path, rest);
            if (sub != NULL) {
                if (sub->next != NULL) {
                    internal_error(NULL, -1, 
                                   "Subtree has too many children");
                } else {
                    list_for_each(c, ast->children) {
                        if (c->match->next == sub->match) {
                            sub->next = c->next;
                            c->next = sub;
                        }
                    }
                }
            }
        }
        break;
    case QUANT_STAR:
    case QUANT_PLUS:
        sub = ast_expand(match->matches, path, rest);
        if (sub == NULL) {
            internal_error(NULL, -1,
                           "Path %s with rest %s can not be generated", 
                           path, rest);
            break;
        }
        list_append(ast->children, sub);
        break;
    default:
        internal_error(NULL, -1, 
       "Strange insertion point %d for path %s and rest %s", match->type, 
                       path, rest);
        break;
    }
}

/*  FIXME: We fundamentally assume that subtrees in the config tree
    correspond to subtrees in the AST, which is not necessarily true.
    We either need to change the language for grammars to enforce that 
    or check for that when constructing the grammar (preferrable) or the
    AST */
static int do_insertion(struct ast *ast, const char *path) {
    int changed = 0;
    const char **children;
    struct ast *sub = ast;
    int nchildren;

    if (aug_get(path) != NULL) {
        sub = ast_find(ast, path);
        if (sub == NULL) {
            changed = 1;
            ast_insert(ast, path);
            sub = ast;
        }
    }

    nchildren = aug_ls(path, &children);
    for (int i=0; i < nchildren; i++) {
        if (do_insertion(sub, children[i]))
            changed = 1;
    }
    
    if (store_value(ast, path))
        changed = 1;

    return changed;
}

static void emit_escaped_chars(FILE *out, const char *text) {
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

int ast_sync(struct ast **ast) {
    const char *root = (*ast)->path;
    int changed = 0;

    if (do_deletion(ast))
        changed = 1;
    if (*ast != NULL && do_insertion(*ast, root))
        changed = 1;

    return changed;
}

void ast_emit(FILE *out, struct ast *ast) {
    if (LEAF_P(ast)) {
        if (ast->token != NULL) {
            fprintf(out, ast->token);
        } else {
            struct literal *literal = NULL;
            if (ast->match->type == ABBREV_REF) {
                literal = ast->match->abbrev->literal;
            } else if (ast->match->type == LITERAL) {
                literal = ast->match->literal;
            } else {
                internal_error(NULL, -1,
                               "illegal match type %d", ast->match->type);
            }
            if (literal != NULL) {
                emit_escaped_chars(out, literal->text);
            } else {
                internal_error(_FM(ast->match), _L(ast->match),
                               "literal is missing a default");
            }
        }
    } else {
        list_for_each(c, ast->children) {
            ast_emit(out, c);
        }
    }
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
