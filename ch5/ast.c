#include "ast.h"
#include "util.h"

A_exp ast_root;

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
    p->u.forr.escape = TRUE;
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
    p->u.var.escape = TRUE;
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
    p->escape = TRUE;
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
        fprintf(out, "    ");
    }
}
void new_line(FILE *out, int d, char *pre_s, char *post_s)
{
    fprintf(out, "%s\n", pre_s);
    indent(out, d);
    fprintf(out, "%s", post_s);
}
void print_var(FILE *out, A_var var ,int d) 
{
    indent(out, d);
    switch (var->kind) {
        case A_simpleVar:
            fprintf(out, "simpleVar(%s)", S_name(var->u.simple));
            break;
        case A_fieldVar:
            fprintf(out, "fieldVar(\n");
            print_var(out, var->u.field.var, d + 1);
            new_line(out, d + 1, ",", "");
            fprintf(out, "%s", S_name(var->u.field.sym));
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
void print_oper(FILE *out, A_oper oper, int d) 
{
    indent(out, d);
    switch (oper) {
        case A_plusOp:
            fprintf(out, "plusOp");
            break;
        case A_minusOp:
            fprintf(out, "minusOp");
            break;
        case A_timesOp:
            fprintf(out, "timesOp");
            break;
        case A_divideOp:
            fprintf(out, "divideOp");
            break;
        case A_eqOp:
            fprintf(out, "eqOp");
            break;
        case A_neqOp:
            fprintf(out, "neqOp");
            break;
        case A_ltOp:
            fprintf(out, "ltOp");
            break;
        case A_leOp:
            fprintf(out, "leOp");
            break;
        case A_gtOp:
            fprintf(out, "gtOp");
            break;
        case A_geOp:
            fprintf(out, "geOp");
            break;
        case A_andOp:
            fprintf(out, "andOp");
            break;
        case A_orOp:
            fprintf(out, "orOp");
            break;
        default:
            assert(0);
    }
}
void print_expList(FILE *out, A_expList list, int d) 
{
    indent(out, d);
    if (list) {
        fprintf(out, "expList(\n");
        while (list) {
            print_exp(out, list->head, d + 1);
            list = list->tail;
            if (list) {
                fprintf(out, ",\n");
            }
        }
        new_line(out, d, "", ")");
    } else {
        fprintf(out, "expList()\n");
    }
}
void print_expSeq(FILE *out, A_expList seq, int d) 
{
    if (seq) {
        while (seq) {
            print_exp(out, seq->head, d);
            seq = seq->tail;
            if (seq) {
                fprintf(out, ",\n");
            }
        }
    }
}
void print_efield(FILE *out, A_efield efield, int d) 
{
    indent(out, d);
    if (efield) {
        fprintf(out, "efield(%s,\n", S_name(efield->name));
        print_exp(out, efield->exp, d + 1);
        new_line(out, d, "", ")");
    } else {
        fprintf(out, "efield()\n");
    }
}
void print_efieldList(FILE *out, A_efieldList list, int d) 
{
    indent(out, d);
    if (list) {
        fprintf(out, "efieldList(\n");
        while (list) {
            print_efield(out, list->head, d + 1);
            list = list->tail;
            if (list) {
                fprintf(out, ",\n");
            }
        }
        new_line(out, d, "", ")");
    } else {
        fprintf(out, "efieldList()\n");
    }
}
void print_field(FILE *out, A_field field, int d) 
{
    indent(out, d);
    fprintf(out, "field(%s", S_name(field->name));
    new_line(out, d + 1, ",", "");
    fprintf(out, "%s", S_name(field->typ));
    new_line(out, d + 1, ",", "");
    fprintf(out, "%s", field->escape ? "TRUE" : "FALSE");
    new_line(out, d, "", ")");
}
void print_fieldList(FILE *out, A_fieldList list, int d) 
{
    indent(out, d);
    if (list) {
        fprintf(out, "fieldList(\n");
        while (list) {
            print_field(out, list->head, d + 1);
            list = list->tail;
            if (list) {
                fprintf(out, ",\n");
            }
        }
        new_line(out, d, "", ")");
    } else {
        fprintf(out, "fieldList()");
    }
}
void print_fundec(FILE *out, A_fundec fundec, int d) 
{
    indent(out, d);
    fprintf(out, "fundec(%s,\n", S_name(fundec->name));
    print_fieldList(out, fundec->params, d + 1);
    if (fundec->result) {
        new_line(out, d + 1, ",", "");
        fprintf(out, "%s", S_name(fundec->result));
    }
    fprintf(out, ",\n");
    print_exp(out, fundec->body, d + 1);
    new_line(out, d, "", ")");
}
void print_ty(FILE *out, A_ty ty, int d) 
{
    indent(out, d);
    switch (ty->kind) {
        case A_nameTy:
            fprintf(out, "nameTy(%s)\n", S_name(ty->u.name));
            break;
        case A_recordTy:
            fprintf(out, "recordTy(\n");
            print_fieldList(out, ty->u.record, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_arrayTy:
            fprintf(out, "arrayTy(%s)", S_name(ty->u.array));
            break;
        default:
            assert(0);
    }
}
void print_namety(FILE *out, A_namety namety, int d) 
{
    indent(out, d);
    fprintf(out, "namety(%s,\n", S_name(namety->name));
    print_ty(out, namety->ty, d + 1);
    new_line(out, d, "", ")");
}
void print_dec(FILE *out, A_dec dec, int d) 
{
    indent(out, d);
    switch (dec->kind) {
        case A_functionDec:
            fprintf(out, "functionDec(\n");
            print_fundec(out, dec->u.function, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_varDec:
            fprintf(out, "varDec(%s,\n", S_name(dec->u.var.var));
            if (dec->u.var.typ) {
                indent(out, d + 1);
                fprintf(out, "%s\n", S_name(dec->u.var.typ));
            }
            print_exp(out, dec->u.var.init, d + 1);
            new_line(out, d + 1, ",", "");
            fprintf(out, "%s", dec->u.var.escape ? "TRUE" : "FALSE");
            new_line(out, d, "", ")");
            break;
        case A_typeDec:
            fprintf(out, "typeDec(\n");
            print_namety(out, dec->u.type, d + 1);
            new_line(out, d, "", ")");
            break;
        default:
            assert(0);
    }
}
void print_decList(FILE *out, A_decList list, int d) 
{
    indent(out, d);
    if (list) {
        fprintf(out, "decList(\n");
        while (list) {
            print_dec(out, list->head, d + 1);
            list = list->tail;
            if (list) {
                fprintf(out, ",\n");
            }
        }
        new_line(out, d, "", ")");
    } else {
        fprintf(out, "decList()");
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
            fprintf(out, "callExp(%s\n", S_name(exp->u.call.func));
            print_expList(out, exp->u.call.args, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_opExp:
            fprintf(out, "opExp(\n");
            print_oper(out, exp->u.op.oper, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.op.left, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.op.right, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_recordExp:
            fprintf(out, "recordExp(%s,\n", S_name(exp->u.record.typ));
            print_efieldList(out, exp->u.record.fields, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_seqExp:
            fprintf(out, "seqExp(\n");
            print_expSeq(out, exp->u.seq, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_assignExp:
            fprintf(out, "assignExp(\n");
            print_var(out, exp->u.assign.var, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.assign.exp, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_ifExp:
            fprintf(out, "iffExp(\n");
            print_exp(out, exp->u.iff.test, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.iff.then, d + 1);
            if (exp->u.iff.elsee) { // else exists
                fprintf(out, ",\n");
                print_exp(out, exp->u.iff.elsee, d + 1);
            }
            new_line(out, d, "", ")");
            break;
        case A_whileExp:
            fprintf(out, "whileExp(\n");
            print_exp(out, exp->u.whilee.test, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.whilee.body, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_forExp:
            fprintf(out, "forExp(%s\n", S_name(exp->u.forr.var));
            print_exp(out, exp->u.forr.lo, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.forr.hi, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.forr.body, d + 1);
            fprintf(out, "%s\n", exp->u.forr.escape ? "TRUE" : "FALSE");
            new_line(out, d, "", ")");
            break;
        case A_breakExp:
            fprintf(out, "breakExp()");
            break;
        case A_letExp:
            fprintf(out, "letExp(\n");
            print_decList(out, exp->u.let.decs, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.let.body, d + 1);
            new_line(out, d, "", ")");
            break;
        case A_arrayExp:
            fprintf(out, "arrayExp(%s,\n", S_name(exp->u.array.typ));
            print_exp(out, exp->u.array.size, d + 1);
            fprintf(out, ",\n");
            print_exp(out, exp->u.array.init, d + 1);
            new_line(out, d, "", ")");
            break;
        default:
            assert(0);
    }
}
#pragma endregion