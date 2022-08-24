#include "regalloc.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"
#include "assem.h"
#include "temp.h"
#include "color.h"
#include "env.h"

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

void RewriteProgram(F_frame f, AS_instrList il, struct COL_result* col)
{
    Temp_tempList tl = col->spills, t_uses, t_defs;
    E_map tmp_map = E_empty_env();
    F_access ac = NULL;
    AS_instr i, i_tmp;
    AS_instrList il_last = NULL;

    while(tl) {
        ac = F_allocLocal(f, TRUE);
        Temp_enter(tmp_map, tl->head, ac);
        tl = tl->tail;
    }
    while(il) {
        i = il->head;
        t_uses = Temp_intersect(AS_use(i), col->spills);
        t_defs = Temp_intersect(AS_def(i), col->spills);
        while (t_uses) {
            ac = E_look(tmp_map, t_uses->head);
            if (!ac) {
                assert(0);
            }
            i_tmp = AS_Oper(
                get_heap_str("mov `d0, [`s0 + %d]\n", ac->u.offset), 
                Temp_TempList(t_uses->head, NULL), 
                Temp_TempList(F_FP(), NULL), 
                NULL
            );
            AS_InsertAfter(il_last, i_tmp);
            t_uses = t_uses->tail;
        }
        while (t_defs) {
            ac = E_look(tmp_map, t_defs->head);
            if (!ac) {
                assert(0);
            }
            i_tmp = AS_Oper(
                get_heap_str("mov [`s0 + %d], `s1\n", ac->u.offset), 
                NULL,
                Temp_TempLists(F_FP(), t_defs->head, NULL),  
                NULL
            );
            AS_InsertAfter(il, i_tmp);
            il = il->tail;
            t_defs = t_defs->tail;
        }
        il_last = il;
        il = il->tail;
    }
}

struct RA_result RA_regAlloc(F_frame f, AS_instrList il)
{
    struct RA_result ra;
    G_graph flow;
    struct Live_graph lg;
    struct COL_result col;
    Temp_tempList tl;

    while (TRUE) {
        flow = FG_AssemFlowGraph(il);
        // fprintf(stdout, "**********************************\n");
        // G_show(stdout, G_nodes(flow), print_inst);
        lg = Live_liveness(flow);
        // fprintf(stdout, "**********************************\n");
        // G_show(stdout, G_nodes(lg.graph), print_temp);
        // break;
        col = COL_color(lg.graph, lg.worklistMoves, lg.moveList, lg.spillCost, lg.tmp2node);
        if (col.spills == NULL) {
            break;
        } else {
            // fprintf(stdout, "**********************************\n");
            // tl = col.spills;
            // while (tl) {
            //     printf("r%d\n", tl->head->num);
            //     tl = tl->tail;
            // }
            RewriteProgram(f, il, &col);
            // fprintf(stdout, "**********************************\n");
            // AS_printInstrList(stdout, il, Temp_name());
        }
    }

    // Delete coalesced moves
    AS_instrList il_tmp = il;
    while (il_tmp) {
        if (AS_instrInList(il_tmp->head, col.coalescedMoves)) {
            if (il_tmp->head->kind != I_MOVE) {
                assert(0);
            }
            il_tmp->head->u.MOVE.assem = get_heap_str("# %s", il_tmp->head->u.MOVE.assem);
        }
        il_tmp = il_tmp->tail;
    }
    ra.il = il;
    ra.coloring = col.coloring;
    return ra;
}