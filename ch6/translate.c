#include "translate.h"
#include "frame.h"
#include "temp.h"

struct Tr_level_ {
    Tr_level parent;
    F_frame frame;
    Tr_accessList formals;
};

struct Tr_access_ {
    Tr_level level; 
    F_access access;
};

Tr_level out_most = NULL;

Tr_level Tr_outermost(void)
{
    if (out_most == NULL) {
        out_most = checked_malloc(sizeof(*out_most));
        out_most->parent = NULL;
        out_most->frame = F_newFrame(Temp_newlabel(), NULL);
        out_most->formals = NULL;
    }
    return out_most;
}

Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail)
{
    Tr_accessList list = checked_malloc(sizeof(*list));
    list->head = head;
    list->tail = tail;
    return list;
}

Tr_access Tr_Access(Tr_level level, F_access access)
{
    Tr_access a = checked_malloc(sizeof(*a));
    a->level = level;
    a->access = access;
    return a;
}

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals)
{
    Tr_level level = checked_malloc(sizeof(*level));
    level->parent = parent;
    level->formals = NULL;
    level->frame = F_newFrame(name, U_BoolList(TRUE, formals));

    F_accessList tmp_formals = F_formals(level->frame);
    while (tmp_formals) {
        Tr_access access = Tr_Access(level, tmp_formals->head);
        level->formals = Tr_AccessList(access, level->formals);
        tmp_formals = tmp_formals->tail;
    }
    return level;
}

Tr_access Tr_allocLocal(Tr_level level, int escape)
{
    F_access access = F_allocLocal(level->frame, escape);
    return Tr_Access(level, access);
}

Tr_accessList Tr_formals(Tr_level level)
{
    return level->formals;
}

Temp_label Tr_name(Tr_level level)
{
    return F_name(level->frame);
}