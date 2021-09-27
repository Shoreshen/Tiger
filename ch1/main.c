#include "util.h"
#include "slp.h"

int maxargs(A_stm a);
void visit_stmt(A_stm a, void* ptr);
void visit_exp_list(A_expList a, void* ptr);
void visit_exp(A_exp a, void* ptr);

int maxcnt = 0;

int main() 
{
    A_stm prog = A_CompoundStm(
        A_AssignStm(
            "a",
            A_OpExp(
                A_NumExp(5), 
                A_plus, 
                A_NumExp(3)
            )
        ),
        A_CompoundStm(
            A_AssignStm("b",
                A_EseqExp(
                    A_PrintStm(
                        A_PairExpList(
                            A_IdExp("a"),
                            A_LastExpList(
                                A_OpExp(
                                    A_IdExp("a"), 
                                    A_minus,
                                    A_NumExp(1)
                                )
                            )
                        )
                    ),
                    A_OpExp(
                        A_NumExp(10), 
                        A_times, 
                        A_IdExp("a")
                    )
                )
            ),
            A_PrintStm(
                A_LastExpList(
                    A_IdExp("b")
                )
            )
        )
    );


    return 0;
}

void visit_stmt(A_stm a, void* ptr) 
{
    switch (a->kind) {
        case A_compoundStm:
            visit_stmt(a->u.compound.stm1, ptr);
            visit_stmt(a->u.compound.stm2, ptr);
            break;
        case A_assignStm:
            visit_exp(a->u.assign.exp, ptr);
            break;
        case A_printStm:
            int prt_cnt = 0;
            visit_exp_list(a->u.print.exps, &prt_cnt);
            if (prt_cnt > maxcnt) {
                maxcnt = prt_cnt;
            }
            break;
        default:
            break;
    }
}

void visit_exp_list(A_expList a, void* ptr)
{
    *(int*)ptr++;
    switch (a->kind) {
        case A_pairExpList:
            visit_exp(a->u.pair.head, ptr);
            visit_exp_list(a->u.pair.tail, ptr);
            break;
        case A_lastExpList:
            visit_exp(a->u.last, ptr);
            break;
        default:
            break;
    }
}

void visit_exp(A_exp a, void* ptr) 
{
    switch (a->kind) {
        case A_opExp:
            visit_exp(a->u.op.left, ptr);
            visit_exp(a->u.op.right, ptr);
            break;
        case A_eseqExp:
            visit_stmt(a->u.eseq.stm, ptr);
            visit_exp(a->u.eseq.exp, ptr);
            break;
        case A_idExp:
            break;
        case A_numExp:
            break;
        default:
            break;
    }
}

int maxargs(A_stm a)
{
    visit_stmt(a, NULL);
}