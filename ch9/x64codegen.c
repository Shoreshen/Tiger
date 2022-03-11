#include "codegen.h"
#include "assem.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"

static AS_instrList iList=NULL, last=NULL;

void emit(AS_instr inst) {
    if (last!=NULL) {
        last = last->tail = AS_InstrList(inst,NULL);
    } else { 
        last = iList = AS_InstrList(inst,NULL);
    }
}

Temp_temp munchExp(T_exp e);
void munchStm(T_stm s);

AS_instrList F_codegen(F_frame f, T_stmList stmList);