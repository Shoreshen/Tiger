#include "ast.h"
#include "util.h"

#pragma region AST-constructor
A_var A_SimpleVar(A_pos pos, S_symbol sym)
{
    A_var p = checked_malloc(sizeof(*p));
    p->kind = A_simpleVar;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.simple = sym;
    return p;
}
A_var A_FieldVar(A_pos pos, A_var var, S_symbol sym)
{
    A_var p = checked_malloc(sizeof(*p));
    p->kind = A_fieldVar;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.field.var = var;
    p->u.field.sym = sym;
    return p;
}
A_var A_SubscriptVar(A_pos pos, A_var var, A_exp exp)
{
    A_var p = checked_malloc(sizeof(*p));
    p->kind = A_subscriptVar;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.subscript.var = var;
    p->u.subscript.exp = exp;
    return p;
}
A_exp A_VarExp(A_pos pos, A_var var)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_varExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.var = var;
    return p;
}
A_exp A_NilExp(A_pos pos)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_nilExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    return p;
}
A_exp A_IntExp(A_pos pos, int i)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_intExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.intt = i;
    return p;
}
A_exp A_StringExp(A_pos pos, char* s)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_stringExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.string = s;
    return p;
}
A_exp A_CallExp(A_pos pos, S_symbol func, A_expList args)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_callExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.call.func = func;
    p->u.call.args = args;
    return p;
}
A_exp A_OpExp(A_pos pos, A_oper oper, A_exp left, A_exp right)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_opExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.op.oper = oper;
    p->u.op.left = left;
    p->u.op.right = right;
    return p;
}
A_exp A_RecordExp(A_pos pos, S_symbol typ, A_efieldList fields)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_recordExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.record.typ = typ;
    p->u.record.fields = fields;
    return p;
}
A_exp A_SeqExp(A_pos pos, A_expList seq)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_seqExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.seq = seq;
    return p;
}
A_exp A_AssignExp(A_pos pos, A_var var, A_exp exp)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_assignExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.assign.var = var;
    p->u.assign.exp = exp;
    return p;
}
A_exp A_IfExp(A_pos pos, A_exp test, A_exp then, A_exp elsee)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_ifExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.iff.test = test;
    p->u.iff.then = then;
    p->u.iff.elsee = elsee;
    return p;
}
A_exp A_WhileExp(A_pos pos, A_exp test, A_exp body)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_whileExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.whilee.test = test;
    p->u.whilee.body = body;
    return p;
}
A_exp A_ForExp(A_pos pos, S_symbol var, A_exp lo, A_exp hi, A_exp body)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_forExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.forr.var = var;
    p->u.forr.lo = lo;
    p->u.forr.hi = hi;
    p->u.forr.body = body;
    return p;
}
A_exp A_BreakExp(A_pos pos)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_breakExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    return p;
}
A_exp A_LetExp(A_pos pos, A_decList decs, A_exp body)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_letExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.let.decs = decs;
    p->u.let.body = body;
    return p;
}
A_exp A_ArrayExp(A_pos pos, S_symbol typ, A_exp size, A_exp init)
{
    A_exp p = checked_malloc(sizeof(*p));
    p->kind = A_arrayExp;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.array.typ = typ;
    p->u.array.size = size;
    p->u.array.init = init;
    return p;
}
A_dec A_FunctionDec(A_pos pos, A_fundec function)
{
    A_dec p = checked_malloc(sizeof(*p));
    p->kind = A_functionDec;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.function = function;
    return p;
}
A_dec A_VarDec(A_pos pos, S_symbol var, S_symbol typ, A_exp init)
{
    A_dec p = checked_malloc(sizeof(*p));
    p->kind = A_varDec;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.var.var = var;
    p->u.var.typ = typ;
    p->u.var.init = init;
    return p;
}
A_dec A_TypeDec(A_pos pos, A_namety type)
{
    A_dec p = checked_malloc(sizeof(*p));
    p->kind = A_typeDec;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.type = type;
    return p;
}
A_ty A_NameTy(A_pos pos, S_symbol name)
{
    A_ty p = checked_malloc(sizeof(*p));
    p->kind = A_nameTy;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.name = name;
    return p;
}
A_ty A_RecordTy(A_pos pos, A_fieldList record)
{
    A_ty p = checked_malloc(sizeof(*p));
    p->kind = A_recordTy;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.record = record;
    return p;
}
A_ty A_ArrayTy(A_pos pos, S_symbol array)
{
    A_ty p = checked_malloc(sizeof(*p));
    p->kind = A_arrayTy;
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->u.array = array;
    return p;
}
A_field A_Field(A_pos pos, S_symbol name, S_symbol typ)
{
    A_field p = checked_malloc(sizeof(*p));
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->name = name;
    p->typ = typ;
    return p;
}
A_fieldList A_FieldList(A_field head, A_fieldList tail)
{
    A_fieldList p = checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}
A_expList A_ExpList(A_exp head, A_expList tail)
{
    A_expList p = checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}
A_fundec A_Fundec(A_pos pos, S_symbol name, A_fieldList params, S_symbol result, A_exp body)
{
    A_fundec p = checked_malloc(sizeof(*p));
    memcpy(&p->pos, pos, sizeof(struct A_pos_));
    p->name = name;
    p->params = params;
    p->result = result;
    p->body = body;
    return p;
}
A_decList A_DecList(A_dec head, A_decList tail)
{
    A_decList p = checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}
A_namety A_Namety(S_symbol name, A_ty ty)
{
    A_namety p = checked_malloc(sizeof(*p));
    p->name = name;
    p->ty = ty;
    return p;
}
A_efield A_Efield(S_symbol name, A_exp exp)
{
    A_efield p = checked_malloc(sizeof(*p));
    p->name = name;
    p->exp = exp;
    return p;
}
A_efieldList A_EfieldList(A_efield head, A_efieldList tail)
{
    A_efieldList p = checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}
#pragma endregion

#pragma region AST print
void indent(FILE *out, int d)
{
    int i;
    for (i = 0; i < d; i++) {
        fprintf(out, "\t");
    }
}
void new_line(FILE *out, int d, char *pre_s, char *post_s)
{
    fprintf(out, "%s\n", pre_s);
    indent(out, d);
    fprintf(out, "%s", post_s);
}
void print_var(FILE *out, A_var var ,int d) {
    indent(out, d);
    switch (var->kind) {
        case A_simpleVar:
            fprintf(out, "simpleVar(%s)\n", var->u.simple->id);
            break;
        case A_fieldVar:
            fprintf(out, "fieldVar(\n");
            print_var(out, var->u.field.var, d + 1);
            new_line(out, d + 1, ",", "");
            fprintf(out, "%s", var->u.field.sym->id);
            new_line(out, d, "", ")");
            break;
        case A_subscriptVar:
            fprintf(out, "subscriptVar(\n");
            print_var(out, var->u.subscript.var, d + 1);
            fprintf(out, ",\n");
            print_exp(out, var->u.subscript.exp, d + 1);
            new_line(out, d, "", ")");
            break;
        default:
            assert(0);
    }
}
void print_exp_list(FILE *out, A_expList list, int d) {
    indent(out, d);
    if (list) {
        fprintf(out, "expList(\n");
        print_exp(out, list->head, d + 1);
        if (list->tail) {
            fprintf(out, ",\n");
            print_exp_list(out, list->tail, d + 1);
        }
        new_line(out, d, "", ")");
    } else {
        fprintf(out, "expList()");
    }
}
void print_exp(FILE *out, A_exp exp ,int d)
{
    indent(out, d);
    switch (exp->kind) {
        case A_varExp:
            fprintf(out, "varExp(\n");
            print_var(out, exp->u.var, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_nilExp:
            fprintf(out, "nilExp()");
            break;
        case A_intExp:
            fprintf(out, "intExp(%d)", exp->u.intt);
            break;
        case A_stringExp:
            fprintf(out, "stringExp(%s)", exp->u.string);
            break;
        case A_callExp:
            fprintf(out, "callExp(%s\n", exp->u.call.func->id);

    }
}
#pragma endregion