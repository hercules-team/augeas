/* Scanner for config specs   -*- C -*- */
%option 8bit never-interactive yylineno
%option bison-bridge bison-locations
%option reentrant noyywrap
%option warn nodefault
%option outfile="lex.yy.c" prefix="spec_"

%{
#include "ast.h"
#include "spec-parse.h"

#define YY_USER_ACTION \
		yylloc->first_line = yylineno;\
		yylloc->first_column=1;\
		yylloc->last_line = yylineno;\
		yylloc->last_column=1;

#define YY_EXTRA_TYPE const char *

int spec_get_column  (yyscan_t yyscanner);
void spec_set_column (int  column_no , yyscan_t yyscanner);
int spec_init_lexer(const char *name, yyscan_t *scanner);

static int to_int(const char *str) {
  int v;
  if (sscanf(str, "%d", &v) != 1) {
    fprintf(stderr, "Weird: failed to convert int\n");
    v = -1;
  }
  return v;
}

%}

DIGIT [0-9]
DOTS  \.\.\.
DOTSQ \.\.\?
ID    [A-Za-z][A-Za-z0-9_-]*
AT    @
%x SC_AFTER_ID
%%

<*>
{
  [ \t]*#.*\n  /* eat comments */;
  [ \t\n]+     /* eat blanks */;
}

<INITIAL>
{
  \${ID}    {
               yylval->string = strndup(yytext+1, yyleng);
               return T_GLOBAL;
  }

  {DIGIT}+   {
             yylval->intval = to_int(yytext);
             return T_NUMBER;
  }

  '[^\']*'   {
               yylval->string = strndup(yytext+1, yyleng-2);
               return T_QUOTED;
  }

  \/([^/]|\\\/)+\/ {
               yylval->string = strndup(yytext+1, yyleng-2);
               return T_REGEX;
  }

  {DOTS}    {
               yylval->intval = 0;
               return T_ANY;
            }
  {DOTSQ}   {
               yylval->intval = 1;
               return T_ANY;
            }

  [|*?+()=@\.\[\]]       return yytext[0];

  grammar      return T_GRAMMAR;
  token        return T_TOKEN;
  map          return T_MAP;
  include      return T_INCLUDE;
  end          return T_END;

  {ID}         {
               yylval->string = strndup(yytext, yyleng);
               BEGIN(SC_AFTER_ID);
  }

  .             {
                  fprintf(stderr, "%s:%d: Unexpected character %c\n", spec_get_extra(yyscanner), yylineno, yytext[0]);
                }

  <<EOF>>       {
                  fclose(spec_get_in(yyscanner));
                  spec_lex_destroy(yyscanner);
                  yyterminate();
                }

}

<SC_AFTER_ID>
{
  ":" {
    BEGIN(INITIAL);
    return T_NAME_COLON;
  }
  . {
    yyless(0);
    BEGIN(INITIAL);
    return T_NAME;
  }
  <<EOF>> {
    BEGIN(INITIAL);
    return T_NAME;
  }
 
}
%%

int spec_init_lexer(const char *name, yyscan_t *scanner) {
  FILE *f;

  f = fopen(name, "r");
  if (f == NULL) {
    fprintf(stderr, "Failed to open %s\n", name);
    return -1;
  }
  spec_lex_init(scanner);
  spec_set_extra(name, *scanner);
  spec_set_in(f, *scanner);
  return 0;
}
