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

// Translate into tree
Tr_exp Tr_simpleVar(Tr_access access, Tr_level level);
Tr_exp Tr_subscriptVar(Tr_exp var, Tr_exp index);
Tr_exp Tr_fieldVar(Tr_exp var, int pos);
Tr_exp Tr_nilExp();
Tr_exp Tr_intExp(int i);
Tr_exp Tr_stringExp(char* str);
Tr_exp Tr_ifExp(Tr_exp test, Tr_exp then, Tr_exp elsee);

// Fragment
F_fragList Tr_getResult(void);