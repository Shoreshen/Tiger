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

    while (flag)
    {
        nodes = G_nodes(flow);
        flag = FALSE;
        while (nodes)
        {
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
            if (!Temp_equal(ins, last_ins) || !Temp_equal(outs, last_outs)) {
                flag = TRUE;
            }
            nodes = nodes->tail;
        }
    }
}

struct Live_graph solveLiveness(G_graph flow, G_table in, G_table out)
{
    
}

struct Live_graph Live_liveness(G_graph flow)
{
    G_table in = G_empty();
    G_table out = G_empty();

    calc_liveMap(flow, in, out);

    return solveLiveness(flow, in, out);
}