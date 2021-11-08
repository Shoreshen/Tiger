#include "util.h"
#include "errormsg.h"
#include "tiger.tab.h"

typedef struct intList {int i; struct intList *rest;} *IntList;

bool anyErrors          = FALSE;
static int lineNum      = 1;
int EM_tokPos           = 0;
static IntList linePos  = NULL;
int charPos             = 1;

void yyerror(char *s) {
  EM_error("%s", s);
}

void EM_error(char *message,...)
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
