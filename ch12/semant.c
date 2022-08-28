#include "semant.h"
#include "env.h"
#include "symbol.h"
#include "types.h"
#include "util.h"
#include "errormsg.h"
#include "top_sort.h"
#include "translate.h"

typedef struct expty_ *expty;
struct expty_ {
    Tr_exp exp;
    Ty_ty ty;
};

expty transExp(Tr_level level, E_map venv, E_map tenv, Tr_exp done, A_exp e);
expty transVar(Tr_level level, E_map venv, E_map tenv, Tr_exp done, A_var v);
Tr_exp transDec(Tr_level level, E_map venv, E_map tenv, Tr_exp done, A_decList d);
Ty_ty transTy (E_map tenv, A_ty t);

int inloop = 0;

expty expTy(Tr_exp exp, Ty_ty ty) 
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

expty transVar(Tr_level level, E_map venv, E_map tenv, Tr_exp done, A_var v)
{
    switch (v->kind) {
        case A_simpleVar: {
            E_enventry x = S_look(venv, v->u.simple);
            if (x && x->kind == E_varEntry) {
                return expTy(Tr_simpleVar(x->u.var.access, level), actual_ty(x->u.var.ty));
            } else {
                EM_error(&v->pos, "Var->simple: undefined variable %s", S_name(v->u.simple));
                exit(1);
            }
        }
        case A_fieldVar: {
            expty e = transVar(level, venv, tenv, done, v->u.field.var);
            if (e->ty->kind != Ty_record) {
                EM_error(&v->pos, "Var->field: not a record type");
                exit(1);
            } else {
                Ty_fieldList fields = e->ty->u.record;
                int index = 0;
                while (fields) {
                    if (fields->head->sym == v->u.field.sym) {
                        return expTy(Tr_fieldVar(e->exp, index), actual_ty(fields->head->ty));
                    }
                    fields = fields->tail;
                    index++;
                }
                EM_error(&v->pos, "Var->field: field %s doesn't exist", S_name(v->u.field.sym));
                exit(1);
            }
        }
        case A_subscriptVar: {
            expty var = transVar(level, venv, tenv, done, v->u.subscript.var);
            if (var->ty->kind != Ty_array) {
                EM_error(&v->pos, "Var->sub: not an array type");
                exit(1);
            } else {
                expty index = transExp(level, venv, tenv, done, v->u.subscript.exp);
                if (index->ty->kind != Ty_int) {
                    EM_error(&v->pos, "Var->sub: array index must be an integer");
                    exit(1);
                } else {
                    return expTy(Tr_subscriptVar(var->exp, index->exp), actual_ty(var->ty->u.array));
                }
            }
        }
    }
    assert(0);
}

expty transExp(Tr_level level, E_map venv, E_map tenv, Tr_exp done, A_exp e)
{
    switch (e->kind) {
        case A_varExp: {
            return transVar(level, venv, tenv, done, e->u.var);
        }
        case A_nilExp: {
            return expTy(Tr_nilExp(), Ty_Nil());
        }
        case A_intExp: {
            return expTy(Tr_intExp(e->u.intt), Ty_Int());
        }
        case A_stringExp: {
            return expTy(Tr_stringExp(e->u.string), Ty_String());
        }
        case A_callExp: {
            E_enventry fun = S_look(venv, e->u.call.func);
            if (fun && fun->kind == E_funEntry) {
                Ty_tyList formals = fun->u.fun.formals;
                A_expList args = e->u.call.args;
                Tr_expList tr_args = NULL;
                expty arg = NULL;
                while (formals && args) {
                    arg = transExp(level, venv, tenv, done, args->head);
                    if (!actual_eq(formals->head, arg->ty)) {
                        EM_error(&e->pos, "Expr->call: para type mismatch");
                        exit(1);
                    }
                    tr_args = Tr_ExpList(arg->exp, tr_args);
                    formals = formals->tail;
                    args = args->tail;
                }
                if (formals || args) {
                    EM_error(&e->pos, "Expr->call: para num mismatch");
                    exit(1);
                }
                return expTy(
                    Tr_callExp(fun->u.fun.label, level, fun->u.fun.level, tr_args, e->u.call.func), 
                    actual_ty(fun->u.fun.result)
                );
            } else {
                EM_error(&e->pos, "Expr->call: undefined function %s", S_name(e->u.call.func));
                exit(1);
            }
        }
        case A_opExp: {
            expty left = transExp(level, venv, tenv, done, e->u.op.left);
            expty right = transExp(level, venv, tenv, done, e->u.op.right);
            if (
                e->u.op.oper == A_plusOp || 
                e->u.op.oper == A_minusOp ||
                e->u.op.oper == A_timesOp || 
                e->u.op.oper == A_divideOp
            ) {
                if (left->ty->kind != Ty_int || right->ty->kind != Ty_int) {
                    EM_error(&e->pos, "Expr->op: integer required");
                    exit(1);
                } else {
                    return expTy(Tr_arithExp(e->u.op.oper, left->exp, right->exp), Ty_Int());
                }
            } else if (e->u.op.oper == A_eqOp || e->u.op.oper == A_neqOp) {
                if (!actual_eq(left->ty, right->ty)) {
                    EM_error(&e->pos, "Expr->op: same type required");
                    exit(1);
                } else {
                    if(actual_eq(left->ty, Ty_String())) {
                        return expTy(Tr_stringCmp(e->u.op.oper, left->exp, right->exp), Ty_Int());
                    } else {
                        return expTy(Tr_relExp(e->u.op.oper, left->exp, right->exp), Ty_Int());
                    }
                }
            } else if (
                e->u.op.oper == A_ltOp || 
                e->u.op.oper == A_leOp ||
                e->u.op.oper == A_gtOp || 
                e->u.op.oper == A_geOp
            ) {
                if (left->ty->kind != Ty_int || right->ty->kind != Ty_int) {
                    EM_error(&e->pos, "Expr->op: integer required");
                    exit(1);
                } else {
                    return expTy(Tr_relExp(e->u.op.oper, left->exp, right->exp), Ty_Int());
                }
            } else if (e->u.op.oper == A_andOp || e->u.op.oper == A_orOp) {
                if (left->ty->kind != Ty_int || right->ty->kind != Ty_int) {
                    EM_error(&e->pos, "Expr->op: integer required");
                    exit(1);
                } else {
                    return expTy(Tr_logicExp(e->u.op.oper, left->exp, right->exp), Ty_Int());
                }
            } else {
                EM_error(&e->pos, "Expr->op: not recorgnized operator");
                exit(1);
            }
        }
        case A_recordExp: {
            Ty_ty tmp = S_look(tenv, e->u.record.typ);
            if (!tmp) {
                EM_error(&e->pos, "Expr->record: undefined type %s", S_name(e->u.record.typ));
                exit(1);
            } 
            Ty_ty ty = actual_ty(tmp);
            if (ty->kind == Ty_record) {
                A_efieldList fields = e->u.record.fields;
                Ty_fieldList record = ty->u.record;
                expty field;
                Tr_expList tr_fields = NULL;
                int size = 0;
                while (fields && record) {
                    if (fields->head->name != record->head->sym) {
                        EM_error(&e->pos, "Expr->record: field name mismatch");
                        exit(1);
                    }
                    field = transExp(level, venv, tenv, done, fields->head->exp);
                    if (!actual_eq(record->head->ty, field->ty)) {
                        EM_error(&e->pos, "Expr->record: field type mismatch");
                        exit(1);
                    }
                    tr_fields = Tr_ExpList(field->exp, tr_fields);
                    size++;
                    fields = fields->tail;
                    record = record->tail;
                }
                if (fields || record) {
                    EM_error(&e->pos, "Expr->record: field num mismatch");
                    exit(1);
                }
                return expTy(Tr_recordExp(tr_fields, size), ty);
            } else {
                EM_error(&e->pos, "Expr->record: record expression: <%s> is not a record type", S_name(e->u.record.typ));
                exit(1);
            }
        }
        case A_seqExp: {
            A_expList seq = e->u.seq;
            expty res = expTy(NULL, Ty_Void());
            Tr_expList tr_seq = NULL;
            while (seq) {
                res = transExp(level, venv, tenv, done, seq->head);
                if (res->exp) {
                    tr_seq = Tr_ExpList(res->exp, tr_seq);
                }
                seq = seq->tail;
            }
            if (tr_seq) {
                res->exp = Tr_seqExp(tr_seq);
            }
            return res;
        }
        case A_assignExp: {
            expty var = transVar(level, venv, tenv, done, e->u.assign.var);
            expty exp = transExp(level, venv, tenv, done, e->u.assign.exp);
            if (!actual_eq(var->ty, exp->ty)) {
                EM_error(&e->pos, "Expr->assign: unmatched assign exp");
                exit(1);
            }
            return expTy(Tr_assignExp(var->exp, exp->exp), Ty_Void());
        }
        case A_ifExp: {
            expty test = transExp(level, venv, tenv, done, e->u.iff.test);
            if (test->ty->kind != Ty_int) {
                EM_error(&e->pos, "Expr->assign: test not int");
                exit(1);
            }
            expty then = transExp(level, venv, tenv, done, e->u.iff.then);
            if (e->u.iff.elsee) {
                expty elsee = transExp(level, venv, tenv, done, e->u.iff.elsee);
                if (!actual_eq(then->ty, elsee->ty)) {
                    EM_error(&e->pos, "Expr->assign: then and else exp type mismatch");
                    exit(1);
                }
                return expTy(Tr_ifExp(test->exp, then->exp, elsee->exp), then->ty);
            } else {
                if (then->ty->kind != Ty_void) {
                    EM_error(&e->pos, "Expr->assign: then exp must be void type");
                    exit(1);
                }
                return expTy(Tr_ifExp(test->exp, then->exp, NULL), then->ty);
            }
        }
        case A_whileExp: {
            expty test = transExp(level, venv, tenv, done, e->u.whilee.test);
            if (test->ty->kind != Ty_int) {
                EM_error(&e->pos, "Expr->while: test not int");
                exit(1);
            }
            inloop++;
            Tr_exp new_done = Tr_doneExp();
            expty body = transExp(level, venv, tenv, new_done, e->u.whilee.body);
            inloop--;
            if (body->ty->kind != Ty_void) {
                EM_error(&e->pos, "Expr->while: body must be void type");
                exit(1);
            }
            return expTy(Tr_whileExp(test->exp, body->exp, new_done), Ty_Void());
        }
        case A_forExp: {
            expty lo = transExp(level, venv, tenv, done, e->u.forr.lo);
            expty hi = transExp(level, venv, tenv, done, e->u.forr.hi);
            if (hi->ty->kind != Ty_int) {
                EM_error(&e->pos, "Expr->for: hi exp's range type is not int");
                exit(1);
            }
            if (lo->ty->kind != Ty_int) {
                EM_error(&e->pos, "Expr->for: lo exp's range type is not int");
                exit(1);
            }
            S_beginScope(&venv);
            inloop++;
            Tr_exp new_done = Tr_doneExp();
            Tr_access ac = Tr_allocLocal(level, e->u.forr.escape);
            S_enter(venv, e->u.forr.var, E_VarEntry(ac, Ty_Int()));
            expty body = transExp(level, venv, tenv, new_done, e->u.forr.body);
            inloop--;
            if (body->ty->kind != Ty_void) {
                EM_error(&e->pos, "Expr->for: body must be void type");
                exit(1);
            }
            S_endScope(&venv, NULL);
            return expTy(Tr_forExp(body->exp, ac, lo->exp, hi->exp, new_done), Ty_Void());
        }
        case A_breakExp: {
            if (inloop == 0) {
                EM_error(&e->pos, "Expr->break: break must be in loop");
            }
            return expTy(Tr_breakExp(done), Ty_Void());
        }
        case A_letExp: {
            A_decList decs = e->u.let.decs;
            expty body;
            Tr_expList seqs;
            S_beginScope(&venv);
            S_beginScope(&tenv);
            Tr_exp tr_decs = transDec(level, venv, tenv, done, decs);
            body = transExp(level, venv, tenv, done, e->u.let.body);
            S_endScope(&tenv, NULL);
            S_endScope(&venv, NULL);
            if (tr_decs) {
                Tr_expList seqs = Tr_ExpList(body->exp, Tr_ExpList(tr_decs, NULL));
                return expTy(Tr_seqExp(seqs), body->ty);
            } else {
                return body;
            }
        }
        case A_arrayExp: {
            Ty_ty ty = S_look(tenv, e->u.array.typ);
            if (!ty) {
                EM_error(&e->pos, "Expr->array: undefined type %s", S_name(e->u.array.typ));
                exit(1);
            }
            Ty_ty actual = actual_ty(ty);
            if (actual->kind != Ty_array) {
                EM_error(&e->pos, "Expr->array: array expression: array type required but given another %s", S_name(e->u.array.typ));
                exit(1);
            }
            expty size = transExp(level, venv, tenv, done, e->u.array.size);
            if (size->ty->kind != Ty_int) {
                EM_error(&e->pos, "Expr->array: rray size must be int");
                exit(1);
            }
            expty init = transExp(level, venv, tenv, done, e->u.array.init);
            if (!actual_eq(init->ty, actual->u.array)) {
                EM_error(&e->pos, "Expr->array: array expression: initialize type does not match with given type");
                exit(1);
            }
            return expTy(Tr_arrayExp(init->exp, size->exp), actual);
        }
    }
}

Tr_exp transDec(Tr_level level, E_map venv, E_map tenv, Tr_exp done, A_decList d) 
{
    A_dec dec;
    A_decList decs = d;
    TS_node top_table = NULL;
    Tr_exp var_inits = NULL;
    while (decs) {
        dec = decs->head;
        switch (dec->kind) {
            case A_varDec: {
                break;
            }
            case A_typeDec: {
                // Check redefinition
                if (S_look(tenv, dec->u.type->name)) {
                    EM_error(&dec->pos, "Desc->type: redefined type %s", S_name(dec->u.type->name));
                    exit(1);
                }
                S_enter(tenv, dec->u.type->name, Ty_Name(dec->u.type->name, NULL));
                break;
            }
            case A_functionDec: {
                // Check redefinition
                if (S_look(venv, dec->u.function->name)) {
                    EM_error(&dec->pos, "Desc->func: redefined function %s", S_name(dec->u.function->name));
                    exit(1);
                }
                // Check return type
                Ty_ty result = NULL;
                if (dec->u.function->result) {
                    result = S_look(tenv, dec->u.function->result);
                    if (!result) {
                        EM_error(&dec->pos, "Desc->func: undefined type %s", S_name(dec->u.function->result));
                        exit(1);
                    }
                } else {
                    result = Ty_Void();
                }
                // Check parameters
                Ty_ty param_ty;
                Ty_tyList params_head = NULL;
                Ty_tyList params_tail = NULL;
                U_boolList escapes_head = NULL;
                U_boolList escapes_tail = NULL;
                A_fieldList params = dec->u.function->params;
                while (params) {
                    param_ty = S_look(tenv, params->head->typ);
                    if (!param_ty) {
                        EM_error(&dec->pos, "Desc->func: undefined type %s", S_name(params->head->typ));
                        exit(1);
                    }
                    if (params_head) {
                        params_tail->tail = Ty_TyList(param_ty, NULL);
                        params_tail = params_tail->tail;
                        escapes_tail->tail = U_BoolList(params->head->escape, NULL);
                        escapes_tail = escapes_tail->tail;
                    } else {
                        params_head = Ty_TyList(param_ty, NULL);
                        params_tail = params_head;
                        escapes_head = U_BoolList(params->head->escape, NULL);
                        escapes_tail = escapes_head;
                    }
                    params = params->tail;
                }
                // Enter function declaration
                Tr_level new_level = Tr_newLevel(level, escapes_head, dec->u.function->name);
                S_enter(
                    venv, 
                    dec->u.function->name, 
                    E_FunEntry(new_level, Tr_name(new_level), params_head, result)
                );
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
        if (dec->kind == A_typeDec) {
            Ty_ty ty = S_look(tenv, dec->u.type->name);
            ty->u.name.ty = transTy(tenv, dec->u.type->ty);
            if (ty->u.name.ty->kind == Ty_name) {
                // For declaration as `type a = b`, both name type
                // Add into topology table for loop checking
                TS_Add(&top_table, ty->u.name.ty->u.name.sym, ty->u.name.sym);
            }
        }
        decs = decs->tail;
    }
    // Using topology sort to check loop definition
    if (TS_Sort(&top_table)) {
        EM_error(&d->head->pos, "Desc->type: illegal recursive definition");
        TS_free(&top_table);
        exit(1);
    }
    // Dealing with function, var declaration at last
    // Because type declaration may create loop definition or NULL name type
    // which cause dead loop or crash when calling `actual_eq`
    decs = d;
    while (decs) {
        dec = decs->head;
        switch (dec->kind) {
            case A_varDec: {
                Ty_ty dec_ty = NULL;
                if (dec->u.var.typ) {
                    dec_ty = S_look(tenv, dec->u.var.typ);
                    if (!dec_ty) {
                        EM_error(&dec->pos, "Desc->var: undefined type %s", S_name(dec->u.var.typ));
                        exit(1);
                    }
                }
                expty init = transExp(level, venv, tenv, done, dec->u.var.init);
                if (dec_ty) {
                    if (!actual_eq(init->ty, dec_ty)) {
                        EM_error(&dec->pos, "Desc->var: initialize type does not match with given type");
                        exit(1);
                    }
                } else {
                    if (init->ty->kind == Ty_nil) {
                        EM_error(&dec->pos, "Desc->var: variable declare: illegal nil type: nil must be assign to a explictly record type");
                        exit(1);
                    }
                }
                Tr_access ac = Tr_allocLocal(level, dec->u.var.escape);
                S_enter(venv, dec->u.var.var, E_VarEntry(ac, init->ty));
                if(var_inits) {
                    var_inits = Tr_seqence(var_inits, Tr_assignExp(Tr_simpleVar(ac, level), init->exp));
                } else {
                    var_inits = Tr_assignExp(Tr_simpleVar(ac, level), init->exp);
                }
                break;
            }
            case A_typeDec: {
                break;
            }
            case A_functionDec: {
                E_enventry fun_entry = S_look(venv, dec->u.function->name);
                S_beginScope(&venv);
                // Enter parameters
                Ty_tyList params_tl = fun_entry->u.fun.formals;
                A_fieldList params_fl = dec->u.function->params;
                Tr_accessList ac_list = Tr_formals(fun_entry->u.fun.level)->tail; // First is fp
                while (params_fl) {
                    S_enter(venv, params_fl->head->name, E_VarEntry(ac_list->head, params_tl->head));
                    params_tl = params_tl->tail;
                    params_fl = params_fl->tail;
                    ac_list = ac_list->tail;
                }
                // Check function body type with return type
                expty body = transExp(fun_entry->u.fun.level, venv, tenv, done, dec->u.function->body);
                if (!actual_eq(body->ty, fun_entry->u.fun.result)) {
                    EM_error(&dec->pos, "Desc->func: function body type does not match with return type");
                    exit(1);
                }
                Tr_procEntryExit(
                    fun_entry->u.fun.level, body->exp, 
                    Tr_formals(fun_entry->u.fun.level)
                );
                S_endScope(&venv, NULL);
                break;
            }
        }
        decs = decs->tail;
    }
    return var_inits;
}

Ty_ty transTy (E_map tenv, A_ty t)
{
    switch (t->kind) {
        case A_nameTy: {
            Ty_ty ty = S_look(tenv, t->u.name);
            if (!ty) {
                EM_error(&t->pos, "ty->name: undefined type %s", S_name(t->u.name));
                exit(1);
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
                    EM_error(&t->pos, "ty->record: undefined type %s", S_name(fields->head->typ));
                    exit(1);
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
                EM_error(&t->pos, "ty->array: undefined type %s", S_name(t->u.array));
                exit(1);
            }
            return Ty_Array(ty);
        }
    }
}

F_fragList SEM_transProg(A_exp exp)
{
    // Apply base environment for type
    E_map tenv = E_base_tenv();
    // Apply base environment for value & function
    E_map venv = E_base_venv();
    // Apply sematic analyze
    expty trans_exp = transExp(Tr_outermost(), venv, tenv, NULL, exp);
    // Adding to fraglist
    Tr_procEntryExit(Tr_outermost(), trans_exp->exp, NULL);
    
    return Tr_getResult();
}