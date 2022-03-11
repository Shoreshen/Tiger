#pragma once
#include "util.h"


typedef struct Tr_accessList_ *Tr_accessList;

struct Tr_accessList_ {
    Tr_access head;
    Tr_accessList tail;
};

Tr_level Tr_outermost(void);
Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);
Tr_access Tr_allocLocal(Tr_level level, int escape);
Tr_accessList Tr_formals(Tr_level level);
Temp_label Tr_name(Tr_level level);