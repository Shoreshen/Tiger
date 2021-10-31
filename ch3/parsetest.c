#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "errormsg.h"

int main(int argc, char **argv) {
    int tok;
    if (argc!=2) {
        fprintf(stderr,"usage: a.out filename\n"); 
        exit(1);
    }

    EM_reset(argv[1]);

    // tok = yylex();
    // while (tok) {
    //     printf("%d\n", tok);
    //     tok = yylex();
    // }

    if(yyparse() == 0) {
        printf("Parsing successful!\n");
    } else {
        printf("Parsing failed\n");
    }
    
    return 0;
}