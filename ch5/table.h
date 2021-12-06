#pragma once
#include "util.h"

struct TAB_table_ {
    void* key;
    void* value;
    UT_hash_handle hh;
};

void TAB_enter(TAB_table table, void* key, void* value);
void TAB_delete(TAB_table table, void* key);
TAB_table TAB_look(TAB_table table, void* key);
void TAB_free(TAB_table table);
void TAB_dump(TAB_table table, void (*show)(void* value));