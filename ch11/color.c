#include "color.h"
#include "env.h"
#include "graph.h"
#include "temp.h"
#include "assem.h"
#include "frame.h"
#include "liveness.h"

G_graph ig;
E_map map_degree;
E_map moveList; 
E_map spillCost;
E_map alias;
E_map tmp2node;
AS_instrList worklistMoves;
AS_instrList activeMoves;
G_nodeList spillWorklist;
G_nodeList freezeWorklist;
G_nodeList simplifyWorklist;
G_nodeList selectStack;
G_nodeList coalescedNodes;

void calc_degree(G_graph ig)
{
    G_nodeList nodes = G_nodes(ig), preds  = NULL, succs = NULL;
    long deg;
    while(nodes) {
        if (Live_preColored(nodes->head)) {
            E_enter(map_degree, nodes->head, (void*)999);
        } else {
            deg = 0; 
            preds = nodes->head->preds;
            while (preds) {
                deg++;
                preds = preds->tail; 
            }
            succs = nodes->head->succs;
            while(succs) {
                deg++;
                succs = succs->tail;
            }
            E_enter(map_degree, nodes->head, (void*)deg);
        }
        nodes = nodes->tail;
    }
}
void initData(G_graph ig_in, AS_instrList worklistMoves_in, E_map moveList_in, E_map spillCost_in, E_map tmp2node_in)
{
    ig                  = ig_in;
    map_degree          = E_empty_env();
    alias               = E_empty_env();
    worklistMoves       = worklistMoves_in;
    moveList            = moveList_in;
    spillCost           = spillCost_in;
    tmp2node            = tmp2node_in;
    worklistMoves       = NULL;
    activeMoves         = NULL;
    spillWorklist       = NULL;
    freezeWorklist      = NULL;
    simplifyWorklist    = NULL;
    selectStack         = NULL;
    coalescedNodes      = NULL;
    calc_degree(ig);
}
AS_instrList NodeMoves(G_node n)
{
    return AS_instrIntersect(
        (AS_instrList)E_look(moveList, n), 
        AS_instrUnion(activeMoves, worklistMoves)
    );
}
int MoveRelated(G_node n)
{
    return NodeMoves(n) != NULL;
}
void MakeWorklist()
{
    G_nodeList nodes = G_nodes(ig);
    long degree;
    while(nodes) {
        if (!pre_colored((Temp_temp)nodes->head->info)) {
            degree = (long)E_look(map_degree, nodes->head);
            if (degree >= F_COLORABLE_REGS) {
                spillWorklist = G_union(spillWorklist, G_NodeList(nodes->head, NULL));
            } else if(MoveRelated(nodes->head)) {
                freezeWorklist = G_union(freezeWorklist, G_NodeList(nodes->head, NULL));
            } else {
                simplifyWorklist = G_union(simplifyWorklist, G_NodeList(nodes->head, NULL));
            }
        }
        nodes = nodes->tail;
    }
}
void EnableMoves(G_nodeList nodes)
{
    AS_instrList i_ml;
    while(nodes) {
        i_ml = NodeMoves(nodes->head);
        worklistMoves = AS_instrUnion(worklistMoves, AS_instrIntersect(activeMoves, i_ml));
        activeMoves = AS_instrMinus(activeMoves, i_ml);
        nodes= nodes->tail;
    }
}
G_nodeList Adjacent(G_node n)
{
    return G_minus(G_adj(n), G_union(selectStack, coalescedNodes));
}
void DecrementDegree(G_node n)
{
    long degree = (long)E_look(map_degree, n);
    E_enter(map_degree, n, (void*)(degree - 1));
    if (degree == F_COLORABLE_REGS) {
        EnableMoves(G_union(G_NodeList(n, NULL), Adjacent(n)));
        if (MoveRelated(n)) {
            freezeWorklist = G_union(freezeWorklist, G_NodeList(n, NULL));
        } else {
            simplifyWorklist = G_union(simplifyWorklist, G_NodeList(n, NULL));
        }
    }
}
void Simplify()
{
    G_nodeList adjs;
    G_node n;
    while(simplifyWorklist) {
        n = simplifyWorklist->head;
        simplifyWorklist = simplifyWorklist->tail;
        // push into selected
        selectStack = G_NodeList(n, selectStack);
        adjs = Adjacent(n);
        while (adjs) {
            DecrementDegree(adjs->head);
            adjs = adjs->tail;
        }
    }
}
void AddWorkList(G_node n)
{
    long degree = (long)E_look(map_degree, n);
    if (!Live_preColored(n) && degree < F_COLORABLE_REGS && !MoveRelated(n)) {
        freezeWorklist = G_minus(freezeWorklist, G_NodeList(n, NULL));
        simplifyWorklist = G_union(simplifyWorklist, G_NodeList(n, NULL));
    }
}
int OK(G_node t, G_node r)
{
    long degree = (long)E_look(map_degree, t);
    return (degree < F_COLORABLE_REGS || Live_preColored(t) || G_adjacent(t, r));
}
int Conservative(G_nodeList nodes)
{
    int k = 0;
    long degree;
    while (nodes) {
        degree = (long)E_look(map_degree, nodes->head);
        if (degree >= F_COLORABLE_REGS) {
            k++;
        }
        nodes = nodes->tail;
    }
    return k < F_COLORABLE_REGS;
}
G_node GetAlias(G_node n)
{
    if (G_inList(coalescedNodes, n)) {
        return GetAlias((G_node)E_look(alias, n));
    } else {
        return n;
    }
}
void AddEdge(G_node u, G_node v)
{
    long degree;
    if (!G_adjacent(u,v) && u != v) {
        G_addEdge(u, v);
        if (!Live_preColored(u)) {
            degree = (long)E_look(map_degree, u);
            E_enter(map_degree, u, (void*)(degree + 1));
        } 
        if (!Live_preColored(v)) {
            degree = (long)E_look(map_degree, v);
            E_enter(map_degree, v, (void*)(degree + 1));
        }
    }
}
void Combine(G_node u, G_node v)
{
    AS_instrList i_ml;
    G_nodeList adjs;
    long degree;
    if (G_inList(freezeWorklist, v)) {
        freezeWorklist = G_minus(freezeWorklist, G_NodeList(v, NULL));
    } else {
        spillWorklist = G_minus(spillWorklist, G_NodeList(v, NULL));
    }
    coalescedNodes = G_union(coalescedNodes, G_NodeList(v, NULL));
    E_enter(alias, v, (void*)u);
    i_ml = AS_instrUnion(
        (AS_instrList)E_look(moveList, v),
        (AS_instrList)E_look(moveList, u)
    );
    EnableMoves(G_NodeList(v, NULL));

    adjs = Adjacent(v);
    while(adjs) {
        AddEdge(adjs->head, u);
        DecrementDegree(adjs->head);
        adjs = adjs->tail;
    }
    degree = (long)E_look(map_degree, u);
    if (degree >= F_COLORABLE_REGS && G_inList(freezeWorklist, u)) {
        freezeWorklist = G_minus(freezeWorklist, G_NodeList(u, NULL));
        spillWorklist = G_union(spillWorklist, G_NodeList(u, NULL));
    }
}
void Coalesce()
{
    G_node u, v, tmp;
    AS_instr i_m;
    while(worklistMoves) {
        i_m = worklistMoves->head;
        worklistMoves = worklistMoves->tail;
        u = (G_node)E_look(tmp2node, i_m->u.MOVE.src->head);
        v = (G_node)E_look(tmp2node, i_m->u.MOVE.dst->head);
        u = GetAlias(u);
        v = GetAlias(v);
        if (Live_preColored(v)) {
            tmp = u;
            u = v;
            v = tmp;
        }
    }
}
struct COL_result COL_color(G_graph ig_in, AS_instrList worklistMoves_in, E_map moveList_in, E_map spillCost_in, E_map tmp2node_in)
{
    initData(ig_in, worklistMoves_in, moveList_in, spillCost_in, tmp2node_in);
    
    MakeWorklist();
    while (simplifyWorklist || worklistMoves || freezeWorklist || spillWorklist) {
        if (simplifyWorklist) {
            Simplify();
        } else if (worklistMoves) {

        } else if (freezeWorklist) {

        } else if (spillWorklist) {

        }
    }
}