#include "color.h"
#include "env.h"
#include "graph.h"
#include "temp.h"
E_map map_degree = NULL;

void calc_degree(G_graph ig)
{
    G_nodeList nodes = G_nodes(ig), preds  = NULL, succs = NULL;
    long deg;
    while(nodes) {
        if (((Temp_temp)nodes->head->info)->num < PRE_COLOR_LIMIT) {
            E_enter(map_degree, nodes->head, (void*)999);
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

struct COL_result COL_color(G_graph ig, AS_instrList worklistMoves, E_map moveList, E_map spillCost)
{
    map_degree = E_empty_env();
    calc_degree(ig);

    E_clear(map_degree);
}