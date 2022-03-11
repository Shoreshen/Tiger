#include "interp.h"

void eval_stmt(A_stm a);
long eval_exp(A_exp a);

// Hash table header for UT-HASH
// Must be initialized as NULL
// struct table_entry *head = NULL;
struct table_entry *head = NULL;

void interp(A_stm a) 
{
    eval_stmt(a);
}

void eval_stmt(A_stm a) 
{
    switch (a->kind) {
        case A_compoundStm:
            eval_stmt(a->u.compound.stm1);
            eval_stmt(a->u.compound.stm2);
            return;
        case A_assignStm:
            update_table_str(&head, a->u.assign.id, eval_exp(a->u.assign.exp));
            return;
        case A_printStm:
            A_expList list = a->u.print.exps;
            while (list->kind == A_pairExpList) {
                printf("%d,", eval_exp(list->u.pair.head));
                list = list->u.pair.tail;
            }
            printf("%d\n", eval_exp(list->u.last));
            return;
        default:
            printf("Unaccepted stmt\n");
    }
}

long eval_exp(A_exp a) 
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
        case A_eseqExp:
            eval_stmt(a->u.eseq.stm);
            return eval_exp(a->u.eseq.exp);
        case A_idExp:
            return lookup_str(&head, a->u.id);
        case A_numExp:
            return (long)a->u.num;
        default:
            printf("Unaccepted exp\n");
    }
    return 0;
}