#pragma once
#include "util.h"

Temp_tempList FG_def(G_node n);
Temp_tempList FG_use(G_node n);
int FG_isMove(G_node n);
G_graph FG_AssemFlowGraph(AS_instrList il);
