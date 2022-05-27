#pragma once
#include "util.h"

struct COL_result {
    E_map coloring; 
    Temp_tempList spills;
    E_map alias;
    AS_instrList coalescedMoves;
};

struct COL_result COL_color(G_graph ig_in, AS_instrList worklistMoves_in, E_map moveList_in, E_map spillCost_in, E_map tmp2node_in);
