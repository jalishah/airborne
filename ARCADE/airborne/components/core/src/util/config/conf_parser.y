%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void module_add(char *module_name);

static void option_int_add(char *ident, int val);
static void option_float_add(char *ident, float val);
static void option_string_add(char *ident, char *val);

static void yyerror(const char *s);

extern int yylex(void);

extern FILE *yyin;

struct option_s *option_list = NULL;
struct module_s *module_list = NULL;
%}

%union {
	int ival;
	float fval;
	char *sval;
}

%token <ival> IVAL
%token <fval> FVAL
%token <sval> STR VAR
%token ASSIGN MODULE END OPEN CLOSE

%expect 1

%%
input	: /* nothing */
		| lines
		;

lines	: module
		| lines module
		;

stmt	: VAR ASSIGN IVAL END		{option_int_add($1, $3);}
		| VAR ASSIGN FVAL END		{option_float_add($1, $3);}
		| VAR ASSIGN STR END		{option_string_add($1, $3);}
		;

stmts	: /* empty */
		| stmt
		| stmts stmt
		;

module	: MODULE STR OPEN stmts CLOSE	{module_add($2);}
		;
%%

/* Add new option to list. */
static struct option_s *prefix_option_object(char *ident, int type) {
	struct option_s *temp;
	
	temp = malloc(sizeof(struct option_s));
	temp->ident = ident;
	temp->type = type;
	temp->next = option_list;
	option_list = temp;

	return temp;
}

static void option_int_add(char *ident, int val) {
	struct option_s *optn;

	optn = prefix_option_object(ident, CONFIG_INT);
	optn->value.i = val;
}

static void option_float_add(char *ident, float val) {
	struct option_s *optn;

	optn = prefix_option_object(ident, CONFIG_FLOAT);
	optn->value.f = val;
}

static void option_string_add(char *ident, char *val) {
	struct option_s *optn;

	optn = prefix_option_object(ident, CONFIG_STRING);
	optn->value.s = val;
}

/* Add module to list and connect option list. */
static void module_add(char *module_name) {
	struct module_s *temp;

	temp = malloc(sizeof(struct module_s));
	temp->name = module_name;
	temp->options = option_list;
	temp->next = module_list;
	module_list = temp;
	option_list = NULL;
}

/* Run parser on given file */
void config_parse(char *filename) {
	if(filename)
		yyin = fopen(filename, "r");
	else
		return;

	if(yyparse())
		exit(1);

	return;
}

static void yyerror(const char *s) {
	fprintf(stderr, "parser error: %s\n", s);
}

