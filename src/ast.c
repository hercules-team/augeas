/*
 * ast.c: low-level anipulation of grammar structures
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

#include <stdio.h>
#include <stdarg.h>
#include "ast.h"
#include "list.h"

static void dot_grammar(FILE *out, struct grammar *grammar, int flags);

/*
 * Debug printing
 */

int print_chars(FILE *out, const char *text, int cnt) {
    int total = 0;
    int print = (out != NULL);
    
    for (int i=0; i<cnt; i++) {
        switch(text[i]) {
        case '\n':
            if (print)
                fprintf(out, "~n");
            total += 2;
            break;
        case '\t':
            if (print)
                fprintf(out, "~t");
            total += 2;
            break;
        case '\0':
            if (print)
                fprintf(out, "~0");
            return total + 2;
        case '~':
            if (print)
                fprintf(out, "~~");
            total += 2;
            break;
        default:
            if (print)
                fputc(text[i], out);
            total += 1;
            break;
        }
    }
    return total;
}

void print_literal(FILE *out, struct literal *l) {
    if (l == NULL) {
        fprintf(out, "<NULL>");
        return;
    }

    if (l->type == QUOTED) {
        fprintf(out, "'%s'", l->text);
    } else if (l->type == REGEX) {
        fputc('/', out);
        print_chars(out, l->pattern, strlen(l->pattern));
        fputc('/', out);
    } else {
        fprintf(out, "<U:lit:%d>", l->type);
    }
}

static void dump_abbrevs(FILE *out, struct abbrev *a) {
    struct abbrev *p;

    for (p=a; p != NULL; p = p->next) {
        fprintf(out, "  %s: ", p->name);
        print_literal(out, p->literal);
        if (p->literal->type == REGEX)
            fprintf(out, " = '%s'", p->literal->text);
        if (p->literal->epsilon)
            fprintf(out, " <>");
        fprintf(out, "\n");
    }
}

static void print_quantc(FILE *out, struct match *m) {
    if (m->type == QUANT_PLUS)
        fputc('+', out);
    else if (m->type == QUANT_STAR)
        fputc('*', out);
    else if (m->type == QUANT_MAYBE)
        fputc('?', out);
    else {
        fprintf(out, " <U:q:%d>", m->type);
    }
}

static void print_indent(FILE *out, int indent) {
    for (int i=0; i < indent; i++)
        fputc(' ', out);
}

static void print_any(FILE *out, struct match *any, int flags) {
    if (flags & GF_ANY_RE && 
        any->literal != NULL && any->literal->pattern != NULL) {
        print_literal(out, any->literal);
    } else if (any->epsilon) {
        fprintf(out, "..?");
    } else {
        fprintf(out, "...");
    }
}

static void print_matches(FILE *out, struct match *m, const char *sep, 
                          int flags) {

    if (m == NULL) {
        fprintf(out, "<NULL>\n");
        return;
    }

    list_for_each(p, m) {
        switch (p->type) {
        case LITERAL:
            print_literal(out, p->literal);
            break;
        case NAME:
            fprintf(out, p->name);
            break;
        case ANY:
            print_any(out, p, flags);
            break;
        case FIELD:
            fprintf(out, "$%d", p->field);
            break;
        case ALTERNATIVE:
            fprintf(out, "(");
            print_matches(out, p->matches, " | ", flags);
            fprintf(out, ")");
            break;
        case SEQUENCE:
            fprintf(out, "(");
            print_matches(out, p->matches, " . ", flags);
            fprintf(out, ")");
            break;
        case ABBREV_REF:
            fprintf(out, p->abbrev->name);
            break;
        case RULE_REF:
            fprintf(out, p->rule->name);
            break;
        case QUANT_PLUS:
        case QUANT_STAR:
        case QUANT_MAYBE:
            print_matches(out, p->matches, sep, flags);
            print_quantc(out, p);
            break;
        default:
            fprintf(out, "<U:match:%d>", p->type);
            break;
        }
        if (p->next != NULL)
            fprintf(out, sep);
    }
}

void print_literal_set(FILE *out, struct literal_set *set,
                       struct grammar *grammar,
                       char begin, char end) {
    if (set != NULL) {
        fprintf (out, " %c", begin);
        list_for_each(f, set) {
            struct abbrev *abbrev = NULL;
            list_for_each(a, grammar->abbrevs) {
                if (a->literal == f->literal) {
                    abbrev = a;
                    break;
                }
            }
            if (abbrev != NULL)
                fprintf(out, abbrev->name);
            else
                print_literal(out, f->literal);
            if (f->next != NULL) {
                fputc(' ', out);
            }
        }
        fputc(end, out);
    }
}

static void print_entry(FILE *out, struct entry *entry) {
    switch(entry->type) {
    case E_CONST:
        fprintf(out, "'%s'", entry->text);
        break;
    case E_GLOBAL:
        fprintf(out, "$%s", entry->text);
        break;
    case E_FIELD:
        fprintf(out, "$%d", entry->field);
        break;
    default:
        fprintf(out, "<U:%d>", entry->type);
        break;
    }
}

static void print_action(FILE *out, struct action *action) {
    if (action == NULL) {
        return;
    }
    list_for_each(p, action->path) {
        fprintf(out, " enter(");
        print_entry(out, p);
        fprintf(out, ")");
    }
    if (action->value != NULL) {
        fprintf(out, " store(");
        print_entry(out, action->value);
        fputc(')', out);
    }
}


static void print_follow(FILE *out, struct match *m, int flags) {
    if (flags & GF_ACTIONS && m->action != NULL) {
        print_action(out, m->action);
    }

    if (flags & GF_FIRST) {
        print_literal_set(out, m->first, m->owner->grammar, '[', ']');
    }

    if (flags & GF_FOLLOW) {
        print_literal_set(out, m->follow, m->owner->grammar, '{', '}');
    }

    if (flags & GF_HANDLES) {
        print_literal_set(out, m->handle, m->owner->grammar, '<', '>');
    }
}

static void dump_matches(FILE *out, struct match *m, int indent, int flags) {

    if (m == NULL) {
        fprintf(out, "<NULL>\n");
        return;
    }

    list_for_each(p, m) {
        print_indent(out, indent);

        switch (p->type) {
        case LITERAL:
            print_literal(out, p->literal);
            print_follow(out, p, flags);
            fprintf(out, "\n");
            break;
        case NAME:
            fprintf(out, ":%s", p->name);
            print_follow(out, p, flags);
            putchar('\n');
            break;
        case ANY:
            print_any(out, p, flags);
            print_follow(out, p, flags);
            fputc('\n', out);
            break;
        case FIELD:
            fprintf(out, "$%d\n", p->field);
            print_follow(out, p, flags);
            break;
        case ALTERNATIVE:
            fprintf(out, "|");
            print_follow(out, p, flags);
            putchar('\n');
            dump_matches(out, p->matches, indent + 2, flags);
            break;
        case SEQUENCE:
            fprintf(out, ".");
            print_follow(out, p, flags);
            putchar('\n');
            dump_matches(out, p->matches, indent + 2, flags);
            break;
        case ABBREV_REF:
            fprintf(out, "%s", p->abbrev->name);
            print_follow(out, p, flags);
            fputc('\n', out);
            break;
        case RULE_REF:
            fprintf(out, "%s", p->rule->name);
            print_follow(out, p, flags);
            fputc('\n', out);
            break;
        case QUANT_PLUS:
        case QUANT_STAR:
        case QUANT_MAYBE:
            print_quantc(out, p);
            print_follow(out, p, flags);
            putchar('\n');
            dump_matches(out, p->matches, indent + 2, flags);
            break;
        default:
            fprintf(out, "<U>\n");
            break;
        }
    }
}

static void dump_rules(FILE *out, struct rule *r, int flags) {
    list_for_each(p, r) {
        if (flags & GF_PRETTY) {
            fprintf(out, "  %s: ", p->name);
            print_matches(out, p->matches, " @ ", flags);
            fprintf(out, "\n");
        } else {
            fprintf(out, "  %s\n", p->name);
            dump_matches(out, p->matches, 4, flags);
        }
        if ((flags & GF_FIRST) && p->matches->first != NULL) {
            fprintf(out, "    [");
            list_for_each(f, p->matches->first) {
                struct abbrev *abbrev = NULL;
                list_for_each(a, p->matches->owner->grammar->abbrevs) {
                    if (a->literal == f->literal) {
                        abbrev = a;
                        break;
                    }
                }
                if (abbrev)
                    fprintf(out, abbrev->name);
                else
                    print_literal(out, f->literal);
                if (f->next != NULL)
                    fputc(' ', out);
            }
            fprintf(out, "]\n");
        }
        fputc('\n', out);
    }
}

static void dump_grammar(struct grammar *grammar, FILE *out, int flags) {
    if (flags & ~ GF_DOT) {
        fprintf(out, "abbrevs:\n");
        dump_abbrevs(out, grammar->abbrevs);
        fprintf(out, "rules:\n");
        dump_rules(out, grammar->rules, flags);
    }
}

/*
 * Error handling
 */
static int set_literal_text(struct literal *literal, const char *text);

void grammar_error(const char *fname, int lineno,
                   const char *format, ...) {

    va_list ap;

    fprintf(stderr, "%s:%d:", fname, lineno);
	va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void internal_error_at(const char *srcfile, int srclineno,
                       const char *fname, int lineno,
                       const char *format, ...) {

    va_list ap;

    fprintf(stderr, "%s:%d:%s:%d:Internal error:", 
            srcfile, srclineno, fname, lineno);
	va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

/*
 * Semantic checks for a grammar
 *
 * Missing checks:
 *   rules + abbrevs must have unique names across both
 */

static int check_abbrevs(struct grammar *grammar) {
    int r = 1;
    /* abbrev->name must be unique */
    list_for_each(p, grammar->abbrevs) {
        list_for_each(q, p->next) {
            if (STREQ(p->name, q->name)) {
                grammar_error(_FG(grammar), _L(p), "token %s defined twice", 
                              p->name);
                r = 0;
            }
        }
    }
    /* regex abbrevs must have a default value */
    list_for_each(p, grammar->abbrevs) {
        if (p->literal->type == REGEX && p->literal->text == NULL) {
            grammar_error(_FG(grammar), _L(p), 
                          "regex token %s is missing a default value",
                          p->name);
            r = 0;
        }
    }
    return r;
}

static int check_matches(struct match *matches) {
    int r = 1;

    list_for_each(m, matches) {
        if (m->type == FIELD) {
            if (find_field(m->owner->matches, m->field) == NULL) {
                grammar_error(_FM(m), _L(m),
                              "Field $%d in rule %s does not exist\n",
                              m->field, m->owner->name);
                r = 0;
            }
        }
        if (SUBMATCH_P(m)) {
            if (! check_matches(m->matches))
                r = 0;
        }
    }
    return r;
}

static int check_rules(struct rule *rules) {
    int r = 1;
    
    /* rule->name must be unique */
    list_for_each(p, rules) {
        list_for_each(q, p->next) {
            if (STREQ(p->name, q->name)) {
                grammar_error(_FR(p), _L(p), 
                              "rule %s defined twice", p->name);
                r = 0;
            }
        }
        if (! check_matches(p->matches))
            r = 0;
    }
    return r;
}

static int semantic_check(struct grammar *grammar) {
    int r = 1;

    if (! check_abbrevs(grammar))
        r = 0;
    if (! check_rules(grammar->rules))
        r = 0;
    return r;
}

/*
 * Compute first/follow sets for the grammar
 */
static struct literal_set *make_literal_set(struct literal *literal) {
    struct literal_set *new;
    CALLOC(new, 1);
    new->literal = literal;
    return new;
}

/* The set HEAD contains LITERAL iff head has an entry using
   LITERAL as its literal or if it has an entry where the
   pattern is identical to LITERAL's pattern.
   
   Literals with a NULL pattern will be filled at some later point,
   so we optimistically assume that they will be different
*/
static int literal_set_contains(struct literal_set *head, 
                                struct literal *literal) {
    list_for_each(f, head) {
        if (literal == f->literal)
            return 1;
        if (literal->pattern != NULL && f->literal->pattern != NULL
            && STREQ(literal->pattern, f->literal->pattern))
            return 1;
    }
    return 0;
}

static int merge_literal_set(struct literal_set **first, 
                             struct literal_set *merge) {
    int changed = 0;

    if (merge == NULL)
        return 0;
    list_for_each(m, merge) {
        if (! literal_set_contains(*first, m->literal)) {
            struct literal_set *new;
            new = make_literal_set(m->literal);
            list_append(*first, new);
            changed = 1;
        }
    }
    return changed;
}

__attribute__((pure))
struct match *find_field(struct match *matches, int fid) {
    struct match *result = NULL;

    list_for_each(m, matches) {
        if (m->fid == fid)
            return m;
        if (SUBMATCH_P(m) &&
            (result = find_field(m->matches, fid)) != NULL)
            return result;
    }
    return NULL;
}

__attribute__((pure))
static struct match *find_group(struct match *matches, int gid) {
    struct match *result = NULL;

    list_for_each(m, matches) {
        if (m->gid == gid)
            return m;
        if (SUBMATCH_P(m) &&
            (result = find_group(m->matches, gid)) != NULL)
            return result;
    }
    return NULL;
}

static int make_match_firsts(struct match *matches) {
    int changed = 0;

    list_for_each(cur, matches) {
        switch(cur->type) {
        case LITERAL:
            if (cur->epsilon != cur->literal->epsilon) {
                changed = 1;
                cur->epsilon = cur->literal->epsilon;
            }
            /* fall through, missing break intentional */
        case ANY:
            /* cur->literal */
            if (! literal_set_contains(cur->first, cur->literal)) {
                struct literal_set *ls = make_literal_set(cur->literal);
                list_append(cur->first, ls);
                changed = 1;
            }
            break;
        case ABBREV_REF:
            if (cur->epsilon != cur->abbrev->literal->epsilon) {
                changed = 1;
                cur->epsilon = cur->abbrev->literal->epsilon;
            }
            /* cur->abbrev->literal */
            if (! literal_set_contains(cur->first, cur->abbrev->literal)) {
                struct literal_set *ls = make_literal_set(cur->abbrev->literal);
                list_append(cur->first, ls);
                changed = 1;
            }
            break;
        case RULE_REF:
            if (cur->epsilon != cur->rule->matches->epsilon) {
                changed = 1;
                cur->epsilon = cur->rule->matches->epsilon;
            }
            /* first(rule) */
            if (merge_literal_set(&(cur->first), cur->rule->matches->first))
                changed = 1;
            break;
        case FIELD:
            /* first(cur->field) */
            {
                struct match *field = find_field(cur->owner->matches, 
                                                 cur->field);
                if (field == NULL) {
                    internal_error(_FM(cur), _L(cur->owner), 
                                   "rule %s: unresolved field %d", 
                                   cur->owner->name, cur->field);
                } else {
                    if (cur->epsilon != field->epsilon) {
                        cur->epsilon = field->epsilon;
                        changed = 1;
                    }
                    if (merge_literal_set(&(cur->first), field->first))
                        changed = 1;
                }
            }
            break;
        case ALTERNATIVE:
            if (make_match_firsts(cur->matches))
                changed = 1;
            /* union(first(cur->matches)) */
            {
                int eps = 0;
                list_for_each(a, cur->matches) {
                    if (a->epsilon) eps = 1;
                    if (merge_literal_set(&(cur->first), a->first))
                        changed = 1;
                }
                /* Never reset cur->epsilon; it might have been set
                   if this alternative is qualified with a * or ? */
                if (! cur->epsilon && eps) {
                    changed = 1;
                    cur->epsilon = eps;
                }
            }
            break;
        case SEQUENCE:
            if (make_match_firsts(cur->matches))
                changed = 1;
            {
                int eps = 1;
                list_for_each(p, cur->matches) {
                    if (! p->epsilon) eps = 0;
                }
                /* Never reset cur->epsilon; it might have been set
                   if this sequence is qualified with a * or ? */
                if (! cur->epsilon && eps) {
                    changed = 1;
                    cur->epsilon = eps;
                }
            }
            /* union(p in cur->matches) up to the first one with p->epsilon == 0*/
            list_for_each(p, cur->matches) {
                if (merge_literal_set(&(cur->first), p->first))
                    changed = 1;
                if (! p->epsilon)
                    break;
            }
            break;
        case QUANT_PLUS:
        case QUANT_STAR:
        case QUANT_MAYBE:
            if (make_match_firsts(cur->matches))
                changed = 1;
            if (merge_literal_set(&(cur->first), cur->matches->first))
                changed = 1;
            if (! cur->epsilon) {
                if (cur->type == QUANT_STAR || cur->type == QUANT_MAYBE
                    || (cur->type == QUANT_PLUS 
                        && cur->matches->epsilon)) {
                    cur->epsilon = 1;
                    changed = 1;
                }
            }
            break;
        default:
            internal_error(_FM(cur), _L(cur), "unexpected type %d", cur->type);
            break;
        }
    }
    return changed;
}

static int make_match_follows(struct match *matches, struct match *parent) {
    int changed = 0;

    list_for_each(cur, matches) {
        if (cur->next == NULL || (parent->type == ALTERNATIVE)) {
            if (merge_literal_set(&(cur->follow), parent->follow))
                changed = 1;
        } else {
            struct match *next = cur->next;
            if (merge_literal_set(&(cur->follow), next->first))
                changed = 1;
            if (next->epsilon && 
                merge_literal_set(&(cur->follow), next->follow))
                changed = 1;
        }
        if (cur->type == QUANT_PLUS || cur->type == QUANT_STAR) {
            if (merge_literal_set(&(cur->follow), cur->first))
                changed = 1;
        }
        if (cur->type == RULE_REF) {
            if (merge_literal_set(&(cur->rule->matches->follow), cur->follow))
                changed = 1;
        } else if (SUBMATCH_P(cur)) {
            if (make_match_follows(cur->matches, cur))
                changed = 1;
        }
    }
    return changed;
}

static int make_any_literal(struct match *any) {
    if (any->literal == NULL) {
        internal_error(_FM(any), _L(any), "any misses literal");
        return 0;
    }
    if (any->literal->pattern != NULL || any->literal->text != NULL) {
        internal_error(_FM(any), _L(any),
                       "any pattern already initialized");
        return 0;
    }
    if (any->follow == NULL) {
        grammar_error(_FM(any), _L(any), "any pattern ... not followed by anything");
        return 0;
    }
    int size = 8;
    char *pattern;
    list_for_each(f, any->follow) {
        size += strlen(f->literal->pattern) + 1;
    }
    CALLOC(pattern, size+1);
    if (any->epsilon)
        strcpy(pattern, "(.*?)(?=");
    else
        strcpy(pattern, "(.+?)(?=");
        
    list_for_each(f, any->follow) {
        strcat(pattern, f->literal->pattern);
        if (f->next == NULL)
            strcat(pattern, ")");
        else
            strcat(pattern, "|");
    }
    if (strlen(pattern) != size) {
        internal_error(_FM(any), _L(any), "misallocation: allocated %d for a string of size %d", size, (int) strlen(pattern));
        return 0;
    }
    set_literal_text(any->literal, pattern);
    return 1;
}

static int resolve_any(struct match *match) {
    int result = 1;
    
    list_for_each(m, match) {
        if (m->type == ANY) {
            if (! make_any_literal(m))
                result = 0;
        } 
        if (SUBMATCH_P(m)) {
            if (! resolve_any(m->matches))
                result = 0;
        }
    }
    return result;
}

static int literal_sets_intersect(struct literal_set *set1, 
                                  struct literal_set *set2) {
    list_for_each(s1, set1) {
        if (literal_set_contains(set2, s1->literal))
            return 1;
    }
    return 0;
}
static int check_match_ambiguity(struct rule *rule, struct match *matches) {
    list_for_each(m, matches) {
        if (m->type == ALTERNATIVE) {
            list_for_each(a1, m->matches) {
                list_for_each(a2, a1->next) {
                    if (literal_sets_intersect(a1->first, a2->first)) {
                        grammar_error(_FR(rule), _L(rule), "rule %s is ambiguous", rule->name);
                        return 0;
                    }
                }
            }
        }
        if (SUBMATCH_P(m)) {
            if (! check_match_ambiguity(rule, m->matches))
                return 0;
        }
    }
    return 1;
}

/* Find the regexp pattern that matches path components
   generated from ENTRY */
__attribute__((pure))
static const char *entry_pattern(const struct entry *entry) {
    static const char *DIGITS = "[0-9]+";
    static const char *PATH_COMPONENT = "[^/]+";

    const struct rule *rule = entry->action->rule;

    if (entry->type == E_CONST) {
        return entry->text;
    } else if (entry->type == E_GLOBAL) {
        if (STREQ(entry->text, "seq")) {
            return DIGITS;
        } else if (STREQ(entry->text, "basename")) {
            return PATH_COMPONENT;
        } else {
            grammar_error(_FR(rule), _L(entry), 
                          "Reference to unknown global %s", 
                          entry->text);
            return NULL;
        }
    } else if (entry->type == E_FIELD) {
        struct match *field = find_field(rule->matches, entry->field);
        if (field == NULL) {
            grammar_error(_FR(rule), _L(entry), 
                          "Reference to nonexistant field %d", 
                          entry->field);
            return NULL;
        } else {
            if (field->type == LITERAL) {
                return field->literal->pattern;
            } else if (field->type == ANY) {
                return PATH_COMPONENT;
            } else if (field->type == ABBREV_REF) {
                return field->abbrev->literal->pattern;
            } else {
                grammar_error(_FR(rule), _L(entry),
                              "Field reference $%d is not a literal",
                              field->fid);
                return NULL;
            }
        }
    } else {
        internal_error(_FR(rule), _L(entry),
                       "Illegal entry type %d\n", entry->type);
        return NULL;
    }
}

/* Compute handles for each action; return 1 on success, 0 on failure */
static int make_action_handles(struct match *matches) {
    list_for_each(m, matches) {
        if (SUBMATCH_P(m)) {
            make_action_handles(m->matches);
        }
        if (m->action != NULL) {
            /* Initial size for pattern is arbitrary; we just need pattern to be
               a valid empty string */
            char *pattern = calloc(10, sizeof(char));
            list_for_each(e, m->action->path) {
                const char *pat = entry_pattern(e);
                if (pat == NULL) {
                    free(pattern);
                    return 0;
                }

                int len = strlen(pattern) + strlen(pat) + 1;
                if (e->next != NULL)
                    len += 1;
                pattern = realloc(pattern, len);
                strcat(pattern, pat);
                if (e->next != NULL)
                    strcat(pattern, "/");
            }
            m->action->handle = make_literal(pattern, REGEX, 
                                             m->action->lineno);
        }
    }
    return 1;
}

/* Compute the handles in each match. Return 0 if no changes were made, 
   1 otherwise */
static int make_match_handles(struct match *matches) {
    int changed = 0;

    list_for_each(cur, matches) {
        if (SUBMATCH_P(cur)) {
            if (make_match_handles(cur->matches))
                changed = 1;
        }
        
        if (cur->action != NULL && cur->action->handle != NULL) {
            if (cur->handle == NULL) {
                cur->handle = make_literal_set(cur->action->handle);
                changed = 1;
            }
        } else {
            switch(cur->type) {
            case ALTERNATIVE:
                list_for_each(m, cur->matches) {
                    if (merge_literal_set(&(cur->handle), m->handle))
                        changed = 1;
                }
                break;
            case SEQUENCE:
                list_for_each(m, cur->matches) {
                    if (m->handle != NULL) {
                        if (merge_literal_set(&(cur->handle), m->handle))
                            changed = 1;
                        break;
                    }
                }
                break;
            case RULE_REF:
                if (merge_literal_set(&(cur->handle), 
                                      cur->rule->matches->handle))
                    changed = 1;
                break;
            case QUANT_STAR:
            case QUANT_PLUS:
            case QUANT_MAYBE:
                if (merge_literal_set(&(cur->handle), cur->matches->handle))
                    changed = 1;
                break;
            default:
                // all others keep a NULL handle
                break;
            }
        }
    }

    return changed;
}

/* Prepare the grammar by 
 *
 * (1) computing first/follow sets for all rules and propagating epsilon
 * (2) determining regular expressions for all ANY symbols (... and ..?)
 * (3) computing handles for the actions and for alll rules/matches
 *
 * Only a grammar that has been successfully prepared can be used for
 * parsing
 */
static int prepare(struct grammar *grammar) {
    int changed;

    /* First/epsilon */
    do {
        changed = 0;
        list_for_each(r, grammar->rules) {
            if (make_match_firsts(r->matches))
                changed = 1;
        }
    } while (changed);

    list_for_each(r, grammar->rules) {
        if (r->matches->first == NULL) {
            grammar_error(_FR(r), _L(r), "rule '%s' recurses infintely", r->name);
            return 0;
        }
    }

    /* Follow */
    do {
        changed = 0;
        list_for_each(r, grammar->rules) {
            if (make_match_follows(r->matches, r->matches))
                changed = 1;
        }
    } while (changed);

    /* Resolve any */
    list_for_each(r, grammar->rules) {
        if (! resolve_any(r->matches))
            return 0;
    }

    /* Handles */
    list_for_each(r, grammar->rules) {
        if (!make_action_handles(r->matches))
            return 0;
    }

    do {
        changed = 0;
        list_for_each(r, grammar->rules) {
            if (make_match_handles(r->matches))
                changed = 1;
        }
    } while (changed);

    int result = 1;
    list_for_each(r, grammar->rules) {
        if (! check_match_ambiguity(r, r->matches))
            result =0;
    }

    return result;
}

/*
 * Bind names to the rules/abbrevs with their name
 */
static struct abbrev *find_abbrev(struct grammar *grammar, const char *name) {
    list_for_each(a, grammar->abbrevs) {
        if (STREQ(name, a->name))
            return a;
    }
    return NULL;
}

static struct rule *find_rule(struct grammar *grammar, const char *name) {
    list_for_each(r, grammar->rules) {
        if (STREQ(name, r->name))
            return r;
    }
    return NULL;
}

static int bind_match_names(struct grammar *grammar, struct match *matches) {
    struct abbrev *a;
    struct rule *r;
    int result = 1;

    list_for_each(cur, matches) {
        if (cur->type == NAME) {
            if ((a = find_abbrev(grammar, cur->name)) != NULL) {
                free((void *) cur->name);
                cur->type = ABBREV_REF;
                cur->abbrev = a;
            } else if ((r = find_rule(grammar, cur->name)) != NULL) {
                free((void *) cur->name);
                cur->type = RULE_REF;
                cur->rule = r;
            } else {
                grammar_error(_FM(cur), _L(cur), "Unresolved symbol %s", cur->name);
                result = 0;
            }
        } else if (QUANT_P(cur)) {
            if (! bind_match_names(grammar, cur->matches))
                result = 0;
            else {
                if (cur->matches->type == ABBREV_REF) {
                    grammar_error(_FM(cur), _L(cur),
                 "quantifiers '*', '+', or '?' can not be applied to a token");
                    result = 0;
                }
            }
        } else if (SUBMATCH_P(cur)) {
            if (! bind_match_names(grammar, cur->matches))
                result = 0;
        }
    }
    return result;
}

static void calc_match_id(struct rule *rule, struct match *matches,
                            int *fid, int *gid) {
    list_for_each(m, matches) {
        m->owner = rule;
        m->fid = -1;
        m->gid = (m->gid) ? (*gid)++ : -1;
        if (m->type == ANY ||
            m->type == LITERAL ||
            m->type == FIELD ||
            m->type == NAME) {
            m->fid = (*fid)++;
        } else if (SUBMATCH_P(m)) {
            calc_match_id(rule, m->matches, fid, gid);
        }
    }
}

static int check_entry(struct rule *rule, struct entry *entry,
                       const char *role) {
    struct match *m;

    list_for_each(e, entry) {
        if (e->type == E_FIELD) {
            m = find_field(rule->matches, e->field);
            if (m == NULL) {
                grammar_error(_FR(rule), _L(e),
                              "Reference to nonexistant field %d in %s",
                              e->field, role);
                return 0;
            } else if (m->type == RULE_REF) {
                grammar_error(_FR(rule), _L(e),
                              "Field %d, used as the %s, is a rule. That is not implemented.",
                              e->field, role);
                return 0;
            }
        }
    }
    return 1;
}

static int push_stores(struct match *matches) {
    int r = 1;

    /* Attach actions that store into the tree to the field that is being 
       stored. This can only work if the field being assigned is part of 
       the subtree to which the action */
    list_for_each(m, matches) {
        if (m->action != NULL && m->action->value != NULL) {
            struct match *val = find_field(m, m->action->value->field);
            if (val == NULL) {
                grammar_error(_FM(m), _L(m->action),
                              "Assignment of $%d outside the current tree in %s",
                              m->action->value->field, m->owner->name);
                r = 0;
            } else if (val->action != NULL && val->action->value != NULL) {
                if (val != m) {
                    grammar_error(_FM(m), _L(m->action),
                                  "Field $%d in %s stored in more than one tree node",
                                  m->action->value->field, m->owner->name);
                    r = 0;
                }
            } else if (val->action != NULL) {
                /* A completely pointless enter attached to field
                 * VAL. It's pointless since enters don't have any
                 * persistent effect until something is assigned
                 */
                grammar_error(_FM(val), _L(val),
                              "Useless path enter for field $%d in %s",
                              m->action->value->field, m->owner->name);
                r = 0;
            } else {
                if (m->action->path == NULL) {
                    /* This can currently not be expressed in the language,
                       but better safe than sorry. It corresponds to an
                       action that stores without entering a subtree */
                    val->action = m->action;
                    m->action = NULL;
                } else {
                    struct action *a;
                    CALLOC(a, 1);
                    a->lineno = m->action->lineno;
                    a->rule = m->action->rule;
                    a->scope = m->action->scope;
                    a->id    = m->action->id;
                    a->value = m->action->value;
                    val->action = a;
                    m->action->value = NULL;
                }
            }
        }
        if (SUBMATCH_P(m))
            if (! push_stores(m->matches))
                r = 0;
    }

    return r;
}

static int bind_actions(struct rule *rule) {
    int r = 1;

    list_for_each(a, rule->actions) {
        if (! check_entry(rule, a->path, "path"))
            r = 0;
        if (! check_entry(rule, a->value, "value"))
            r = 0;
        if (a->value != NULL && a->value->type != E_FIELD) {
            grammar_error(_FR(rule), _L(a->value),
                          "Only fields can be used as values in assignments");
        }
    }

    list_for_each(a, rule->actions) {
        const char *n = (a->scope == A_FIELD) ? "field" : "group";
        struct match *m;

        if (a->id == 0)       /* Group 0 refers to the whole rule */
            continue;

        if (a->scope == A_FIELD) {
            m = find_field(rule->matches, a->id);
        } else if (a->scope == A_GROUP) {
            m = find_group(rule->matches, a->id);
        } else {
            internal_error(_FR(rule), _L(a),
                           "Illegal action scope %d", a->scope);
            r = 0;
        }
        if (m == NULL) {
            grammar_error(_FR(rule), _L(a),
                          "Action references nonexistent %s %d", n, a->id);
            r = 0;
        } else {
            if (m->action != NULL) {
                grammar_error(_FR(rule), _L(a), "Duplicate actions for %s %d", n, a->id);
                r = 0;
            }
            m->action = a;
        }
    }
    /* Add action with id 0 to the very first match in the rule
       There may already be one (for rules like 'r: ( a | b )' with an action
       for @1. In that case we enclose the whole action in a sequence and
       attach the id 0 action
     */
    list_for_each(a, rule->actions) {
        if (a->scope == A_GROUP && a->id == 0) {
            if (rule->matches->action != NULL) {
                if (rule->matches->action->id == 0) {
                    grammar_error(_FR(rule), _L(a), "Duplicate actions for group %d", a->id);
                    r = 0;
                } else {
                    struct match *shim;
                    CALLOC(shim, 1);
                    shim->type = SEQUENCE;
                    shim->lineno = rule->lineno;
                    shim->owner  = rule;
                    shim->fid    = -1;
                    shim->gid    = 0;
                    shim->action = a;

                    shim->matches = rule->matches;
                    rule->matches = shim;
                }
            } else {
                rule->matches->action = a;
            }
        }
    }

    if (!push_stores(rule->matches))
        r = 0;

    return r;
}

/*
 * Resolve names to their rules/abbrevs. Resolve ANY matches to a regular 
 * expression. Convert quoted strings in matches to regexps by enclosing them
 * in '\Q..\E'
 *
 * Returns 1 on success, 0 on error. After a successful return, the grammar
 * does not contain anymore NAME or ANY matches
 */
static int resolve(struct grammar *grammar) {
    list_for_each(r, grammar->rules) {
        int gid = 1, fid = 1;
        r->grammar = grammar;
        calc_match_id(r, r->matches, &fid, &gid);
        if (! bind_match_names(grammar, r->matches))
            return 0;
        if (! bind_actions(r))
            return 0;
    }
    return 1;
}

/*
 * Constructors for some of the data structures
 */

static int set_literal_text(struct literal *literal, const char *text) {
    const char *errptr;
    int erroffset;
    int r;
    int matches[3];

    if (literal->type == REGEX) {
        literal->pattern = text;
    } else if (literal->type == QUOTED) {
        char *pattern, *p;
        
        pattern = alloca(2*strlen(text)+1+4);
        strcpy(pattern, "\\Q");
        p = pattern + 2;
        for (const char *t = text; *t != '\0'; t++) {
            if (*t == '\\') {
                t += 1;
                switch (*t) {
                case 'n':
                    *p++ = '\n';
                    break;
                case 't':
                    *p++ = '\t';
                    break;
                case 'r':
                    *p++ = '\r';
                    break;
                default:
                    *p++ = *t;
                }
            } else {
                *p++ = *t;
            }
        }
        *p++ = '\\';
        *p++ = 'E';
        *p = '\0';

        literal->text = text;
        literal->pattern = strdup(pattern);
    } else {
        grammar_error(NULL, _L(literal), "Illegal literal type %d", 
                      literal->type);
        return -1;
    }
  
    literal->re = pcre_compile(literal->pattern, PCRE_MULTILINE, 
                               &errptr, &erroffset, NULL);
    if (literal->re == NULL) {
        grammar_error(NULL, _L(literal), "ill-formed regular expression /%s/",
                      literal->pattern);
        return -1;
    }
    r = pcre_exec(literal->re, NULL, "", 0, 0, 0, matches, 3);
    if (r < 0 && r != PCRE_ERROR_NOMATCH) {
        internal_error(NULL, _L(literal), "empty match failed on /%s/ %d",
                       literal->pattern, r);
        return -1;
    }
    literal->epsilon = (r != -1);
    return 0;
}

struct literal *make_literal(const char *text, enum literal_type type,
                             int lineno) {
  struct literal *result;

  CALLOC(result, 1);
  result->type = type;
  result->lineno = lineno;
  
  if (text != NULL)
      /* FIXME: We need a way to pass errors up through the parser */
      set_literal_text(result, text);

  return result;
}

/* defined in spec-parse.y */
int spec_parse_file(const char *name, struct grammar **grammars,
                    struct map **maps);

int load_spec(const char *filename, FILE *log, int flags,
              struct grammar **grammars, struct map **maps) {
    int ok = 1, r;
    const char *errmsg;

    r = spec_parse_file(filename, grammars, maps);
    ok = (r != -1);

    list_for_each(g, *grammars) {
        if (ok) {
            ok = resolve(g);
            errmsg = "Resolve failed";
        } 
        if (ok) {
            ok = semantic_check(g);
            errmsg = "Semantic checks failed";
        }
        if (ok) {
            ok = prepare(g);
            errmsg = "First computation failed";
        }
        if (!ok) {
            fprintf(stderr, "%s: %s\n", g->filename, errmsg);
        }
    
        if (log == NULL)
            log = stdout;

        if (g != NULL) {
            dump_grammar(g, log, flags);
            dot_grammar(log, g, flags);
        }
    }

    return ok ? 0 : -1;
}

/*
 * Freeing some data structures
 */

void augs_map_free(struct map *map) {
    if (map != NULL) {
        safe_free((void *) map->filename);
        safe_free((void *) map->grammar_name);
        while (map->filters != NULL) {
            struct filter *f = map->filters;
            safe_free((void *) f->glob);
            map->filters = f->next;
            free(f);
        }
        free(map);
    }
}

/*
 * Produce a dot graph for the grammar
 */

static void dot_action(FILE *out, struct match *match) {
    if (match->action != NULL) {
        list_for_each(p, match->action->path) {
            print_entry(out, p);
            if (p->next != NULL)
                fprintf(out, " / ");
        }
        if (match->action->value != NULL) {
            fprintf(out, " = ");
            print_entry(out, match->action->value);
        }
    }
}

static int dot_match(FILE *out, struct match *matches, int parent, int next) {
    list_for_each(m, matches) {
        int self = next++;
        const char *name = "???";

        switch(m->type) {
        case LITERAL:
            name = "literal";
            break;
        case NAME:
            name = "name";
            break;
        case ANY:
            name = m->epsilon ? "..?" : "...";
            break;
        case FIELD:
            name = "$i";
            break;
        case ALTERNATIVE:
            name = "\\|";
            break;
        case SEQUENCE:
            name = ".";
            break;
        case RULE_REF:
            name = m->rule->name;
            break;
        case ABBREV_REF:
            name = m->abbrev->name;
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
        default:
            name = "???";
            break;
        }
        fprintf(out, "n%d [\n", self);
        fprintf(out, "  label = \" %s | ", name);
        dot_action(out, m);
        //fprintf(out, " | ");
        //print_literal_set(out, m->handle, m->owner->grammar,
        //                  ' ', ' ');
        fprintf(out, "\"\n  shape = \"record\"\n");
        fprintf(out, "];\n");
        fprintf(out, "n%d -> n%d;\n", parent, self);

        if (SUBMATCH_P(m)) {
            next = dot_match(out, m->matches, self, next);
        }
    }
    return next;
}

static int dot_rule(FILE *out, struct rule *rule, int parent, int next) {
    int self = next++;

    fprintf(out, "n%d [\n", self);
    fprintf(out, "  label = \"%s\"\n  shape = \"record\"\n", rule->name);
    fprintf(out, "];\n");
    fprintf(out, "n%d -> n%d;\n", parent, self);
    next = dot_match(out, rule->matches, self, next);
    return next;
}

static void dot_grammar(FILE *out, struct grammar *grammar, int flags) {
    int next = 0;

    if (!(flags & GF_DOT))
        return;
    fprintf(out, "strict digraph ast {\n");
    fprintf(out, "  graph [ rankdir = \"LR\" ];\n");
    list_for_each(r, grammar->rules) {
        int self = next++;
        next = dot_rule(out, r, self, next);
    }
    fprintf(out, "}\n");
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
