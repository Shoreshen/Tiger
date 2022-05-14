#include "regalloc.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"

struct RA_result RA_regAlloc(F_frame f, AS_instrList il)
{
    struct RA_result ra;
    G_graph flow;
    struct Live_graph live;

    while (TRUE) {
        flow = FG_AssemFlowGraph(il);
        live = Live_liveness(flow);
    }
    return ra;
}