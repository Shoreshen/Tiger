#include "../uthash/include/uthash.h"

typedef struct S_symbol_ *S_symbol;

struct S_symbol_ {
    char *id;
    UT_hash_handle hh;
};

void update_table_str(S_symbol *head, char *id, long value);
long lookup_str(S_symbol *head, char *id);