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

Temp_label Temp_newlabel(void)
{
    char buf[100];
    sprintf(&buf[0], "L%d", labels++);
    return Temp_namedlabel(buf);
}

Temp_label Temp_namedlabel(char* name)
{
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
