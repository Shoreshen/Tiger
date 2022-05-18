#include "liveness.h"
#include "env.h"
#include "graph.h"
#include "assem.h"
#include "temp.h"
#include "flowgraph.h"
#include "frame.h"

void enterLiveMap(E_map t, G_node flowNode, Temp_tempList s)
{
    G_enter(t, flowNode, s);
}

Temp_tempList lookupLiveMap(E_map t, G_node flownode)
{
    return (Temp_tempList)G_look(t, flownode);
}

G_node findOrCreateNode(E_map m, G_graph g, Temp_temp t)
{
    G_node node = Temp_look(m, t);
    if (node) {
        return node;
    }
    node = G_Node(g, t);
    Temp_enter(m, t, node);
    return node;
}

Temp_temp Live_gtemp(G_node n)
{
    return (Temp_temp)n->info;
}

void calc_liveMap(G_graph flow, E_map in_map, E_map out_map)
{
    int flag = TRUE;
    G_nodeList nodes = NULL;
    G_node flowNode = NULL;
    G_nodeList succs = NULL;
    Temp_tempList ins = NULL, outs = NULL;
    Temp_tempList last_ins = NULL, last_outs = NULL;
    AS_instr instr = NULL;

    while (flag) {
        nodes = G_nodes(flow);
        flag = FALSE;
        while (nodes) {
            flowNode = nodes->head;
            instr = (AS_instr)G_nodeInfo(flowNode);
            ins = (Temp_tempList)G_look(in_map, flowNode);
            outs = (Temp_tempList)G_look(out_map, flowNode);
            last_ins = ins;
            last_outs = outs;

            ins = Temp_union(FG_use(flowNode), Temp_minus(outs, FG_def(flowNode)));
            succs = flowNode->succs;
            while (succs) {
                outs = Temp_union(outs, (Temp_tempList)G_look(in_map, succs->head));
                succs = succs->tail;
            }
            enterLiveMap(in_map, flowNode, ins);
            enterLiveMap(out_map, flowNode, outs);
            if (!Temp_equal(ins, last_ins) || !Temp_equal(outs, last_outs)) {
                flag = TRUE;
            }
            nodes = nodes->tail;
        }
    }
}

struct Live_graph solveLiveness(G_graph flow, E_map in_map, E_map out_map)
{
    G_nodeList nodes = flow->mynodes;
    G_node node = NULL, n_dst = NULL, n_src = NULL, n_out = NULL, n_spill;
    G_node n_move_src = NULL, n_move_dst = NULL;
    Temp_tempList t_src = NULL, t_dst = NULL, t_out = NULL, t_spill;
    AS_instr i_move;
    AS_instrList i_move_list;
    long spill;

    struct Live_graph lg = {
        .graph = G_Graph(),
        .worklistMoves = NULL,
        .moveList = Temp_empty(),
        .spillCost = E_empty_env()
    };
    E_map t_map = Temp_empty();

    while(nodes) {
        node = nodes->head;
        t_src = FG_use(node);
        t_dst = FG_def(node);
        t_out = (Temp_tempList)G_look(out_map, node);
        // calculate spill cost
        t_spill = Temp_union(t_dst, t_src);
        while (t_spill) {
            n_spill = findOrCreateNode(t_map, lg.graph, t_spill->head);
            // will be 0 if node not exists
            spill = (long)E_look(lg.spillCost, n_spill);
            spill++;
            E_enter(lg.spillCost, n_spill, (void*)spill);
            t_spill = t_spill->tail;
        }
        // special dealt with move (mov tmp, tmp)
        if (FG_isMove(node)) {
            if (t_src->tail || t_dst->tail) {
                printf("Error: move instruction has more than one source or destination.\n");
                assert(0);
            }
            i_move = (AS_instr)G_nodeInfo(node);
            n_move_src = findOrCreateNode(t_map, lg.graph, t_src->head);
            n_move_dst = findOrCreateNode(t_map, lg.graph, t_dst->head);
            // handling moveList
            i_move_list = AS_InstrList(i_move, E_look(lg.moveList, n_move_src));
            E_enter(lg.moveList, n_move_src, i_move_list);
            i_move_list = AS_InstrList(i_move, E_look(lg.moveList, n_move_dst));
            E_enter(lg.moveList, n_move_dst, i_move_list);
            // handling worklistMove
            lg.worklistMoves = AS_instrUnion(lg.worklistMoves, AS_InstrList(i_move, NULL));
        }
        // Creating inference graph
        while (t_dst) {
            n_dst = findOrCreateNode(t_map, lg.graph, t_dst->head);
            while (t_out) {
                n_out = findOrCreateNode(t_map, lg.graph, t_out->head);
                if (n_out != n_dst && n_out != n_move_src && !G_adjacent(n_dst, n_out)) {
                    G_addEdge(n_out, n_dst);
                }
                t_out = t_out->tail;
            }
            t_dst = t_dst->tail;
        }
        nodes = nodes->tail;
    }
    E_clear(t_map);
    E_clear(in_map);
    E_clear(out_map);
    return lg;
}

struct Live_graph Live_liveness(G_graph flow)
{
    E_map in_map = G_empty();
    E_map out_map = G_empty();

    calc_liveMap(flow, in_map, out_map);

    return solveLiveness(flow, in_map, out_map);
}

int Live_preColored(G_node n)
{
    return pre_colored((Temp_temp)n->info);
}