#pragma once

#include <stdarg.h>

typedef char bool;

typedef struct U_boolList_ *U_boolList;
struct U_boolList_ {
    bool head; 
    U_boolList tail;
};

extern bool EM_anyErrors;
extern int EM_tokPos;
extern int charPos;

void EM_newline(void);
void EM_error(int, char *,...);
void EM_impossible(char *,...);
void EM_reset(char * filename);
void adjust(char* yytext);
U_boolList U_BoolList(bool head, U_boolList tail);

#pragma region Wrapper for flex & bison
// yyleng = length of matched token, for flex
extern int yyleng;
// yylex() read the next token and return, for flex
extern int yylex();
// Line number from flex
extern int yylineno;
// Parse function for bision
extern int yyparse();
// File pointer that will be parsed when calling `yyparse()`
extern FILE *yyin;
// Defining yyerror for Bision when encounter errors
void yyerror(char *s);
#pragma endregion