#pragma once
#include "util.h"

typedef char bool;

typedef struct U_boolList_ *U_boolList;
struct U_boolList_ {
    bool head; 
    U_boolList tail;
};

extern bool EM_anyErrors;
extern int EM_tokPos;
extern int charPos;

void EM_error(A_pos pos, char *message,...);
void EM_reset(char * filename);
U_boolList U_BoolList(bool head, U_boolList tail);