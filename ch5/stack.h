#include "util.h"

#define STACK_SIZE 16

struct STK_stack_ {
    void* data[STACK_SIZE];
    STK_stack next;
    int top;
};

STK_stack STK_push(STK_stack stack, void* value);
void* STK_pop(STK_stack stack);
int STK_check_empty(STK_stack stack);