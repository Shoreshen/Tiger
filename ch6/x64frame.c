#include "frame.h"
#include "temp.h"

#define F_KEEP 6 // keep 6 formal param in registers
#define F_WORD_SIZE 8 // x64 architecture

struct F_access_ {
    enum {
        inFrame, 
        inReg
    } kind;
    union {
        int offset;
        Temp_temp reg;
    } u;
};

F_access InFrame(int offset)
{
    F_access a = checked_malloc(sizeof(*a));
    a->kind = inFrame;
    a->u.offset = offset;
    return a;
}

F_access InReg(Temp_temp reg)
{
    F_access a = checked_malloc(sizeof(*a));
    a->kind = inReg;
    a->u.reg = reg;
    return a;
}

F_accessList F_AccessList(F_access head, F_accessList tail)
{
    F_accessList f = checked_malloc(sizeof(*f));
    f->head = head;
    f->tail = tail;
    return f;
}

F_frame F_newFrame(Temp_label name, U_boolList formals)
{
    F_frame f = checked_malloc(sizeof(*f));
    f->name = name;
    f->formals = NULL;
    f->locals = NULL;
    f->inReg_count = 0;
    f->inFrame_count = 1; // First resarve for return address

    while (formals) {
        if (f->inReg_count < F_KEEP) {
            f->formals = F_AccessList(InReg(Temp_newtemp()), f->formals);
            f->inReg_count++;
        } else {
            f->formals = F_AccessList(InFrame(-(f->inFrame_count * F_WORD_SIZE)), f->formals);
            f->inFrame_count++;
        }
    }

    return f;
}

F_access F_allocLocal(F_frame f, int escape)
{
    F_access a = NULL;
    if (escape) {
        a = InReg(Temp_newtemp());
        f->inReg_count++;
    } else {
        a = InFrame(-(f->inFrame_count - F_KEEP + 1) * F_WORD_SIZE);
        f->inFrame_count++;
    }
    return a;
}

F_accessList F_formals(F_frame f)
{
    return f->formals;
}

Temp_label F_name(F_frame f)
{
    return f->name;
}