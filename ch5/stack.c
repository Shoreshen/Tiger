#include "stack.h"

void STK_push(STK_stack *stack, void* value)
{
    STK_stack new_stack;
    if ((*stack)->top == STACK_SIZE) {
        new_stack = STK_init();
        new_stack->next = (*stack);
        (*stack) = new_stack;
    }
    (*stack)->data[(*stack)->top] = value;
    (*stack)->top++;
}

STK_stack STK_init()
{
    STK_stack s = checked_malloc(sizeof(*s));
    memset(s, 0, sizeof(*s));
    return s;
}

void* STK_pop(STK_stack *stack)
{
    STK_stack tmp;
    void* ret = NULL;

    if ((*stack) == NULL) {
        return NULL;
    } 
    
    (*stack)->top--;
    // If the stack is empty and next is not NULL, pop the top of the next stack
    if ((*stack)->top == 0 && (*stack)->next != NULL) {
        tmp = (*stack)->next;
        free(*stack);
        (*stack) = tmp;
    }
    
    ret = (*stack)->data[(*stack)->top];
    
    return ret;
}

void* STK_head(STK_stack stack)
{
    return stack->data[stack->top - 1];
}