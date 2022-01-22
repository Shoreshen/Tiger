#pragma once
#include "util.h"


typedef struct Tr_accessList_ *Tr_accessList;
typedef struct Tr_exp_ *Tr_exp;
struct Tr_accessList_ {
    Tr_access head;
    Tr_accessList tail;
};
// level 
Tr_level Tr_outermost(void);
Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);
Tr_access Tr_allocLocal(Tr_level level, int escape);
Tr_accessList Tr_formals(Tr_level level);
Temp_label Tr_name(Tr_level level);

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level);