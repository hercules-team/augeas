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


#include "ast.h"
#include "augeas.h"
#include "list.h"

// FIXME: Should be in a global header
#define NMATCHES 100

static const char *_t(struct match *match) {
    static const char *types[] = { 
        "literal", "name", "any", "field", "alt", "seq",
        "rule", "abbrev", "plus", "star", "maybe"
    };
    return types[match->type];
}

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
        free((void *) ast->token);
    }
    free((void *) ast->path);
    free(ast);
}

// Find the most appropriate match amongst MATCHES that leads to
// REST
static struct match *find_handle_match(struct match *matches, 
                                       const char *rest) {
    int rc;
    int ovec[NMATCHES];
    
    if (rest == NULL)
        rest = "";

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

static void synthesize_path(struct ast *ast) {
    free((void *) ast->path);
    ast->path = longest_prefix(ast->children);
    if (is_seq_iter(ast->match)) {
        int cnt = 0;
        list_for_each(c, ast->children) {
            if (ast->path != NULL)
                cnt += 1;
        }
        if (cnt == 1) {
            char *sep = strrchr(ast->path, SEP);
            *sep = '\0';
        }
    }
}

static int set_field_value(struct ast *ast, struct action *action,
                            const char *value) {
    if (LEAF_P(ast)) {
        if (ast->match->owner == action->rule 
            && ast->match->fid == action->path->field) {
            free((void*) ast->token);
            ast->token = strdup(value);
            return 1;
        }
    } else {
        list_for_each(c, ast->children) {
            if (set_field_value(c, action, value))
                return 1;
        }
    }
    return 0;
}

static void store_field_value(struct ast *ast) {
    struct action *action = ast->match->action;

    if (action == NULL || action->path == NULL 
        || action->path->type != E_FIELD)
        return;

    const char *value = strrchr(ast->path, SEP);
    if (value == NULL) {
        internal_error(NULL, -1, "Failed to get value from path %s", 
                       ast->path);
        return;
    }

    value += 1;
    if (! set_field_value(ast, action, value)) {
        internal_error(NULL, -1, "Could not find field $%s:%d to store %s",
                       action->rule->name, action->path->field, ast->path);
    }
}

static struct ast *ast_expand(struct match *match, const char *path, 
                              const char *rest);

/* Generate an AST for REST iff one of match->matches indicates that
   that is needed. SUBMTAHC_P(match) must be true
 */
static struct ast *ast_expand_maybe(struct match *match, const char *path, 
                                    const char *rest, const char *crest) {
    struct ast *result = NULL;
    struct match *m = find_handle_match(match->matches, rest);
    /* m could be NULL, either because the tree doesn't generate
       syntactically correct output, or because we will later insert
       more nodes from the tree that will make the AST syntactically 
       correct.
    */
    if (m != NULL) {
        struct ast *c = ast_expand(m, path, crest);
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

    /* The rest that children will see; only different from rest
     * if match is a '*' or '+' with $seq as the action.
     * FIXME: Kludge for special semantics of $seq
     */
    const char *crest = rest; 

    if (crest != NULL && match->action != NULL 
        && match->action->path != NULL) {
            crest = strchr(crest, SEP);
            if (crest != NULL)
                crest = (crest[0] == '\0') ? NULL : crest + 1;
            if (!is_seq_iter(match))
                rest = crest;
    }

    switch(match->type) {
    case ALTERNATIVE:
        result = ast_expand_maybe(match, path, rest, crest);
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
        result = ast_expand_maybe(match, path, rest, crest);
        break;
    case LITERAL:
    case ANY:
    case FIELD:
    case ABBREV_REF:
        result = make_ast(match);
        if (match->action != NULL && match->action->value != NULL) {
            const char *value = aug_get(path);
            if (value != NULL)
                result->token = strdup(value);
        }
        break;
    default:
        internal_error(_FM(match), _L(match),
                       "Unexpected item %d\n", match->type);
        break;
    }

    if (result != NULL) {
        if (match->action != NULL && 
            (match->action->path != NULL || match->action->value != NULL)) {
            int len = strlen(path);
            if (rest != NULL)
                len -= strlen(rest) + 1;
            result->path = strndup(path, len);
        } else {
            if (! LEAF_P(result))
                synthesize_path(result);
        }
        store_field_value(result);
    }
    return result;
}

/* Insert entries to go to PATH into AST. If NEXT is non-null and AST is
 * QUANT_PLUS or QUANT_STAR, the new subtree is inserted before NEXT,
 * otherwise it is appended to the QUANT_PLUS/QUANT_STAR. If AST is a
 * sequence, NEXT is ignored and insertion is done based on the grammar. If
 * AST is any other kind of node, an error is flagged.
 */
static struct ast *ast_insert(struct ast *ast, const char *path, 
                              struct ast *next) {
    int moved;

    do {
        moved = 0;
        if (! LEAF_P(ast)) {
            list_for_each(c, ast->children) {
                if (c->path != NULL && pathprefix(c->path, path)) {
                    ast = c;
                    next = NULL;
                    moved = 1;
                    break;
                }
            }
        }
    } while (moved);

    if (LEAF_P(ast)) {
        /* Should never get here */
        internal_error(NULL, -1, "Unexpected leaf");
        return NULL;
    }
    
    struct match *match = ast->match;
    struct ast *sub = NULL;
    struct match *child;
    const char *rest = path + strlen(ast->path);
    if (*rest == SEP)
        rest += 1;
    if (is_seq_iter(match)) {
        rest = strchr(rest, SEP);
        if (rest != NULL)
            rest += 1;
    }
    if (rest != NULL && *rest == '\0') {
        // FIXME: This really means that PATH can't be generated
        // from the grammar. And since we only try to insert
        // paths that have values assigned to them, the user put
        // something in the tree that will vanish when the tree is
        // saved
        internal_error(NULL, -1, "Exhausted path %s", path);
        return NULL;
    }

    switch(match->type) {
    case SEQUENCE:
        child = find_handle_match(match->matches, rest);
        if (child != NULL) {
            sub = ast_expand(child, path, rest);
            if (sub != NULL) {
                /* Insert sub in ast->children so that the list formed by
                   c->match for c in ast->children is a subsequence of
                   match->matches. It is important that the ast->children
                   already form a valid subsequence of match->matches. */
                if (sub->next != NULL) {
                    internal_error(NULL, -1, 
                                   "Subtree has too many children");
                } else if (ast->children == NULL) {
                    ast->children = sub;
                } else {
                    struct match *m = match->matches;
                    while (m != sub->match && m != ast->children->match)
                        m = m->next;
                    if (m == sub->match) {
                        if (m == ast->children->match) {
                            internal_error(NULL, -1,
                              "Match in sequence for %s already used for %s",
                                           path, ast->children->path);
                            // FIXME: The grammar should be checked to make sure
                            // we can never get here
                        }
                        sub->next = ast->children;
                        ast->children = sub;
                    } else {
                        struct ast *c = ast->children;
                        while (m != sub->match && c->next != NULL) {
                            for (m = c->match->next; 
                                 m != c->next->match && m != sub->match;
                                 m = m->next);
                            if (m != sub->match)
                                c = c->next;
                        }
                        if (sub->match == c->match) {
                            internal_error(NULL, -1,
                              "Match in sequence for %s already used for %s",
                                           path, ast->children->path);
                            // FIXME: The grammar should be checked to make sure
                            // we can never get here
                        }
                        sub->next = c->next;
                        c->next = sub;
                    }
                }
                synthesize_path(ast);
            }
        } else {
            internal_error(NULL, -1, "Could not insert (%s,%s) into sequence",
                           path, rest);
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
        if (next == NULL)
            list_append(ast->children, sub);
        else
            list_insert_before(sub, next, ast->children);
        synthesize_path(ast);
        break;
    default:
        internal_error(NULL, -1, 
          "Strange insertion point %s for path %s and rest %s", _t(match), 
                       path, rest);
        break;
    }

    return sub;
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

static int is_store(struct ast *ast, const char *path) {
    if (ast->path == NULL)
        return 0;
    if (ast->match->action == NULL || ast->match->action->value == NULL)
        return 0;
    return STREQ(ast->path, path);
}

static void create(struct ast *parent, const char *path, struct ast *next) {
    struct ast *sub = NULL;
    int nchildren;
    const char **children;

    if (aug_get(path) != NULL) {
        sub = ast_insert(parent, path, next);

        if (sub == NULL) {
            internal_error(NULL, -1,
                           "Failed to insert %s into %s %s",
                           path, _t(parent->match), parent->path);
            return;
        }
    }

    nchildren = aug_ls(path, &children);
    for (int i=0; i<nchildren; i++) {
        if (sub == NULL)
            create(parent, children[i], next);
        else
            create(sub, children[i], NULL);
    }
}

static int is_branch(struct ast *ast) {
    if (ast->match->type == QUANT_PLUS || ast->match->type == QUANT_STAR)
        return 1;
    if (ast->path == NULL)
        return 0;
    if (ast->match->type == SEQUENCE) {
        list_for_each(c, ast->children) {
            if (c->path != NULL && STRNEQ(ast->path, c->path))
                return 1;
        }
    }
    return 0;
}

static int traverse(struct ast *ast, const char *path) {
    int changed = 0;

    if (! LEAF_P(ast)) {
        if (is_branch(ast)) {
            struct ast *next; // First child of AST we haven't dealt with
            const char **children;
            int nchildren;

            for (next = ast->children; next != NULL && next->path == NULL;
                 next = next->next);

            nchildren = aug_ls(path, &children);
            for (int i=0; i < nchildren; i++) {
                struct ast *c;
                for (c=next; c != NULL; c = c->next) {
                    if (c->path != NULL && STREQ(c->path, children[i]))
                        break;
                }
                if (c == NULL) {
                    create(ast, children[i], next);
                    changed = 1;
                } else {
                    // next != NULL since c != NULL
                    if (c != next) {
                        list_remove(c, next);
                        list_insert_before(c, next, ast->children);
                        changed = 1;
                    } else {
                        for (next = next->next; 
                             next != NULL && next->path == NULL;
                             next = next->next);
                    }
                    if (traverse(c, children[i]))
                        changed = 1;
                }
            }
            while (next != NULL) {
                struct ast *del = next;
                for (next = next->next; 
                     next != NULL && next->path == NULL;
                     next = next->next);
                list_remove(del, ast->children);
                ast_delete(del);
                changed = 1;
            }
        } else {
            list_for_each(ac, ast->children) {
                if (ac->path != NULL && pathprefix(ac->path, path)) {
                    if (traverse(ac, path))
                        changed = 1;
                }
            }
        }
    } else if (is_store(ast, path)) {
        const char *value = aug_get(path);
        if (value == NULL) {
            if (ast->token != NULL) {
                free((void *) ast->token);
                ast->token = NULL;
                changed = 1;
            }
        } else {
            if (ast->token == NULL || STRNEQ(ast->token, value)) {
                free((void *) ast->token);
                ast->token = strdup(value);
                changed = 1;
            }
        }
    }

    return changed;
}

int ast_sync(struct ast **ast) {
    const char *root = (*ast)->path;
    int changed = 0;

    if (! aug_exists(root)) {
        ast_delete(*ast);
        *ast = NULL;
        changed = 1;
    } else {
        if (traverse(*ast, root))
            changed = 1;
    }
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
                internal_error(_FM(ast->match), _L(ast->match),
                               "illegal match type '%s'", _t(ast->match));
            }
            if (literal != NULL) {
                emit_escaped_chars(out, literal->text);
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
