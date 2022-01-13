#include "types.h"
#include "symbol.h"

struct Ty_ty_ tynil = {
    .kind = Ty_nil,
};
struct Ty_ty_ tyint = {
    .kind = Ty_int,
};
struct Ty_ty_ tystring = {
    .kind = Ty_string,
};
struct Ty_ty_ tyvoid = {
    .kind = Ty_void,
};

Ty_ty Ty_Nil(void)
{
    return &tynil;
}
Ty_ty Ty_Int(void)
{
    return &tyint;
}
Ty_ty Ty_String(void)
{
    return &tystring;
}
Ty_ty Ty_Void(void)
{
    return &tyvoid;
}
Ty_ty Ty_Array(Ty_ty ty)
{
    Ty_ty p = (Ty_ty) checked_malloc(sizeof(*p));
    p->kind = Ty_array;
    p->u.array = ty;
    return p;
}
Ty_ty Ty_Record(Ty_fieldList fields)
{
    Ty_ty p = (Ty_ty) checked_malloc(sizeof(*p));
    p->kind = Ty_record;
    p->u.record = fields;
    return p;
}
Ty_ty Ty_Name(S_symbol sym, Ty_ty ty)
{
    Ty_ty p = (Ty_ty) checked_malloc(sizeof(*p));
    p->kind = Ty_name;
    p->u.name.sym = sym;
    p->u.name.ty = ty;
    return p;
}
Ty_tyList Ty_TyList(Ty_ty head, Ty_tyList tail)
{
    Ty_tyList p = (Ty_tyList) checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}
Ty_field Ty_Field(S_symbol sym, Ty_ty ty)
{
    Ty_field p = (Ty_field) checked_malloc(sizeof(*p));
    p->sym = sym;
    p->ty = ty;
    return p;
}
Ty_fieldList Ty_FieldList(Ty_field head, Ty_fieldList tail)
{
    Ty_fieldList p = (Ty_fieldList) checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}
void Ty_print(Ty_ty t)
{
    switch (t->kind) {
        case Ty_record:
            printf("record");
            break;
        case Ty_array:
            printf("array");
            break;
        case Ty_int:
            printf("int");
            break;
        case Ty_string:
            printf("string");
            break;
        case Ty_nil:
            printf("nil");
            break;
        case Ty_void:
            printf("void");
            break;
        case Ty_name:
            printf("%s", S_name(t->u.name.sym));
            break;
    }
}
void Ty_List_print(Ty_tyList t)
{
    if (t) {
        printf("TyList(");
        Ty_print(t->head);
        printf(", ");
        Ty_List_print(t->tail);
        printf(")");
    } else {
        printf("NULL");
    }
}