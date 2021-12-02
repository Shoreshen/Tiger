#pragma once
#include "env.h"

typedef struct S_symbol_ *S_symbol;

struct S_symbol_ {
    char *id;
    UT_hash_handle hh;
};

S_symbol S_Symbol(char *id);
char *S_name(S_symbol sym);

void S_enter(E_stack stack, char *key, void* value);
void* S_look(E_stack stack, char *key);
void S_beginScope(E_stack stack);
void S_endScope(E_stack stack);