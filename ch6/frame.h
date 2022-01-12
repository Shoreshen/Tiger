#pragma once

#include "util.h"

typedef struct F_access_ *F_access;
typedef struct F_accessList_ *F_accessList;
typedef struct F_frame_ *F_frame;

struct F_frame_ {
    Temp_label name;
    F_accessList formals;
    F_accessList locals;
    int inFrame_count;
    int inReg_count;
};

struct F_accessList_ {
    F_access head;
    F_accessList tail;
};

F_frame F_newFrame(Temp_label name, U_boolList formals);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, int escape);
Temp_label F_name(F_frame f);