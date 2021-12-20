#include "stack.h"

STK_stack STK_push(STK_stack stack, void* value)
{
    STK_stack new_stack;
    if (stack == NULL || stack->top == STACK_SIZE) {
        new_stack = checked_malloc(sizeof(*new_stack));
        memset(new_stack, 0, sizeof(*new_stack));
        new_stack->next = stack;
        stack = new_stack;
    }
    stack->data[stack->top] = value;
    stack->top++;
    return stack;
}

void* STK_pop(STK_stack stack)
{
    STK_stack tmp;
    if (stack == NULL) {
        return NULL;
    } else if (stack->top == 0) {
        tmp = stack;
        stack = stack->next;
        free(tmp);
        if (stack == NULL) {
            return NULL;
        }
    }
    stack->top--;
    return stack->data[stack->top];
}

void* STK_head(STK_stack stack)
{
    return stack->data[stack->top - 1];
}

int STK_check_empty(STK_stack stack)
{
    if (stack == NULL) {
        return 1;
    }
    return 0;
}