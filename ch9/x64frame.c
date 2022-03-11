#include "frame.h"
#include "temp.h"
#include "tree.h"
#include "assem.h"

#define F_KEEP 6 // keep 6 formal param in registers
const int F_WORD_SIZE = 8; // x64 architecture

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

T_exp F_Exp(F_access acc, T_exp framePtr) {
    if (acc->kind == inFrame) {
        return T_Mem(T_Binop(T_plus, framePtr, T_Const(acc->u.offset)));
    } else {
        return T_Temp(acc->u.reg);
    }
}

static Temp_temp fp = NULL;
Temp_temp F_FP() {
    // x86-64 architecture use rbp as frame pointer
    // it is a constant register, thus return a const Temp_temp
    if(!fp) {
        fp = Temp_newtemp();
    }
    return fp;
}

static Temp_temp sp = NULL;
Temp_temp F_SP() {
    // x86-64 architecture use rsp as stack pointer
    // it is a constant register, thus return a const Temp_temp
    if(!sp) {
        sp = Temp_newtemp();
    }
    return sp;
}

static Temp_temp rv = NULL;
Temp_temp F_RV() {
    // x86-64 architecture use rax to store return value from function
    // it is a constant register, thus return a const Temp_temp
    if(!rv) {
        rv = Temp_newtemp();
    }
    return rv;
}

static Temp_temp f_regs[F_KEEP] = {NULL};
Temp_temp F_Keep_Regs(int i)
{
    // x86-64 architecture use rdi, rsi, rdx, rcx, r8, r9 to pass first 6 formal parameters
    // Thus returning constant reguster number
    if (!f_regs[i]) {
        f_regs[i] = Temp_newtemp();
    }
    return f_regs[i];
}

F_fragList F_FragList(F_frag head, F_fragList tail)
{
    F_fragList f = (F_fragList) checked_malloc(sizeof(*f));
    f->head = head;
    f->tail = tail;
    return f;
}
F_frag F_StringFrag(Temp_label label, char* str)
{
    F_frag f = (F_frag) checked_malloc(sizeof(*f));
    f->kind = F_stringFrag;
    f->u.stringg.label = label;
    f->u.stringg.str = str;
    return f;
}
F_frag F_ProcFrag(T_stm body, F_frame frame)
{
    F_frag f = (F_frag) checked_malloc(sizeof(*f));
    f->kind = F_procFrag;
    f->u.proc.body = body;
    f->u.proc.frame = frame;
    return f;
}
T_exp F_externalCall(char *s, T_expList args) {
    // cdcel need to 
    return T_Call(T_Name(Temp_namedlabel(s)), args);
}
T_stm F_procEntryExit1(F_frame frame, T_stm stm)
{
    return stm;
}
static Temp_tempList returnSink = NULL;

AS_instrList F_procEntryExit2(AS_instrList body) {
    Temp_tempList calleeSaves = NULL;
    if (!returnSink) {
        returnSink = Temp_TempList(
            F_ZERO(),
            Temp_TempList(
                F_RA(),
                Temp_TempList(
                    F_SP(), 
                    calleeSaves
                )
            )
        );
    }
    return AS_splice(
        body, 
        AS_InstrList(
            AS_Oper("", NULL, returnSink, NULL), 
            NULL
        )
    );
}

AS_proc F_procEntryExit3(F_frame frame, AS_instrList body) {
    char buf[100];
    sprintf(buf, "PROCEDURE %s\n", S_name(frame->name));
    return AS_Proc(String(buf), body, "END\n");
}

void F_printFrags(FILE* out, F_fragList frags)
{
    while(frags) {
        switch (frags->head->kind){
            case F_stringFrag:
                fprintf(out, "string: \n%s=%s\n", Temp_labelstring(frags->head->u.stringg.label), frags->head->u.stringg.str);
                break;
            case F_procFrag:
                fprintf(out, "proc %s: \n", Temp_labelstring(frags->head->u.proc.frame->name));
                pr_stm(out, frags->head->u.proc.body, 0);
                break;
            default:
                exit(1);
        }
        frags = frags->tail;
        fprintf(out, "\n");
    }
}
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
        if (f->inReg_count < F_KEEP && !(formals->head)) {
            f->formals = F_AccessList(InReg(F_Keep_Regs(f->inReg_count)), f->formals);
            f->inReg_count++;
        } else {
            /* 
                In x64:
                1. push formals (including rbp)
                2. call
                3. rsp->rbp
                while rbp is the frame pointer:
                ...
                rbp -  8: first local variable
                rbp     : return addrss
                rbp +  8: static link
                rbp + 16: first non-escape arg
                ...
            */
            f->formals = F_AccessList(InFrame((f->inFrame_count * F_WORD_SIZE)), f->formals);
            f->inFrame_count++;
        }
        formals = formals->tail;
    }

    return f;
}

F_access F_allocLocal(F_frame f, int escape)
{
    F_access a = NULL;
    if (escape) {
        a = InFrame(-(f->inFrame_count) * F_WORD_SIZE);
        f->inFrame_count++;
    } else {
        a = InReg(Temp_newtemp());
        f->inReg_count++;
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