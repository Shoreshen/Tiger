#include "util.h"

struct TS_edge_ {
    TS_node to;
    TS_edge next;
};

struct TS_node_ {
    void* key;
    TS_edge edges;
    int indegree;
    UT_hash_handle hh;
};

TS_node lookup_or_insert(TS_node* table, void* key);
TS_node TS_Node(void* key, TS_edge edges, int indegree);
TS_edge TS_Edge(TS_node to, TS_edge next);
void TS_Add(TS_node* table, void* from, void* to);
int TS_Sort(TS_node* table);
void TS_free(TS_node* table);