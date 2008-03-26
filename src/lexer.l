/* Scanner for config specs   -*- C -*- */
%option 8bit never-interactive yylineno
%option bison-bridge bison-locations
%option reentrant noyywrap
%option warn nodefault
%option outfile="lex.yy.c" prefix="augl_"

%{
#include "syntax.h"

typedef struct info YYLTYPE;
#define YYLTYPE_IS_DECLARED 1

#include "parser.h"

/* Advance of NUM lines. */
# define LOCATION_LINES(Loc, Num)             \
  (Loc).last_column = 0;                      \
  (Loc).last_line += Num;

/* Restart: move the first cursor to the last position. */
# define LOCATION_STEP(Loc)                   \
  (Loc).first_column = (Loc).last_column;     \
  (Loc).first_line = (Loc).last_line;

/* The lack of reference counting for filename is intentional */
#define YY_USER_ACTION                                  \
   do {                                                 \
     yylloc->last_column += yyleng;                     \
     yylloc->filename = augl_get_extra(yyscanner);      \
   } while(0);

#define YY_USER_INIT LOCATION_STEP(*yylloc)

#define YY_EXTRA_TYPE struct string *

int augl_get_column  (yyscan_t yyscanner);
static void augl_set_column (int  column_no , yyscan_t yyscanner);
int augl_init_lexer(struct string *name, yyscan_t * scanner);

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
UID    [A-Z][A-Za-z0-9_]*
LID    [a-z][A-Za-z0-9_]*
QID    ({UID}\.)*LID
ARROW  ->

%s COMMENT

%{
    int comment_depth = 0;
%}

%%
<*>
{
  [ \t]*        LOCATION_STEP(*yylloc);
  \n+           LOCATION_LINES(*yylloc, yyleng); LOCATION_STEP(*yylloc);
}

<INITIAL>
{
  \"([^\"]|\\\")*\"   {
               yylval->string = strndup(yytext+1, yyleng-2);
               return DQUOTED;
  }

  \/([^/]|\\\/)+\/ {
               yylval->string = strndup(yytext+1, yyleng-2);
               return REGEXP;
  }

  [|*?+()=:;\.\[\]{}]     return yytext[0];

  module        return KW_MODULE;
  let           return KW_LET;
  string        return KW_STRING;
  regexp        return KW_REGEXP;
  lens          return KW_LENS;
  /* tests */
  test          return KW_TEST;
  get           return KW_GET;
  put           return KW_PUT;
  after         return KW_AFTER;

  {ARROW}       return ARROW;

  {LID}         {
                   yylval->string = strndup(yytext, yyleng);
                   return LIDENT;
                }
  {UID}         {
                   yylval->string = strndup(yytext, yyleng);
                   return UIDENT;
                }
  {QID}         {
                   yylval->string = strndup(yytext, yyleng);
                   return QIDENT;
                }
  \(\*          {
                   comment_depth = 1;
                   BEGIN(COMMENT);
                }
  .             {
    fprintf(stderr, "%s:%d:%d: Unexpected character %c\n", augl_get_extra(yyscanner)->str, yylineno, yylloc->first_column, yytext[0]);
                }

  <<EOF>>       {
                  fclose(augl_get_in(yyscanner));
                  yyterminate();
                }

}

<COMMENT>
{
  \(\*          {
                   comment_depth += 1;
                }
  \*\)          {
                   comment_depth -= 1;
                   if (comment_depth == 0)
                       BEGIN(INITIAL);
                }
  .             /* Skip */;
  <<EOF>>       {
                  fprintf(stderr, "%s:%d:%d: Missing *)\n", augl_get_extra(yyscanner)->str, yylineno, yylloc->first_column);
                  yyterminate();
                }
}
%%

int augl_init_lexer(struct string *name, yyscan_t *scanner) {
  FILE *f;

  f = fopen(name->str, "r");
  if (f == NULL)
    return -1;

  if (augl_lex_init(scanner) != 0) {
    fprintf(stderr, "Failed to init scanner\n");
    return -1;
  }
  augl_set_extra(name, *scanner);
  augl_set_in(f, *scanner);
  return 0;
}
