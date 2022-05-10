#include "liveness.h"
#include "env.h"
#include "graph.h"
#include "assem.h"
#include "temp.h"
#include "flowgraph.h"

void enterLiveMap(G_table t, G_node flowNode, Temp_tempList s)
{
    G_enter(t, flowNode, s);
}

Temp_tempList lookupLiveMap(G_table t, G_node flownode)
{
    return (Temp_tempList)G_look(t, flownode);
}

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail)
{
    Live_moveList p = checked_malloc(sizeof(*p));
    p->src = src;
    p->dst = dst;
    p->tail = tail;
    return p;
}

G_node findOrCreateNode(Temp_map m, G_graph g, Temp_temp t)
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

void calc_liveMap(G_graph flow, G_table in, G_table out)
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
            ins = (Temp_tempList)G_look(in, flowNode);
            outs = (Temp_tempList)G_look(out, flowNode);
            last_ins = ins;
            last_outs = outs;

            ins = Temp_union(FG_use(flowNode), Temp_minus(ins, FG_def(flowNode)));
            succs = flowNode->succs;
            while (succs) {
                outs = Temp_union(outs, (Temp_tempList)G_look(in, succs->head));
                succs = succs->tail;
            }
            enterLiveMap(in, flowNode, ins);
            enterLiveMap(out, flowNode, outs);
            if (!Temp_equal(ins, last_ins) || !Temp_equal(outs, last_outs)) {
                flag = TRUE;
            }
            nodes = nodes->tail;
        }
    }
}

struct Live_graph solveLiveness(G_graph flow, G_table in, G_table out)
{
    G_nodeList nodes = flow->mynodes;
    G_node node = NULL, n_src = NULL, n_dst = NULL;
    Temp_tempList t_src = NULL, t_dst = NULL;
    struct Live_graph lg = {
        .graph = G_Graph(),
        .moves = NULL
    };
    Temp_map t_map = Temp_empty();

    while(nodes) {
        node = nodes->head;
        if (FG_isMove(node)) {
            t_src = FG_use(node);
            t_dst = FG_def(node);
            if (t_src->tail || t_dst->tail) {
                printf("Error: move instruction has more than one source or destination.\n");
                assert(0);
            }
            n_src = findOrCreateNode(t_map, lg.graph, t_src->head);
            n_dst = findOrCreateNode(t_map, lg.graph, t_dst->head);
            lg.moves = Live_MoveList(n_src, n_dst, lg.moves);
            if (n_src != n_dst && !G_goesTo(n_src, n_dst) && !G_goesTo(n_dst, n_src)) {
                G_addEdge(n_src, n_dst);
            }
        } else {
            t_src = FG_use(node);
            t_dst = FG_def(node);
            while (t_src) {
                n_src = findOrCreateNode(t_map, lg.graph, t_src->head);
                while (t_dst) {
                    n_dst = findOrCreateNode(t_map, lg.graph, t_dst->head);
                    if (n_src != n_dst && !G_goesTo(n_src, n_dst) && !G_goesTo(n_dst, n_src)) {
                        G_addEdge(n_src, n_dst);
                    }
                    t_dst = t_dst->tail;
                }
                t_src = t_src->tail;
            }
        }
        nodes = nodes->tail;
    }
}

struct Live_graph Live_liveness(G_graph flow)
{
    G_table in = G_empty();
    G_table out = G_empty();

    calc_liveMap(flow, in, out);

    return solveLiveness(flow, in, out);
}