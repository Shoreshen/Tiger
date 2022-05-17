#pragma once
#include "util.h"

struct COL_result {
    E_map coloring; 
    Temp_tempList spills;
};

struct COL_result COL_color(G_graph ig, AS_instrList worklistMoves, E_map moveList, E_map spillCost);
