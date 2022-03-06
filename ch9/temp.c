#include "temp.h"
#include "symbol.h"
#include "env.h"
#include "table.h"

static int temps = 0;
static int labels = 0;

Temp_temp Temp_newtemp(void)
{
    // Allocate temp struct
    Temp_temp p = checked_malloc(sizeof(*p));
    p->num = temps++;
    // Create string representation of temp & insert into map
    char r[100];
    sprintf(r, "%d", p->num);
    Temp_enter(Temp_name(), p, get_heap_str(&r[0]));
    return p;
}

char* Temp_labelstring(Temp_label s)
{
    return S_name(s);
}

Temp_label Temp_newlabel(void)
{
    char buf[100];
    sprintf(&buf[0], "L%d", labels++);
    return Temp_namedlabel(get_heap_str(&buf[0]));
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

void Temp_layerMap(E_stack* stack)
{
    E_stack_push(stack);
}

E_stack Temp_empty(void)
{
    return E_empty_env();
}

void Temp_enter(E_stack m, Temp_temp t, char* s)
{
    TAB_enter(&m->table, t, s);
}

char* Temp_look(E_stack m, Temp_temp t)
{
    return E_look(m, t);
}

static E_stack m = NULL;
E_stack Temp_name()
{
    if (!m) {
        m = Temp_empty();
    }
    return m;
}