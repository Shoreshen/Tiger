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
    G_nodeList nodes = G_nodes(ig), nl;
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
        i_ml = E_look(moveList, nodes->head);
        worklistMoves = AS_instrUnion(worklistMoves, AS_instrIntersect(activeMoves, i_ml));
        activeMoves = AS_instrMinus(activeMoves, i_ml);
        nodes= nodes->tail;
    }
}
G_nodeList Adjacent(G_node n)
{
    G_nodeList nl = G_adj(n);
    return G_minus(nl, G_union(selectStack, coalescedNodes));
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
    while(simplifyWorklist) {
        selectStack = G_NodeList(simplifyWorklist->head, selectStack);
        adjs = Adjacent(simplifyWorklist->head);
        while (adjs) {
            DecrementDegree(adjs->head);
            adjs = adjs->tail;
        }
        simplifyWorklist = simplifyWorklist->tail;;
    }
}

void AddWorkList(G_node n)
{
    long degree = (long)E_look(map_degree, n);
    if (!Live_preColored(n) && degree < F_COLORABLE_REGS && !MoveRelated(n)) {
        G_nodeList nl = G_NodeList(n, NULL);
        freezeWorklist = G_minus(freezeWorklist, nl);
        simplifyWorklist = G_union(simplifyWorklist, nl);
    }
}
int OK(G_node t, G_node r)
{
    long degree = (long)E_look(map_degree, t);
    return (degree < F_COLORABLE_REGS || Live_preColored(t) || G_adjacent(t, r));
}
void initData(G_graph ig_in, AS_instrList worklistMoves_in, E_map moveList_in, E_map spillCost_in)
{
    ig                  = ig_in;
    map_degree          = E_empty_env();
    worklistMoves       = worklistMoves_in;
    moveList            = moveList_in;
    spillCost           = spillCost_in;;
    worklistMoves       = NULL;
    activeMoves         = NULL;
    spillWorklist       = NULL;
    freezeWorklist      = NULL;
    simplifyWorklist    = NULL;
    selectStack         = NULL;
    coalescedNodes      = NULL;
    calc_degree(ig);
}
struct COL_result COL_color(G_graph ig_in, AS_instrList worklistMoves_in, E_map moveList_in, E_map spillCost_in)
{
    initData(ig_in, worklistMoves_in, moveList_in, spillCost_in);
    
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