#pragma once
#include "util.h"

typedef struct E_map_* E_map;
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

struct E_map_{
    TAB_table table;
    E_map next;
};

void E_map_push(E_map* stack);
void E_map_pop(E_map* stack, void (*free_entry)(void* value));
void* E_look(E_map stack, void* key);
void E_enter(E_map stack, void* key, void* value);
void E_clear(E_map stack);
E_map E_base_venv(void);
E_map E_base_tenv(void);
E_map E_empty_env(void);
E_enventry E_VarEntry(Tr_access access, Ty_ty ty);
E_enventry E_FunEntry(Tr_level level, Temp_label label, Ty_tyList formals, Ty_ty result);