#pragma once
#include "util.h"

struct S_symbol_ {
    char *id;
    UT_hash_handle hh;
};

S_symbol S_Symbol(char *id);
char *S_name(S_symbol sym);

void S_enter(E_map stack, S_symbol key, void* value);
void* S_look(E_map stack, S_symbol key);
void S_beginScope(E_map* stack);
void S_endScope(E_map *stack, void (*free_entry)(void* value)) ;