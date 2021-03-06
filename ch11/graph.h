#pragma once

#include "util.h"

struct G_nodeList_ {
    G_node head;
    G_nodeList tail;
};

struct G_graph_ {
    int nodecount;
    G_nodeList mynodes;
    // Last availiable node, non-null if mynodes is non-null
    G_nodeList mylast;
};

struct G_node_ {
    G_graph mygraph;
    int mykey;
    G_nodeList succs;
    G_nodeList preds;
    void *info;
};

/* Make a new graph */
G_graph G_Graph(void);
/* Make a new node in graph "g", with associated "info" */
G_node G_Node(G_graph g, void *info);

/* Make a NodeList cell */
G_nodeList G_NodeList(G_node head, G_nodeList tail);

/* Get the list of nodes belonging to "g" */
G_nodeList G_nodes(G_graph g);

/* Tell if "a" is in the list "l" */
int G_inNodeList(G_node a, G_nodeList l);

/* Make a new edge joining nodes "from" and "to", which must belong
    to the same graph */
void G_addEdge(G_node from, G_node to);

/* Delete the edge joining "from" and "to" */
void G_rmEdge(G_node from, G_node to);

/* Show all the nodes and edges in the graph, using the function "showInfo"
    to print the name of each node */
void G_show(FILE *out, G_nodeList p, void showInfo(void *));

/* Get all the successors of node "n" */
G_nodeList G_succ(G_node n);

/* Get all the predecessors of node "n" */
G_nodeList G_pred(G_node n);

/* Tell if there is an edge from "from" to "to" */
int G_goesTo(G_node from, G_node n);

/* Tell how many edges lead to or from "n" */
int G_degree(G_node n);

/* Get all the successors and predecessors of "n" */
G_nodeList G_adj(G_node n);

/* Get the "info" associated with node "n" */
void *G_nodeInfo(G_node n);

/* Make a new table */
E_map G_empty(void);

/* Enter the mapping "node"->"value" to the table "t" */
void G_enter(E_map t, G_node node, void *value);

/* Tell what "node" maps to in table "t" */
void *G_look(E_map t, G_node node);

/* Tell if "node" a, b adjacent */
int G_adjacent(G_node a, G_node b);

/* Graph clearing */
void G_clearNode(G_node n);
void G_clearList(G_nodeList nl);
void G_clearGraph(G_graph g);

/* Graph node set operation */
G_nodeList G_union(G_nodeList a, G_nodeList b);
G_nodeList G_intersect(G_nodeList a, G_nodeList b);
G_nodeList G_minus(G_nodeList a, G_nodeList b);
int G_inList(G_nodeList a, G_node b);