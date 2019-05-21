%{
#include "mysh.h"
#include <stdio.h>
#include <string.h>
#define YYERROR_VERBOSE 1
node_t *root = NULL;

int yyerror(char *msg) {
    fprintf(stderr, "Syntax error: %s\n", msg);
    root = NULL;
    return 1;
}
%}

%union {
    node_t *node;
    alist_t *alist;
    char *string;
}

%start root

%type  <node>   root exp cmd_exp
%type  <alist>  strs
%type  <string> str

%token <int> tANDAND tOROR tGTGT tSCOLON tAND tOR tLPAREN tRPAREN tLT tGT tGT_ERR
%token <string> tBARE tQSTRING tQQSTRING

%%
root
    :     { root = NULL; }
    | exp { root = $1; }
    ;

exp /* foo bar baz & */
    : cmd_exp tAND { $$ = $1 ; $$->async = 1; }
    | cmd_exp
    ;

cmd_exp /* foo bar baz */
    : strs
      { $$ = make_node(N_COMMAND, NULL, NULL, convert_to_argv($1), NULL); }
    ;

strs: str strs { $$ = make_alist($1, $2); }
    | str      { $$ = make_alist($1, NULL); }
    ;

str : tBARE     { $$ = strdup($1); }
    | tQSTRING  { $1[strlen($1) - 1] = '\0'; $$ = strdup($1 + 1); }
    | tQQSTRING { $1[strlen($1) - 1] = '\0'; $$ = strdup($1 + 1); }

%%

node_t *yacc_parse(char *source) {
    lex_set_source(source);
    yyparse();
    return root;
}
