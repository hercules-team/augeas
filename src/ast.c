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

static const char * const action_type_names[] = {
    "undef", "counter", "seq", "label", "store", "key", NULL
};

/*
 * Debug printing
 */

int print_chars(FILE *out, const char *text, int cnt) {
    int total = 0;
    int print = (out != NULL);
    
    if (text == NULL) {
        fprintf(out, "nil");
        return 3;
    }
    if (cnt < 0)
        cnt = strlen(text);

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
        if (l->pattern == NULL)
            fprintf(out, "%p", l);
        else
            print_chars(out, l->pattern, -1);
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
        case ACTION:
            if (p->action->type == UNDEF)
                fputc('_', out);
            fprintf(out, "%s ", p->action->name);
            print_matches(out, p->action->arg, " ", flags);
            break;
        case SUBTREE:
            fprintf(out, "[");
            print_matches(out, p->matches, " ", flags);
            fprintf(out, "]");
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

ATTRIBUTE_UNUSED
static void print_entry(FILE *out, struct entry *entry) {
    switch(entry->type) {
    case E_CONST:
        fprintf(out, "'%s'", entry->text);
        break;
    case E_GLOBAL:
        fprintf(out, "$%s", entry->text);
        break;
    default:
        fprintf(out, "<U:%d>", entry->type);
        break;
    }
}

static void print_match_attrs(FILE *out, struct match *m, int flags) {
    if (flags & GF_RE) {
        fprintf(out, " %%r");
        print_literal(out, m->re);
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
        case ACTION:
            if (p->action->type == UNDEF)
                fputc('_', out);
            fprintf(out, "%s ", p->action->name);
            print_match_attrs(out, p, flags);
            fprintf(out, " : ");
            print_matches(out, p->action->arg, " ", flags);
            print_match_attrs(out, p->action->arg, flags);
            putchar('\n');
            break;
        case SUBTREE:
            fprintf(out, "[]");
            print_match_attrs(out, p, flags);
            putchar('\n');
            dump_matches(out, p->matches, indent + 2, flags);
            break;
        case LITERAL:
            print_literal(out, p->literal);
            print_match_attrs(out, p, flags);
            fprintf(out, "\n");
            break;
        case NAME:
            fprintf(out, ":%s", p->name);
            print_match_attrs(out, p, flags);
            putchar('\n');
            break;
        case ALTERNATIVE:
            fprintf(out, "|");
            print_match_attrs(out, p, flags);
            putchar('\n');
            dump_matches(out, p->matches, indent + 2, flags);
            break;
        case SEQUENCE:
            fprintf(out, ".");
            print_match_attrs(out, p, flags);
            putchar('\n');
            dump_matches(out, p->matches, indent + 2, flags);
            break;
        case ABBREV_REF:
            fprintf(out, "%s", p->abbrev->name);
            print_match_attrs(out, p, flags);
            fputc('\n', out);
            break;
        case RULE_REF:
            fprintf(out, "%s", p->rule->name);
            print_match_attrs(out, p, flags);
            fputc('\n', out);
            break;
        case QUANT_PLUS:
        case QUANT_STAR:
        case QUANT_MAYBE:
            print_quantc(out, p);
            print_match_attrs(out, p, flags);
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
 * Sets of literals
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

static char *string_append(char *s, const char *p) {
    if (p == NULL)
        return s;
    if (s == NULL) {
        s = strdup(p);
    } else {
        s = realloc(s, strlen(s) + strlen(p) + 1);
        strcat(s, p);
    }
    return s;
}

static void set_match_re(struct match *match, const char *pattern) {
    if (match->re == NULL) {
        match->re = make_literal(pattern, REGEX, match->lineno);
    } else {
        set_literal_text(match->re, pattern);
    }
}

static int make_match_re(struct match *matches);

static int make_submatch_re(struct match *match, const char *pre,
                            const char *sep, const char *suff) {
    char *p = string_append(NULL, pre);
    if (! make_match_re(match->matches))
        return 0;
    list_for_each(m, match->matches) {
        p = string_append(p, m->re->pattern);
        if (m->next != NULL)
            p = string_append(p, sep);
    }
    p = string_append(p, suff);
    set_match_re(match, p);
    return 1;
}

static int make_rule_re(struct match *match) {
    struct rule *rule = match->rule;
    if (rule->matches->re == NULL) {
        rule->matches->re = make_literal(NULL, REGEX, rule->matches->lineno);
    } else {
        if (rule->matches->re->pattern == NULL) {
            grammar_error(_FM(match), _L(match),
                          "Rule %s used recursively from %s",
                          rule->name, match->owner->name);
            return 0;
        }
    }
    if (!make_match_re(rule->matches))
        return 0;
    match->re = rule->matches->re;
    return 1;
}

static int make_match_re(struct match *matches) {
    list_for_each(cur, matches) {
        switch(cur->type) {
        case LITERAL:
            cur->re = cur->literal;
            break;
        case ABBREV_REF:
            cur->re = cur->abbrev->literal;
            break;
        case RULE_REF:
            if (! make_rule_re(cur))
                return 0;
            break;
        case ALTERNATIVE:
            if (!make_submatch_re(cur, "(", "|", ")"))
                return 0;
            break;
        case SEQUENCE:
            if (!make_submatch_re(cur, NULL, NULL, NULL))
                return 0;
            break;
        case SUBTREE:
            if (!make_submatch_re(cur, NULL, NULL, NULL))
                return 0;
            break;
        case QUANT_PLUS:
            if (!make_submatch_re(cur, "(", NULL, ")+"))
                return 0;
            break;
        case QUANT_STAR:
            if (!make_submatch_re(cur, "(", NULL, ")*"))
                return 0;
            break;
        case QUANT_MAYBE:
            if (!make_submatch_re(cur, "(", NULL, ")?"))
                return 0;
            break;
        case ACTION:
            if (ACTION_P(cur, STORE) || ACTION_P(cur, KEY)) {
                if (!make_match_re(cur->action->arg))
                    return 0;
                cur->re = cur->action->arg->re;
            } else {
                set_match_re(cur, "");
            }
            break;
        default:
            internal_error(_FM(cur), _L(cur), "unexpected type %d", cur->type);
            break;
        }
    }
    return 1;
}

/* Find the regexp pattern that matches path components
   generated from MATCH->ACTION. MATCH must be of type ACTION */
static struct literal_set *make_action_handle(const struct action *action) {
    static const char *DIGITS = "[0-9]+";

    const struct match *arg = action->arg;
    const char *pattern = NULL;
    enum literal_type type;

    switch(action->type) {
    case SEQ:
        assert(arg->type == LITERAL && arg->literal->type == QUOTED);
        pattern = DIGITS;
        type = REGEX;
        break;
    case LABEL:
        assert(arg->type == LITERAL && arg->literal->type == QUOTED);
        pattern = arg->literal->text;
        type = QUOTED;
        break;
    case KEY:
        if (arg->type == LITERAL) {
            const struct literal *l = arg->literal;
            type = l->type;
            pattern = (type == REGEX) ? l->pattern : l->text;
        } else if (arg->type == ABBREV_REF) {
            const struct literal *l = arg->abbrev->literal;
            type = l->type;
            pattern = (type == REGEX) ? l->pattern : l->text;
        } else {
            assert(0);
        }
        break;
    default:
        assert(0);
        break;
    }
    struct literal *literal = make_literal(strdup(pattern), type,
                                           action->lineno);
    return make_literal_set(literal);
}

static int make_subtree_handle(struct match *subtree, struct match *matches) {
    int changed = 0;

    list_for_each(m, matches) {
        if (ACTION_P(m, SEQ) || ACTION_P(m, LABEL) || ACTION_P(m, KEY)) {
            if (subtree->handle != NULL) {
                grammar_error(_FM(m), _L(m),
                   "Handle for subtree starting at line %d already defined",
                              subtree->lineno);
                merge_literal_set(&(subtree->handle),
                                  make_action_handle(m->action));
            } else {
                subtree->handle = make_action_handle(m->action);
                changed = 1;
            }
        }
        if (SUBMATCH_P(m) && m->type != SUBTREE) {
            if (make_subtree_handle(subtree, m->matches))
                changed = 1;
        }
    }
    return changed;
}


/* Compute the handles in each match. Return 0 if no changes were made, 
   1 otherwise */
static int make_match_handles(struct match *matches) {
    int changed = 0;

    list_for_each(cur, matches) {
        if (SUBMATCH_P(cur))
            if (make_match_handles(cur->matches))
                changed = 1;
        
        switch(cur->type) {
        case ALTERNATIVE:
            list_for_each(m, cur->matches) {
                if (merge_literal_set(&(cur->handle), m->handle))
                    changed = 1;
            }
            break;
        case SEQUENCE:
            list_for_each(m, cur->matches) {
                if (merge_literal_set(&(cur->handle), m->handle)) {
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
        case ACTION:
            /* Actions have no handles */
            break;
        case SUBTREE:
            if (cur->handle == NULL && make_subtree_handle(cur, cur->matches))
                changed = 1;
            break;
        default:
            // all others keep a NULL handle
            break;
        }
    }

    return changed;
}

/* Prepare the grammar by
 *
 * (1) computing first/follow sets for all rules and propagating epsilon
 * (2) computing handles for the actions and for alll rules/matches
 *
 * Only a grammar that has been successfully prepared can be used for
 * parsing
 */
static int prepare(struct grammar *grammar) {
    int changed;
    int result = 1;

    /* Regular expressions */
    if (! make_match_re(grammar->rules->matches))
        return 0;

    list_for_each(r, grammar->rules) {
        if (r->matches->re == NULL) {
            grammar_error(_FR(r), _L(r), "rule '%s' not used", r->name);
            result = 0;
        }
    }
    if (!result)
        return 0;

    /* Handles */
    do {
        changed = 0;
        list_for_each(r, grammar->rules) {
            if (make_match_handles(r->matches))
                changed = 1;
        }
    } while (changed);

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
        } else if (cur->type == ACTION) {
            if (! bind_match_names(grammar, cur->action->arg))
                result = 0;
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

static void bind_owner(struct rule *owner, struct match *matches) {
    list_for_each(m, matches) {
        m->owner = owner;
        if (SUBMATCH_P(m))
            bind_owner(owner, m->matches);
        if (m->type == ACTION) {
            bind_owner(owner, m->action->arg);
            for (int type = 0; action_type_names[type] != NULL; type++) {
                if (STREQ(m->action->name, action_type_names[type])) {
                    m->action->type = type;
                    break;
                }
            }
        }
    }
}

/*
 * Resolve names to their rules/abbrevs. Convert quoted strings in matches
 * to regexps by enclosing them in '\Q..\E'
 *
 * Returns 1 on success, 0 on error. After a successful return, the grammar
 * does not contain anymore NAME matches
 */
static int resolve(struct grammar *grammar) {
    list_for_each(r, grammar->rules) {
        r->grammar = grammar;
        bind_owner(r, r->matches);
        if (! bind_match_names(grammar, r->matches))
            return 0;
    }
    return 1;
}

/*
 * Constructors for some of the data structures
 */

static void tr_chars(char *s, const char *from, const char *to) {
    /* Be very careful to not alter S in any way if it doesn't need it.
       This makes it possible to run over strings that are true constants
       as long as they contain no escape sequences */
    for (char *p = s; *p != '\0'; p++) {
        if (*p == '\\') {
            char *f = strchr(from, *(p+1));
            if (f == NULL) {
                *s++ = *p;
            } else {
                p += 1;
                *s++ = to[f-from];
            }
        } else {
            if (s != p)
                *s = *p;
            s += 1;
        }
    }
    if (*s != '\0')
        *s = '\0';
}

static int set_literal_text(struct literal *literal, const char *text) {
    int r;
    const char *c;

    if (literal->type == REGEX) {
        literal->pattern = text;
    } else if (literal->type == QUOTED) {
        char *pattern, *p;

        /* Escape special characters in text since it should be taken
           literally */
        CALLOC(pattern, 2*strlen(text)+1);
        p = pattern;
        for (const char *t = text; *t != '\0'; t++) {
            if (*t == '\\') {
                *p++ = *t++;
                *p++ = *t;
            } else if (strchr(".{}[]()+*?", *t) != NULL) {
                *p++ = '\\';
                *p++ = *t;
            } else {
                *p++ = *t;
            }
        }
        *p = '\0';

        literal->text = text;
        literal->pattern = pattern;
    } else {
        grammar_error(NULL, _L(literal), "Illegal literal type %d",
                      literal->type);
        return -1;
    }
    tr_chars((char *) literal->pattern, "nrt", "\n\r\t");

    CALLOC(literal->re, 1);
    re_syntax_options = RE_SYNTAX_POSIX_MINIMAL_EXTENDED & ~(RE_DOT_NEWLINE);
    c = re_compile_pattern(literal->pattern, strlen(literal->pattern),
                           literal->re);
    if (c != NULL) {
        grammar_error(NULL, _L(literal),
                      "error %s compiling regular expression /%s/",
                      c, literal->pattern);
        return -1;
    }
    r = re_match(literal->re, "", 0, 0, NULL);
    if (r == -2) {
        internal_error(NULL, _L(literal), "empty match failed on /%s/",
                       literal->pattern);
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

    list_for_each(m, *maps) {
        list_for_each(f, m->filters) {
            if (f->path == NULL) {
                grammar_error(m->filename, _L(f), 
                              "Include filter for '%s' is missing a root path",
                              f->glob);
                ok = 0;
            }
            list_for_each(e, f->path) {
                if (e->type == E_GLOBAL && ! STREQ("basename", e->text)) {
                    grammar_error(m->filename, _L(e),
                          "Include filter '%s' uses unknown global $%s",
                                  f->glob, e->text);
                    ok = 0;
                }
            }
        }
    }

    return ok ? 0 : -1;
}

/*
 * Freeing some data structures
 */

void augs_map_free(struct map *map) {
    if (map != NULL) {
        free((void *) map->filename);
        free((void *) map->grammar_name);
        while (map->filters != NULL) {
            struct filter *f = map->filters;
            free((void *) f->glob);
            map->filters = f->next;
            free(f);
        }
        free(map);
    }
}

/*
 * Produce a dot graph for the grammar
 */

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
        case ACTION:
            name = m->action->name;
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
        fprintf(out, " | ");
        print_literal_set(out, m->handle, m->owner->grammar,
                          ' ', ' ');
        fprintf(out, "\"\n  shape = \"record\"\n");
        fprintf(out, "];\n");
        fprintf(out, "n%d -> n%d;\n", parent, self);

        if (SUBMATCH_P(m)) {
            next = dot_match(out, m->matches, self, next);
        } else if (m->type == ACTION) {
            next = dot_match(out, m->action->arg, self, next);
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
