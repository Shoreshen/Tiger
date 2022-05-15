#include "assem.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "symbol.h"

#define FLAG_OFFSET_SET 1
#define FLAG_INDEX_SET 1 << 1
#define FLAG_BASE_SET 1 << 2
#define FLAG_ALL (FLAG_BASE_SET & FLAG_INDEX_SET & FLAG_OFFSET_SET)
#define F_KEEP 6 // keep 6 formal param in registers

static AS_instrList iList = NULL, last = NULL;
Temp_temp func_formals[F_KEEP] = {NULL};
Temp_temp munchExp(T_exp e);
void munchStm(T_stm s);
struct EffAddr {
    char* assem;
    Temp_tempList src;
};
void emit(AS_instr inst) {
    if (last != NULL) {
        last->tail = AS_InstrList(inst,NULL);
        last = last->tail;
    } else { 
        last = iList = AS_InstrList(inst,NULL);
    }
}

void find_elem(T_exp e, int* scale, int* offset, Temp_temp* index, Temp_temp* base, long *flag)
{
    if (e->kind == T_CONST && !(*flag & FLAG_OFFSET_SET)) {
        *offset = e->u.CONST;
        *flag |= FLAG_OFFSET_SET; // offset setted
        return;
    } else if (e->kind == T_BINOP && e->u.BINOP.op == T_mul && !(*flag & FLAG_INDEX_SET)) {
        T_exp left = e->u.BINOP.left;
        T_exp right = e->u.BINOP.right;
        if (right->kind == T_CONST && right->u.CONST == 8) {
            *scale = 8;
            *index = munchExp(left);
            *flag |= FLAG_INDEX_SET;
            return;
        }
        if (left->kind == T_CONST && left->u.CONST == 8) {
            *scale = 8;
            *index = munchExp(right);
            *flag |= FLAG_INDEX_SET;
            return;
        }
    } 
    if (!(*flag & FLAG_BASE_SET)) {
        *base = munchExp(e);
        *flag |= FLAG_BASE_SET;
    } else if (!(*flag & FLAG_INDEX_SET)) {
        *index = munchExp(e);
        *scale = 1;
        *flag |= FLAG_INDEX_SET;
        return;
    }
}
int handle_3(T_exp e1, T_exp e2, T_exp e3, struct EffAddr* addr)
{
    Temp_temp index = NULL;
    Temp_temp base = NULL;
    int scale = 0;
    int offset = 0;
    long flag = 0;
    find_elem(e1, &scale, &offset, &index, &base, &flag);
    find_elem(e2, &scale, &offset, &index, &base, &flag);
    find_elem(e3, &scale, &offset, &index, &base, &flag);
    if (!(flag ^ FLAG_ALL)) {
        return FALSE;
    }
    addr->assem = get_heap_str("`s0 + `s1 * %d + %d", scale, offset);
    addr->src = Temp_TempLists(base, index, NULL);
    return TRUE;
}
int handle_2(T_exp e1, T_exp e2, struct EffAddr* addr)
{
    Temp_temp index = NULL;
    Temp_temp base = NULL;
    int scale = 0;
    int offset = 0;
    long flag = 0;
    find_elem(e1, &scale, &offset, &index, &base, &flag);
    find_elem(e2, &scale, &offset, &index, &base, &flag);
    if (flag & FLAG_OFFSET_SET) {
        if (scale) {
            addr->assem = get_heap_str("`s0 * %d + %d", scale, offset);
            addr->src = Temp_TempLists(index, NULL);
        } else {
            addr->assem = get_heap_str("`s0 + %d", offset);
            addr->src = Temp_TempLists(base, NULL);
        }
        return 1;
    } else {
        addr->assem = get_heap_str("`s0 + `s1 * %d", scale);
        addr->src = Temp_TempLists(base, index, NULL);
        return 2;
    }
}
void handle_1(T_exp e1, struct EffAddr* addr)
{
    Temp_temp index = NULL;
    Temp_temp base = NULL;
    int scale = 0;
    int offset = -1;
    long flag = 0;
    find_elem(e1, &scale, &offset, &index, &base, &flag);
    if (flag & FLAG_OFFSET_SET) {
        addr->assem = get_heap_str("%d", offset);
    } else if (flag & FLAG_INDEX_SET) {
        addr->assem = get_heap_str("`s0*%d", scale);
        addr->src = Temp_TempLists(index, NULL);
    } else {
        addr->assem = get_heap_str("`s0");
        addr->src = Temp_TempLists(base, NULL);
    }
}
int32_t x86_eff_addr(T_exp e, struct EffAddr* addr) 
{
    if (e->kind == T_BINOP && e->u.BINOP.op == T_plus) {
        T_exp left = e->u.BINOP.left;
        T_exp right = e->u.BINOP.right;
        if (left->kind == T_BINOP && left->u.BINOP.op == T_plus) {
            if (handle_3(left->u.BINOP.left, left->u.BINOP.right, right, addr)) {
                return 3;
            }
        }
        if (right->kind == T_BINOP && right->u.BINOP.op == T_plus) {
            if (handle_3(right->u.BINOP.left, right->u.BINOP.right, left, addr)) {
                return 3;
            }
        }
        return handle_2(left, right, addr);
    }
    handle_1(e, addr);
    if (addr->src) {
        return 1;
    } else {
        return 0;
    }
}
int fill_last_tmplist(struct EffAddr *addr, Temp_temp last)
{
    int i = 0;
    if (!(addr->src)) {
        addr->src = Temp_TempLists(last, NULL);
    } else {
        Temp_tempList tmp = addr->src;
        i++;
        while (tmp->tail) {
            tmp = tmp->tail;
            i++;
        }
        tmp->tail = Temp_TempLists(last, NULL);
    }
    return i;
}
Temp_tempList muncArgs(T_expList args, F_accessList accs, Temp_tempList* tmp_regs)
{
    Temp_tempList list = NULL;
    if (args == 0) {
        return NULL;
    }
    if (args->tail) {
        list = muncArgs(args->tail, accs->tail, tmp_regs);
    }
    if (accs->head->kind == F_inReg) {
        Temp_temp r = Temp_newtemp();
        (*tmp_regs) = Temp_TempList(r, (*tmp_regs));
        emit(AS_Move(
            get_heap_str("mov `d0, `s0\n"), 
            Temp_TempLists(r, NULL), 
            Temp_TempLists(accs->head->u.reg, NULL)
        ));
        if (args->head->kind == T_CONST) {
            emit(AS_Oper( //Only form of `mov reg, reg` is decided as AS_Move
                get_heap_str("mov `d0, %d\n", args->head->u.CONST), 
                Temp_TempLists(accs->head->u.reg, NULL), 
                NULL, NULL
            ));
        } else {
            emit(AS_Move(
                get_heap_str("mov `d0, `s0\n"), 
                Temp_TempLists(accs->head->u.reg, NULL), 
                Temp_TempLists(munchExp(args->head), NULL)
            ));
        }
        list = Temp_TempList(accs->head->u.reg, list);
    } else if (accs->head->kind == F_inFrame) {
        if (args->head->kind == T_CONST) {
            emit(AS_Oper(
                get_heap_str("push %d\n", args->head->u.CONST), 
                NULL, NULL, NULL
            ));
        } else {
            emit(AS_Oper(
                get_heap_str("push `s0\n"), 
                Temp_TempLists(F_SP(), NULL), 
                Temp_TempLists(munchExp(args->head), NULL), 
                NULL
            ));
        }
    } else {
        assert(0);
    }
    return list;
}
void munchArgRestore(F_accessList accs, Temp_tempList tmp_regs)
{
    int count = 0;
    while (accs) {
        if (accs->head->kind == F_inFrame) {
            count++;
        } else {
            emit(AS_Move(
                get_heap_str("mov `d0, `s0\n"), 
                Temp_TempLists(accs->head->u.reg, NULL),
                Temp_TempLists(tmp_regs->head, NULL)
            ));
            tmp_regs = tmp_regs->tail;
        }
        accs = accs->tail;
    }
    if (count) {
        emit(AS_Oper(
            get_heap_str("add `s0, %d\n", count * F_WORD_SIZE), 
            NULL, 
            Temp_TempLists(F_SP(), NULL), 
            NULL
        ));
    }
}
Temp_temp munchExp(T_exp e)
{
    switch (e->kind) {
        case T_BINOP: {
            T_exp left = e->u.BINOP.left;
            T_exp right = e->u.BINOP.right;
            T_binOp op = e->u.BINOP.op;
            Temp_temp r_dst = Temp_newtemp();
            Temp_temp r_src = NULL;
            switch (op) {
                case T_plus:
                    if (left->kind == T_CONST) {
                        r_src = munchExp(right);
                        emit(AS_Oper(
                            get_heap_str("mov `d0, %d\n",left->u.CONST),
                            Temp_TempList(r_dst, NULL),
                            NULL, NULL
                        ));
                    } else if (right->kind == T_CONST) {
                        r_src = munchExp(left);
                        emit(AS_Oper(
                            get_heap_str("mov `d0, %d\n",right->u.CONST),
                            Temp_TempList(r_dst, NULL),
                            NULL, NULL
                        ));
                    } else {
                        r_src = munchExp(right);
                        emit(AS_Move(
                            get_heap_str("mov `d0, `s0\n"),
                            Temp_TempList(r_dst, NULL),
                            Temp_TempList(munchExp(left), NULL)
                        ));
                    }
                    emit(AS_Oper(
                        get_heap_str("add `d0, `s0\n", right->u.CONST),
                        Temp_TempLists(r_dst, NULL),
                        Temp_TempLists(r_src, r_dst, NULL), 
                        NULL
                    ));
                    return r_dst;
                case T_minus:
                    r_src = munchExp(right);
                    if (left->kind == T_CONST) {
                        emit(AS_Oper(
                            get_heap_str("mov `d0, %d\n",left->u.CONST),
                            Temp_TempList(r_dst, NULL),
                            NULL, NULL
                        ));
                    } else {
                        emit(AS_Move(
                            get_heap_str("mov `d0, `s0\n"),
                            Temp_TempList(r_dst, NULL),
                            Temp_TempList(munchExp(left), NULL)
                        ));
                    }
                    emit(AS_Oper(
                        get_heap_str("sub `d0, `s0\n", right->u.CONST),
                        Temp_TempLists(r_dst, NULL),
                        Temp_TempLists(r_src, r_dst, NULL), 
                        NULL
                    ));
                    return r_dst;
                case T_mul: {
                    Temp_temp tmp_ax = Temp_newtemp();
                    Temp_temp tmp_dx = Temp_newtemp();
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(tmp_ax, NULL),
                        Temp_TempLists(F_AX(), NULL)
                    ));
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(tmp_dx, NULL),
                        Temp_TempLists(F_DX(), NULL)
                    ));
                    if (left->kind == T_CONST) {
                        r_src = munchExp(right);
                        emit(AS_Oper(
                            get_heap_str("mov `d0, %d\n", left->u.CONST),
                            Temp_TempLists(F_AX(), NULL),
                            NULL, NULL
                        ));
                    } else if (right->kind == T_CONST){
                        r_src = munchExp(left);
                        emit(AS_Oper(
                            get_heap_str("mov `d0, %d\n", right->u.CONST),
                            Temp_TempLists(F_AX(), NULL),
                            NULL, NULL
                        ));
                    } else {
                        r_src = munchExp(right);
                        emit(AS_Move(
                            get_heap_str("mov `d0, `s0\n"),
                            Temp_TempLists(F_AX(), NULL),
                            Temp_TempLists(munchExp(left), NULL)
                        ));
                    }
                    // Both rax, rdx are source and dest of the imul instruction
                    emit(AS_Oper(
                        get_heap_str("imul `s0\n"),
                        Temp_TempLists(F_AX(), F_DX(), NULL),
                        Temp_TempLists(r_src, F_AX(), F_DX(), NULL),
                        NULL
                    ));
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(r_dst, NULL),
                        Temp_TempLists(F_AX(), NULL)
                    ));
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(F_AX(), NULL),
                        Temp_TempLists(tmp_ax, NULL)
                    ));
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(F_DX(), NULL),
                        Temp_TempLists(tmp_dx, NULL)
                    ));
                    return r_dst;
                }
                case T_div: {
                    Temp_temp tmp_ax = Temp_newtemp();
                    Temp_temp tmp_dx = Temp_newtemp();
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(tmp_ax, NULL),
                        Temp_TempLists(F_AX(), NULL)
                    ));
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(tmp_dx, NULL),
                        Temp_TempLists(F_DX(), NULL)
                    ));
                    r_src = munchExp(right);
                    if (left->kind == T_CONST) {
                        emit(AS_Oper(
                            get_heap_str("mov `d0, %d\n", left->u.CONST),
                            Temp_TempLists(F_AX(), NULL),
                            NULL, NULL
                        ));
                    } else {
                        emit(AS_Move(
                            get_heap_str("mov `d0, `s0\n"),
                            Temp_TempLists(F_AX(), NULL),
                            Temp_TempLists(munchExp(left), NULL)
                        ));
                    }
                    // Both rax, rdx are source and dest of the imul instruction
                    emit(AS_Oper(
                        get_heap_str("idiv `s0\n"),
                        Temp_TempLists(F_AX(), F_DX(), NULL),
                        Temp_TempLists(r_src, F_AX(), F_DX(), NULL),
                        NULL
                    ));
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(r_dst, NULL),
                        Temp_TempLists(F_AX(), NULL)
                    ));
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(F_AX(), NULL),
                        Temp_TempLists(tmp_ax, NULL)
                    ));
                    emit(AS_Move(
                        get_heap_str("mov `d0, `s0\n", left->u.CONST),
                        Temp_TempLists(F_DX(), NULL),
                        Temp_TempLists(tmp_dx, NULL)
                    ));
                    return r_dst;
                }
                default:
                    assert(0);
            }
        }
        case T_MEM: {
            struct EffAddr addr = {
                .assem = NULL,
                .src = NULL
            };
            int pos = x86_eff_addr(e->u.MEM, &addr);
            Temp_temp r = Temp_newtemp();
            emit(AS_Oper(
                get_heap_str("mov `d0, [%s]\n", addr.assem),
                Temp_TempLists(r, NULL),
                addr.src,
                NULL
            ));
            return r;
        }
        case T_TEMP:
            if (e->u.TEMP->num < F_KEEP) {
                return func_formals[e->u.TEMP->num];
            } else {
                return e->u.TEMP;
            }
        case T_NAME: {
            Temp_temp r = Temp_newtemp();
            emit(AS_Oper(
                get_heap_str("mov `d0, %s\n", S_name(e->u.NAME)),
                Temp_TempLists(r, NULL),
                NULL, NULL
            ));
            return r;
        }
        case T_CONST: {
            Temp_temp r = Temp_newtemp();
            emit(AS_Oper(
                get_heap_str("mov `d0, %d\n", e->u.CONST),
                Temp_TempLists(r, NULL),
                NULL, NULL
            ));
            return r;
        }
        case T_CALL: {
            Temp_tempList tmp_regs = NULL;
            Temp_tempList list = muncArgs(e->u.CALL.args, e->u.CALL.accs, &tmp_regs);
            Temp_temp r = Temp_newtemp();
            Temp_temp tmp_rax= Temp_newtemp();
            emit(AS_Move(
                get_heap_str("mov `d0, `s0\n"),
                Temp_TempLists(tmp_rax, NULL),
                Temp_TempLists(F_RV(), NULL)
            ));
            emit(AS_Oper(
                get_heap_str("call %s\n", Temp_labelstring(e->u.CALL.fun->u.NAME)),
                Temp_TempLists(F_RV(), NULL),
                list,
                NULL
            ));
            munchArgRestore(e->u.CALL.accs, tmp_regs);
            // rax is pre-colored, thus un-spillable (cannot save in stack memory)
            // if left until "use", instructions along the way will all create conflict
            // thus, use a fresh temp which can be spilled to replace rax
            emit(AS_Move(
                get_heap_str("mov `d0, `s0\n"),
                Temp_TempLists(r, NULL),
                Temp_TempLists(F_RV(), NULL)
            ));
            emit(AS_Move(
                get_heap_str("mov `d0, `s0\n"),
                Temp_TempLists(F_RV(), NULL),
                Temp_TempLists(tmp_rax, NULL)
            ));
            return r;
        }
        default:
            assert(0);
    }
    return NULL;
}
void munchStm(T_stm s)
{
    switch (s->kind) {
        case T_SEQ:
            munchStm(s->u.SEQ.left);
            munchStm(s->u.SEQ.right);
            break;
        case T_LABEL:
            emit(AS_Label(
                get_heap_str("%s\n", S_name(s->u.LABEL)),
                s->u.LABEL
            ));
            break;
        case T_JUMP:
            emit(AS_Oper(
                get_heap_str("jmp `j0\n"),
                NULL, NULL,
                AS_Targets(s->u.JUMP.jumps)
            ));
            break;
        case T_CJUMP:
            Temp_temp left = munchExp(s->u.CJUMP.left);
            Temp_temp right = munchExp(s->u.CJUMP.right);
            emit(AS_Oper(
                get_heap_str("cmp `s0, `s1\n"),
                NULL,
                Temp_TempLists(left, right, NULL),
                NULL
            ));
            char* instr = NULL;
            switch (s->u.CJUMP.op) {
                case T_eq:
                    instr = "je";
                    break;
                case T_ne:
                    instr = "jne";
                    break;
                case T_lt:
                    instr = "jl";
                    break;
                case T_le:
                    instr = "jle";
                    break;
                case T_gt:
                    instr = "jg";
                    break;
                case T_ge:
                    instr = "jge";
                    break;
            }
            emit(AS_Oper(
                get_heap_str("%s `j0\n", instr),
                NULL,
                NULL,
                AS_Targets(Temp_LabelList(s->u.CJUMP.true, NULL))
            ));
            break;
        case T_MOVE: {
            T_exp dst = s->u.MOVE.dst;
            T_exp src = s->u.MOVE.src;
            if (dst->kind == T_MEM) {
                struct EffAddr addr = {
                    .assem = NULL,
                    .src = NULL
                };
                int pos = x86_eff_addr(dst->u.MEM, &addr);
                int check = fill_last_tmplist(&addr, munchExp(s->u.MOVE.src));
                emit(AS_Oper(
                    get_heap_str("mov [%s], `s%d\n", addr.assem, pos),
                    NULL,
                    addr.src,
                    NULL
                ));
            } else if (dst->kind == T_TEMP) { 
                emit(AS_Move(
                    get_heap_str("mov `d0, `s0\n"),
                    Temp_TempLists(munchExp(s->u.MOVE.dst),NULL),
                    Temp_TempLists(munchExp(s->u.MOVE.src),NULL)
                ));
            } else {
                assert(0);
            }
            break;
        }
        case T_EXP:
            munchExp(s->u.EXP);
            break;
        default:
            assert(0);
    }
}

AS_instrList F_codegen(F_frame f, T_stmList stmList)
{
    T_stmList sl = stmList;
    AS_instrList as_list;
    F_accessList formals = f->formals;
    while (formals) {
        if (formals->head->kind == F_inReg) {
            int i = formals->head->u.reg->num;
            if (!func_formals[i]) {
                func_formals[i] = Temp_newtemp();
            }
            emit(AS_Move(
                get_heap_str("mov `d0, `s0\n"),
                Temp_TempLists(func_formals[i], NULL),
                Temp_TempLists(formals->head->u.reg,NULL)
            ));
        }
        formals = formals->tail;
    }
    while (sl) {
        munchStm(sl->head);
        sl = sl->tail;
    }
    as_list = iList;
    iList = NULL;
    last = NULL;
    return as_list;
}