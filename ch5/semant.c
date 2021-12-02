#include "semant.h"
#include "env.h"
#include "symbol.h"
#include "types.h"
#include "util.h"

typedef struct expty_ *expty;

struct expty_ {
    A_exp exp;
    Ty_ty ty;
};

expty expTy(A_exp exp, Ty_ty ty) 
{
    expty et = checked_malloc(sizeof(*et));
    et->exp = exp;
    et->ty = ty;
    return et;
}

Ty_ty actual_ty(Ty_ty ty) 
{
    Ty_ty t = ty;
    while (t->kind == Ty_name) {
        t = t->u.name.ty;
    }
    return t;
}

int actual_eq(Ty_ty source, Ty_ty target) 
{
    Ty_ty t1 = actual_ty(source);
    Ty_ty t2 = actual_ty(target);
    return ((t1->kind == Ty_record || t1->kind == Ty_array) && t1 == t2) ||
           (t1->kind == Ty_record && t2->kind == Ty_nil) ||
           (t1->kind == Ty_nil && t2->kind == Ty_record) ||
           (t1->kind != Ty_record && t1->kind != Ty_array && t1->kind == t2->kind);
}

expty transVar(E_stack venv, E_stack tenv, A_var v)
{
    switch (v->kind) {
        case A_simpleVar: {
            E_enventry x = S_look(venv, v->u.simple);
            if (x && x->kind == E_varEntry) {
                return expTy(NULL, x->u.var.ty);
            } else {
                EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
                return expTy(NULL, Ty_Int());
            }
        }
        case A_fieldVar: {
            expty e = transVar(venv, tenv, v->u.field.var);
            if (e->ty->kind != Ty_record) {
                EM_error(v->pos, "not a record type");
                return expTy(NULL, Ty_Int());
            } else {
                Ty_fieldList fields = e->ty->u.record;
                while (fields) {
                    if (fields->head->sym == v->u.field.sym) {
                        return expTy(NULL, fields->head->ty);
                    }
                    fields = fields->tail;
                }
                EM_error(v->pos, "field %s doesn't exist", S_name(v->u.field.sym));
                return expTy(NULL, Ty_Int());
            }
        }
        case A_subscriptVar: {
            expty var = transVar(venv, tenv, v->u.subscript.var);
            if (var->ty->kind != Ty_array) {
                EM_error(v->pos, "not an array type");
                return expTy(NULL, Ty_Int());
            } else {
                expty index = transExp(venv, tenv, v->u.subscript.exp);
                if (index->ty->kind != Ty_int) {
                    EM_error(v->pos, "array index must be an integer");
                    return expTy(NULL, Ty_Int());
                } else {
                    return expTy(NULL, actual_ty(var->ty->u.array));
                }
            }
        }
    }
    assert(0);
}

expty transExp(E_stack venv, E_stack tenv, A_exp e)
{
    switch (e->kind) {
        case A_varExp: {
            return transVar(venv, tenv, e->u.var);
        }
        case A_nilExp: {
            return expTy(NULL, Ty_Nil());
        }
        case A_intExp: {
            return expTy(NULL, Ty_Int());
        }
        case A_stringExp: {
            return expTy(NULL, Ty_String());
        }
        case A_callExp: {
            E_enventry fun = S_look(venv, e->u.call.func);
            if (fun && fun->kind == E_funEntry) {
                Ty_tyList formals = fun->u.fun.formals;
                A_expList args = e->u.call.args;
                while (formals && args) {
                    expty arg = transExp(venv, tenv, args->head);
                    if (!actual_eq(formals->head, arg->ty)) {
                        EM_error(e->pos, "para type mismatch");
                        return expTy(NULL, Ty_Int());
                    }
                    formals = formals->tail;
                    args = args->tail;
                }
                if (formals || args) {
                    EM_error(e->pos, "para num mismatch");
                    return expTy(NULL, Ty_Int());
                }
                return expTy(NULL, fun->u.fun.result);
            } else {
                EM_error(e->pos, "undefined function %s", S_name(e->u.call.func));
                return expTy(NULL, Ty_Int());
            }
        }
    }
}

void SEM_transProg(A_exp exp)
{
    E_stack tenv = E_base_tenv();
    E_stack venv = E_base_venv();

    transExp(venv, tenv, exp);
}