#pragma once
#include "util.h"

typedef struct E_stack_* E_stack;
typedef struct E_enventry_ *E_enventry;

struct E_enventry_ {
    enum { 
        E_varEntry, 
        E_funEntry 
    } kind;
    union {
        struct {
            Tr_access access;
            Ty_ty ty; 
        } var;
        struct {
            Tr_level level;
            Temp_label label;
            Ty_tyList formals; 
            Ty_ty result;
        } fun;
    } u;
};

struct E_stack_{
    TAB_table table;
    E_stack next;
};

void E_stack_push(E_stack* stack);
void E_stack_pop(E_stack* stack, void (*free_entry)(void* value)) ;
E_stack E_base_venv(void);
E_stack E_base_tenv(void);
E_stack E_empty_env(void);
E_enventry E_VarEntry(Tr_access access, Ty_ty ty);
E_enventry E_FunEntry(Tr_level level, Temp_label label, Ty_tyList formals, Ty_ty result);