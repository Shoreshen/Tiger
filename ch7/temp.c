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