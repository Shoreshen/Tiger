#include "translate.h"
#include "frame.h"
#include "temp.h"
#include "tree.h"
#include "symbol.h"

#pragma region internal struct & statics
typedef struct patchList_ *patchList;
typedef struct Tr_cjx_* Tr_cjx;
struct Tr_level_ {
    Tr_level parent;
    F_frame frame;
    Tr_accessList formals;
};
struct Tr_access_ {
    Tr_level level; 
    F_access access;
};
struct patchList_ {
    Temp_label *head; 
    patchList tail;
};
struct Tr_cjx_ {
    patchList trues; 
    patchList falses; 
    T_stm stm;
};
struct Tr_exp_ {
    enum {
        Tr_ex, 
        Tr_nx, 
        Tr_cx
    } kind;
    union {
        T_exp ex; 
        T_stm nx; 
        Tr_cjx cx; 
    } u;
};
static Tr_level out_most = NULL;
static F_fragList fragList = NULL;
#pragma endregion

#pragma region internal function
Tr_exp Tr_Ex(T_exp ex)
{
    Tr_exp e = checked_malloc(sizeof(*e));
    e->kind = Tr_ex;
    e->u.ex = ex;
    return e;
}
Tr_exp Tr_Nx(T_stm nx)
{
    Tr_exp e = checked_malloc(sizeof(*e));
    e->kind = Tr_nx;
    e->u.nx = nx;
    return e;
}
Tr_cjx Tr_Cjx(patchList trues, patchList falses, T_stm stm)
{
    Tr_cjx cjx = checked_malloc(sizeof(*cjx));
    cjx->trues = trues;
    cjx->falses = falses;
    cjx->stm = stm;
    return cjx;
}
Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm)
{
    Tr_exp e = checked_malloc(sizeof(*e));
    e->u.cx = checked_malloc(sizeof(*(e->u.cx)));
    e->kind = Tr_cx;
    e->u.cx = Tr_Cjx(trues, falses, stm);
    return e;
}
patchList PatchList(Temp_label *head, patchList tail)
{
    patchList l = checked_malloc(sizeof(*l));
    l->head = head;
    l->tail = tail;
    return l;
}

void doPatch(patchList tList, Temp_label label) {
    for (; tList; tList=tList->tail) {
        *(tList->head) = label;
    }
}

patchList joinPatch(patchList first, patchList second) {
    if (!first) {
        return second;
    }
    patchList tmp = first;
    while (tmp->tail) {
        tmp = tmp->tail;
    }
    tmp->tail = second;
    return first;
}

T_exp unEx(Tr_exp e)
{
    switch(e->kind) {
        case Tr_ex:
            return e->u.ex;
        case Tr_nx:
            return T_Eseq(e->u.nx, T_Const(0));
        case Tr_cx: {
            // Create expressions that:
            //  1. Return 1 if e->u.cx->stm jump to e->u.cs->trues
            //  2. Return 0 if e->u.cx->stm jump to e->u.cs->falses
            Temp_temp r = Temp_newtemp();
            Temp_label t = Temp_newlabel();
            Temp_label f = Temp_newlabel();
            doPatch(e->u.cx->trues, t);
            doPatch(e->u.cx->falses, f);
            return T_Eseq(
                T_Move(T_Temp(r), T_Const(1)),              // step1: move 1 to r
                T_Eseq(
                    e->u.cx->stm,                           // step2: execute stm
                    T_Eseq(
                        T_Label(f),                         // step3: step2 will jump to f if evaluated as false
                        T_Eseq(
                            T_Move(T_Temp(r), T_Const(0)),  // step4: if jump to f, move 0 to r 
                            T_Eseq(
                                T_Label(t),                 // step5: step2 will jump to t if evaluated as true
                                T_Temp(r)                   // step6: return r
                            )
                        )
                    )
                )
            );
        }
    }
    assert(0);
}
T_stm unNx(Tr_exp e)
{
    switch(e->kind) {
        case Tr_ex:
            return T_Exp(e->u.ex);
        case Tr_nx:
            return e->u.nx;
        case Tr_cx:
            return T_Exp(unEx(e));
    }
    assert(0);
}
Tr_cjx unCx(Tr_exp e)
{
    switch(e->kind) {
        case Tr_ex:
            // If exp is evaluated as true, jump to t, else jump to f
            // Both true and false need to be filled
            T_stm stm = NULL;
            if (e->kind == Tr_ex && e->u.ex->kind == T_CONST) {
                stm = T_Jump(T_Name(NULL), Temp_LabelList(NULL, NULL)); // Directly jump for CONST
                patchList pl = PatchList(
                    &(stm->u.JUMP.exp->u.NAME), // T_Name will be filled later
                    PatchList(
                        &(stm->u.JUMP.jumps->head), // Temp_list will be filled later
                        NULL
                    )
                );
                if (e->u.ex->u.CONST == 0) {
                    return Tr_Cjx(NULL, pl, stm); // Directly jump to false lable
                } else {
                    return Tr_Cjx(pl, NULL, stm); // Directly jump to true lable
                }
            }
            stm = T_Cjump(T_ne, e->u.ex, T_Const(0), NULL, NULL);
            return Tr_Cjx(
                PatchList(&(stm->u.CJUMP.true), NULL), 
                PatchList(&(stm->u.CJUMP.false), NULL), 
                stm
            );
        case Tr_nx:
            assert(0);
        case Tr_cx:
            return e->u.cx;
    }
    assert(0);
}
#pragma endregion

#pragma region level
Tr_level Tr_outermost(void)
{
    if (out_most == NULL) {
        out_most = checked_malloc(sizeof(*out_most));
        out_most->parent = NULL;
        out_most->frame = F_newFrame(Temp_newlabel(), NULL);
        out_most->formals = NULL;
    }
    return out_most;
}

Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail)
{
    Tr_accessList list = checked_malloc(sizeof(*list));
    list->head = head;
    list->tail = tail;
    return list;
}

Tr_access Tr_Access(Tr_level level, F_access access)
{
    Tr_access a = checked_malloc(sizeof(*a));
    a->level = level;
    a->access = access;
    return a;
}

Tr_level Tr_newLevel(Tr_level parent, U_boolList formals)
{
    Tr_level level = checked_malloc(sizeof(*level));
    level->parent = parent;
    level->formals = NULL;
    level->frame = F_newFrame(Temp_newlabel(), U_BoolList(TRUE, formals));

    F_accessList tmp_formals = F_formals(level->frame);
    while (tmp_formals) {
        Tr_access access = Tr_Access(level, tmp_formals->head);
        level->formals = Tr_AccessList(access, level->formals);
        tmp_formals = tmp_formals->tail;
    }
    return level;
}

Tr_access Tr_allocLocal(Tr_level level, int escape)
{
    F_access access = F_allocLocal(level->frame, escape);
    return Tr_Access(level, access);
}

Tr_accessList Tr_formals(Tr_level level)
{
    return level->formals;
}

Temp_label Tr_name(Tr_level level)
{
    return F_name(level->frame);
}
#pragma endregion

#pragma region fragment & print
F_fragList Tr_getResult(void)
{
    return fragList;
}
void Tr_print(FILE *out, Tr_exp e)
{
    T_stm stm = unNx(e);
    pr_stm(out, stm, 0);
    fprintf(out, "\n");
    F_printFrags(out, Tr_getResult());
}
#pragma endregion

#pragma region create tree
Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail)
{
    Tr_expList list = checked_malloc(sizeof(*list));
    list->head = head;
    list->tail = tail;
    return list;
}
Tr_exp Tr_simpleVar(Tr_access access, Tr_level level)
{
    // access:  target simpleVar
    // level:   current level
    T_exp fp = T_Temp(F_FP());
    while (access->level != level) {
        // Get last(outside) fp from current fp
        // The first parameter is static link, 
        fp = F_Exp(Tr_formals(level)->head->access, fp);
        level = level->parent;
    }
    return Tr_Ex(F_Exp(access->access, fp));
}
Tr_exp Tr_subscriptVar(Tr_exp var, Tr_exp index)
{
    return Tr_Ex(
        T_Mem(
            T_Binop(
                T_plus, 
                unEx(var), 
                T_Binop(T_mul, unEx(index), T_Const(F_WORD_SIZE))
            )
        )
    );
}
Tr_exp Tr_fieldVar(Tr_exp var, int index)
{
    return Tr_Ex(
        T_Mem(
            T_Binop(
                T_plus, 
                unEx(var), 
                T_Const(index * F_WORD_SIZE)
            )
        )
    );
}
Tr_exp Tr_nilExp()
{
    return Tr_Ex(T_Const(0));
}
Tr_exp Tr_intExp(int i)
{
    return Tr_Ex(T_Const(i));
}
Tr_exp Tr_stringExp(char* str)
{
    Temp_label str_l = Temp_newlabel();
    F_frag str_f = F_StringFrag(str_l, str);
    fragList = F_FragList(str_f, fragList);
    return Tr_Ex(T_Name(str_l));
}
Tr_exp Tr_arithExp(A_oper oper, Tr_exp left, Tr_exp right)
{
    T_binOp op;
    switch (oper) {
        case A_plusOp:
            op = T_plus;
            break;
        case A_minusOp:
            op = T_minus;
            break;
        case A_timesOp:
            op = T_mul;
            break;
        case A_divideOp:
            op = T_div;
            break;
        default:
            assert(0);
    }
    return Tr_Ex(T_Binop(op, unEx(left), unEx(right)));
}
Tr_exp Tr_relExp(A_oper oper, Tr_exp left, Tr_exp right)
{
    T_relOp op;
    switch (oper) {
        case A_eqOp:
            op = T_eq;
            break;
        case A_neqOp:
            op = T_ne;
            break;
        case A_ltOp:
            op = T_lt;
            break;
        case A_leOp:
            op = T_le;
            break;
        case A_gtOp:
            op = T_gt;
            break;
        case A_geOp:
            op = T_ge;
            break;
    }
    T_stm cjump = T_Cjump(op, unEx(left), unEx(right), NULL, NULL);
    return Tr_Cx(
        PatchList(&(cjump->u.CJUMP.true), NULL), 
        PatchList(&(cjump->u.CJUMP.false), NULL), 
        cjump
    );
}
Tr_exp Tr_logicExp(A_oper oper, Tr_exp left, Tr_exp right)
{
    Tr_cjx cjl = unCx(left);
    Tr_cjx cjr = unCx(right);
    Temp_label z = Temp_newlabel();
    if (oper == A_andOp) {
        doPatch(cjl->trues, z);
        patchList new_f = joinPatch(cjl->falses, cjr->falses);
        return Tr_Cx(cjr->trues, new_f,
            T_Seqs(
                // If true, Jump to label "z", then evaluate cjr->stm
                // If false, jump to joined new_f, which will be filled later
                cjl->stm,
                T_Label(z),
                // If true, jump to cjr->trues
                // If false, jump to joined new_f
                // Both will be filled
                cjr->stm, 
                NULL
            )
        );
    } else if (oper == A_orOp) {
        doPatch(cjl->falses, z);
        patchList new_t = joinPatch(cjl->trues, cjr->trues);
        return Tr_Cx(new_t, cjr->falses,
            T_Seqs(
                // If true, jump to joined new_t, which will be filled later
                // If false, jump to label "z", then evaluate cjr->stm
                cjl->stm,
                T_Label(z),
                // If true, jump to joined new_t
                // If false, jump to cjr->falses
                // Both will be filled
                cjr->stm, 
                NULL
            )
        );
    }
    return NULL;
}
Tr_exp Tr_stringCmp(A_oper oper, Tr_exp left, Tr_exp right)
{
    T_expList args = T_ExpList(unEx(left), T_ExpList(unEx(right), NULL));
    T_exp res = F_externalCall(
        "stringEqual", args, 
        F_AccessList(F_InReg(F_Keep_Regs(0)), F_AccessList(F_InReg(F_Keep_Regs(1)), NULL))
    );
    if (oper == A_neqOp) {
        return Tr_Ex(T_Binop(T_minus, T_Const(1), res));
    } else {
        return Tr_Ex(res);
    }
}
Tr_exp Tr_callExp(Temp_label func, Tr_level level, Tr_level fun_level, Tr_expList args, S_symbol name)
{
    // label:   function label
    // level:   current level
    // fun_level:  function level
    // args:    function arguments
    T_expList tmp_args = NULL;
    F_accessList tmp_formals = NULL;
    F_accessList formals_reverse = fun_level->frame->formals;
    int regCount = 0, memCount = 0;
    // args is reversly constructed, thus reverse to right order
    while (args) {
        tmp_args = T_ExpList(unEx(args->head), tmp_args);
        if (formals_reverse) {
            tmp_formals = F_AccessList(formals_reverse->head, tmp_formals);
            formals_reverse = formals_reverse->tail;
        } else {
            tmp_formals = F_AccessList(F_GetAccess(&regCount, &memCount, 0), tmp_formals);
        }
        args = args->tail;
    }
    // Function level is outer level of current level
    // indicating a external function call
    if (!fun_level->parent) {
        return Tr_Ex(F_externalCall(S_name(name), tmp_args, tmp_formals));
    }
    if (formals_reverse) {
        // Append FP to accesses
        tmp_formals = F_AccessList(formals_reverse->head, tmp_formals);
    }
    T_exp fp = T_Temp(F_FP());
    while (level != fun_level->parent) {
        // Get last(outside) fp from current fp
        // The first parameter is static link, 
        fp = F_Exp(Tr_formals(level)->head->access, fp);
        level = level->parent;
    }
    return Tr_Ex(T_Call(T_Name(func), T_ExpList(fp, tmp_args), tmp_formals, name));
}
Tr_exp Tr_ifExp(Tr_exp test, Tr_exp then, Tr_exp elsee)
{
    Temp_label t = Temp_newlabel();
    Temp_label f = Temp_newlabel();
    Tr_cjx cond = unCx(test);
    doPatch(cond->trues, t);
    doPatch(cond->falses, f);

    if (elsee) {
        Temp_label join = Temp_newlabel();
        if (then->kind == Tr_nx && elsee->kind == Tr_nx) {
            // Both then and else are nx, no value for if expression
            return Tr_Nx(
                T_Seqs(
                    // Jump to label "t" if cond is true, label "f" if cond is false
                    cond->stm,
                    T_Label(t),
                    then->u.nx,
                    T_Jump(T_Name(join), Temp_LabelList(join, NULL)),
                    T_Label(f),
                    elsee->u.nx,
                    T_Label(join),
                    NULL
                )
            );
        } else {
            T_exp then_exp = unEx(then);
            T_exp else_exp = unEx(elsee);
            Temp_temp r = Temp_newtemp();
            return Tr_Ex(
                T_Eseq(
                    T_Seqs(
                        cond->stm,
                        T_Label(t),
                        T_Move(T_Temp(r), then_exp),
                        T_Jump(T_Name(join), Temp_LabelList(join, NULL)),
                        T_Label(f),
                        T_Move(T_Temp(r), else_exp),
                        T_Label(join),
                        NULL
                    ),
                    T_Temp(r)
                )
            );
        }
    } else {
        // If no else, then must be no return value for if expression
        if (then->kind == Tr_cx) {
            return Tr_Nx(
                T_Seqs(
                    cond->stm,
                    T_Label(t),
                    then->u.cx->stm,
                    T_Label(f),
                    NULL
                )
            );
        } else {
            return Tr_Nx(
                T_Seqs(
                    cond->stm,
                    T_Label(t),
                    unNx(then),
                    T_Label(f),
                    NULL
                )
            );
        }
    }
}
Tr_exp Tr_recordExp(Tr_expList fields, int size)
{
    Temp_temp r = Temp_newtemp();
    int offset = 0;
    T_stm stm = T_Move(
        T_Temp(r), 
        F_externalCall(
            "check_malloc", 
            T_ExpList(T_Const(size * F_WORD_SIZE), NULL),
            F_AccessList(F_InReg(F_Keep_Regs(0)), NULL)
        )
    );
    while(fields) {
        stm = T_Seq(
            stm, 
            T_Move(
                T_Mem(T_Binop(T_plus, T_Temp(r), T_Const(offset))), 
                unEx(fields->head)
            )
        );
        fields = fields->tail;
        offset += F_WORD_SIZE;
    }
    return Tr_Ex(T_Eseq(stm, T_Temp(r)));
}
Tr_exp Tr_arrayExp(Tr_exp init, Tr_exp size)
{
    Temp_temp r = Temp_newtemp();
    return Tr_Ex(
        T_Eseq(
            T_Move(
                T_Temp(r), 
                F_externalCall(
                    "initArray", 
                    T_ExpList(
                        T_Binop(T_mul, T_Const(F_WORD_SIZE), unEx(size)), // size
                        T_ExpList(unEx(init), NULL)  // init value   
                    ),
                    F_AccessList(
                        F_InReg(F_Keep_Regs(0)), 
                        F_AccessList(
                            F_InReg(F_Keep_Regs(1)), NULL
                        )
                    )
                )
            ),
            T_Temp(r)
        )
    );
}
Tr_exp Tr_whileExp(Tr_exp test, Tr_exp body, Tr_exp done)
{
    Tr_cjx cond = unCx(test);
    Temp_label t = Temp_newlabel();
    Temp_label start = Temp_newlabel();
    doPatch(cond->trues, t);
    doPatch(cond->falses, unEx(done)->u.NAME);

    return Tr_Nx(
        T_Seqs(
            T_Label(start),
            cond->stm,
            T_Label(t),
            unNx(body),
            T_Jump(T_Name(start), Temp_LabelList(start, NULL)),
            T_Label(unEx(done)->u.NAME),
            NULL
        )
    );
}
Tr_exp Tr_forExp(Tr_exp body, Tr_access i, Tr_exp lo, Tr_exp hi, Tr_exp done)
{
    Temp_temp r = Temp_newtemp();
    Temp_label start = Temp_newlabel();
    // Looping variable must in the same level with for expression
    T_exp lo_var = F_Exp(i->access, NULL);
    return Tr_Nx(
        T_Seqs(
            T_Move(lo_var, unEx(lo)),
            T_Label(start),
            unNx(body),
            T_Move(lo_var, T_Binop(T_plus, lo_var, T_Const(1))),
            T_Cjump(T_le, lo_var, unEx(hi), start, unEx(done)->u.NAME),
            T_Label(unEx(done)->u.NAME),
            NULL
        )
    );
}
Tr_exp Tr_breakExp(Tr_exp done)
{
    return Tr_Nx(T_Jump(unEx(done), Temp_LabelList(unEx(done)->u.NAME, NULL)));
}
Tr_exp Tr_doneExp()
{
    return Tr_Ex(T_Name(Temp_newlabel()));
}
Tr_exp Tr_seqExp(Tr_expList expList)
{
    // At least one expression
    T_exp seq = unEx(expList->head);
    expList = expList->tail;
    // The expList is reversly linked, thus the first is the last expression
    // which provide the return value
    while(expList) {
        seq = T_Eseq(unNx(expList->head), seq);
        expList = expList->tail;
    }
    return Tr_Ex(seq);
}
Tr_exp Tr_assignExp(Tr_exp lhs, Tr_exp rhs)
{
    return Tr_Nx(T_Move(unEx(lhs), unEx(rhs)));
}
Tr_exp Tr_seqence(Tr_exp left, Tr_exp right)
{
    return Tr_Nx(T_Seq(unNx(left), unNx(right)));
}
void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals)
{
    T_stm stm = T_Move(T_Temp(F_RV()), unEx(body));
    F_frag pfrag = F_ProcFrag(stm, level->frame);
	fragList = F_FragList(pfrag, fragList);
}
#pragma endregion