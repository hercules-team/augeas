%{

#include "internal.h"
#include "ast.h"
#include "list.h"
#include <stdio.h>

#define YYDEBUG 1

struct grammar *spec_parse_file(const char *name);

/* AST cosntruction */
static struct abbrev *make_abbrev(const char *name, struct literal *literal,
                                  const char *deflt, int lineno);

static struct rule *make_rule(const char *name, struct match *matches, 
                             struct node *nodes, int lineno);

static struct match* make_match_list(struct match *head, struct match *tail,
                                     enum match_type type, int lineno);
static struct match *make_literal_match(struct literal *literal, int lineno);
static struct match *make_name_match(const char *name, enum quant quant, 
                                     int lineno);
static struct match *make_match(enum match_type type, int lineno);
 static struct match *make_any_match(int epsilon, int lineno);
static struct match *make_field_match(int field, int lineno);

static struct node *make_node(enum node_type, int lineno);

typedef void *yyscan_t;
%}
%locations
%pure-parser
%parse-param    {struct grammar **grammar}
%parse-param    {yyscan_t scanner}
%lex-param      {yyscan_t scanner}

%token <intval> T_FIELD    /* $3 */
%token <string> T_QUOTED   /* 'foo' */
%token <string> T_REGEX    /* '/[ \t]+/' */
%token <string> T_NAME     /* [a-z]+ */
%token <string> T_NAME_COLON
%token <string> T_GLOBAL   /* '$seq' or '$file_name' */
%token <intval> T_ANY      /* '...' or '..?' */
 /* Keywords */
%token          T_NODE
%token          T_GRAMMAR
%token          T_TOKEN

%union {
  struct grammar  *grammar;
  struct abbrev  *abbrev;
  struct rule    *rule;
  struct literal *literal;
  struct match   *match;
  struct action  *action;
  struct node    *node;
  enum quant     quant;
  char           *string;
  int            intval;
}

%type <abbrev>  tokens token
%type <rule>    rules rule
%type <literal> literal
%type <string>  token_opts
%type <match>   match match_seq match_prim
%type <node>    node nodes node_value
%type <quant>   match_quant

%{
/* Lexer */
extern int spec_lex (YYSTYPE * yylval_param,YYLTYPE * yylloc_param ,yyscan_t yyscanner);
int spec_init_lexer(const char *name, yyscan_t *scanner);
int spec_get_lineno (yyscan_t yyscanner );
const char *spec_get_extra (yyscan_t yyscanner );
char *spec_get_text (yyscan_t yyscanner );

void spec_error(YYLTYPE *locp, struct grammar **grammar, 
                yyscan_t scanner, const char *s);
%}

%%

start: T_GRAMMAR '{' tokens rules '}'
       { 
         (*grammar)->abbrevs = $3;
         (*grammar)->rules = $4;
         (*grammar)->lineno = @1.first_line; 
       }

tokens: token
        { $$ = $1; } 
      | tokens token
        { $$ = $1; list_append($1, $2); }

token: T_TOKEN T_NAME literal token_opts
        { $$ = make_abbrev($2, $3, $4, @1.first_line); }

token_opts: /* empty */
            { $$ = NULL; }
          | '=' T_QUOTED
            { $$ = $2; }

rules: rule
       { $$ = $1; }
     | rules rule
       { $$ = $1; list_append($1, $2); }

rule: T_NAME_COLON match
      { $$ = make_rule($1, $2, NULL, @1.first_line); }
    | T_NAME_COLON match '{' nodes '}'
      { $$ = make_rule($1, $2, $4, @1.first_line); }

/* Matches describe the structure of the file */
match: match_seq
       { $$  = $1;  }
     | match '|' match_seq
       { $$ = make_match_list($1, $3, ALTERNATIVE, @1.first_line); }

match_seq: match_prim
           { $$ = $1; }
         | match_seq match_prim
           { $$ = make_match_list($1, $2, SEQUENCE, @1.first_line); }

match_prim:  literal
            { $$ = make_literal_match($1, @1.first_line); }
          | T_NAME match_quant
            { $$ = make_name_match($1, $2, @1.first_line); }
          | T_ANY
            { $$ = make_any_match($1, @1.first_line); }
          | T_FIELD
            { $$ = make_field_match($1, @1.first_line); }
          | '(' match ')' match_quant
            { $$ = $2; $$->quant = $4;
              $$->epsilon = ($$->quant == Q_MAYBE || $$->quant == Q_STAR);
            }


match_quant: /* empty */
             { $$ = Q_ONCE; }
           | '*'
             { $$ = Q_STAR; }
           | '+'
             { $$ = Q_PLUS; }
           | '?'
             { $$ = Q_MAYBE; }

literal: T_QUOTED
         { $$ = make_literal($1, QUOTED, @1.first_line); }
       | T_REGEX
         { $$ = make_literal($1, REGEX, @1.first_line); }

/* Actions describe the transformation from matches into the tree */
nodes: node_value
       { $$ = $1; }
     | nodes '{' nodes '}'
       {
         if ($1->children == NULL)
           $1->children = $3;
         else
           list_append($1, $3);
         $$ = $1;
       }
     | nodes nodes
       { $$ = $1; list_append($1, $2); }

node_value: node
{ $$ = $1; }
| node '=' T_QUOTED
{ $$ = $1; $$->val_type = N_QUOTED; $$->val_label = $3; }
| node '=' T_FIELD
{ $$ = $1; $$->val_type = N_FIELD; $$->val_field = $3; }

node: T_NODE T_GLOBAL
      { $$ = make_node(N_GLOBAL, @1.first_line); $$->label = $2; }
    | T_NODE T_QUOTED
      { $$ = make_node(N_QUOTED, @1.first_line); $$->label = $2; }
    | T_NODE T_FIELD
      { $$ = make_node(N_FIELD, @1.first_line); $$->field = $2; }

%%

struct grammar *spec_parse_file(const char *name) {
  struct grammar *result;
  yyscan_t      scanner;
  int r;

  CALLOC(result, 1);
  result->lineno = 0;
  result->filename = strdup(name);

  if (spec_init_lexer(name, &scanner) == -1) {
    spec_error(NULL, &result, NULL, "Could not open input");
    return NULL;
  }

  yydebug = getenv("YYDEBUG") != NULL;
  r = spec_parse(&result, scanner);
  if (r == 1) {
    spec_error(NULL, &result, NULL, "Parsing failed - syntax error");
    goto error;
  } else if (r == 2) {
    spec_error(NULL, &result, NULL, "Ran out of memory");
    goto error;
  }
  return result;

 error:
  // free result
  return NULL;
}

// FIXME: Nothing here checks for alloc errors.
struct abbrev *make_abbrev(const char *name, struct literal *literal,
                           const char *deflt, int lineno) {
  struct abbrev *result;

  CALLOC(result, 1);
  result->lineno = lineno;
  result->name = name;
  result->literal = literal;
  if (deflt != NULL) {
    if (literal->type == QUOTED) {
      grammar_error(NULL, lineno, "Ignoring default for quoted %s", name);
    } else {
      literal->text = deflt;
    }
  }
  return result;
}

struct rule *make_rule(const char *name, struct match *matches, 
                       struct node *nodes, int lineno) {
  struct rule *result;

  CALLOC(result, 1);
  result->lineno = lineno;
  result->name = name;
  result->matches = matches;
  result->nodes = nodes;
  return result;
}

struct match *make_match_list(struct match *head, struct match *tail, 
                              enum match_type type, int lineno) {
  struct match *result;

  if (head->type != type) {
    result = make_match(type, lineno);
    result->matches = head;
  } else {
    result = head;
  }
  list_append(result->matches, tail);
  return result;
}

struct match *make_literal_match(struct literal *literal, int lineno) {
  struct match *result;

  if (literal == NULL)
    internal_error(NULL, lineno, "NULL literal\n");

  result = make_match(LITERAL, lineno);
  result->literal = literal;
  return result;
}

struct match *make_name_match(const char *name, enum quant quant, int lineno) {
  struct match *result;

  if (name == NULL)
    internal_error(NULL, lineno, "NULL name\n");

  result = make_match(NAME, lineno);
  result->name = name;
  result->quant = quant;
  result->epsilon = (quant == Q_MAYBE || quant == Q_STAR);
  return result;
}

struct match *make_match(enum match_type type, int lineno) {
  struct match *result;

  CALLOC(result, 1);
  result->type = type;
  result->lineno = lineno;
  return result;
}

struct match *make_any_match(int epsilon, int lineno) {
  struct match *result;

  result = make_match(ANY, lineno);
  result->literal = make_literal(NULL, REGEX, lineno);
  result->epsilon = epsilon;
  return result;
}

struct match *make_field_match(int field, int lineno) {
  struct match *result;

  result = make_match(FIELD, lineno);
  result->field = field;
  return result;
}

struct node *make_node(enum node_type type, int lineno) {
  struct node *result;

  CALLOC(result, 1);
  result->type = type;
  result->lineno = lineno;
  return result;
}

void spec_error(YYLTYPE *locp, 
                struct grammar **grammar, 
                yyscan_t scanner,
                const char *s) {
  if (scanner != NULL) {
    int line;
    line = spec_get_lineno(scanner);
    grammar_error(spec_get_extra(scanner), line, "%s reading %s", s,
                  spec_get_text(scanner));
  } else {
    int line = (*grammar)->lineno;
    if (locp != NULL)
      line = locp->first_line;
    grammar_error((*grammar)->filename, line, "error: %s\n", s);
  }
}
