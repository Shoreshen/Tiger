#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "../uthash/include/uthash.h"

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

#pragma region Global typedefs
// env.h
typedef struct E_stack_* E_stack;
// symbol.h
typedef struct S_symbol_ *S_symbol;
// table.h
typedef struct TAB_table_* TAB_table;
// types.h
typedef struct Ty_ty_ *Ty_ty;
typedef struct Ty_field_ *Ty_field;
typedef struct Ty_tyList_ *Ty_tyList;
typedef struct Ty_fieldList_ *Ty_fieldList;
// ast.h
typedef struct A_pos_ *A_pos;
// top_sort.h
typedef struct TS_node_ *TS_node;
typedef struct TS_edge_ *TS_edge;
#pragma endregion
