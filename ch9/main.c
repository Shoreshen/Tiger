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

void do_proc(FILE *out, F_frame frame, T_stm body)
{
    T_stmList stm_l;
    stm_l = C_linearize(body);
    stm_l = C_traceSchedule(C_basicBlocks(stm_l));
    // Printing
    fprintf(out, "proc %s: \n", Temp_labelstring(frame->name));
    printStmList(out, stm_l);
}

void do_str(FILE *out, Temp_label label, char *str)
{
    // Printing
    fprintf(out, ".string %s: \"%s\"\n", Temp_labelstring(label), str);
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
            do_str(stdout, frags->head->u.stringg.label, frags->head->u.stringg.str);
        } else {
            assert(0);
        }
        frags = frags->tail;
    }
    return 0;
}