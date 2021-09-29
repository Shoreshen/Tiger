#include "slp.h"

int maxargs(A_stm a);
void visit_stmt(A_stm a, void* ptr);
void visit_exp_list(A_expList a, void* ptr);
void visit_exp(A_exp a, void* ptr);
