#include "regalloc.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"
#include "assem.h"
#include "temp.h"

void print_inst(void* info)
{
    AS_instr inst = (AS_instr)info;
    AS_print(stdout, inst, Temp_name());
}

void print_temp(void* info)
{
    E_map m = Temp_name();
    printf("node: %s\n", Temp_look(m, (Temp_temp)info));
}

struct RA_result RA_regAlloc(F_frame f, AS_instrList il)
{
    struct RA_result ra;
    G_graph flow;
    struct Live_graph live;

    while (TRUE) {
        flow = FG_AssemFlowGraph(il);
        // fprintf(stdout, "**********************************\n");
        // G_show(stdout, G_nodes(flow), print_inst);
        live = Live_liveness(flow);
        // fprintf(stdout, "**********************************\n");
        // G_show(stdout, G_nodes(live.graph), print_temp);
        break;
    }
    return ra;
}