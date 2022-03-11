#include "util.h"

void* checked_malloc(int len) 
{
    void *p = malloc(len);
    assert(p);
    return p;
}

void update_table_str(struct table_entry **head, char *id, long value)
{
    struct table_entry* s;

    HASH_FIND_STR(*head, id, s);
    if (s) {
        s->value = value;
    }
    s = checked_malloc(sizeof(struct table_entry));
    s->id = id;
    s->value =value;
    HASH_ADD_KEYPTR(hh, *head, s->id, strlen(s->id), s);
}

long lookup_str(struct table_entry **head, char *id)
{
    struct table_entry* s;

    HASH_FIND_STR(*head, id, s);
    if(!s) {
        printf("Cannot find: ID %s may not defined\n", id);
    }

    return s->value;
}