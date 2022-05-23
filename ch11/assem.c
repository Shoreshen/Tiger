/*
 * mipscodegen.c - Functions to translate to Assem-instructions for
 *             the Jouette assembly language using Maximal Munch.
 */

#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "errormsg.h"
#include "env.h"

AS_targets AS_Targets(Temp_labelList labels)
{
    AS_targets p = checked_malloc(sizeof *p);
    p->labels = labels;
    return p;
}

AS_instr AS_Oper(char *a, Temp_tempList d, Temp_tempList s, AS_targets j)
{
    AS_instr p = (AS_instr)checked_malloc(sizeof *p);
    p->kind = I_OPER;
    p->u.OPER.assem = a;
    p->u.OPER.dst = d;
    p->u.OPER.src = s;
    p->u.OPER.jumps = j;
    return p;
}

AS_instr AS_Label(char *a, Temp_label label)
{
    AS_instr p = (AS_instr)checked_malloc(sizeof *p);
    p->kind = I_LABEL;
    p->u.LABEL.assem = a;
    p->u.LABEL.label = label;
    return p;
}

AS_instr AS_Move(char *a, Temp_tempList d, Temp_tempList s)
{
    AS_instr p = (AS_instr)checked_malloc(sizeof *p);
    p->kind = I_MOVE;
    p->u.MOVE.assem = a;
    p->u.MOVE.dst = d;
    p->u.MOVE.src = s;
    return p;
}

AS_instrList AS_InstrList(AS_instr head, AS_instrList tail)
{
    AS_instrList p = (AS_instrList)checked_malloc(sizeof *p);
    p->head = head;
    p->tail = tail;
    return p;
}
AS_instrList AS_InstrLists(AS_instr instrs, ...)
{
    va_list i;
    AS_instr arg;
    AS_instrList head, tail;
    va_start(i, instrs);

    head = AS_InstrList(instrs, NULL);
    tail = head;
    while (arg = va_arg(i, AS_instr)) {
        tail->tail = AS_InstrList(arg, NULL);
        tail = tail->tail;
    }

    va_end(i);
    return head;
}
/* put list b at the end of list a */
AS_instrList AS_splice(AS_instrList a, AS_instrList b)
{
    AS_instrList p = a;
    if (a == NULL) {
        return b;
    }
    while (p->tail != NULL) {
        p = p->tail;
    }
    p->tail = b;
    return a;
}

static Temp_temp nthTemp(Temp_tempList list, int i)
{
    assert(list);
    int k;
    for (k = 0; k < i; k++) {
        list = list->tail;
    }
    return list->head;
}

static Temp_label nthLabel(Temp_labelList list, int i)
{
    assert(list);
    int k;
    for (k = 0; k < i; k++) {
        list = list->tail;
    }
    return list->head;
}

/* first param is char* created by this function by reading 'assem' char*
 * and replacing `d `s and `j stuff.
 * Last param is function to use to determine what to do with each temp.
 */
void format(char *result, char *assem, Temp_tempList dst, Temp_tempList src, AS_targets jumps, E_map m)
{
    char *p;
    int i = 0; /* offset to result char* */
    for (p = assem; p && *p != '\0'; p++)
        if (*p == '`') {
            switch (*(++p)) {
                case 's': {
                    int n = atoi(++p);
                    Temp_temp t = nthTemp(src, n);
                    char *s = NULL;
                    if (pre_colored(nthTemp(src, n))) {
                        s = x64_reg_names[t->num];
                    } else {
                        s = Temp_look(m, nthTemp(src, n));
                    }
                    strcpy(result + i, s);
                    i += strlen(s);
                    break;
                }
                case 'd': {
                    int n = atoi(++p);
                    Temp_temp t = nthTemp(src, n);
                    char *s = NULL;
                    if (pre_colored(nthTemp(src, n))) {
                        s = x64_reg_names[t->num];
                    } else {
                        s = Temp_look(m, nthTemp(src, n));
                    }
                    strcpy(result + i, s);
                    i += strlen(s);
                    break;
                }
                case 'j': {
                    assert(jumps);
                    int n = atoi(++p);
                    char *s = Temp_labelstring(nthLabel(jumps->labels, n));
                    strcpy(result + i, s);
                    i += strlen(s);
                    break;
                }
                case '`':
                    result[i] = '`';
                    i++;
                    break;
                default:
                    assert(0);
            }
        } else{
            result[i] = *p;
            i++;
        }
    result[i] = '\0';
}

void AS_print(FILE *out, AS_instr i, E_map m)
{
    char r[200]; /* result */
    switch (i->kind)
    {
        case I_OPER:
            format(r, i->u.OPER.assem, i->u.OPER.dst, i->u.OPER.src, i->u.OPER.jumps, m);
            fprintf(out, "%s", r);
            break;
        case I_LABEL:
            format(r, i->u.LABEL.assem, NULL, NULL, NULL, m);
            fprintf(out, "%s", r);
            /* i->u.LABEL->label); */
            break;
        case I_MOVE:
            format(r, i->u.MOVE.assem, i->u.MOVE.dst, i->u.MOVE.src, NULL, m);
            fprintf(out, "%s", r);
            break;
    }
}

/* c should be COL_color; temporarily it is not */
void AS_printInstrList(FILE *out, AS_instrList iList, E_map m)
{
    for (; iList; iList = iList->tail) {
        AS_print(out, iList->head, m);
    }
    fprintf(out, "\n");
}

AS_proc AS_Proc(char *p, AS_instrList b, char *e)
{
    AS_proc proc = checked_malloc(sizeof(*proc));
    proc->prolog = p;
    proc->body = b;
    proc->epilog = e;
    return proc;
}

AS_instrList AS_instrUnion(AS_instrList ia, AS_instrList ib)
{
    if (!ia) {
        return ib;
    }
    if (!ib) {
        return ia;
    }

    E_map tmp_map = E_empty_env();
    AS_instrList il = NULL;
    
    while (ia) {
        E_enter(tmp_map, ia->head, "valid");
        il = AS_InstrList(ia->head, il);
        ia = ia->tail;
    }
    
    while (ib) {
        if (!E_look(tmp_map, ib->head)) {
            il =AS_InstrList(ib->head, il);
        }
        ib = ib->tail;
    }
    E_clear(tmp_map);
    return il;
}
AS_instrList AS_instrMinus(AS_instrList ia, AS_instrList ib)
{
    if (!ia || !ib) {
        // ia empty, return empty
        // ib empty, return ia
        return ia;
    }

    E_map tmp_map = E_empty_env();
    AS_instrList il = NULL;
    
    while (ib) {
        E_enter(tmp_map, ib->head, "valid");
        ib = ib->tail;
    }
    
    while (ia) {
        if (!E_look(tmp_map, ia->head)) {
            il = AS_InstrList(ia->head, il);
        }
        ia = ia->tail;
    }
    E_clear(tmp_map);
    return il;
}
AS_instrList AS_instrIntersect(AS_instrList ia, AS_instrList ib)
{
    if (!ia || !ib) {
        return NULL;
    }

    E_map tmp_map = E_empty_env();
    AS_instrList il = NULL;

    while (ib) {
        E_enter(tmp_map, ib->head, "valid");
        ib = ib->tail;
    }

    while (ia) {
        if (E_look(tmp_map, ia->head)) {
            il = AS_InstrList(ia->head, il);
        }
        ia = ia->tail;
    }
    E_clear(tmp_map);
    return il;
}
int AS_instrInList(AS_instr i, AS_instrList il)
{
    if (!il) {
        return 0;
    }
    while (il) {
        if (i == il->head) {
            return TRUE;
        }
        il = il->tail;
    }
    return FALSE;
}
void AS_clearList(AS_instrList il)
{
    if (il == NULL) {
        return;
    }
    if (il->tail) {
        AS_clearList(il->tail);
    }
    free(il);
    il = NULL;
}