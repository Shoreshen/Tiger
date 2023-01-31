#include "temp.h"
#include "symbol.h"
#include "env.h"
#include "table.h"

static int temps = PRE_COLOR_LIMIT;
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
    Temp_enter(Temp_name(), p, get_heap_str("%s::t%d", name, p->num));
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

void Temp_layerMap(E_map* stack)
{
    E_map_push(stack);
}

E_map Temp_empty(void)
{
    return (E_map)E_empty_env();
}

void Temp_enter(E_map m, Temp_temp t, void* s)
{
    TAB_enter(&m->table, t, s);
}

void* Temp_look(E_map m, Temp_temp t)
{
    return E_look(m, t);
}

static E_map m = NULL;
E_map Temp_name()
{
    if (!m) {
        m = Temp_empty();
    }
    return m;
}

Temp_tempList Temp_union(Temp_tempList a, Temp_tempList b)
{
    if (!a) {
        return b;
    }
    if (!b) {
        return a;
    }

    E_map tmp_map = E_empty_env();
    Temp_tempList tl = NULL;
    
    while (a) {
        Temp_enter(tmp_map, a->head, "valid");
        tl = Temp_TempList(a->head, tl);
        a = a->tail;
    }
    
    while (b) {
        if (!Temp_look(tmp_map, b->head)) {
            tl = Temp_TempList(b->head, tl);
        }
        b = b->tail;
    }
    E_clear(tmp_map);
    return tl;
}

Temp_tempList Temp_intersect(Temp_tempList a, Temp_tempList b)
{
    if (!a || !b) {
        return NULL;
    }

    E_map tmp_map = E_empty_env();
    Temp_tempList tl = NULL;
    
    while (a) {
        Temp_enter(tmp_map, a->head, "valid");
        a = a->tail;
    }
    
    while (b) {
        if (Temp_look(tmp_map, b->head)) {
            tl = Temp_TempList(b->head, tl);
        }
        b = b->tail;
    }
    E_clear(tmp_map);
    return tl;
}

Temp_tempList Temp_minus(Temp_tempList a, Temp_tempList b)
{
    if (!a || !b) {
        // a is NULL, return null
        // b is NULL, return a
        return a;
    }

    E_map tmp_map = E_empty_env();
    Temp_tempList tl = NULL;
    
    while (b) {
        Temp_enter(tmp_map, b->head, "valid");
        b = b->tail;
    }
    
    while (a) {
        if (!Temp_look(tmp_map, a->head)) {
            tl = Temp_TempList(a->head, tl);
        }
        a = a->tail;
    }
    E_clear(tmp_map);
    return tl;
}

int Temp_equal(Temp_tempList a, Temp_tempList b)
{
    if (!a && !b) {
        return TRUE;
    }
    if (!a || !b) {
        return FALSE;
    }
    int count_a = 0, count_b = 0;
    E_map tmp_map = E_empty_env();
    Temp_tempList tl;
    
    while (a) {
        Temp_enter(tmp_map, a->head, "valid");
        count_a++;
        a = a->tail;
    }
    
    while (b) {
        if (!Temp_look(tmp_map, b->head)) {
            return FALSE;
        }
        count_b++;
        b = b->tail;
    }
    E_clear(tmp_map);
    return (count_a == count_b);
}
void Temp_clearList(Temp_tempList tl)
{
    if (tl == NULL) {
        return;
    }
    if (tl->tail) {
        Temp_clearList(tl->tail);
    }
    free(tl);
    tl = NULL;
}