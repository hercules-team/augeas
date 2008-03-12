/*
 * ast.h: Structures to represent the config file specs
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

#ifndef __AST_H
#define __AST_H

#include <regex.h>
#include "internal.h"

/*
 * Error reporting
 */
#define _L(obj)  (obj)->lineno
/* Various ways of getting at a filename */
#define _FG(obj) (obj)->filename
#define _FR(obj) (obj)->grammar->filename
#define _FM(obj) (obj)->owner->grammar->filename
#define _FA(obj) _FM((obj)->match)

void grammar_error(const char *fname, int lineno,
                   const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 3, 4);

void internal_error_at(const char *srcfile, int srclineno,
                       const char *fname, int lineno,
                       const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 5, 6);

#define internal_error(fname, lineno, format, args ...)                   \
    internal_error_at(__FILE__, __LINE__, fname, lineno, format, ## args)

/*
 * Mappings of files into the tree. Each mapping specifies a grammar to use
 * to do the actual parsing/mapping and a list of filters describing
 * which files to map.
 */
struct map {
    struct map     *next;
    const char     *filename;
    int             lineno;
    struct grammar *grammar;
    const  char    *grammar_name;
    struct filter  *filters;
};

void augs_map_free(struct map *map);

/*
 * A filter is a list of glob patterns describing which files to
 * include.
 */
struct filter {
    struct filter  *next;
    int             lineno;
    const char     *glob;
    struct entry   *path;
};

/*
 * Representation of grammars for parsing/mapping files into the tree
 */

struct grammar {
    struct grammar *next;
    const char     *filename;
    int             lineno;
    const char     *name;
    struct abbrev  *abbrevs;
    struct rule    *rules;
};

/* Flags to control debug printing during parsing */
enum parse_debug_flags {
    PF_NONE    = 0,
    PF_ADVANCE = (1 << 0),  /* Show how the lexer advances through the input */
    PF_MATCH   = (1 << 1),  /* Show regex matches */
    PF_TOKEN   = (1 << 2)   /* Show tokenization */
};


/* Parse text TEXT according to GRAMMAR. FILENAME indicates what file TEXT
 * was read from.
 * LOG is used to print logging messages. FLAGS controls what is printed
 * and should be a set of flags from enum parse_flags
 */
struct tree *parse(struct aug_file *file, const char *text, 
                   FILE *log, int flags);

enum grammar_debug_flags {
    GF_NONE = 0,
    GF_HANDLES = (1 << 3),
    GF_RE     = (1 << 4),
    GF_PRETTY = (1 << 5),
    GF_DOT    = (1 << 6),         /* Produce a dot graph for the grammar */
    GF_LENS_TYPE_CHECK = (1 << 7) /* Do (expensive) lens type checks */
};

/*
 * Load a spec file FILENAME. Return -1 on error, 0 on success.  The list
 * of grammars read from the file is assigned to GRAMMARS, and the list of
 * maps to MAPS. LOG is used to print logging messages. FLAGS controls
 * what is printed and should be a set of flags from enum
 * grammar_debug_flags
 */
int load_spec(const char *filename, FILE *log, int flags,
              struct grammar **grammars, struct map **map);

enum literal_type {
    QUOTED,
    REGEX
};

/* A literal regex or quoted string. For a regex, PATTERN contains the
 * original pattern, and TEXT the default text the user specified if this
 * literal is part of an abbrev. For a quoted string, TEXT contains the raw
 * text, and PATTERN the regex that matches TEXT.
 */
struct literal {
    int                       lineno;
    enum literal_type         type;
    const char               *pattern;
    const char               *text;
    struct re_pattern_buffer *re;
    int                       epsilon;
};

/* Allocate a new literal. TEXT is not copied any further, but will be
 * freed when the literal is freed
*/
struct literal *make_literal(const char *text, enum literal_type type,
                             int lineno);

/* A 'token' in the grammar; we reserve the term 'token' for
   something we read from a config file, and call the tokens
   from grammar specs 'abbreviations'
*/

struct abbrev {
    struct abbrev  *next;
    int            lineno;
    const char     *name;
    struct literal *literal;    
};

struct rule {
    struct rule       *next;
    struct grammar    *grammar;  /* the grammar this rule belongs to */
    int               lineno;
    const char        *name;
    struct match      *matches;
};

struct match *find_field(struct match *matches, int field);

enum match_type {
    ACTION,       /* call of one of the builtin functions */
    SUBTREE,      /* enter a subtree */
    LITERAL,      /* literal string or regex */
    NAME,         /* use a rule or abbrev */
    ALTERNATIVE,  /* match one of a number of other matches */
    SEQUENCE,     /* match a list of matches */
    RULE_REF,     /* reference to a rule */
    ABBREV_REF,   /* reference to an abbrev */
    QUANT_PLUS,
    QUANT_STAR,
    QUANT_MAYBE
};

#define QUANT_P(m) ((m)->type == QUANT_PLUS                             \
                    || (m)->type == QUANT_STAR                          \
                    || (m)->type == QUANT_MAYBE)

/* Does the match M have submatches in M->matches that might
   be treated recursively ? */
#define SUBMATCH_P(m) ((m)->type == ALTERNATIVE                         \
                       || (m)->type == SEQUENCE                         \
                       || (m)->type == SUBTREE                          \
                       || QUANT_P(m))

/* A set of literals, for the handles */
struct literal_set {
    struct literal_set   *next;
    struct literal       *literal;
};

/*
 * An item on the right hand side of a rule. After parsing the grammar,
 * only items of types LITERAL, NAME, FIELD, ALTERNATIVE and SEQUENCE
 * are used. Name binding/resolution replaces all NAME entries with either
 * RULE_REF or ABBREV_REF entries.
 *
 * EPSILON indicates if this match can match the empty string, where it is
 * obvious.
 *
 * A HANDLE is similar to a first set, though it is used when translating
 * back from the config tree to the underlying file. The handle guides the
 * traversal of the AST for the file when new nodes have been inserted into
 * the config tree. Primitives are handles for actions (computed based on
 * how they change the current path) and are then propagated throughout the
 * grammar.
 *
 */
struct match {
    struct match    *next;
    int             lineno;
    enum match_type  type;
    union {
        struct literal *literal;     /* LITERAL */
        const char     *name;        /* NAME */
        struct match   *matches;   /* SUBTREE, ALTERNATIVE, SEQUENCE, QUANT_* */
        struct rule    *rule;        /* RULE_REF */
        struct abbrev  *abbrev;      /* ABBREV_REF */
        struct action  *action;      /* ACTION */
    };
    struct rule        *owner;       /* the rule this match belongs to */
    int                 epsilon;     /* produces the empty string */
    struct literal_set *handle;
    struct literal     *re;
};

/* A hardwired list of actions, which are all functions with
   one argument ARG */
#define ACTION_P(match, atype) ((match)->type == ACTION &&              \
                                (match)->action->type == atype)

enum action_type {
    UNDEF,          /* NAME is garbage */
    COUNTER,        /* Initialize counter named ARG */
    SEQ,            /* Next value from counter ARG */
    LABEL,          /* Use ARG as path component in tree */
    STORE,          /* Parse ARG and store result in tree */
    KEY             /* Parse ARG and use as path component in tree */
};

struct action {
    int                    lineno;
    enum action_type       type;
    const char            *name;
    struct match          *arg;    /* Only LITERAL, NAME, ABBREV_REF */
};

enum entry_type {
    E_CONST,
    E_GLOBAL
};

struct entry {
    struct entry   *next;
    int             lineno;
    struct action  *action;
    enum entry_type type;
    const char     *text;
};


struct skel {
    struct skel *next;
    struct match *match;
    int lineno;
    enum match_type type;
    union {
        const char *text;    /* LITERAL */
        struct skel *skels;  /* SEQUENCE, QUANT_* */
    };
    /* Also type == SUBTREE, with no data in the union */
};

/* A dictionary that maps key to a list of (skel, dict) */
struct dict_entry {
    struct dict_entry *next;
    struct skel *skel;
    struct dict *dict;
};

struct dict {
    struct dict *next;
    const char *key;
    struct dict_entry *entry;
    struct dict_entry *mark; /* Mark that will never change */
};

/*
 * Helpers to print (ast.c)
 */
void print_literal(FILE *out, struct literal *l);
void print_literal_set(FILE *out, struct literal_set *set,
                       struct grammar *grammar,
                       char begin, char end);

/* Print CNT characters from TEXT. Escape newlines/tabs */
int print_chars(FILE *out, const char *text, int cnt);

/* in put.c */
void put(FILE *out, struct tree *tree, struct aug_file *file);

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
