%{
    #include <stdio.h>
    #include "util.h"
    #include "symbol.h"
    #include "ast.h"

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
                (Current).filename     = NULL;                              \
            }                                                               \
        } while (0)
}

%union {
    int pos;
    int ival;
    char *sval;
    A_var var;
    A_exp exp;
    A_dec dec;
    A_ty ty;
    A_decList decList;
    A_expList expList;
    A_field field;
    A_fieldList fieldList;
    A_fundec fundec;
    A_fundecList fundecList;
    A_namety namety;
    A_nametyList nametyList;
    A_efield efield;
    A_efieldList efieldList;
}

%token <sval> ID STRING
%token <ival> INT
%type <exp> exp
%type <dec> dec
%type <ty> ty
%type <decList> decs
%type <expList> exp_list exp_seq
%type <efieldList> field_list
%type <var> lvalue

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
    lvalue {
        $$ = A_VarExp(&@$, $1);
    }
    | NIL {
        $$ = A_NilExp(&@$);
    }
    | '(' exp_seq ')' {
        $$ = A_SeqExp(&@$, $2);
    }
    | '(' ')' {
        $$ = A_SeqExp(&@$, NULL);
    }
    | INT {
        $$ = A_IntExp(&@$, $1);
    }
    | STRING {
        $$ = A_StringExp(&@$, $1);
    }
    | '-' exp %prec UMINUS {
        $$ = A_OpExp(&@$, A_minusOp, A_IntExp(&@$, 0), $2);
    }
    | ID '(' exp_list ')' {
        $$ = A_CallExp(&@$, S_Symbol($1), $3);
    }
    | ID '(' ')' {
        $$ = A_CallExp(&@$, S_Symbol($1), NULL);
    }
    | exp '+' exp {
        $$ = A_OpExp(&@$, A_plusOp, $1, $3);
    }
    | exp '-' exp {
        $$ = A_OpExp(&@$, A_minusOp, $1, $3);
    }
    | exp '*' exp {
        $$ = A_OpExp(&@$, A_timesOp, $1, $3);
    }
    | exp '/' exp {
        $$ = A_OpExp(&@$, A_divideOp, $1, $3);
    }
    | exp '=' exp {
        $$ = A_OpExp(&@$, A_eqOp, $1, $3);
    }
    | exp '>' exp {
        $$ = A_OpExp(&@$, A_gtOp, $1, $3);
    }
    | exp '<' exp {
        $$ = A_OpExp(&@$, A_ltOp, $1, $3);
    }
    | exp NEQ exp {
        $$ = A_OpExp(&@$, A_neqOp, $1, $3);
    }
    | exp GE exp {
        $$ = A_OpExp(&@$, A_geOp, $1, $3);
    }
    | exp LE exp {
        $$ = A_OpExp(&@$, A_leOp, $1, $3);
    }
    | exp '&' exp {
        $$ = A_IfExp(&@$, $1, $3, A_IntExp(&@$, 0));
    }
    | exp '|' exp {
        $$ = A_IfExp(&@$, $1, A_IntExp(&@$, 1), $3);
    }
    | ID '{' field_list '}' {
        $$ = A_RecordExp(&@$, S_Symbol($1), $3);
    }
    | ID '{' '}' {
        $$ = A_RecordExp(&@$, S_Symbol($1), NULL);
    }
    | ID '[' exp ']' OF exp {
        $$ = A_ArrayExp(&@$, S_Symbol($1), $3, $6);
    }
    | lvalue ASSIGN exp {
        $$ = A_AssignExp(&@$, $1, $3);
    }
    | IF exp THEN exp ELSE exp {
        $$ = A_IfExp(&@$, $2, $4, $6);
    }
    | IF exp THEN exp {
        $$ = A_IfExp(&@$, $2, $4, NULL);
    }
    | WHILE exp DO exp {
        $$ = A_WhileExp(&@$, $2, $4);
    }
    | FOR ID ASSIGN exp TO exp DO exp {
        $$ = A_ForExp(&@$, S_Symbol($2), $4, $6, $8);
    }
    | BREAK {
        $$ = A_BreakExp(&@$);
    }
    | LET decs IN exp_seq END {
        $$ = A_LetExp(&@$, $2, $4);
    }
    | LET decs IN END {
        $$ = A_LetExp(&@$, $2, NULL);
    }
    | '(' error ')' {
        $$ = A_SeqExp(&@$, NULL);
    }
    | error ';' exp {
        $$ = $3;
    }
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