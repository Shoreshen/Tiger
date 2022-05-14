#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "errormsg.h"
#include "ast.h"
#include "semant.h"
#include "escape.h"
#include "frame.h"
#include "canon.h"
#include "tree.h"
#include "temp.h"
#include "assem.h"
#include "codegen.h"

void do_proc(FILE *out, F_frame frame, T_stm body)
{
    T_stmList stm_l;
    AS_instrList iList;
    // Printing
    fprintf(out, "====================================================================\n");
    fprintf(out, "proc %s: \n", Temp_labelstring(frame->name));
    fprintf(out, "**********************************\n");
    pr_stm(out, body, 0);
    fprintf(out, "\n");
    stm_l = C_linearize(body);
    stm_l = C_traceSchedule(C_basicBlocks(stm_l));
    fprintf(out, "**********************************\n");
    fprintf(out, "proc %s: \n", Temp_labelstring(frame->name));

    printStmList(out, stm_l);
    
    fprintf(out, "**********************************\n");
    fprintf(out, "proc instr %s: \n", Temp_labelstring(frame->name));

    iList = F_codegen(frame, stm_l);
    AS_printInstrList(out, iList, Temp_name());

    // fprintf(out, "**********************************\n");
    // fprintf(out, "proc instr %s: \n", Temp_labelstring(frame->name));

    fprintf(out, "====================================================================\n");
}

int main(int argc, char **argv) {
    F_fragList frags = NULL;

    if (argc!=2) {
        fprintf(stderr,"usage: a.out filename\n"); 
        exit(1);
    }
    EM_reset(argv[1]);
    // Parsing
    if (yyparse()) {
        printf("Parsing failed\n");
    }
    // Semantic analysis
    Esc_findEscape(ast_root);
    frags = SEM_transProg(ast_root);
    // Lowering
    while (frags) {
        if (frags->head->kind == F_procFrag) {
            do_proc(stdout, frags->head->u.proc.frame, frags->head->u.proc.body);
        } else if (frags->head->kind == F_stringFrag) {
            fprintf(stdout, ".string %s: \"%s\"\n", 
                Temp_labelstring(frags->head->u.stringg.label), 
                frags->head->u.stringg.str
            );
        } else {
            assert(0);
        }
        frags = frags->tail;
    }
    return 0;
}