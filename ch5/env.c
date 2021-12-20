#include "env.h"
#include "table.h"
#include "types.h"

void E_stack_push(E_stack stack) {
    E_stack tmp = (E_stack) checked_malloc(sizeof(struct E_stack_));
    tmp->table = NULL;
    tmp->next = stack;
    stack = tmp;
}
void E_stack_pop(E_stack stack) {
    E_stack tmp = stack;
    if (stack != NULL) {
        stack = stack->next;
        TAB_free(tmp->table);
        free(tmp);
    }
}
E_stack E_base_tenv(void)
{
    TAB_table tenv = NULL;
    TAB_enter(tenv, "int", Ty_Int());
    TAB_enter(tenv, "string", Ty_String());
    TAB_enter(tenv, "nil", Ty_Nil());
    E_stack stack = (E_stack) checked_malloc(sizeof(struct E_stack_));
    stack->table = tenv;
    stack->next = NULL;
    return stack;
}
E_stack E_base_venv(void) 
{
    TAB_table venv = NULL;
    TAB_enter(venv, "print", E_FunEntry(NULL, Ty_Void()));
    TAB_enter(venv, "flush", E_FunEntry(NULL, Ty_Void()));
    TAB_enter(venv, "getchar", E_FunEntry(NULL, Ty_String()));
    TAB_enter(venv, "ord", E_FunEntry(NULL, Ty_Int()));
    TAB_enter(venv, "chr", E_FunEntry(NULL, Ty_Int()));
    TAB_enter(venv, "size", E_FunEntry(NULL, Ty_Int()));
    TAB_enter(venv, "substring", E_FunEntry(NULL, Ty_String()));
    TAB_enter(venv, "concat", E_FunEntry(NULL, Ty_String()));
    TAB_enter(venv, "not", E_FunEntry(NULL, Ty_Int()));
    TAB_enter(venv, "exit", E_FunEntry(NULL, Ty_Void()));
    E_stack stack = (E_stack) checked_malloc(sizeof(struct E_stack_));
    stack->table = venv;
    stack->next = NULL;
    return stack;
}

E_enventry E_VarEntry(Ty_ty ty) {
    E_enventry e = (E_enventry) checked_malloc(sizeof(struct E_enventry_));
    e->kind = E_varEntry;
    e->u.var.ty = ty;
    return e;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result) {
    E_enventry e = (E_enventry) checked_malloc(sizeof(struct E_enventry_));
    e->kind = E_funEntry;
    e->u.fun.formals = formals;
    e->u.fun.result = result;
    return e;
}