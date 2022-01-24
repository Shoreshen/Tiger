#include "translate.h"
#include "frame.h"
#include "temp.h"
#include "tree.h"

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
            Temp_temp r = Temp_newtemp();
            Temp_label t = Temp_newlabel();
            Temp_label f = Temp_newlabel();
            doPatch(e->u.cx->trues, t);
            doPatch(e->u.cx->falses, f);
            return T_Eseq(
                T_Move(T_Temp(r), T_Const(1)),              // step1: move 1 to r
                T_Eseq(
                    e->u.cx->stm,                            // step2: execute stm
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

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals)
{
    Tr_level level = checked_malloc(sizeof(*level));
    level->parent = parent;
    level->formals = NULL;
    level->frame = F_newFrame(name, U_BoolList(TRUE, formals));

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

#pragma region fragment
F_fragList Tr_getResult(void)
{
    return fragList;
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
Tr_exp Tr_fieldVar(Tr_exp var, int pos)
{
    return Tr_Ex(
        T_Mem(
            T_Binop(
                T_plus, 
                unEx(var), 
                T_Const(pos * F_WORD_SIZE)
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
Tr_exp Tr_stringEq(Tr_exp left, Tr_exp right)
{
    T_expList args = T_ExpList(unEx(left), T_ExpList(unEx(right), NULL));
    return Tr_Ex(F_externalCall("stringEqual", args));
}
Tr_exp Tr_callExp(Temp_label func, Tr_level level, Tr_level fun_level, Tr_expList args)
{
    // label:   function label
    // level:   current level
    // fun_level:  function level
    // args:    function arguments
    T_expList tmp_args = NULL;
    while (args) {
        tmp_args = T_ExpList(unEx(args->head), tmp_args);
        args = args->tail;
    }
    T_exp fp = T_Temp(F_FP());
    while (level != fun_level) {
        // Get last(outside) fp from current fp
        // The first parameter is static link, 
        fp = F_Exp(Tr_formals(level)->head->access, fp);
        level = level->parent;
    }
    return Tr_Ex(T_Call(T_Name(func), T_ExpList(fp, tmp_args)));
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
        } else if (then->kind == Tr_cx && elsee->kind == Tr_cx) {
            // Both then and else are cx
            patchList new_t = joinPatch(then->u.cx->trues, elsee->u.cx->trues);
            patchList new_f = joinPatch(then->u.cx->falses, elsee->u.cx->falses);
            return Tr_Cx(new_t, new_f,
                T_Seqs(
                    cond->stm, // Jump to label "t" if cond is true, label "f" if cond is false
                    T_Label(t),
                    // Jump to label "new_t" or "new_f", which will be filled later
                    // Thus no need to add further jump (e.g to label "join") 
                    then->u.cx->stm, 
                    T_Label(f),
                    // Same as above
                    elsee->u.cx->stm,
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
            T_ExpList(T_Const(size * F_WORD_SIZE), NULL)
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
Tr_exp Tr_arrayExp(Tr_exp init, int size)
{
    Temp_temp r = Temp_newtemp();
    return Tr_Ex(
        T_Eseq(
            T_Move(
                T_Temp(r), 
                F_externalCall(
                    "check_malloc", 
                    T_ExpList(T_Const(size * F_WORD_SIZE), NULL)
                )
            ),
            T_Temp(r)
        )
    );
}
Tr_exp Tr_whileExp(Tr_exp test, Tr_exp body)
{
    Tr_cjx cond = unCx(test);
    Temp_label t = Temp_newlabel();
    Temp_label f = Temp_newlabel();
    Temp_label start = Temp_newlabel();
    doPatch(cond->trues, t);
    doPatch(cond->trues, f);

    return Tr_Nx(
        T_Seqs(
            T_Label(start),
            cond->stm,
            T_Label(t),
            unNx(body),
            T_Jump(T_Name(start), Temp_LabelList(start, NULL)),
            T_Label(f),
            NULL
        )
    );
}
Tr_exp Tr_forExp(Tr_exp body, int hi, int lo)
{
    Temp_temp r = Temp_newtemp();
    Temp_label start = Temp_newlabel();
    T_Seqs(
        T_Move(T_Temp(r), T_Const(lo)),
        T_Label(start),
        unNx(body),
        T_Move(T_Temp(r), T_Binop(T_plus, T_Temp(r), T_Const(1))),
        T_Cjump(T_le, T_Temp(r), T_Const(hi), start, NULL),
        NULL
    );
    return NULL;
}
Tr_exp Tr_breakExp(Temp_label break_label)
{
    return Tr_Nx(T_Jump(T_Name(break_label), NULL));
}
Tr_exp Tr_seqExp(Tr_expList expList)
{
    T_stm stm = unNx(expList->head);
    expList = expList->tail;
    while(expList) {
        stm = T_Seq(stm, unNx(expList->head));
        expList = expList->tail;
    }
    return Tr_Nx(stm);
}
Tr_exp Tr_assignExp(Tr_exp lhs, Tr_exp rhs)
{
    return Tr_Nx(T_Move(unEx(lhs), unEx(rhs)));
}
#pragma endregion