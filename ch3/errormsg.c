#include "util.h"
#include "errormsg.h"

typedef struct intList {int i; struct intList *rest;} *IntList;

bool anyErrors          = FALSE;
static char* fileName   = "";
static int lineNum      = 1;
int EM_tokPos           = 0;
static IntList linePos  = NULL;
int charPos             = 1;

void yyerror(char *s) {
  EM_error(EM_tokPos, "%s", s);
}

U_boolList U_BoolList(bool head, U_boolList tail)
{ 
    U_boolList list = checked_malloc(sizeof(struct U_boolList_));
    list->head = head;
    list->tail = tail;
    return list;
}


static IntList intList(int i, IntList rest) 
{
    IntList l   = checked_malloc(sizeof *l);
    l->i        =i; 
    l->rest     =rest;
    return l;
}

void adjust(char* yytext)
{
    EM_tokPos = charPos;
    charPos += yyleng;
}

void EM_newline(void)
{
    lineNum++;
    linePos = intList(EM_tokPos, linePos);
}

void EM_error(int pos, char *message,...)
{
    va_list ap;
    IntList lines   = linePos; 
    int num         = lineNum;
    anyErrors       = TRUE;
    
    while (lines && lines->i >= pos) {
        lines=lines->rest; 
        num--;
    }

    if (fileName) {
        fprintf(stderr,"%s:",fileName);
    }
    
    if (lines) {
        fprintf(stderr,"%d.%d: ", num, pos-lines->i);
    }

    va_start(ap, message);
    vfprintf(stderr, message, ap);
    va_end(ap);
    
    fprintf(stderr,"\n");
}

void EM_reset(char* fname)
{
    anyErrors   = FALSE; 
    fileName    = fname; 
    lineNum     = 1;
    linePos     = intList(0,NULL);
    yyin        = fopen(fname,"r"); // Setting the input parsing file

    if (!yyin) {
        EM_error(0,"cannot open"); exit(1);
    }
}
