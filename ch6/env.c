#include "env.h"
#include "table.h"
#include "types.h"
#include "symbol.h"
#include "translate.h"
#include "temp.h"

void E_stack_push(E_stack* stack) {
    E_stack tmp = (E_stack) checked_malloc(sizeof(struct E_stack_));
    tmp->table = NULL;
    tmp->next = *stack;
    *stack = tmp;
}
void E_stack_pop(E_stack* stack) {
    E_stack tmp = *stack;
    if (stack != NULL) {
        *stack = (*stack)->next;
        TAB_free(&tmp->table);
        free(tmp);
    }
}
E_stack E_empty_env(void)
{
    E_stack s = checked_malloc(sizeof(struct E_stack_));
    s->table = NULL;
    s->next = NULL;
    return s;
}
E_stack E_base_tenv(void)
{
    E_stack tenv = E_empty_env();
    TAB_enter(&tenv->table, S_Symbol("int"), Ty_Int());
    TAB_enter(&tenv->table, S_Symbol("string"), Ty_String());
    TAB_enter(&tenv->table, S_Symbol("nil"), Ty_Nil());
    return tenv;
}
E_stack E_base_venv(void) 
{
    E_stack venv = E_empty_env();
    TAB_enter(&venv->table, S_Symbol("print"), 
        E_FunEntry(
            Tr_outermost(), 
            Temp_newlabel(), 
            Ty_TyList(Ty_String(), NULL), 
            Ty_Void()
        )
    );
    TAB_enter(&venv->table, S_Symbol("flush"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(),
            NULL, 
            Ty_Void()
        )
    );
    TAB_enter(&venv->table, S_Symbol("getchar"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(),
            NULL, 
            Ty_String()
        )
    );
    TAB_enter(&venv->table, S_Symbol("ord"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(),
            Ty_TyList(Ty_String(), NULL), 
            Ty_Int()
        )
    );
    TAB_enter(&venv->table, S_Symbol("chr"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(),
            Ty_TyList(Ty_Int(), NULL), 
            Ty_Int()
        )
    );
    TAB_enter(&venv->table, S_Symbol("size"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(), 
            Ty_TyList(Ty_String(), NULL), 
            Ty_Int()
        )
    );
    TAB_enter(&venv->table, S_Symbol("substring"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(), 
            Ty_TyList(Ty_String(), Ty_TyList(Ty_Int(), Ty_TyList(Ty_Int(), NULL))), 
            Ty_String()
        )
    );
    TAB_enter(&venv->table, S_Symbol("concat"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(), 
            Ty_TyList(Ty_String(), Ty_TyList(Ty_String(), NULL)),
            Ty_String()
        )
    );
    TAB_enter(&venv->table, S_Symbol("not"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(), 
            Ty_TyList(Ty_Int(), NULL), 
            Ty_Int()
        )
    );
    TAB_enter(&venv->table, S_Symbol("exit"), 
        E_FunEntry(
            Tr_outermost(),
            Temp_newlabel(), 
            Ty_TyList(Ty_Int(), NULL), 
            Ty_Void()
        )
    );
    return venv;
}

E_enventry E_VarEntry(Tr_access access, Ty_ty ty) 
{
    E_enventry e = (E_enventry) checked_malloc(sizeof(struct E_enventry_));
    e->kind = E_varEntry;
    e->u.var.ty = ty;
    e->u.var.access = access;
    return e;
}

E_enventry E_FunEntry(Tr_level level, Temp_label label, Ty_tyList formals, Ty_ty result) 
{
    E_enventry e = (E_enventry) checked_malloc(sizeof(struct E_enventry_));
    e->kind = E_funEntry;
    e->u.fun.level = level;
    e->u.fun.label = label;
    e->u.fun.formals = formals;
    e->u.fun.result = result;
    return e;
}