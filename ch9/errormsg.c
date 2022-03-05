#include "errormsg.h"
#include "tiger.tab.h"

char *yyfilename;

typedef struct intList {int i; struct intList *rest;} *IntList;

bool anyErrors          = FALSE;

void yyerror(char *s) {
  EM_error((A_pos)&yylloc, "%s", s);
}

void EM_error(A_pos pos, char *message,...)
{
    va_list ap;
    anyErrors       = TRUE;

    if (yylloc.filename) {
        fprintf(stderr,"%s:", yylloc.filename);
    }

    fprintf(stderr,"%d.%d: ", yylloc.first_line, yylloc.first_column);

    va_start(ap, message);
    vfprintf(stderr, message, ap);
    va_end(ap);
    
    fprintf(stderr,"\n");
}

void EM_reset(char* fname)
{
    anyErrors   = FALSE; 
    yyfilename  = fname; 
    yyin        = fopen(fname, "r"); // Setting the input parsing file

    if (!yyin) {
        EM_error(0,"cannot open"); exit(1);
    }
}
