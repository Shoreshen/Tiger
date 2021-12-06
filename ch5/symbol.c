#include "symbol.h"
#include "table.h"
#include "env.h"

// The symbol table makes same ID(char *) as a pointer
// pointing to the same `struct S_symbol_`
// This avoid the string comparison in the later procedures
S_symbol g_head = NULL;

// All using the same table, so has to be different name
S_symbol S_Symbol(char *id)
{
    S_symbol s;
    HASH_FIND_STR(g_head, id, s);
    if (s) {
        return s;
    }
    s = checked_malloc(sizeof(struct S_symbol_));
    s->id = id;
    HASH_ADD_KEYPTR(hh, g_head, s->id, strlen(s->id), s);
    return s;
}

char *S_name(S_symbol sym)
{
    return sym->id;
}

void S_enter(E_stack stack, S_symbol key, void* value) 
{
    TAB_enter(stack->table, key, value);
}

void* S_look(E_stack stack, S_symbol key) 
{
    TAB_table tab = TAB_look(stack->table, key);
    while (tab == NULL && stack != NULL) {
        stack = stack->next;
        tab = TAB_look(stack->table, key);
    }
    return tab->value;
}

void S_beginScope(E_stack stack) 
{
    E_stack_push(stack);
}

void S_endScope(E_stack stack) 
{
    E_stack_pop(stack);
}