%{
    #include <stdio.h>
    #include "util.h"

    void yyerror(char *s);
%}

%code requires {
    char *yyfilename;

    typedef struct YYLTYPE {
        int first_line;
        int first_column;
        int last_line;
        int last_column;
        char *filename;
    } YYLTYPE;

    # define YYLLOC_DEFAULT(Current, Rhs, N)                                \
        do {                                                                \
            if (N) {                                                        \
                (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;      \
                (Current).first_column = YYRHSLOC (Rhs, 1).first_column;    \
                (Current).last_line    = YYRHSLOC (Rhs, N).last_line;       \
                (Current).last_column  = YYRHSLOC (Rhs, N).last_column;     \
                (Current).filename     = YYRHSLOC (Rhs, N).filename;        \
            } else {                                                        \
                (Current).first_line   = (Current).last_line   =            \
                    YYRHSLOC (Rhs, 0).last_line;                            \
                (Current).first_column = (Current).last_column =            \
                    YYRHSLOC (Rhs, 0).last_column;                          \
                (Current).filename     = NULL                               \
            }                                                               \
        } while (0)
}

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
%precedence '[' // Avoid ambiguity with 'ID'
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