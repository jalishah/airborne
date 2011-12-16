%{
#include <string.h>
#include <stdlib.h>
#include "conf_parser.h"

static char *duptok(void);
%}

%option noyywrap nounput
%option noinput

HVAL	0x[0-9a-fA-F]+
DVAL	[0-9]+
IVAL	({HVAL})|({DVAL})
DFVAL	-?[0-9]*\.[0-9]+
EVAL	[0-9]*\.?[0-9]*e-?[0-9]+
FVAL	({DFVAL})|({EVAL})
STRING	\"[a-zA-Z0-9/.:_-]*\"
VAR		[a-z][a-z0-9_]*
ASSIGN	[ \t]*=[ \t]*
COMMENT	#.*$

%%
{COMMENT}
module				return MODULE;
{FVAL}				{yylval.fval = (float)atof(yytext); return FVAL;}
{IVAL}				{yylval.ival = (int)strtouq(yytext, NULL, 0); return IVAL;}
{VAR}				{yylval.sval = duptok(); return VAR;}
{STRING}			{yylval.sval = duptok(); return STR;}
{ASSIGN}			return ASSIGN;
;					return END;
\{					return OPEN;
\}					return CLOSE;
[ \n\t]
%%

static char *duptok(void) {
	char *x;
	int cut;

	cut = yytext[0] == '"' && yytext[yyleng - 1] == '"';
	x = malloc(yyleng - cut * 2 + 1);
	memcpy(x, yytext + cut, yyleng - cut * 2);
	x[yyleng - cut * 2] = '\0';
	return x;
}
