#pragma once

#include "util.h"

typedef struct F_access_ *F_access;
typedef struct F_frag_ *F_frag;

struct F_frame_ {
    Temp_label name;
    F_accessList formals;
    F_accessList locals;
    int inFrame_count;
    int inReg_count;
    int local_count;
    int local_frame_count;
};
struct F_accessList_ {
    F_access head;
    F_accessList tail;
};
struct F_access_ {
    enum {
        F_inFrame, 
        F_inReg
    } kind;
    union {
        int offset;
        Temp_temp reg;
    } u;
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
Temp_temp F_SP();
Temp_temp F_RV();
Temp_temp F_AX();
Temp_temp F_DX();
Temp_temp F_Keep_Regs(int i);
Temp_temp get_x64_reg(int reg);
int pre_colored(Temp_temp t);
T_exp F_Exp(F_access acc, T_exp framePtr);
F_fragList F_FragList(F_frag head, F_fragList tail);
F_accessList F_AccessList(F_access head, F_accessList tail);
F_access F_InFrame(int offset);
F_access F_InReg(Temp_temp reg);
F_frag F_StringFrag(Temp_label label, char* str);
F_frag F_ProcFrag(T_stm body, F_frame frame);
F_access F_GetAccess(int *regCount, int *memCount, int escape);
T_exp F_externalCall(char *s, T_expList args, F_accessList accs);
AS_proc F_procEntryExit(F_frame frame, AS_instrList body);
void F_printFrags(FILE* out, F_fragList frags);
Temp_tempList F_rm_uncolorable_regs(Temp_tempList a);

extern const int F_WORD_SIZE;
extern char* x64_reg_names[16];