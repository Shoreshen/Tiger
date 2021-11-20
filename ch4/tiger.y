%{
    #include <stdio.h>
    #include "util.h"
    #include "symbol.h"
    #include "ast.h"

    void yyerror(char *s);
%}

%code requires {
    extern char *yyfilename;

    typedef struct YYLTYPE {
        int first_line;
        int first_column;
        int last_line;
        int last_column;
        char *filename;
    } YYLTYPE;
    # define YYLTYPE_IS_DECLARED 1
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
    A_namety namety;
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
%type <dec> vardec
%type <fundec> fundec
%type <namety> tydec
%type <fieldList> tyfields


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
    exp {
        ast_root = $1;
    }
;

decs:
    %empty {
        $$ = NULL;
    }
    | decs dec {
        $$ = A_DecList($2, $1);
    }
;

dec:
    tydec {
        $$ = A_TypeDec((A_pos)&@$, $1);
    }
    | vardec {
        $$ = $1;
    }
    | fundec {
        $$ = A_FunctionDec((A_pos)&@$, $1);
    }
;

tydec: 
    TYPE ID '=' ty {
        $$ = A_Namety(S_Symbol($2), $4);
    }
;

ty: 
    ID {
        $$ = A_NameTy((A_pos)&@$, S_Symbol($1));
    }
    | '{' tyfields '}' {
        $$ = A_RecordTy((A_pos)&@$, $2);
    }
    | ARRAY OF ID {
        $$ = A_ArrayTy((A_pos)&@$, S_Symbol($3));
    }

tyfields:
    %empty {
        $$ = NULL;
    }
    | ID ':' ID {
        $$ = A_FieldList(A_Field((A_pos)&@$, S_Symbol($1), S_Symbol($3)), NULL);
    }
    | tyfields ',' ID ':' ID {
        $$ = A_FieldList(A_Field((A_pos)&@$, S_Symbol($3), S_Symbol($5)), $1);
    }
;

vardec:
    VAR ID ASSIGN exp {
        $$ = A_VarDec((A_pos)&@$, S_Symbol($2), NULL, $4);
    }
    | VAR ID ':' ID ASSIGN exp {
        $$ = A_VarDec((A_pos)&@$, S_Symbol($2), S_Symbol($4), $6);
    }
;

fundec:
    FUNCTION ID '(' tyfields ')' '=' exp {
        $$ = A_Fundec((A_pos)&@$, S_Symbol($2), $4, NULL, $7);
    }
    | FUNCTION ID '(' tyfields ')' ':' ID '=' exp {
        $$ = A_Fundec((A_pos)&@$, S_Symbol($2), $4, S_Symbol($7), $9);
    }
;

exp: 
    lvalue {
        $$ = A_VarExp((A_pos)&@$, $1);
    }
    | NIL {
        $$ = A_NilExp((A_pos)&@$);
    }
    | '(' exp_seq ')' {
        $$ = A_SeqExp((A_pos)&@$, $2);
    }
    | '(' ')' {
        $$ = A_SeqExp((A_pos)&@$, NULL);
    }
    | INT {
        $$ = A_IntExp((A_pos)&@$, $1);
    }
    | STRING {
        $$ = A_StringExp((A_pos)&@$, $1);
    }
    | '-' exp %prec UMINUS {
        $$ = A_OpExp((A_pos)&@$, A_minusOp, A_IntExp((A_pos)&@$, 0), $2);
    }
    | ID '(' exp_list ')' {
        $$ = A_CallExp((A_pos)&@$, S_Symbol($1), $3);
    }
    | ID '(' ')' {
        $$ = A_CallExp((A_pos)&@$, S_Symbol($1), NULL);
    }
    | exp '+' exp {
        $$ = A_OpExp((A_pos)&@$, A_plusOp, $1, $3);
    }
    | exp '-' exp {
        $$ = A_OpExp((A_pos)&@$, A_minusOp, $1, $3);
    }
    | exp '*' exp {
        $$ = A_OpExp((A_pos)&@$, A_timesOp, $1, $3);
    }
    | exp '/' exp {
        $$ = A_OpExp((A_pos)&@$, A_divideOp, $1, $3);
    }
    | exp '=' exp {
        $$ = A_OpExp((A_pos)&@$, A_eqOp, $1, $3);
    }
    | exp '>' exp {
        $$ = A_OpExp((A_pos)&@$, A_gtOp, $1, $3);
    }
    | exp '<' exp {
        $$ = A_OpExp((A_pos)&@$, A_ltOp, $1, $3);
    }
    | exp NEQ exp {
        $$ = A_OpExp((A_pos)&@$, A_neqOp, $1, $3);
    }
    | exp GE exp {
        $$ = A_OpExp((A_pos)&@$, A_geOp, $1, $3);
    }
    | exp LE exp {
        $$ = A_OpExp((A_pos)&@$, A_leOp, $1, $3);
    }
    | exp '&' exp {
        $$ = A_IfExp((A_pos)&@$, $1, $3, A_IntExp((A_pos)&@$, 0));
    }
    | exp '|' exp {
        $$ = A_IfExp((A_pos)&@$, $1, A_IntExp((A_pos)&@$, 1), $3);
    }
    | ID '{' field_list '}' {
        $$ = A_RecordExp((A_pos)&@$, S_Symbol($1), $3);
    }
    | ID '{' '}' {
        $$ = A_RecordExp((A_pos)&@$, S_Symbol($1), NULL);
    }
    | ID '[' exp ']' OF exp {
        $$ = A_ArrayExp((A_pos)&@$, S_Symbol($1), $3, $6);
    }
    | lvalue ASSIGN exp {
        $$ = A_AssignExp((A_pos)&@$, $1, $3);
    }
    | IF exp THEN exp ELSE exp {
        $$ = A_IfExp((A_pos)&@$, $2, $4, $6);
    }
    | IF exp THEN exp {
        $$ = A_IfExp((A_pos)&@$, $2, $4, NULL);
    }
    | WHILE exp DO exp {
        $$ = A_WhileExp((A_pos)&@$, $2, $4);
    }
    | FOR ID ASSIGN exp TO exp DO exp {
        $$ = A_ForExp((A_pos)&@$, S_Symbol($2), $4, $6, $8);
    }
    | BREAK {
        $$ = A_BreakExp((A_pos)&@$);
    }
    | LET decs IN exp_seq END {
        $$ = A_LetExp((A_pos)&@$, $2, A_SeqExp((A_pos)&@$, $4));
    }
    | LET decs IN END {
        $$ = A_LetExp((A_pos)&@$, $2, NULL);
    }
    | '(' error ')' {
        $$ = A_SeqExp((A_pos)&@$, NULL);
    }
    | error ';' exp {
        $$ = $3;
    }
;

exp_seq:
    exp {
        $$ = A_ExpList($1, NULL);
    }
    | exp_seq ';' exp {
        $$ = A_ExpList($3, $1);
    }
;

exp_list:
    exp {
        $$ = A_ExpList($1, NULL);
    }
    | exp_list ',' exp {
        $$ = A_ExpList($3, $1);
    }
;

field_list:
    ID '=' exp {
        $$ = A_EfieldList(A_Efield(S_Symbol($1), $3), NULL);
    }
    | field_list ',' ID '=' exp {
        $$ = A_EfieldList(A_Efield(S_Symbol($3), $5), $1);
    }
;

lvalue:
    ID {
        $$ = A_SimpleVar((A_pos)&@$, S_Symbol($1));
    }
    /*
        When stack{ID . '['} due to precedence{line:21} do not reduce to `lvalue`
        Then stack{ID '[' exp ']' . tok} and tok is not `OF`.
        Because bision has to reduce from the stack top, so now it has no rule to reduce or no next state to jump
        Thus adding the following rule to handle this situation
    */
    | ID '[' exp ']' {
        $$ = A_SubscriptVar((A_pos)&@$, A_SimpleVar((A_pos)&@$, S_Symbol($1)), $3);
    }
    | lvalue '.' ID {
        $$ = A_FieldVar((A_pos)&@$, $1, S_Symbol($3));
    }
    | lvalue '[' exp ']' {
        $$ = A_SubscriptVar((A_pos)&@$, $1, $3);
    }
;
%%