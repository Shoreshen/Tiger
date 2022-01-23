#pragma once

#include "util.h"

typedef struct F_access_ *F_access;
typedef struct F_accessList_ *F_accessList;
typedef struct F_frame_ *F_frame;
typedef struct F_frag_ *F_frag;
typedef struct F_fragList_ *F_fragList;

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
struct F_frag_ { 
    enum {
        F_stringFrag, 
        F_procFrag
    } kind;
    union {
        struct {
            Temp_label label;
            char* str;
        } stringg;
        struct {
            T_stm body; 
            F_frame frame;
        } proc;
    } u;
};
struct F_fragList_ {
    F_frag head; 
    F_fragList tail;
};

F_frame F_newFrame(Temp_label name, U_boolList formals);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, int escape);
Temp_label F_name(F_frame f);
Temp_temp F_FP();
T_exp F_Exp(F_access acc, T_exp framePtr);
F_fragList F_FragList(F_frag head, F_fragList tail);
F_frag F_StringFrag(Temp_label label, char* str);
T_exp F_externalCall(char* fun,T_expList args);
extern const int F_WORD_SIZE;