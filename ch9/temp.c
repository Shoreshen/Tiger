#include "temp.h"
#include "symbol.h"
#include "env.h"
#include "table.h"

static int temps = 0;
static int labels = 0;

Temp_temp Temp_newtemp()
{
    // Allocate temp struct
    Temp_temp p = checked_malloc(sizeof(*p));
    p->num = temps++;
    Temp_enter(Temp_name(), p, get_heap_str("r%d", p->num));
    return p;
}
Temp_temp Temp_tempstring(char* name)
{
    // Allocate temp struct
    Temp_temp p = checked_malloc(sizeof(*p));
    p->num = temps++;
    Temp_enter(Temp_name(), p, get_heap_str("%s(%d)", name, p->num));
    return p;
}

char* Temp_labelstring(Temp_label s)
{
    return S_name(s);
}

Temp_label Temp_newlabel(void)
{
    return Temp_namedlabel(get_heap_str("L%d", labels++));
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
Temp_tempList Temp_TempLists(Temp_temp temps, ...)
{
    va_list temp;
    Temp_temp arg;
    Temp_tempList head, tail;
    va_start(temp, temps);

    head = Temp_TempList(temps, NULL);
    tail = head;
    while (arg = va_arg(temp, Temp_temp)) {
        tail->tail = Temp_TempList(arg, NULL);
        tail = tail->tail;
    }

    va_end(temp);
    return head;
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