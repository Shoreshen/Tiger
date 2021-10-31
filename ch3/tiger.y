%{
    #include <stdio.h>
    #include "util.h"
    #include "errormsg.h"
%}

%union {
    int pos;
    int ival;
    char *sval;
}

%token <sval> ID STRING
%token <ival> INT

%token 
    NEQ LE GE ASSIGN ARRAY IF THEN ELSE 
    WHILE FOR TO DO LET IN END OF BREAK NIL FUNCTION VAR TYPE 

%left ';'
%precedence ID
%precedence '['
%precedence THEN 
%precedence ELSE DO OF
%right ASSIGN
%left '|'
%left '&'
%nonassoc '=' '<' '>' NEQ GE LE
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%start program

%%
program: 
    exp
;

decs:
    dec
    | decs dec
;

dec:
    tydec
    | vardec
    | fundec
;

tydec: 
    TYPE ID '=' ty
;

ty: 
    ID
    | '{' tyfields '}'
    | ARRAY OF ID

tyfields:
    %empty
    | ID ':' ID
    | tyfields ',' ID ':' ID
;

vardec:
    VAR ID ASSIGN exp
    | VAR ID ':' ID ASSIGN exp
;

fundec:
    FUNCTION ID '(' tyfields ')' '=' exp
    | FUNCTION ID '(' tyfields ')' ':' ID '=' exp
;

exp: 
    lvalue
    | NIL
    | '(' exp_seq ')'
    | '(' ')'
    | INT
    | STRING
    | '-' exp %prec UMINUS
    | ID '(' exp_list ')'
    | ID '(' ')' 
    | exp '+' exp
    | exp '-' exp
    | exp '*' exp
    | exp '/' exp
    | exp '=' exp
    | exp '>' exp
    | exp '<' exp
    | exp NEQ exp
    | exp GE exp
    | exp LE exp
    | exp '&' exp
    | exp '|' exp
    | ID '{' field_list '}'
    | ID '{' '}'
    | ID '[' exp ']' OF exp
    | lvalue ASSIGN exp
    | IF exp THEN exp ELSE exp
    | IF exp THEN exp
    | WHILE exp DO exp
    | FOR ID ASSIGN exp TO exp DO exp 
    | BREAK
    | LET decs IN exp_seq END
    | LET decs IN END
    | '(' error ')'
    | error ';'
;

exp_seq:
    exp
    | exp_seq ';' exp
;

exp_list:
    exp
    | exp_list ',' exp
;

field_list:
    ID '=' exp
    | field_list ',' ID '=' exp

lvalue:
    ID
    /*
        When stack{ID . '['} due to precedence{line:21} do not reduce to `lvalue`
        Then stack{ID '[' exp ']' . tok} and tok is not `OF`.
        Because bision has to reduce from the stack top, so now it has no rule to reduce or no next state to jump
        Thus adding the following rule to handle this situation
    */
    | ID '[' exp ']'
    | lvalue '.' ID
    | lvalue '[' exp ']' 
;
%%