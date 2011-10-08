/* Scanner for config specs   -*- C -*- */
%option 8bit never-interactive yylineno
%option bison-bridge bison-locations
%option reentrant noyywrap
%option warn nodefault
%option outfile="lex.yy.c" prefix="augl_"

%top{
/* config.h must precede flex's inclusion of <stdio.h>
   in order for its _GNU_SOURCE definition to take effect.  */
#include <config.h>
}

%{
#include "syntax.h"
#include "errcode.h"

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
#define YY_USER_ACTION                                            \
  do {                                                            \
     yylloc->last_column += yyleng;                               \
     yylloc->filename = augl_get_info(yyscanner)->filename;       \
     yylloc->error = augl_get_info(yyscanner)->error;             \
   } while(0);

#define YY_USER_INIT LOCATION_STEP(*yylloc)

#define YY_EXTRA_TYPE struct state *

int augl_init_lexer(struct state *state, yyscan_t * scanner);
void augl_close_lexer(yyscan_t *scanner);
struct info *augl_get_info(yyscan_t yyscanner);

static void loc_update(YYLTYPE *yylloc, const char *s, int len) {
  for (int i=0; i < len; i++) {
    if (s[i] == '\n') {
      LOCATION_LINES(*yylloc, 1);
    }
  }
}

static char *regexp_literal(const char *s, int len) {
  char *u = unescape(s, len, RX_ESCAPES);
  size_t u_len = strlen(u);

  if (u == NULL)
    return NULL;

  regexp_c_locale(&u, &u_len);

  return u;
}
%}

DIGIT [0-9]
UID    [A-Z][A-Za-z0-9_]*
LID    [a-z_][A-Za-z0-9_]*
LETREC   let[ \t]+rec
WS     [ \t\n]
QID    {UID}\.{LID}
ARROW  ->

%s COMMENT

%%
<*>
{
  [ \t]*        LOCATION_STEP(*yylloc);
  \n+           LOCATION_LINES(*yylloc, yyleng); LOCATION_STEP(*yylloc);
  (\r\n)+       LOCATION_LINES(*yylloc, yyleng/2); LOCATION_STEP(*yylloc);
}

<INITIAL>
{
  \"([^\"]|\\\")*\"   {
               loc_update(yylloc, yytext, yyleng);
               yylval->string = unescape(yytext+1, yyleng-2, STR_ESCAPES);
               return DQUOTED;
  }

  \/([^/]|\\\/)*\/i {
               loc_update(yylloc, yytext, yyleng);
               yylval->regexp.nocase = 1;
               yylval->regexp.pattern = regexp_literal(yytext+1, yyleng-3);
               return REGEXP;
  }

  \/([^/]|\\\/)*\/ {
               loc_update(yylloc, yytext, yyleng);
               yylval->regexp.nocase = 0;
               yylval->regexp.pattern = regexp_literal(yytext+1, yyleng-2);
               return REGEXP;
  }

  [|*?+()=:;\.\[\]{}-]    return yytext[0];

  module        return KW_MODULE;

  {LETREC}/{WS} return KW_LET_REC;

  let           return KW_LET;
  string        return KW_STRING;
  regexp        return KW_REGEXP;
  lens          return KW_LENS;
  in            return KW_IN;
  autoload      return KW_AUTOLOAD;

  /* tests */
  test          return KW_TEST;
  get           return KW_GET;
  put           return KW_PUT;
  after         return KW_AFTER;

  {ARROW}       return ARROW;

  {QID}         {
                   yylval->string = strndup(yytext, yyleng);
                   return QIDENT;
                }
  {LID}         {
                   yylval->string = strndup(yytext, yyleng);
                   return LIDENT;
                }
  {UID}         {
                   yylval->string = strndup(yytext, yyleng);
                   return UIDENT;
                }
  \(\*          {
                   augl_get_extra(yyscanner)->comment_depth = 1;
                   BEGIN(COMMENT);
                }
  .             {
    report_error(augl_get_info(yyscanner)->error, AUG_ESYNTAX,
                 "%s:%d:%d: Unexpected character %c",
                 augl_get_info(yyscanner)->filename->str,
                 yylineno, yylloc->first_column, yytext[0]);
                }

  <<EOF>>       {
                  augl_close_lexer(yyscanner);
                  yyterminate();
                }

}

<COMMENT>
{
  \(\*          {
                   augl_get_extra(yyscanner)->comment_depth += 1;
                }
  \*\)          {
                   augl_get_extra(yyscanner)->comment_depth -= 1;
                   if (augl_get_extra(yyscanner)->comment_depth == 0)
                       BEGIN(INITIAL);
                }
  .             /* Skip */;
  <<EOF>>       {
                  report_error(augl_get_info(yyscanner)->error, AUG_ESYNTAX,
                               "%s:%d:%d: Missing *)",
                               augl_get_info(yyscanner)->filename->str,
                               yylineno, yylloc->first_column);
                  augl_close_lexer(yyscanner);
                  yyterminate();
                }
}
%%

void augl_close_lexer(yyscan_t *scanner) {
  FILE *fp = augl_get_in(scanner);

  if (fp != NULL) {
    fclose(fp);
    augl_set_in(NULL, scanner);
  }
}

int augl_init_lexer(struct state *state, yyscan_t *scanner) {
  FILE *f;
  struct string *name = state->info->filename;

  f = fopen(name->str, "r");
  if (f == NULL)
    return -1;

  if (augl_lex_init(scanner) != 0) {
    fclose(f);
    return -1;
  }
  augl_set_extra(state, *scanner);
  augl_set_in(f, *scanner);
  return 0;
}

struct info *augl_get_info(yyscan_t scanner) {
  return augl_get_extra(scanner)->info;
}
