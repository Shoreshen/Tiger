#include "symbol.h"

// Hash table header for UT-HASH
// Must be initialized as NULL
S_symbol g_head = NULL;

void insert_sym(S_symbol s)
{
    HASH_FIND_STR(g_head, s->id, s);
    if (s) {
        return;
    }
    HASH_ADD_KEYPTR(hh, g_head, s->id, strlen(s->id), s);
}

S_symbol lookup_sym(char *id)
{
    S_symbol s = NULL;
    HASH_FIND_STR(g_head, id, s);
    if(!s) {
        printf("Cannot find: ID %s may not defined\n", id);
    }
    return s;
}