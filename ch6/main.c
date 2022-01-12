#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "errormsg.h"
#include "ast.h"
#include "semant.h"
#include "escape.h"

int main(int argc, char **argv) {
    int tok;
    if (argc!=2) {
        fprintf(stderr,"usage: a.out filename\n"); 
        exit(1);
    }

    EM_reset(argv[1]);

    if(yyparse() == 0) {
        // print_exp(stdout, ast_root, 0);
        Esc_findEscape(ast_root);
        SEM_transProg(ast_root);
    } else {
        printf("Parsing failed\n");
    }
    
    return 0;
}