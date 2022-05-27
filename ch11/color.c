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
E_map coloring;
E_map num_coloring;
AS_instrList worklistMoves;
AS_instrList activeMoves;
AS_instrList coalescedMoves;
AS_instrList constrainedMoves;
AS_instrList frozenMoves;
G_nodeList spillWorklist;
G_nodeList freezeWorklist;
G_nodeList simplifyWorklist;
G_nodeList selectStack;
G_nodeList coalescedNodes;
G_nodeList coloredNodes;
G_nodeList precoloredNodes;
G_nodeList spilledNodes;

void calc_degree(G_graph ig)
{
    G_nodeList nodes = G_nodes(ig), preds  = NULL, succs = NULL;
    long deg;
    while(nodes) {
        if (Live_preColored(nodes->head)) {
            E_enter(map_degree, nodes->head, (void*)999);
            precoloredNodes = G_NodeList(nodes->head, precoloredNodes);
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
    coloring            = E_empty_env();
    num_coloring        = E_empty_env();
    worklistMoves       = worklistMoves_in;
    moveList            = moveList_in;
    spillCost           = spillCost_in;
    tmp2node            = tmp2node_in;
    activeMoves         = NULL;
    coalescedMoves      = NULL;
    constrainedMoves    = NULL;
    frozenMoves         = NULL;
    spillWorklist       = NULL;
    freezeWorklist      = NULL;
    simplifyWorklist    = NULL;
    selectStack         = NULL;
    coalescedNodes      = NULL;
    coloredNodes        = NULL;
    precoloredNodes     = NULL;
    spilledNodes        = NULL;
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
        if (!Live_preColored(nodes->head)) {
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
    if (!Live_preColored(n)) {
        long degree = (long)E_look(map_degree, n);
        E_enter(map_degree, n, (void*)(degree - 1));
        if (degree == F_COLORABLE_REGS) {
            EnableMoves(G_union(G_NodeList(n, NULL), Adjacent(n)));
            spillWorklist = G_minus(spillWorklist, G_NodeList(n, NULL));
            if (MoveRelated(n)) {
                freezeWorklist = G_union(freezeWorklist, G_NodeList(n, NULL));
            } else {
                simplifyWorklist = G_union(simplifyWorklist, G_NodeList(n, NULL));
            }
        }
    }
}
void Simplify()
{
    if (simplifyWorklist == NULL) {
        return;
    }
    G_nodeList adjs;
    G_node n = simplifyWorklist->head;
    simplifyWorklist = G_minus(simplifyWorklist, G_NodeList(n, NULL));
    // push into selected
    selectStack = G_NodeList(n, selectStack);
    adjs = Adjacent(n);
    while (adjs) {
        DecrementDegree(adjs->head);
        adjs = adjs->tail;
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
int Briggs(G_nodeList nodes)
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
int GeorGe(G_node u, G_node v)
{
    G_nodeList nodes = Adjacent(v);
    while (nodes) {
        if (!OK(nodes->head, u)) {
            return FALSE;
        }
        nodes = nodes->tail;
    }
    return TRUE;
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
// node v must not be pre-colored
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
    E_enter(moveList, u, i_ml);
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
    if (worklistMoves == NULL) {
        return;
    }
    G_node u, v, tmp;
    AS_instr i_m = worklistMoves->head;
    worklistMoves = AS_instrMinus(worklistMoves, AS_InstrList(i_m, NULL));
    u = (G_node)E_look(tmp2node, i_m->u.MOVE.src->head);
    v = (G_node)E_look(tmp2node, i_m->u.MOVE.dst->head);
    u = GetAlias(u);
    v = GetAlias(v);
    if (Live_preColored(v)) {
        tmp = u;
        u = v;
        v = tmp;
    }
    if (u == v) {
        coalescedMoves = AS_instrUnion(coalescedMoves, AS_InstrList(i_m, NULL));
        AddWorkList(u);
    } else if (Live_preColored(v) || G_adjacent(u, v)) {
        constrainedMoves = AS_instrUnion(constrainedMoves, AS_InstrList(i_m, NULL));
        AddWorkList(u);
        AddWorkList(v);
    } else if (
        (Live_preColored(u) && GeorGe(u,v)) ||
        (!Live_preColored(u) && Briggs(G_union(Adjacent(u),Adjacent(v))))
    ) {
        coalescedMoves = AS_instrUnion(coalescedMoves, AS_InstrList(i_m, NULL));
        Combine(u, v);
        AddWorkList(u);
    } else {
        activeMoves = AS_instrUnion(activeMoves, AS_InstrList(i_m, NULL));
    }
}
void FreezeMoves(G_node u)
{
    AS_instrList i_ml = NodeMoves(u);
    G_node x = NULL, y = NULL, v = NULL;
    long degree;
    while(i_ml) {
        x = (G_node)E_look(tmp2node, i_ml->head->u.MOVE.src->head);
        y = (G_node)E_look(tmp2node, i_ml->head->u.MOVE.dst->head);
        if (GetAlias(y) == GetAlias(u)) {
            v = GetAlias(x);
        } else {
            v = GetAlias(y);
        }
        // Called by Freeze & SelectSpill, both called when worklistMove is empty
        // Thus do not need to handle worklistMoves
        activeMoves = AS_instrMinus(activeMoves, AS_InstrList(i_ml->head, NULL));
        frozenMoves = AS_instrUnion(frozenMoves, AS_InstrList(i_ml->head, NULL));
        degree = (long)E_look(map_degree, v);
        // degree of pre-colored node is 999
        if (NodeMoves(v) == NULL && degree < F_COLORABLE_REGS) {
            freezeWorklist = G_minus(freezeWorklist, G_NodeList(v, NULL));
            simplifyWorklist = G_union(simplifyWorklist, G_NodeList(v, NULL));
        }
        i_ml = i_ml->tail;
    }
}
void Freeze()
{
    if (freezeWorklist == NULL) {
        return;
    }
    G_node u = freezeWorklist->head;
    freezeWorklist = G_minus(freezeWorklist, G_NodeList(u, NULL));
    simplifyWorklist = G_union(simplifyWorklist, G_NodeList(u, NULL));
    FreezeMoves(u);
}
void SelectSpill()
{
    if (spillWorklist == NULL) {
        return;
    }
    G_nodeList nodes = spillWorklist;
    G_node n_min = NULL;
    double min_priority = 9999.0f, priority;
    long degree, cost;
    while (nodes) {
        degree = (long)E_look(map_degree, nodes->head);
        cost = (long)E_look(spillCost, nodes->head);
        priority = (double)cost / (double)degree;
        if (priority < min_priority) {
            n_min = nodes->head;
            min_priority = priority;
        }
        nodes = nodes->tail;
    }
    spillWorklist = G_minus(spillWorklist, G_NodeList(n_min, NULL));
    simplifyWorklist = G_union(simplifyWorklist, G_NodeList(n_min, NULL));
    FreezeMoves(n_min);
}
char regs[F_COLORABLE_REGS];
long pick_color()
{
    int i;
    for (i = 0; i < F_COLORABLE_REGS; i++) {
        if (regs[i] == 0) {
            return i;
        }
    }
    return -1;
}
void AssignColors()
{
    G_node n = NULL, w = NULL;
    G_nodeList n_adjs = NULL;
    long color;
    while(selectStack) {
        n = selectStack->head;
        selectStack = selectStack->tail;
        n_adjs = G_adj(n);
        memset(&regs[0], 0, sizeof(regs));
        while (n_adjs) {
            w = GetAlias(n_adjs->head);
            if (G_inList(G_union(coloredNodes, precoloredNodes), w)) {
                if (Live_preColored(w)) {
                    color = ((Temp_temp)w->info)->num;
                } else {
                    color = (long)E_look(num_coloring, ((Temp_temp)w->info));
                }
                if (color >= 0 && color < F_COLORABLE_REGS) {
                    regs[color] = 1;
                } else {
                    assert(0);
                }
            }
            n_adjs = n_adjs->tail;
        }
        color = pick_color();
        if (color == -1) {
            spilledNodes = G_union(spilledNodes, G_NodeList(n, NULL));
        } else {
            E_enter(num_coloring, ((Temp_temp)n->info), (void*)color);
            E_enter(coloring, ((Temp_temp)n->info), x64_reg_names[color]);
            coloredNodes = G_union(coloredNodes, G_NodeList(n, NULL));
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
            Coalesce();
        } else if (freezeWorklist) {
            Freeze();
        } else if (spillWorklist) {
            SelectSpill();
        }
    }
    AssignColors();

    // Nodes to temp list
    Temp_tempList spill = NULL;
    while (spilledNodes) {
        spill = Temp_TempList((Temp_temp)spilledNodes->head->info, spill);
        spilledNodes = spilledNodes->tail;
    }

    // Adding coloring mapping for coalesced nodes
    G_node n = NULL;
    char* reg_name;
    while (coalescedNodes) {
        n = GetAlias(coalescedNodes->head);
        if (((Temp_temp)n->info)->num < PRE_COLOR_LIMIT) {
            reg_name = x64_reg_names[((Temp_temp)n->info)->num];
        } else {
            reg_name = (char*)E_look(coloring, n->info);
        }
        E_enter(coloring, coalescedNodes->head->info, reg_name);
        coalescedNodes = coalescedNodes->tail;
    }
    return (struct COL_result){
        .coloring = coloring,
        .spills = spill,
        .alias = alias,
        .coalescedMoves = coalescedMoves
    };
}