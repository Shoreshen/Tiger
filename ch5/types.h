#pragma once
#include "symbol.h"

typedef struct Ty_ty_ *Ty_ty;
typedef struct Ty_field_ *Ty_field;
typedef struct Ty_tyList_ *Ty_tyList;
typedef struct Ty_fieldList_ *Ty_fieldList;

struct Ty_tyList_ {
    Ty_ty head;
    Ty_tyList tail;
};
struct Ty_field_ {
    S_symbol sym;
    Ty_ty ty;
};
struct Ty_fieldList_ {
    Ty_field head;
    Ty_fieldList tail;
};
struct Ty_ty_ {
    enum {
        Ty_record,
        Ty_nil,
        Ty_int,
        Ty_string,
        Ty_array,
        Ty_name,
        Ty_void
    } kind;
    union {
        Ty_fieldList record;
        Ty_ty array;
        struct Ty_field_ name;
    } u;
};

Ty_ty Ty_Nil(void);
Ty_ty Ty_Int(void);
Ty_ty Ty_String(void);
Ty_ty Ty_Void(void);
Ty_ty Ty_Array(Ty_ty ty);
Ty_ty Ty_Record(Ty_fieldList fields);
Ty_ty Ty_Name(S_symbol sym, Ty_ty ty);

Ty_tyList Ty_TyList(Ty_ty head, Ty_tyList tail);
Ty_field Ty_Field(S_symbol sym, Ty_ty ty);
Ty_fieldList Ty_FieldList(Ty_field head, Ty_fieldList tail);

void Ty_print(Ty_ty t);
void Ty_List_print(Ty_tyList t);