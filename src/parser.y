%{

#include <config.h>

#include "internal.h"
#include "syntax.h"
#include "list.h"
#include "errcode.h"
#include <stdio.h>

/* Work around a problem on FreeBSD where Bison looks for _STDLIB_H
 * to see if stdlib.h has been included, but the system includes
 * use _STDLIB_H_
 */
#if HAVE_STDLIB_H && ! defined _STDLIB_H
#  include <stdlib.h>
#  define _STDLIB_H 1
#endif

#define YYDEBUG 1

int augl_parse_file(struct augeas *aug, const char *name, struct term **term);

typedef void *yyscan_t;
typedef struct info YYLTYPE;
#define YYLTYPE_IS_DECLARED 1
/* The lack of reference counting on filename is intentional */
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
  do {                                                                  \
    (Current).filename = augl_get_info(scanner)->filename;              \
    (Current).error = augl_get_info(scanner)->error;                    \
    if (YYID (N)) {                                                     \
        (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;          \
        (Current).first_column = YYRHSLOC (Rhs, 1).first_column;        \
        (Current).last_line    = YYRHSLOC (Rhs, N).last_line;           \
        (Current).last_column  = YYRHSLOC (Rhs, N).last_column;         \
    } else {                                                            \
      (Current).first_line   = (Current).last_line   =                  \
	    YYRHSLOC (Rhs, 0).last_line;                                    \
	  (Current).first_column = (Current).last_column =                  \
	    YYRHSLOC (Rhs, 0).last_column;                                  \
    }                                                                   \
  } while (0)
%}

%code provides {
#include "info.h"

/* Track custom scanner state */
struct state {
  struct info *info;
  unsigned int comment_depth;
};

}

%locations
%error-verbose
%name-prefix="augl_"
%defines
%pure-parser
%parse-param    {struct term **term}
%parse-param    {yyscan_t scanner}
%lex-param      {yyscan_t scanner}

%initial-action {
  @$.first_line   = 1;
  @$.first_column = 0;
  @$.last_line    = 1;
  @$.last_column  = 0;
  @$.filename     = augl_get_info(scanner)->filename;
  @$.error        = augl_get_info(scanner)->error;
};

%token <string> DQUOTED   /* "foo" */
%token <regexp> REGEXP    /* /[ \t]+/ */
%token <string> LIDENT UIDENT QIDENT
%token          ARROW

/* Keywords */
%token          KW_MODULE
%token          KW_AUTOLOAD
%token          KW_LET KW_LET_REC KW_IN
%token          KW_STRING
%token          KW_REGEXP
%token          KW_LENS
%token          KW_TEST KW_GET KW_PUT KW_AFTER

%union {
  struct term    *term;
  struct type    *type;
  struct ident   *ident;
  struct tree    *tree;
  char           *string;
  struct {
    int             nocase;
    char           *pattern;
  } regexp;
  int            intval;
  enum quant_tag quant;
}

%type<term>   start decls
%type<term>   exp composeexp unionexp minusexp catexp appexp rexp aexp
%type<term>   param param_list
%type<string> qid id autoload
%type<type>  type atype
%type<quant> rep
%type<term>  test_exp
%type<intval> test_special_res
%type<tree>  tree_const tree_const2 tree_branch
%type<string> tree_label

%{
/* Lexer */
extern int augl_lex (YYSTYPE * yylval_param,struct info * yylloc_param ,yyscan_t yyscanner);
int augl_init_lexer(struct state *state, yyscan_t * scanner);
void augl_close_lexer(yyscan_t *scanner);
int augl_lex_destroy (yyscan_t yyscanner );
int augl_get_lineno (yyscan_t yyscanner );
int augl_get_column  (yyscan_t yyscanner);
struct info *augl_get_info(yyscan_t yyscanner);
char *augl_get_text (yyscan_t yyscanner );

static void augl_error(struct info *locp, struct term **term,
                       yyscan_t scanner, const char *s);

/* TERM construction */
 static struct info *clone_info(struct info *locp);
 static struct term *make_module(char *ident, char *autoload,
                                 struct term *decls,
                                 struct info *locp);

 static struct term *make_bind(char *ident, struct term *params,
                             struct term *exp, struct term *decls,
                             struct info *locp);
 static struct term *make_bind_rec(char *ident, struct term *exp,
                                   struct term *decls, struct info *locp);
 static struct term *make_let(char *ident, struct term *params,
                              struct term *exp, struct term *body,
                              struct info *locp);
 static struct term *make_binop(enum term_tag tag,
                               struct term *left, struct term *right,
                               struct info *locp);
 static struct term *make_unop(enum term_tag tag,
                              struct term *exp, struct info *locp);
 static struct term *make_ident(char *qname, struct info *locp);
 static struct term *make_unit_term(struct info *locp);
 static struct term *make_string_term(char *value, struct info *locp);
 static struct term *make_regexp_term(char *pattern,
                                      int nocase, struct info *locp);
 static struct term *make_rep(struct term *exp, enum quant_tag quant,
                             struct info *locp);

 static struct term *make_get_test(struct term *lens, struct term *arg,
                                   struct info *info);
 static struct term *make_put_test(struct term *lens, struct term *arg,
                                   struct term *cmds, struct info *info);
 static struct term *make_test(struct term *test, struct term *result,
                               enum test_result_tag tr_tag,
                               struct term *decls, struct info *locp);
 static struct term *make_tree_value(struct tree *, struct info*);
 static struct tree *tree_concat(struct tree *, struct tree *);

#define LOC_MERGE(a, b, c)                                              \
 do {                                                                   \
   (a).filename     = (b).filename;                                     \
   (a).first_line   = (b).first_line;                                   \
   (a).first_column = (b).first_column;                                 \
   (a).last_line    = (c).last_line;                                    \
   (a).last_column  = (c).last_column;                                  \
   (a).error        = (b).error;                                        \
 } while(0);

%}

%%

start: KW_MODULE UIDENT '=' autoload decls
       { (*term) = make_module($2, $4, $5, &@1); }

autoload: KW_AUTOLOAD LIDENT
          { $$ = $2; }
        | /* empty */
          { $$ = NULL; }

decls: KW_LET LIDENT param_list '=' exp decls
       {
         LOC_MERGE(@1, @1, @5);
         $$ = make_bind($2, $3, $5, $6, &@1);
       }
     | KW_LET_REC LIDENT '=' exp decls
       {
         LOC_MERGE(@1, @1, @4);
         $$ = make_bind_rec($2, $4, $5, &@1);
       }
     | KW_TEST test_exp '=' exp decls
       {
         LOC_MERGE(@1, @1, @4);
         $$ = make_test($2, $4, TR_CHECK, $5, &@1);
       }
     | KW_TEST test_exp '=' test_special_res decls
       {
         LOC_MERGE(@1, @1, @4);
         $$ = make_test($2, NULL, $4, $5, &@1);
       }
     | /* epsilon */
       { $$ = NULL; }

/* Test expressions and results */

test_exp: aexp KW_GET exp
          { $$ = make_get_test($1, $3, &@$); }
        | aexp KW_PUT aexp KW_AFTER exp
          { $$ = make_put_test($1, $3, $5, &@$); }

test_special_res: '?'
                  { $$ = TR_PRINT; }
                | '*'
                  { $$ = TR_EXN; }

/* General expressions */
exp: KW_LET LIDENT param_list '=' exp KW_IN  exp
     {
       LOC_MERGE(@1, @1, @6);
       $$ = make_let($2, $3, $5, $7, &@1);
     }
   | composeexp

composeexp: composeexp ';' unionexp
     { $$ = make_binop(A_COMPOSE, $1, $3, &@$); }
   | unionexp
     { $$ = $1; }

unionexp: unionexp '|' minusexp
     { $$ = make_binop(A_UNION, $1, $3, &@$); }
   | minusexp
     { $$ = $1; }
   | tree_const
     { $$ = make_tree_value($1, &@1); }

minusexp: minusexp '-' catexp
     { $$ = make_binop(A_MINUS, $1, $3, &@$); }
   | catexp
     { $$ = $1; }

catexp: catexp '.' appexp
{ $$ = make_binop(A_CONCAT, $1, $3, &@$); }
      | appexp
{ $$ = $1; }

appexp: appexp rexp
        { $$ = make_binop(A_APP, $1, $2, &@$); }
      | rexp
        { $$ = $1; }

aexp: qid
      { $$ = make_ident($1, &@1); }
    | DQUOTED
      { $$ = make_string_term($1, &@1); }
    | REGEXP
      { $$ = make_regexp_term($1.pattern, $1.nocase, &@1); }
    | '(' exp ')'
      { $$ = $2; }
    | '[' exp ']'
      { $$ = make_unop(A_BRACKET, $2, &@$); }
    | '(' ')'
      { $$ = make_unit_term(&@$); }

rexp: aexp rep
      { $$ = make_rep($1, $2, &@$); }
    | aexp
      { $$ = $1; }

rep: '*'
     { $$ = Q_STAR; }
   | '+'
     { $$ = Q_PLUS; }
   | '?'
     { $$ = Q_MAYBE; }

qid: LIDENT
     { $$ = $1; }
   | QIDENT
     { $$ = $1; }
   | KW_GET
     { $$ = strdup("get"); }
   | KW_PUT
     { $$ = strdup("put"); }

param_list: param param_list
            { $$ = $2; list_cons($$, $1); }
          | /* epsilon */
            { $$ = NULL; }

param: '(' id ':' type ')'
       { $$ = make_param($2, $4, clone_info(&@1)); }

id: LIDENT
    { $$ = $1; }
  | KW_GET
    { $$ = strdup("get"); }
  | KW_PUT
    { $$ = strdup("put"); }

type: atype ARROW type
      { $$ = make_arrow_type($1, $3); }
    | atype
      { $$ = $1; }

atype: KW_STRING
       { $$ = make_base_type(T_STRING); }
     | KW_REGEXP
       { $$ = make_base_type(T_REGEXP); }
     | KW_LENS
       { $$ = make_base_type(T_LENS); }
     | '(' type ')'
       { $$ = $2; }

tree_const: tree_const '{' tree_branch '}'
            { $$ = tree_concat($1, $3); }
          | '{' tree_branch '}'
            { $$ = tree_concat($2, NULL); }

tree_const2: tree_const2 '{' tree_branch '}'
            {
              $$ = tree_concat($1, $3);
            }
          | /* empty */
            { $$ = NULL; }

tree_branch: tree_label tree_const2
             {
               $$ = make_tree($1, NULL, NULL, $2);
             }
           | tree_label '=' DQUOTED tree_const2
             {
               $$ = make_tree($1, $3, NULL, $4);
             }
tree_label: DQUOTED
          | /* empty */
            { $$ = NULL; }
%%

int augl_parse_file(struct augeas *aug, const char *name,
                    struct term **term) {
  yyscan_t          scanner;
  struct state      state;
  struct string  *sname = NULL;
  struct info    info;
  int result = -1;
  int r;

  *term = NULL;

  r = make_ref(sname);
  ERR_NOMEM(r < 0, aug);

  sname->str = strdup(name);
  ERR_NOMEM(sname->str == NULL, aug);

  MEMZERO(&info, 1);
  info.ref = UINT_MAX;
  info.filename = sname;
  info.error = aug->error;

  MEMZERO(&state, 1);
  state.info = &info;
  state.comment_depth = 0;

  if (augl_init_lexer(&state, &scanner) < 0) {
    augl_error(&info, term, NULL, "file not found");
    goto error;
  }

  yydebug = getenv("YYDEBUG") != NULL;
  r = augl_parse(term, scanner);
  augl_close_lexer(scanner);
  augl_lex_destroy(scanner);
  if (r == 1) {
    augl_error(&info, term, NULL, "syntax error");
    goto error;
  } else if (r == 2) {
    augl_error(&info, term, NULL, "parser ran out of memory");
    ERR_NOMEM(1, aug);
  }
  result = 0;

 error:
  unref(sname, string);
  // free TERM
  return result;
}

// FIXME: Nothing here checks for alloc errors.
static struct info *clone_info(struct info *locp) {
  struct info *info;
  make_ref(info);
  info->filename     = ref(locp->filename);
  info->first_line   = locp->first_line;
  info->first_column = locp->first_column;
  info->last_line    = locp->last_line;
  info->last_column  = locp->last_column;
  info->error        = locp->error;
  return info;
}

static struct term *make_term_locp(enum term_tag tag, struct info *locp) {
  struct info *info = clone_info(locp);
  return make_term(tag, info);
}

static struct term *make_module(char *ident, char *autoload,
                                struct term *decls,
                                struct info *locp) {
  struct term *term = make_term_locp(A_MODULE, locp);
  term->mname = ident;
  term->autoload = autoload;
  term->decls = decls;
  return term;
}

static struct term *make_bind(char *ident, struct term *params,
                              struct term *exp, struct term *decls,
                              struct info *locp) {
  struct term *term = make_term_locp(A_BIND, locp);
  if (params != NULL)
    exp = build_func(params, exp);

  term->bname = ident;
  term->exp = exp;
  list_cons(decls, term);
  return decls;
}

static struct term *make_bind_rec(char *ident, struct term *exp,
                                  struct term *decls, struct info *locp) {
  /* Desugar let rec IDENT = EXP as
   *  let IDENT =
   *    let RLENS = (lns_make_rec) in
   *    lns_check_rec ((lambda IDENT: EXP) RLENS) RLENS
   * where RLENS is a brandnew recursive lens.
   *
   * That only works since we know that 'let rec' is only defined for lenses,
   * not general purposes functions, i.e. we know that IDENT has type 'lens'
   *
   * The point of all this is that we make it possible to put a recursive
   * lens (which is a placeholder for the actual recursion) into arbitrary
   * places in some bigger lens and then have LNS_CHECK_REC rattle through
   * to do the special-purpose typechecking.
   */
  char *id;
  struct info *info = exp->info;
  struct term *lambda = NULL, *rlens = NULL;
  struct term *app1 = NULL, *app2 = NULL, *app3 = NULL;

  id = strdup(ident);
  if (id == NULL) goto error;

  lambda = make_param(id, make_base_type(T_LENS), ref(info));
  if (lambda == NULL) goto error;
  id = NULL;

  build_func(lambda, exp);

  rlens = make_term(A_VALUE, ref(exp->info));
  if (rlens == NULL) goto error;
  rlens->value = lns_make_rec(ref(exp->info));
  if (rlens->value == NULL) goto error;
  rlens->type = make_base_type(T_LENS);

  app1 = make_app_term(lambda, rlens, ref(info));
  if (app1 == NULL) goto error;

  id = strdup(LNS_CHECK_REC_NAME);
  if (id == NULL) goto error;
  app2 = make_app_ident(id, app1, ref(info));
  if (app2 == NULL) goto error;
  id = NULL;

  app3 = make_app_term(app2, ref(rlens), ref(info));
  if (app3 == NULL) goto error;

  return make_bind(ident, NULL, app3, decls, locp);

 error:
  free(id);
  unref(lambda, term);
  unref(rlens, term);
  unref(app1, term);
  unref(app2, term);
  unref(app3, term);
  return NULL;
}

static struct term *make_let(char *ident, struct term *params,
                             struct term *exp, struct term *body,
                             struct info *locp) {
  /* let f (x:string) = "f " . x in
     f "a" . f "b" */
  /* (lambda f: f "a" . f "b") (lambda x: "f " . x) */
  /* (lambda IDENT: BODY) (lambda PARAMS: EXP) */
  /* Desugar as (lambda IDENT: BODY) (lambda PARAMS: EXP) */
  struct term *term = make_term_locp(A_LET, locp);
  struct term *p = make_param(ident, NULL, ref(term->info));
  term->left = build_func(p, body);
  if (params != NULL)
    term->right = build_func(params, exp);
  else
    term->right = exp;
  return term;
}

static struct term *make_binop(enum term_tag tag,
                              struct term *left, struct term *right,
                              struct info *locp) {
  assert(tag == A_COMPOSE || tag == A_CONCAT
         || tag == A_UNION || tag == A_APP || tag == A_MINUS);
  struct term *term = make_term_locp(tag, locp);
  term->left = left;
  term->right = right;
  return term;
}

static struct term *make_unop(enum term_tag tag, struct term *exp,
                             struct info *locp) {
  assert(tag == A_BRACKET);
  struct term *term = make_term_locp(tag, locp);
  term->brexp = exp;
  return term;
}

static struct term *make_ident(char *qname, struct info *locp) {
  struct term *term = make_term_locp(A_IDENT, locp);
  term->ident = make_string(qname);
  return term;
}

static struct term *make_unit_term(struct info *locp) {
  struct term *term = make_term_locp(A_VALUE, locp);
  term->value = make_unit(ref(term->info));
  term->type = make_base_type(T_UNIT);
  return term;
}

static struct term *make_string_term(char *value, struct info *locp) {
  struct term *term = make_term_locp(A_VALUE, locp);
  term->value = make_value(V_STRING, ref(term->info));
  term->value->string = make_string(value);
  term->type = make_base_type(T_STRING);
  return term;
}

static struct term *make_regexp_term(char *pattern, int nocase,
                                     struct info *locp) {
  struct term *term = make_term_locp(A_VALUE, locp);
  term->value = make_value(V_REGEXP, ref(term->info));
  term->type = make_base_type(T_REGEXP);
  term->value->regexp = make_regexp(term->info, pattern, nocase);
  return term;
}

static struct term *make_rep(struct term *exp, enum quant_tag quant,
                            struct info *locp) {
  struct term *term = make_term_locp(A_REP, locp);
  term->quant = quant;
  term->exp = exp;
  return term;
}

static struct term *make_get_test(struct term *lens, struct term *arg,
                                  struct info *locp) {
  /* Return a term for "get" LENS ARG */
  struct info *info = clone_info(locp);
  struct term *term = make_app_ident(strdup("get"), lens, info);
  term = make_app_term(term, arg, ref(info));
  return term;
}

static struct term *make_put_test(struct term *lens, struct term *arg,
                                  struct term *cmds, struct info *locp) {
  /* Return a term for "put" LENS (CMDS ("get" LENS ARG)) ARG */
  struct term *term = make_get_test(lens, arg, locp);
  term = make_app_term(cmds, term, ref(term->info));
  struct term *put = make_app_ident(strdup("put"), ref(lens), ref(term->info));
  put = make_app_term(put, term, ref(term->info));
  put = make_app_term(put, ref(arg), ref(term->info));
  return put;
}

static struct term *make_test(struct term *test, struct term *result,
                              enum test_result_tag tr_tag,
                              struct term *decls, struct info *locp) {
  struct term *term = make_term_locp(A_TEST, locp);
  term->tr_tag = tr_tag;
  term->test = test;
  term->result = result;
  term->next = decls;
  return term;
}

static struct term *make_tree_value(struct tree *tree, struct info *locp) {
  struct term *term = make_term_locp(A_VALUE, locp);
  struct value *value = make_value(V_TREE, ref(term->info));
  value->origin = make_tree_origin(tree);
  term->type = make_base_type(T_TREE);
  term->value = value;
  return term;
}

static struct tree *tree_concat(struct tree *t1, struct tree *t2) {
  if (t2 != NULL)
    list_append(t1, t2);
  return t1;
}

void augl_error(struct info *locp,
                struct term **term,
                yyscan_t scanner,
                const char *s) {
  struct info info;
  struct string string;
  MEMZERO(&info, 1);
  info.ref = string.ref = UINT_MAX;
  info.filename = &string;

  if (locp != NULL) {
    info.first_line   = locp->first_line;
    info.first_column = locp->first_column;
    info.last_line    = locp->last_line;
    info.last_column  = locp->last_column;
    info.filename->str = locp->filename->str;
    info.error = locp->error;
  } else if (scanner != NULL) {
    info.first_line   = augl_get_lineno(scanner);
    info.first_column = augl_get_column(scanner);
    info.last_line    = augl_get_lineno(scanner);
    info.last_column  = augl_get_column(scanner);
    info.filename     = augl_get_info(scanner)->filename;
    info.error        = augl_get_info(scanner)->error;
  } else if (*term != NULL && (*term)->info != NULL) {
    memcpy(&info, (*term)->info, sizeof(info));
  } else {
    info.first_line = info.last_line = 0;
    info.first_column = info.last_column = 0;
  }
  syntax_error(&info, "%s", s);
}
