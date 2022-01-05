#include "escape.h"
#include "env.h"
#include "symbol.h"

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
            S_endScope(&env);
            return;
        }
        case A_letExp: {
            A_decList decs = e->u.let.decs;
            S_beginScope(&env);
            traverseDec(env, depth, decs);
            traverseExp(env, depth, e->u.let.body);
            S_endScope(&env);
            return;
        }
        case A_arrayExp: {
            traverseExp(env, depth, e->u.array.size);
            traverseExp(env, depth, e->u.array.init);
            return;
        }
    }
}

void traverseDec(E_stack env, int depth, A_decList d)
{

}
void traverseVar(E_stack env, int depth, A_var v)
{
    
}