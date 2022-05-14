#pragma once
#include "util.h"

struct Live_graph {
    G_graph graph;
    AS_instrList worklistMoves;
    E_map moveList;
};

Temp_temp Live_gtemp(G_node n);
struct Live_graph Live_liveness(G_graph flow);