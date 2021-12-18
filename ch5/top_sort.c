#include "top_sort.h"

TS_node lookup_or_insert(TS_node table, void* key) 
{
    TS_node node;
    HASH_FIND_PTR(table, key, node);
    if (node) {
        return node;
    }
    node = TS_Node(key, NULL, 0);
    HASH_ADD_PTR(table, key, node);
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

void TS_Add(TS_node table, void* from, void* to)
{
    TS_node from_node = lookup_or_insert(table, from);
    TS_node to_node = lookup_or_insert(table, to);
    from_node->edges = TS_Edge(to_node, from_node->edges);
    to_node->indegree++;
}

void TS_free(TS_node table)
{
    TS_node node = NULL;
    TS_node tmp = NULL;
    TS_edge edge = NULL;
    HASH_ITER(hh, table, node, tmp) {
        HASH_DEL(table, node);
        while (node->edges) {
            edge = node->edges;
            node->edges = edge->next;
            free(edge);
        }
        free(node);
    }
    return;
}

int TS_Sort(TS_node table)
{
    int changed = 1;
    TS_node table_tmp = NULL;
    TS_node node = NULL;
    TS_edge edge = NULL;
    while (changed) {
        changed = 0;
        HASH_ITER(hh, table, node, table_tmp) {
            if (node->indegree == 0) {
                edge = node->edges;
                while (node->edges) {
                    edge = node->edges;
                    edge->to->indegree--;
                    node->edges = edge->next;
                    free(edge);
                }
                HASH_DEL(table, node);
                free(node);
                changed = 1;
            }
        }
    }
    return HASH_COUNT(table);
}