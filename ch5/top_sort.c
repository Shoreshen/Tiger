#include "top_sort.h"
#include "stack.h"

TS_node lookup_or_insert(TS_node* table, void* key) 
{
    TS_node node;
    HASH_FIND_PTR(*table, &key, node);
    if (node) {
        return node;
    }
    node = TS_Node(key, NULL, 0);
    HASH_ADD_PTR(*table, key, node);
    return node;
}

TS_node TS_Node(void* key, TS_edge edges, int indegree)
{
    TS_node node;
    node = checked_malloc(sizeof(*node));
    node->key = key;
    node->edges = edges;
    node->indegree = indegree;
    return node;
}

TS_edge TS_Edge(TS_node to, TS_edge next)
{
    TS_edge edge = checked_malloc(sizeof(*edge));
    edge->to = to;
    edge->next = next;
    return edge;
}

void TS_Add(TS_node* table, void* from, void* to)
{
    TS_node from_node = lookup_or_insert(table, from);
    TS_node to_node = lookup_or_insert(table, to);
    from_node->edges = TS_Edge(to_node, from_node->edges);
    to_node->indegree++;
}

void TS_free(TS_node* table)
{
    TS_node node = NULL;
    TS_node tmp = NULL;
    TS_edge edge = NULL;
    HASH_ITER(hh, *table, node, tmp) {
        HASH_DEL(*table, node);
        while (node->edges) {
            edge = node->edges;
            node->edges = edge->next;
            free(edge);
        }
        free(node);
    }
    return;
}

int TS_Sort(TS_node* table)
{
    STK_stack stack = STK_init();
    TS_node table_tmp = NULL;
    TS_node node = NULL;
    TS_edge edge = NULL;
    // Push into the stack all the nodes with indegree 0
    HASH_ITER(hh, *table, node, table_tmp) {
        if (node->indegree == 0) {
            HASH_DEL(*table, node);
            STK_push(&stack, node);
        }
    }
    // While the stack is not empty, pop the top node 
    // Decrease the indegree of the node's edges
    // Push the node into the stack if the indegree is 0
    while(stack->top) {
        node = STK_pop(&stack);
        while (node->edges) {
            edge = node->edges;
            node->edges = edge->next;
            edge->to->indegree--;
            if (edge->to->indegree == 0) {
                HASH_DEL(*table, edge->to);
                STK_push(&stack, edge->to);
            }
            free(edge);
        }
        free(node);
    }
    free(stack);
    return HASH_COUNT(*table);
}