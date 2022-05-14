#include "table.h"
#include "util.h"

void TAB_enter(TAB_table* table, void* key, void* value)
{
    TAB_table t = NULL;
    HASH_FIND_PTR(*table, &key, t);
    if (t) {
        t->value = value;
        return;
    }
    t = checked_malloc(sizeof(*t));
    t->key = key;
    t->value = value;
    HASH_ADD_PTR(*table, key, t);
    return;
}
void TAB_delete(TAB_table* table, void* key)
{
    TAB_table t = NULL;
    HASH_FIND_PTR(*table, &key, t);
    if (t) {
        HASH_DEL(*table, t);
        free(t);
    }
    return;
}
TAB_table TAB_look(TAB_table* table, void* key)
{
    TAB_table t = NULL;
    HASH_FIND_PTR(*table, &key, t);
    return t;
}
void TAB_free(TAB_table* table, void (*free_entry)(void* value))
{
    TAB_table t = NULL;
    TAB_table tmp = NULL;
    HASH_ITER(hh, *table, t, tmp) {
        if (free_entry) {
            free_entry(t);
        }
        HASH_DEL(*table, t);
        free(t);
    }
    return;
}
// Clear the table without freeing each entry with more efficient way
void TAB_clear(TAB_table* table)
{
    HASH_CLEAR(hh, *table);
}
void TAB_dump(TAB_table* table, void (*show)(void* value))
{
    TAB_table t = NULL;
    TAB_table tmp = NULL;
    HASH_ITER(hh, *table, t, tmp) {
        printf("%s: ", t->key);
        show(t->value);
        printf("\n");
    }
    return;
}