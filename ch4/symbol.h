#include "../uthash/include/uthash.h"

typedef struct S_symbol_ *S_symbol;

struct S_symbol_ {
    char *id;
    UT_hash_handle hh;
};

void insert_sym(S_symbol s);
S_symbol lookup_sym(char *id);