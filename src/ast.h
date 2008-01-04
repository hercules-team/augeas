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

#include <pcre.h>
#include "internal.h"

/*
 * Error reporting
 */
#define _L(obj)  (obj)->lineno
#define _FG(obj) (obj)->filename
#define _FR(obj) (obj)->grammar->filename
#define _FM(obj) (obj)->owner->grammar->filename

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
    PF_ADVANCE = (1 << 0),
    PF_MATCH   = (1 << 1),
    PF_TOKEN   = (1 << 2),
    PF_RULE    = (1 << 3),
    PF_ACTION  = (1 << 4)
};


/* Parse text TEXT according to GRAMMAR. FILENAME indicates what file TEXT
 * was read from.
 * LOG is used to print logging messages. FLAGS controls what is printed
 * and should be a set of flags from enum parse_flags
 */
int parse(struct grammar *grammar, struct aug_file *file, const char *text,
           FILE *log, int flags);

enum grammar_debug_flags {
    GF_NONE = 0,
    GF_ANY_RE = (1 << 0),   /* Print any expressions as full regexps */
    GF_FOLLOW = (1 << 1),   /* Print follow sets */
    GF_FIRST  = (1 << 2),   /* Print first sets/epsilon indicator */
    GF_ACTIONS  = (1 << 3),
    GF_PRETTY = (1 << 4)
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
    int                lineno;
    enum literal_type  type;
    const char        *pattern;
    const char        *text;
    pcre              *re;
    int                epsilon;
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

enum quant {
    Q_ONCE,  /* no quantifier, match exactly once */
    Q_MAYBE, /* '?' */
    Q_STAR,  /* '*' */
    Q_PLUS   /* '+' */
};

struct rule {
    struct rule       *next;
    struct grammar    *grammar;  /* the grammar this rule belongs to */
    int               lineno;
    const char        *name;
    struct match      *matches;
    struct action     *actions;
};

struct match *find_field(struct match *matches, int field);

enum match_type {
    LITERAL,      /* literal string or regex */
    NAME,         /* use a rule or abbrev */
    ANY,          /* match '...' */
    FIELD,        /* match previous field */
    ALTERNATIVE,  /* match one of a number of other matches */
    SEQUENCE,     /* match a list of matches */
    RULE_REF,     /* reference to a rule */
    ABBREV_REF    /* reference to an abbrev */
};

/* Does the match M have submatches in M->matches that might
   be treated recursively ? */
#define SUBMATCH_P(m) ((m)->type == ALTERNATIVE || (m)->type == SEQUENCE)

/* A set of literals, for the first/follow sets */
struct literal_set {
    struct literal_set   *next;
    struct literal       *literal;
};

/*
 * An item on the right hand side of a rule. After parsing the grammar,
 * only items of types LITERAL, NAME, ANY, FIELD, ALTERNATIVE and SEQUENCE
 * are used. Name binding/resolution replaces all NAME entries with either
 * RULE_REF or ABBREV_REF entries.
 *
 * ANY items have a literal allocated right after parsing, though its
 * pattern and text are null, until after first/follow sets have been
 * computed. At that point, the pattern of the literal is filled with
 * /(.+?)(?=...)/ where the lookahead assertion matches all entries in the
 * item's follow set. If there are ANY items with empty follow sets, the
 * grammar is invalid.
 *
 * EPSILON indicates if this match can match the empty string, where it is
 * obvious, it is set during parsing (literals and groupings qualified with
 * '*' or '?') , for other matches it is propagated during computing first
 * sets. 
 * 
 * The FIRST and FOLLOW sets are intially null, and filled by calling
 * prepare() on the grammar. Grammars where a rule has an empty first set
 * are rejected (they contain unused rules). TODO: reject grammars where
 * first sets contain the same entry multiple times.
 *
 */
struct match {
    struct match    *next;
    int             lineno;
    enum match_type  type;
    enum quant       quant;          /* only for NAME, ALTERNATIVE, SEQUENCE */
    union {
        struct literal *literal;     /* LITERAL, ANY */
        const char     *name;        /* NAME */
        int             field;       /* FIELD */
        struct match   *matches;     /* ALTERNATIVE, SEQUENCE */
        struct rule    *rule;        /* RULE_REF */
        struct abbrev  *abbrev;      /* ABBREV_REF */
    };
    struct rule        *owner;       /* the rule this match belongs to */
    int                 fid;         /* the number of the field        */
    int                 gid;         /* the number of the group        */
    struct literal_set *first;       /* the first set */
    struct literal_set *follow;
    int                 epsilon;     /* produces the empty string */
    struct action      *action;
};

enum action_scope {
    A_FIELD,
    A_GROUP
};

struct action {
    struct action         *next;
    int                    lineno;
    struct rule           *rule;
    enum action_scope      scope;
    int                    id;
    struct entry          *path;
    struct entry          *value;
};

enum entry_type {
    E_CONST,
    E_GLOBAL,
    E_FIELD
};

struct entry {
    struct entry   *next;
    int             lineno;
    struct action  *action;
    enum entry_type type;
    union {
        const char *text;      /* E_CONST, E_GLOBAL */
        int         field;     /* E_FIELD */
    };
};

/*
 * Helpers to print (ast.c)
 */
void print_literal(FILE *out, struct literal *l);

/* Print CNT characters from TEXT. Escape newlines/tabs */
int print_chars(FILE *out, const char *text, int cnt);

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
