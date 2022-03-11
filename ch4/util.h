#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define STEP_STR_LEN 128
#define GET_CAP_LEN(len) ((len + STEP_STR_LEN - 1) / STEP_STR_LEN * STEP_STR_LEN)

struct string {
    int cap;
    int len;
    char *data;
};

void* checked_malloc(int len);

#pragma region string
void init_str(struct string* str);
void put_str(struct string* str, char src);
void append_str(struct string* str, char* src);
#pragma endregion

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
extern char* yyfilename;
#pragma endregion

