#include "interp.h"

void eval_stmt(A_stm a);
void eval_exp_list(A_expList a);
int eval_exp(A_exp a);

void interp(A_stm a) 
{

}

void eval_stmt(A_stm a) 
{
    switch (a->kind) {
        case A_compoundStm:
            eval_stmt(a->u.compound.stm1);
            eval_stmt(a->u.compound.stm2);
            break;
        case A_assignStm:
            eval_exp(a->u.assign.exp);
            break;
        case A_printStm:
            eval_exp_list(a->u.print.exps);
            break;
        default:
            printf("Unaccepted stmt\n");
    }
}

void eval_exp_list(A_expList a)
{
    switch (a->kind) {
        case A_pairExpList:
            eval_exp(a->u.pair.head);
            eval_exp_list(a->u.pair.tail);
            break;
        case A_lastExpList:
            eval_exp(a->u.last);
            break;
        default:
            printf("Unaccepted exp list\n");
    }
}

int eval_exp(A_exp a) 
{
    switch (a->kind) {
        case A_opExp:
            switch (a->u.op.oper) {
                case A_plus:
                    return eval_exp(a->u.op.left) + eval_exp(a->u.op.right);
                case A_minus:
                    return eval_exp(a->u.op.left) - eval_exp(a->u.op.right);
                case A_times:
                    return eval_exp(a->u.op.left) * eval_exp(a->u.op.right);
                case A_div:
                    return eval_exp(a->u.op.left) / eval_exp(a->u.op.right);
                default:
                    printf("Unrecorgnized operator\n");
            }
            break;
        case A_eseqExp:
            break;
        case A_idExp:
            break;
        case A_numExp:
            break;
        default:
            printf("Unaccepted exp\n");
    }
}