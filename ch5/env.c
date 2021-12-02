#include "env.h"
#include "util.h"

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
E_stack E_base_venv(void)
{
    TAB_table venv = NULL;
    TAB_enter(venv, "int", Ty_Int());
    TAB_enter(venv, "string", Ty_String());
    TAB_enter(venv, "nil", Ty_Nil());
    E_stack stack = (E_stack) checked_malloc(sizeof(struct E_stack_));
    stack->table = venv;
    stack->next = NULL;
    return stack;
}
E_stack E_base_tenv(void) 
{
    TAB_table tenv = NULL;
    TAB_enter(tenv, "print", Ty_Void());
    TAB_enter(tenv, "flush", Ty_Void());
    TAB_enter(tenv, "getchar", Ty_String());
    TAB_enter(tenv, "ord", Ty_Int());
    TAB_enter(tenv, "chr", Ty_Int());
    TAB_enter(tenv, "size", Ty_Int());
    TAB_enter(tenv, "substring", Ty_String());
    TAB_enter(tenv, "concat", Ty_String());
    TAB_enter(tenv, "not", Ty_Int());
    TAB_enter(tenv, "exit", Ty_Void());
    E_stack stack = (E_stack) checked_malloc(sizeof(struct E_stack_));
    stack->table = tenv;
    stack->next = NULL;
    return stack;
}