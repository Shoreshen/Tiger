#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define STEP_STR_LEN 128
#define GET_CAP_LEN(len) ((len + STEP_STR_LEN - 1) / STEP_STR_LEN * STEP_STR_LEN)

struct string {
    int cap;
    int len;
    char *data;
};

void* checked_malloc(int len);

// For string
void init_str(struct string* str);
void put_str(struct string* str, char src);
void append_str(struct string* str, char* src);

