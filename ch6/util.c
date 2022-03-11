#include "util.h"

void* checked_malloc(int len) 
{
    void *p = malloc(len);
    assert(p);
    return p;
}

void init_str(struct string* str)
{
    str->data = checked_malloc(STEP_STR_LEN);
    str->data[0] = 0;
    str->cap = STEP_STR_LEN;
    str->len = 0;
}

void put_str(struct string* str, char src)
{
    char *tmp;
    if (str->len + 1 > str->cap) {
        str->cap = str->cap + STEP_STR_LEN;
        tmp = str->data;
        str->data = checked_malloc(str->cap + STEP_STR_LEN);
        strcpy(str->data, tmp);
        free(tmp);
    }
    str->data[str->len] = src;
    str->data[str->len + 1] = 0;
    str->len++; 
}

void append_str(struct string* str, char* src)
{
    char *tmp;
    int new_len = strlen(src) + str->len;
    if (new_len > str->cap) {
        str->cap = GET_CAP_LEN(new_len);
        tmp = str->data;
        str->data = checked_malloc(str->cap);
        strcpy(str->data, tmp);
        free(tmp);
    }
    strcat(str->data, src);
    str->len = new_len;
}

char* get_heap_str(char* str)
{
    char *tmp = checked_malloc(strlen(str) + 1);
    strcpy(tmp, str);
    return tmp;
}

U_boolList U_BoolList(int head, U_boolList tail)
{
    U_boolList p = checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}