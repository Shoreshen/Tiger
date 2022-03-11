#include "escape.h"
#include "env.h"
#include "symbol.h"
#include "errormsg.h"

typedef struct escapeEntry_* escapeEntry;

struct escapeEntry_ {
    int depth;
    int* escape;
};

void traverseExp(E_stack env, int depth, A_exp e);
void traverseDec(E_stack env, int depth, A_decList d);
void traverseVar(E_stack env, int depth, A_var v);

escapeEntry EscapeEntry(int depth, int* escape) {
    escapeEntry e = checked_malloc(sizeof(*e));
    e->depth = depth;
    e->escape = escape;
    return e;
}

void Esc_findEscape(A_exp exp)
{
    E_stack env = E_empty_env();
    traverseExp(env, 0, exp);
}

void traverseExp(E_stack env, int depth, A_exp e)
{
    switch (e->kind) {
        case A_varExp: {
            traverseVar(env, depth, e->u.var);
            return;
        }
        case A_callExp: {
            A_expList args = e->u.call.args;
            while (args) {
                traverseExp(env, depth, args->head);
                args = args->tail;
            }
            return;
        }
        case A_opExp: {
            traverseExp(env, depth, e->u.op.left);
            traverseExp(env, depth, e->u.op.right);
            return;
        }
        case A_recordExp: {
            A_efieldList fields = e->u.record.fields;
            while (fields) {
                traverseExp(env, depth, fields->head->exp);
                fields = fields->tail;
            }
            return;
        }
        case A_seqExp: {
            A_expList seq = e->u.seq;
            while (seq) {
                traverseExp(env, depth, e->u.seq->head);
                seq = seq->tail;
            }
            return;
        }
        case A_assignExp: {
            traverseExp(env, depth, e->u.assign.exp);
            traverseVar(env, depth, e->u.assign.var);
            return;
        }
        case A_ifExp: {
            traverseExp(env, depth, e->u.iff.test);
            traverseExp(env, depth, e->u.iff.then);
            if (e->u.iff.elsee) {
                traverseExp(env, depth, e->u.iff.elsee);
            }
            return;
        }
        case A_whileExp: {
            traverseExp(env, depth, e->u.whilee.test);
            traverseExp(env, depth, e->u.whilee.body);
            return;
        }
        case A_forExp: {
            traverseExp(env, depth, e->u.forr.lo);
            traverseExp(env, depth, e->u.forr.hi);
            S_beginScope(&env);
            e->u.forr.escape = FALSE;
            S_enter(env, e->u.forr.var, EscapeEntry(depth, &(e->u.forr.escape)));
            traverseExp(env, depth, e->u.forr.body);
            S_endScope(&env, NULL);
            return;
        }
        case A_letExp: {
            A_decList decs = e->u.let.decs;
            S_beginScope(&env);
            traverseDec(env, depth, decs);
            traverseExp(env, depth, e->u.let.body);
            S_endScope(&env, NULL);
            return;
        }
        case A_arrayExp: {
            traverseExp(env, depth, e->u.array.size);
            traverseExp(env, depth, e->u.array.init);
            return;
        }
    }
    return;
}

void traverseDec(E_stack env, int depth, A_decList d)
{
    while (d) {
        switch (d->head->kind) {
            case A_varDec: {
                traverseExp(env, depth, d->head->u.var.init);
                d->head->u.var.escape = FALSE;
                S_enter(env, d->head->u.var.var, EscapeEntry(depth, &(d->head->u.var.escape)));
                break;
            }
            case A_typeDec: {
                break;
            }
            case A_functionDec: {
                A_fundec funcs = d->head->u.function;
                S_beginScope(&env);
                A_fieldList params = funcs->params;
                while (params) {
                    params->head->escape = FALSE;
                    S_enter(env, params->head->name, EscapeEntry(depth + 1, &(params->head->escape)));
                    params = params->tail;
                }
                traverseExp(env, depth + 1, funcs->body);
                S_endScope(&env, NULL);
                break;
            }
        }
        d = d->tail;
    }
}
void traverseVar(E_stack env, int depth, A_var v)
{
    switch (v->kind) {
        case A_simpleVar: {
            escapeEntry entry = S_look(env, v->u.simple);
            if (!entry) {
                EM_error(0, "ESC: Undefined variable: %s", S_name(v->u.simple));
                return;
            }
            if (entry->depth < depth && (!entry->escape == FALSE)) {
                *(entry->escape) = TRUE;
            }
            return;
        }
        case A_fieldVar: {
            traverseVar(env, depth, v->u.field.var);
            return;
        }
        case A_subscriptVar: {
            traverseVar(env, depth, v->u.subscript.var);
            traverseExp(env, depth, v->u.subscript.exp);
        }
    }
}