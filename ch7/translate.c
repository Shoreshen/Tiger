#include "translate.h"
#include "frame.h"
#include "temp.h"
#include "tree.h"

#pragma region internal struct
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
static T_stm unNx(Tr_exp e)
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
static Tr_level out_most = NULL;

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

#pragma region create tree
// access:  target simpleVar
// level:   current level
Tr_exp Tr_simpleVar(Tr_access access, Tr_level level)
{
    T_exp fp = F_FP();
    while (access->level != level) {
        // Get last(outside) fp from current fp
        // The first parameter is static link, 
        fp = F_Exp(Tr_formals(level)->head->access, fp);
        level = level->parent;
    }
    return Tr_Ex(F_Exp(access, fp));
}
#pragma endregion