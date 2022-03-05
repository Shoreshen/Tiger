#include "temp.h"
#include "symbol.h"

static int temps = 0;
static int labels = 0;

Temp_temp Temp_newtemp(void)
{
    Temp_temp p = checked_malloc(sizeof(*p));
    p->num = temps++;
    return p;
}

char* Temp_labelstring(Temp_label s)
{
    return S_name(s);
}

Temp_label Temp_newlabel(void)
{
    char buf[100];
    char *heap_str;
    sprintf(&buf[0], "L%d", labels++);
    heap_str = checked_malloc(strlen(&buf[0]) + 1);
    strcpy(heap_str, &buf[0]);
    return Temp_namedlabel(heap_str);
}

Temp_label Temp_namedlabel(char* name)
{
    // For runtime, label is the name of function
    // no matter what level/frame currently in
    return S_Symbol(name);
}

Temp_tempList Temp_TempList(Temp_temp h, Temp_tempList t) 
{
    Temp_tempList p = checked_malloc(sizeof (*p));
    p->head=h; 
    p->tail=t;
    return p;
}

Temp_labelList Temp_LabelList(Temp_label h, Temp_labelList t)
{
    Temp_labelList p = checked_malloc(sizeof (*p));
    p->head=h; 
    p->tail=t;
    return p;
}
