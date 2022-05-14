#include "flowgraph.h"
#include "assem.h"
#include "graph.h"
#include "env.h"
#include "temp.h"

Temp_tempList FG_def(G_node n)
{
    AS_instr i = (AS_instr) n->info;
    switch (i->kind)
    {
        case I_OPER:
            return i->u.OPER.dst;
        case I_MOVE:
            return i->u.MOVE.dst;
    }
    return NULL;
}

Temp_tempList FG_use(G_node n)
{
    AS_instr i = (AS_instr) n->info;
    switch (i->kind)
    {
        case I_OPER:
            return i->u.OPER.src;
        case I_MOVE:
            return i->u.MOVE.src;
    }
    return NULL;
}

int FG_isMove(G_node n)
{
    AS_instr i = (AS_instr) n->info;
    return i->kind == I_MOVE;
}

G_graph FG_AssemFlowGraph(AS_instrList il)
{
    G_graph g = G_Graph();
    G_node n = NULL;
    G_node n_last = NULL;
    G_nodeList jmp_list = NULL;
    E_map L_map = E_empty_env();
    AS_instr i = NULL;
    AS_instr i_last = NULL;

    while (il) {
        i = il->head;
        if (i->kind != I_LABEL) {
            n = G_Node(g, i);
            if (i_last && i_last->kind == I_LABEL) {
                E_enter(L_map, i_last->u.LABEL.label, n);
            }
            if (n_last) {
                G_addEdge(n_last, n);
            }
            if (i->kind == I_OPER && i->u.OPER.jumps != NULL) {
                jmp_list = G_NodeList(n, jmp_list);
            }
            n_last = n;
        }   
        i_last = i;
        il = il->tail;
    }
    while (jmp_list) {
        n = jmp_list->head;
        i = (AS_instr) n->info;
        Temp_labelList ll = i->u.OPER.jumps->labels;
        while (ll) {
            G_node n_jmp = (G_node) E_look(L_map, ll->head);
            if (!n_jmp) {
                assert(0);
            }
            G_addEdge(n, n_jmp);
            ll = ll->tail;
        }
        jmp_list = jmp_list->tail;
    }
    return g;
}

