#pragma once

#include <stdarg.h>

typedef char bool;

typedef struct U_boolList_ *U_boolList;
struct U_boolList_ {
    bool head; 
    U_boolList tail;
};

typedef struct YYLTYPE {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
    char *filename;
} YYLTYPE;

extern bool EM_anyErrors;
extern int EM_tokPos;
extern int charPos;

void EM_error(char *,...);
void EM_reset(char * filename);
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
#pragma endregion