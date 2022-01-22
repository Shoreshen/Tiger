#include "tree.h"
#include "symbol.h"
#include "temp.h"

#pragma region Lists
T_expList T_ExpList(T_exp head, T_expList tail)
{
    T_expList e = checked_malloc(sizeof(*e));
    e->head = head;
    e->tail = tail;
    return e;
}
T_stmList T_StmList(T_stm head, T_stmList tail)
{
    T_stmList s = checked_malloc(sizeof(*s));
    s->head = head;
    s->tail = tail;
    return s;
}
#pragma endregion

#pragma region T_stm
T_stm T_Seq(T_stm left, T_stm right)
{
    T_stm s = checked_malloc(sizeof(*s));
    s->kind = T_SEQ;
    s->u.SEQ.left = left;
    s->u.SEQ.right = right;
    return s;
}
// The last parameter must be NULL, for va_list to stop
T_stm T_Seqs(T_stm stms, ...)
{
    va_list stm;
    T_stm tmp, last;
    va_start(stm, stms);
    tmp = va_arg(stm, T_stm);
    last = tmp;
    while (tmp = va_arg(stm, T_stm)) {
        last = T_Seq(last, tmp);
    }
    va_end(stm);
    return last;
}
T_stm T_Label(Temp_label lab)
{
    T_stm s = checked_malloc(sizeof(*s));
    s->kind = T_LABEL;
    s->u.LABEL = lab;
    return s;
}
T_stm T_Jump(T_exp exp, Temp_labelList labels)
{
    T_stm s = checked_malloc(sizeof(*s));
    s->kind = T_JUMP;
    s->u.JUMP.exp = exp;
    s->u.JUMP.jumps = labels;
    return s;
}
T_stm T_Cjump(T_relOp op, T_exp left, T_exp right, Temp_label true, Temp_label false)
{
    T_stm s = checked_malloc(sizeof(*s));
    s->kind = T_CJUMP;
    s->u.CJUMP.op = op;
    s->u.CJUMP.left = left;
    s->u.CJUMP.right = right;
    s->u.CJUMP.true = true;
    s->u.CJUMP.false = false;
    return s;
}
T_stm T_Move(T_exp dst, T_exp src)
{
    T_stm s = checked_malloc(sizeof(*s));
    s->kind = T_MOVE;
    s->u.MOVE.dst = dst;
    s->u.MOVE.src = src;
    return s;
}
T_stm T_Exp(T_exp exp)
{
    T_stm s = checked_malloc(sizeof(*s));
    s->kind = T_EXP;
    s->u.EXP = exp;
    return s;
}
#pragma endregion

#pragma region T_exp
T_exp T_Binop(T_binOp op, T_exp left, T_exp right)
{
    T_exp e = checked_malloc(sizeof(*e));
    e->kind = T_BINOP;
    e->u.BINOP.op = op;
    e->u.BINOP.left = left;
    e->u.BINOP.right = right;
    return e;
}
T_exp T_Mem(T_exp mem)
{
    T_exp e = checked_malloc(sizeof(*e));
    e->kind = T_MEM;
    e->u.MEM = mem;
    return e;
}
T_exp T_Temp(Temp_temp temp)
{
    T_exp e = checked_malloc(sizeof(*e));
    e->kind = T_TEMP;
    e->u.TEMP = temp;
    return e;
}
T_exp T_Eseq(T_stm stm, T_exp exp)
{
    T_exp e = checked_malloc(sizeof(*e));
    e->kind = T_ESEQ;
    e->u.ESEQ.exp = exp;
    e->u.ESEQ.stm = stm;
    return e;
}
T_exp T_Name(Temp_label name)
{
    T_exp e = checked_malloc(sizeof(*e));
    e->kind = T_NAME;
    e->u.NAME = name;
    return e;
}
T_exp T_Const(int consti)
{
    T_exp e = checked_malloc(sizeof(*e));
    e->kind = T_CONST;
    e->u.CONST = consti;
    return e;
}
T_exp T_Call(T_exp fun, T_expList args)
{
    T_exp e = checked_malloc(sizeof(*e));
    e->kind = T_CALL;
    e->u.CALL.fun = fun;
    e->u.CALL.args = args;
    return e;
}
#pragma endregion

#pragma region print
static char bin_oper[][12] = {"PLUS", "MINUS", "TIMES", "DIVIDE", "AND", "OR", "LSHIFT", "RSHIFT", "ARSHIFT", "XOR"};
static char rel_oper[][12] = {"EQ", "NE", "LT", "GT", "LE", "GE", "ULT", "ULE", "UGT", "UGE"};
void pr_tree_exp(FILE *out, T_exp exp, int d);

void pr_stm(FILE *out, T_stm stm, int d)
{
    switch (stm->kind) {
        case T_SEQ:
            indent(out, d);
            fprintf(out, "SEQ(\n");
            pr_stm(out, stm->u.SEQ.left, d + 1);
            fprintf(out, ",\n");
            pr_stm(out, stm->u.SEQ.right, d + 1);
            fprintf(out, ")");
            break;
        case T_LABEL:
            indent(out, d);
            fprintf(out, "LABEL %s", S_name(stm->u.LABEL));
            break;
        case T_JUMP:
            indent(out, d);
            fprintf(out, "JUMP(\n");
            pr_tree_exp(out, stm->u.JUMP.exp, d + 1);
            fprintf(out, ")");
            break;
        case T_CJUMP:
            indent(out, d);
            fprintf(out, "CJUMP(%s,\n", rel_oper[stm->u.CJUMP.op]);
            pr_tree_exp(out, stm->u.CJUMP.left, d + 1);
            fprintf(out, ",\n");
            pr_tree_exp(out, stm->u.CJUMP.right, d + 1);
            fprintf(out, ",\n");
            indent(out, d + 1);
            fprintf(out, "%s,", S_name(stm->u.CJUMP.true));
            fprintf(out, "%s", S_name(stm->u.CJUMP.false));
            fprintf(out, ")");
            break;
        case T_MOVE:
            indent(out, d);
            fprintf(out, "MOVE(\n");
            pr_tree_exp(out, stm->u.MOVE.dst, d + 1);
            fprintf(out, ",\n");
            pr_tree_exp(out, stm->u.MOVE.src, d + 1);
            fprintf(out, ")");
            break;
        case T_EXP:
            indent(out, d);
            fprintf(out, "EXP(\n");
            pr_tree_exp(out, stm->u.EXP, d + 1);
            fprintf(out, ")");
            break;
    }
}

void pr_tree_exp(FILE *out, T_exp exp, int d)
{
    switch (exp->kind) {
        case T_BINOP:
            indent(out, d);
            fprintf(out, "BINOP(%s,\n", bin_oper[exp->u.BINOP.op]);
            pr_tree_exp(out, exp->u.BINOP.left, d + 1);
            fprintf(out, ",\n");
            pr_tree_exp(out, exp->u.BINOP.right, d + 1);
            fprintf(out, ")");
            break;
        case T_MEM:
            indent(out, d);
            fprintf(out, "MEM");
            fprintf(out, "(\n");
            pr_tree_exp(out, exp->u.MEM, d + 1);
            fprintf(out, ")");
            break;
        case T_TEMP:
            indent(out, d);
            fprintf(out, "TEMP t%s",exp->u.TEMP->num);
            break;
        case T_ESEQ:
            indent(out, d);
            fprintf(out, "ESEQ(\n");
            pr_stm(out, exp->u.ESEQ.stm, d + 1);
            fprintf(out, ",\n");
            pr_tree_exp(out, exp->u.ESEQ.exp, d + 1);
            fprintf(out, ")");
            break;
        case T_NAME:
            indent(out, d);
            fprintf(out, "NAME %s", S_name(exp->u.NAME));
            break;
        case T_CONST:
            indent(out, d);
            fprintf(out, "CONST %d", exp->u.CONST);
            break;
        case T_CALL: {
            T_expList args = exp->u.CALL.args;
            indent(out, d);
            fprintf(out, "CALL(\n");
            pr_tree_exp(out, exp->u.CALL.fun, d + 1);
            for (; args; args = args->tail)
            {
                fprintf(out, ",\n");
                pr_tree_exp(out, args->head, d + 2);
            }
            fprintf(out, ")");
            break;
        }
    } /* end of switch */
}
void printStmList(FILE *out, T_stmList stmList)
{
    for (; stmList; stmList = stmList->tail) {
        pr_stm(out, stmList->head, 0);
        fprintf(out, "\n");
    }
}
#pragma endregion

T_relOp T_notRel(T_relOp rel)
{
    switch (rel)
    {
    case T_eq:
        return T_ne;
    case T_ne:
        return T_eq;
    case T_lt:
        return T_ge;
    case T_ge:
        return T_lt;
    case T_gt:
        return T_le;
    case T_le:
        return T_gt;
    case T_ult:
        return T_uge;
    case T_uge:
        return T_ult;
    case T_ule:
        return T_ugt;
    case T_ugt:
        return T_ule;
    }
    assert(0);
    return 0;
}
T_relOp T_commute(T_relOp rel)
{
    switch (rel)
    {
    case T_eq:
        return T_eq;
    case T_ne:
        return T_ne;
    case T_lt:
        return T_gt;
    case T_ge:
        return T_le;
    case T_gt:
        return T_lt;
    case T_le:
        return T_ge;
    case T_ult:
        return T_ugt;
    case T_uge:
        return T_ule;
    case T_ule:
        return T_uge;
    case T_ugt:
        return T_ult;
    }
    assert(0);
    return 0;
}