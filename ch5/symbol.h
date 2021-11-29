#pragma once
#include "../uthash/include/uthash.h"

typedef struct S_symbol_ *S_symbol;

struct S_symbol_ {
    char *id;
    UT_hash_handle hh;
};

S_symbol S_Symbol(char *id);
S_symbol lookup_sym(char *id);
char *S_name(S_symbol sym);