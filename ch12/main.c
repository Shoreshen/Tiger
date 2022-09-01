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
#include "regalloc.h"
#include "env.h"

void do_proc(FILE *out, F_frame frame, T_stm body)
{
    T_stmList stm_l;
    AS_instrList iList;
    struct RA_result ra;

    if (DEBUG) {
        pr_stm(stdout, body, 0);
        fprintf(stdout, "\n\n");
    }

    stm_l = C_linearize(body);
    stm_l = C_traceSchedule(C_basicBlocks(stm_l));

    if (DEBUG) {
        printStmList(stdout, stm_l);
        fprintf(stdout, "\n\n");
    }

    iList = F_codegen(frame, stm_l);

    if (DEBUG) {
        AS_printInstrList(stdout, iList, Temp_name());
        fprintf(stdout, "\n\n");
    }
    
    ra = RA_regAlloc(frame, iList);

    if (DEBUG) {
        AS_printInstrList(stdout, iList, Temp_name());
        fprintf(stdout, "\n\n");
    }

    AS_proc iproc = F_procEntryExit(frame, iList);
    fprintf(out, "%s\n", iproc->prolog);
    AS_printInstrList(out, iproc->body, ra.coloring);
    fprintf(out, "%s\n", iproc->epilog);
}

void do_string(FILE *out, char *str, Temp_label L)
{
    fprintf(out, "%s: db ", Temp_labelstring(L));
    int i = 0;
    while (str[i]) {
        fprintf(out, "%d,", str[i]);
        i++;
    }
    fprintf(out, "0\n");
}

int main(int argc, char **argv) {
    F_fragList frags = NULL, frags_head = NULL;
    FILE * out = fopen("prog.asm", "w");
    
    if (argc!=2) {
        fprintf(stderr,"usage: a.out filename\n"); 
        exit(1);
    }
    EM_reset(argv[1]);
    // Parsing
    if (yyparse()) {
        printf("Parsing failed\n");
        exit(1);
    }
    if (DEBUG) {
        print_exp(stdout, ast_root, 0);
        fprintf(stdout, "\n\n");
    }
    // Semantic analysis
    Esc_findEscape(ast_root);
    frags_head = SEM_transProg(ast_root);
    // Lowering & print asm
    fprintf(out, "%s\n", syscalls);
    fprintf(out, "global tigermain\n");
    fprintf(out, "segment .note.GNU-stack\n");
    fprintf(out, "segment .text\n");
    frags = frags_head;
    while (frags) {
        if (frags->head->kind == F_procFrag) {
            do_proc(out, frags->head->u.proc.frame, frags->head->u.proc.body);
        } else if (frags->head->kind != F_stringFrag) {
            assert(0);
        }
        frags = frags->tail;
    }
    fprintf(out, "segment .data\n");
    frags = frags_head;
    while (frags) {
        if (frags->head->kind == F_stringFrag) {
            do_string(out, frags->head->u.stringg.str, frags->head->u.stringg.label);
        } else if (frags->head->kind != F_procFrag) {
            assert(0);
        }
        frags = frags->tail;
    }
    return 0;
}