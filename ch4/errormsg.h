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