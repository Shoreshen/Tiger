#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../uthash/include/uthash.h"

struct table_entry {
    char *id;
    long value;
    UT_hash_handle hh;
};

void* checked_malloc(int len);
void update_table_str(struct table_entry **head, char *id, long value);
long lookup_str(struct table_entry **head, char *id);