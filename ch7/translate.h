#pragma once
#include "util.h"
#include "ast.h"

typedef struct Tr_accessList_ *Tr_accessList;
typedef struct Tr_exp_ *Tr_exp;
typedef struct Tr_expList_ *Tr_expList;

struct Tr_accessList_ {
    Tr_access head;
    Tr_accessList tail;
};
struct Tr_expList_ {
    Tr_exp head; 
    Tr_expList tail;
};

// level 
Tr_level Tr_outermost(void);
Tr_level Tr_newLevel(Tr_level parent, U_boolList formals);
Tr_access Tr_allocLocal(Tr_level level, int escape);
Tr_accessList Tr_formals(Tr_level level);
Temp_label Tr_name(Tr_level level);

// Translate into tree
Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail);
Tr_exp Tr_simpleVar(Tr_access access, Tr_level level);
Tr_exp Tr_subscriptVar(Tr_exp var, Tr_exp index);
Tr_exp Tr_fieldVar(Tr_exp var, int pos);
Tr_exp Tr_nilExp();
Tr_exp Tr_intExp(int i);
Tr_exp Tr_stringExp(char* str);
Tr_exp Tr_arithExp(A_oper oper, Tr_exp left, Tr_exp right);
Tr_exp Tr_relExp(A_oper oper, Tr_exp left, Tr_exp right);
Tr_exp Tr_logicExp(A_oper oper, Tr_exp left, Tr_exp right);
Tr_exp Tr_stringCmp(A_oper oper, Tr_exp left, Tr_exp right);
Tr_exp Tr_callExp(Temp_label func, Tr_level level, Tr_level fun_level, Tr_expList args);
Tr_exp Tr_ifExp(Tr_exp test, Tr_exp then, Tr_exp elsee);
Tr_exp Tr_assignExp(Tr_exp lhs, Tr_exp rhs);
Tr_exp Tr_seqExp(Tr_expList expList);
Tr_exp Tr_doneExp();
Tr_exp Tr_breakExp(Tr_exp break_label);
Tr_exp Tr_forExp(Tr_exp body, Tr_access i, Tr_exp lo, Tr_exp hi, Tr_exp done);
Tr_exp Tr_whileExp(Tr_exp test, Tr_exp body, Tr_exp done);
Tr_exp Tr_arrayExp(Tr_exp init, Tr_exp size);
Tr_exp Tr_recordExp(Tr_expList fields, int size);
Tr_exp Tr_seqence(Tr_exp left, Tr_exp right);
void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals);

// Fragment & Tr_print
F_fragList Tr_getResult(void);
void Tr_print(FILE *out, Tr_exp e);