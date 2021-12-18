#include "semant.h"
#include "env.h"
#include "symbol.h"
#include "types.h"
#include "util.h"
#include "errormsg.h"
#include "top_sort.h"

typedef struct expty_ *expty;
struct expty_ {
    A_exp exp;
    Ty_ty ty;
};

expty transExp(E_stack venv, E_stack tenv, A_exp e);
expty transVar(E_stack venv, E_stack tenv, A_var v);
void transDec(E_stack venv, E_stack tenv, A_decList d);
Ty_ty transTy (E_stack tenv, A_ty t);

int inloop = 0;

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
                EM_error(&v->pos, "undefined variable %s", S_name(v->u.simple));
                return expTy(NULL, Ty_Int());
            }
        }
        case A_fieldVar: {
            expty e = transVar(venv, tenv, v->u.field.var);
            if (e->ty->kind != Ty_record) {
                EM_error(&v->pos, "not a record type");
                return expTy(NULL, Ty_Int());
            } else {
                Ty_fieldList fields = e->ty->u.record;
                while (fields) {
                    if (fields->head->sym == v->u.field.sym) {
                        return expTy(NULL, fields->head->ty);
                    }
                    fields = fields->tail;
                }
                EM_error(&v->pos, "field %s doesn't exist", S_name(v->u.field.sym));
                return expTy(NULL, Ty_Int());
            }
        }
        case A_subscriptVar: {
            expty var = transVar(venv, tenv, v->u.subscript.var);
            if (var->ty->kind != Ty_array) {
                EM_error(&v->pos, "not an array type");
                return expTy(NULL, Ty_Int());
            } else {
                expty index = transExp(venv, tenv, v->u.subscript.exp);
                if (index->ty->kind != Ty_int) {
                    EM_error(&v->pos, "array index must be an integer");
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
                        EM_error(&e->pos, "para type mismatch");
                        return expTy(NULL, Ty_Int());
                    }
                    formals = formals->tail;
                    args = args->tail;
                }
                if (formals || args) {
                    EM_error(&e->pos, "para num mismatch");
                    return expTy(NULL, Ty_Int());
                }
                return expTy(NULL, actual_ty(fun->u.fun.result));
            } else {
                EM_error(&e->pos, "undefined function %s", S_name(e->u.call.func));
                return expTy(NULL, Ty_Int());
            }
        }
        case A_opExp: {
            expty left = transExp(venv, tenv, e->u.op.left);
            expty right = transExp(venv, tenv, e->u.op.right);
            if (e->u.op.oper == A_plusOp || e->u.op.oper == A_minusOp ||
                e->u.op.oper == A_timesOp || e->u.op.oper == A_divideOp) {
                if (left->ty->kind != Ty_int || right->ty->kind != Ty_int) {
                    EM_error(&e->pos, "integer required");
                    return expTy(NULL, Ty_Int());
                } else {
                    return expTy(NULL, Ty_Int());
                }
            } else if (e->u.op.oper == A_eqOp || e->u.op.oper == A_neqOp) {
                if (!actual_eq(left->ty, right->ty)) {
                    EM_error(&e->pos, "same type required");
                    return expTy(NULL, Ty_Int());
                } else {
                    return expTy(NULL, Ty_Int());
                }
            } else if (e->u.op.oper == A_ltOp || e->u.op.oper == A_leOp ||
                       e->u.op.oper == A_gtOp || e->u.op.oper == A_geOp) {
                if (left->ty->kind != Ty_int || right->ty->kind != Ty_int) {
                    EM_error(&e->pos, "integer required");
                    return expTy(NULL, Ty_Int());
                } else {
                    return expTy(NULL, Ty_Int());
                }
            } else if (e->u.op.oper == A_andOp || e->u.op.oper == A_orOp) {
                if (left->ty->kind != Ty_int || right->ty->kind != Ty_int) {
                    EM_error(&e->pos, "integer required");
                    return expTy(NULL, Ty_Int());
                } else {
                    return expTy(NULL, Ty_Int());
                }
            } else {
                assert(0);
            }
        }
        case A_recordExp: {
            Ty_ty tmp = S_look(tenv, e->u.record.typ);
            if (!tmp) {
                EM_error(&e->pos, "undefined type %s", S_name(e->u.record.typ));
                return expTy(NULL, Ty_Record(NULL));
            } 
            Ty_ty ty = actual_ty(tmp);
            if (ty->kind == Ty_record) {
                A_efieldList fields = e->u.record.fields;
                Ty_fieldList record = ty->u.record;
                while (fields && record) {
                    if (fields->head->name != record->head->sym) {
                        EM_error(&e->pos, "field name mismatch");
                        return expTy(NULL, Ty_Record(NULL));
                    }
                    fields = fields->tail;
                    record = record->tail;
                }
                if (fields || record) {
                    EM_error(&e->pos, "field num mismatch");
                    return expTy(NULL, Ty_Record(NULL));
                }
                return expTy(NULL, ty);
            } else {
                EM_error(&e->pos, "record expression: <%s> is not a record type", S_name(e->u.record.typ));
                return expTy(NULL, Ty_Record(NULL));
            }
        }
        case A_seqExp: {
            A_expList seq = e->u.seq;
            expty res = expTy(NULL, Ty_Void());
            while (seq) {
                res = transExp(venv, tenv, seq->head);
                seq = seq->tail;
            }
            return res;
        }
        case A_assignExp: {
            expty var = transVar(venv, tenv, e->u.assign.var);
            expty exp = transExp(venv, tenv, e->u.assign.exp);
            if (!actual_eq(var->ty, exp->ty)) {
                EM_error(&e->pos, "unmatched assign exp");
            }
            return expTy(NULL, Ty_Void());
        }
        case A_ifExp: {
            expty test = transExp(venv, tenv, e->u.iff.test);
            if (test->ty->kind != Ty_int) {
                EM_error(&e->pos, "test not int");
                return expTy(NULL, Ty_Void());
            }
            expty then = transExp(venv, tenv, e->u.iff.then);
            if (e->u.iff.elsee) {
                expty elsee = transExp(venv, tenv, e->u.iff.elsee);
                if (!actual_eq(then->ty, elsee->ty)) {
                    EM_error(&e->pos, "then and else exp type mismatch");
                }
                return expTy(NULL, then->ty);
            } else {
                if (then->ty->kind != Ty_void) {
                    EM_error(&e->pos, "then exp must be void type");
                }
                return expTy(NULL, then->ty);
            }
        }
        case A_whileExp: {
            expty test = transExp(venv, tenv, e->u.whilee.test);
            if (test->ty->kind != Ty_int) {
                EM_error(&e->pos, "test not int");
                return expTy(NULL, Ty_Void());
            }
            inloop++;
            expty body = transExp(venv, tenv, e->u.whilee.body);
            inloop--;
            if (body->ty->kind != Ty_void) {
                EM_error(&e->pos, "body must be void type");
            }
            return expTy(NULL, Ty_Void());
        }
        case A_forExp: {
            expty lo = transExp(venv, tenv, e->u.forr.lo);
            expty hi = transExp(venv, tenv, e->u.forr.hi);
            if (lo->ty->kind != Ty_int || hi->ty->kind != Ty_int) {
                EM_error(&e->pos, "for exp's range type is not int");
                return expTy(NULL, Ty_Void());
            }
            S_beginScope(venv);
            S_enter(venv, e->u.forr.var, Ty_Int());
            inloop++;
            expty body = transExp(venv, tenv, e->u.forr.body);
            inloop--;
            if (body->ty->kind != Ty_void) {
                EM_error(&e->pos, "body must be void type");
            }
            S_endScope(venv);
            return expTy(NULL, Ty_Void());
        }
        case A_breakExp: {
            if (inloop == 0) {
                EM_error(&e->pos, "break must be in loop");
            }
            return expTy(NULL, Ty_Void());
        }
        case A_letExp: {
            A_decList decs = e->u.let.decs;
            expty body;
            S_beginScope(venv);
            S_beginScope(tenv);
            transDec(venv, tenv, decs);
            body = transExp(venv, tenv, e->u.let.body);
            S_endScope(tenv);
            S_endScope(venv);
            return body;
        }
        case A_arrayExp: {
            Ty_ty ty = S_look(tenv, e->u.array.typ);
            if (!ty) {
                EM_error(&e->pos, "undefined type %s", S_name(e->u.array.typ));
                return expTy(NULL, Ty_Array(Ty_Int()));
            }
            Ty_ty actual = actual_ty(ty);
            if (actual->kind != Ty_array) {
                EM_error(&e->pos, "array expression: array type required but given another %s", S_name(e->u.array.typ));
                return expTy(NULL, Ty_Array(Ty_Int()));
            }
            expty size = transExp(venv, tenv, e->u.array.size);
            if (size->ty->kind != Ty_int) {
                EM_error(&e->pos, "array size must be int");
                return expTy(NULL, Ty_Array(NULL));
            }
            expty init = transExp(venv, tenv, e->u.array.init);
            if (!actual_eq(init->ty, actual->u.array)) {
                EM_error(&e->pos, "array expression: initialize type does not match with given type");
                return expTy(NULL, Ty_Array(NULL));
            }
            return expTy(NULL, Ty_Array(init->ty));
        }
    }
}

void transDec(E_stack venv, E_stack tenv, A_decList d) 
{
    A_dec dec;
    A_decList decs = d;
    TS_node top_table = NULL;
    while (decs) {
        dec = decs->head;
        switch (dec->kind) {
            case A_varDec: {
                Ty_ty dec_ty = NULL;
                if (dec->u.var.typ) {
                    dec_ty = S_look(tenv, dec->u.var.typ);
                    if (!dec_ty) {
                        EM_error(&dec->pos, "undefined type %s", S_name(dec->u.var.typ));
                        break;
                    }
                }
                expty init = transExp(venv, tenv, dec->u.var.init);
                if (dec_ty) {
                    if (!actual_eq(init->ty, dec_ty)) {
                        EM_error(&dec->pos, "initialize type does not match with given type");
                        break;
                    }
                } else {
                    if (init->ty->kind == Ty_nil) {
                        EM_error(&dec->pos, "variable declare: illegal nil type: nil must be assign to a explictly record type");
                        break;
                    }
                }
                S_enter(venv, dec->u.var.var, init->ty);
                break;
            }
            case A_typeDec: {
                // Check redefinition
                if (S_look(tenv, dec->u.type->name)) {
                    EM_error(&dec->pos, "redefined type %s", S_name(dec->u.type->name));
                    break;
                }
                S_enter(tenv, dec->u.type->name, Ty_Name(dec->u.type->name, NULL));
                break;
            }
            case A_functionDec: {
                Ty_ty result;
                Ty_ty param_ty;
                Ty_tyList params_head = NULL;
                Ty_tyList params_tail = NULL;
                // Check redefinition
                if (S_look(venv, dec->u.function->name)) {
                    EM_error(&dec->pos, "redefined function %s", S_name(dec->u.function->name));
                    break;
                }
                // Check return type
                if (dec->u.function->result) {
                    result = S_look(tenv, dec->u.function->result);
                    if (!result) {
                        EM_error(&dec->pos, "undefined type %s", S_name(dec->u.function->result));
                        break;
                    }
                }
                // Check parameters
                A_fieldList params = dec->u.function->params;
                while (params) {
                    param_ty = S_look(tenv, params->head->typ);
                    if (!param_ty) {
                        EM_error(&dec->pos, "undefined type %s", S_name(params->head->typ));
                        break;
                    }
                    if (params_head) {
                        params_tail->tail = Ty_TyList(param_ty, NULL);
                        params_tail = params_tail->tail;
                    } else {
                        params_head = Ty_TyList(param_ty, NULL);
                        params_tail = params_head;
                    }
                }
                // Enter function declaration
                S_enter(venv, dec->u.function->name, E_FunEntry(params_head, result));
                break;
            }
            default:
                assert(0);
        }
        decs = decs->tail;
    }
    decs = d;
    while (decs) {
        dec = decs->head;
        switch (dec->kind) {
            case A_varDec: {
                break;
            }
            case A_typeDec: {
                Ty_ty ty = S_look(tenv, dec->u.type->name);
                ty->u.name.ty = transTy(tenv, dec->u.type->ty);
                if (ty->u.name.ty->kind == Ty_name) {
                    // For declaration as `type a = b`, both name type
                    // Add into topologe table for loop checking
                    TS_Add(top_table, ty->u.name.ty->u.name.sym, ty->u.name.sym);
                }
                break;
            }
            case A_functionDec: {
                
            }
            default:
                assert(0);
        }
        decs = decs->tail;
    }
    if (TS_Sort(top_table)) {
        EM_error(&d->head->pos, "illegal recursive definition");
    }
}

Ty_ty transTy (E_stack tenv, A_ty t)
{
    switch (t->kind) {
        case A_nameTy: {
            Ty_ty ty = S_look(tenv, t->u.name);
            if (!ty) {
                EM_error(&t->pos, "undefined type %s", S_name(t->u.name));
                return Ty_Int();
            }
            return ty;
        }
        case A_recordTy: {
            Ty_fieldList fields_head = NULL;
            Ty_fieldList fields_tail = NULL;
            A_fieldList fields = t->u.record;
            Ty_ty ty;
            while (fields) {
                ty = S_look(tenv, fields->head->typ);
                if (!ty) {
                    EM_error(&t->pos, "undefined type %s", S_name(fields->head->typ));
                    return Ty_Int();
                }
                if (fields_head) {
                    fields_tail->tail = Ty_FieldList(Ty_Field(fields->head->name, ty), NULL);
                    fields_tail = fields_tail->tail;
                } else {
                    fields_head = Ty_FieldList(Ty_Field(fields->head->name, ty), NULL);
                    fields_tail = fields_head;
                }
                fields = fields->tail;
            }
            return Ty_Record(fields_head);
        }
        case A_arrayTy: {
            Ty_ty ty = S_look(tenv, t->u.array);
            if (!ty) {
                EM_error(&t->pos, "undefined type %s", S_name(t->u.array));
                return Ty_Int();
            }
            return Ty_Array(ty);
        }
    }
}

void SEM_transProg(A_exp exp)
{
    // Apply base environment for type
    E_stack tenv = E_base_tenv();
    // Apply base environment for value & function
    E_stack venv = E_base_venv();

    transExp(venv, tenv, exp);
}