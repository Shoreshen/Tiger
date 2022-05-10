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

void* checked_malloc(int len);
void indent(FILE *out, int d);

#pragma region string
struct string {
    int cap;
    int len;
    char *data;
};
void init_str(struct string* str);
void put_str(struct string* str, char src);
void append_str(struct string* str, char* src);
char* get_heap_str(char* format, ...);
#pragma endregion

#pragma region boollist
typedef struct U_boolList_ *U_boolList;
struct U_boolList_ {
    int head;
    U_boolList tail;
};
U_boolList U_BoolList(int head, U_boolList tail);
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
// stack.h
typedef struct STK_stack_* STK_stack;
// temp.h
typedef struct Temp_temp_ *Temp_temp;
typedef struct S_symbol_ *Temp_label;
typedef struct Temp_labelList_* Temp_labelList;
typedef struct Temp_tempList_ *Temp_tempList;
typedef E_stack Temp_map;
// translate.c
typedef struct Tr_access_ *Tr_access;
typedef struct Tr_level_ *Tr_level;
typedef struct Tr_accessList_ *Tr_accessList;
// tree.h
typedef struct T_stm_ *T_stm;
typedef struct T_exp_ *T_exp;
typedef struct T_expList_ *T_expList;
typedef struct T_stmList_ *T_stmList;
// frame.h
typedef struct F_fragList_ *F_fragList;
typedef struct F_accessList_ *F_accessList;
// graph.h
typedef struct G_graph_ *G_graph;  /* The "graph" type */
typedef struct G_node_ *G_node;    /* The "node" type */
typedef struct G_nodeList_ *G_nodeList;
typedef E_stack G_table;
// assem.h
typedef struct AS_instrList_ *AS_instrList;
#pragma endregion
