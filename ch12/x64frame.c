#include "frame.h"
#include "temp.h"
#include "tree.h"
#include "assem.h"
#include "symbol.h"

#define F_KEEP 6 // keep 6 formal param in registers
const int F_WORD_SIZE = 8; // x64 architecture
// RBP, RSP not participate in coloring
enum REG {
    x64_RDI = 0,
    x64_RSI = 1,
    x64_RDX = 2,
    x64_RCX = 3,
    x64_R8  = 4,
    x64_R9  = 5, //First 6 registers are used for passing parameters
    x64_RBX = 6,
    x64_R12 = 7,
    x64_R13 = 8,
    x64_R14 = 9,
    x64_R15 = 10, // 6-10 callee saved registers
    x64_RBP = 11,
    x64_RSP = 12,
    x64_RAX = 13,
    x64_R10 = 14,
    x64_R11 = 15,
};
Temp_temp x64_regs_temp[F_COLORABLE_REGS] = { NULL };
char* x64_reg_names[F_COLORABLE_REGS] = {
    "rdi",  // 0
    "rsi",  // 1
    "rdx",  // 2
    "rcx",  // 3
    "r8",   // 4
    "r9",   // 5
    "rbx",  // 6
    "r12",  // 7
    "r13",  // 8
    "r14",  // 9
    "r15",  // 10
    "rbp",  // 11
    "rsp",  // 12
    "rax",  // 13
    "r10",  // 14
    "r11",  // 15
};
int pre_colored(Temp_temp t)
{
    if (t->num < PRE_COLOR_LIMIT) {
        return TRUE;
    }
    return FALSE;
}
Temp_temp get_x64_reg(int reg)
{
    if (!x64_regs_temp[reg]) {
        Temp_temp p = checked_malloc(sizeof(*p));
        p->num = (int) reg;
        Temp_enter(Temp_name(), p, x64_reg_names[reg]);
        x64_regs_temp[reg] = p;
    }
    return x64_regs_temp[reg];
}
T_exp F_Exp(F_access acc, T_exp framePtr) {
    if (acc->kind == F_inFrame) {
        return T_Mem(T_Binop(T_plus, framePtr, T_Const(acc->u.offset)));
    } else {
        return T_Temp(acc->u.reg);
    }
}
Temp_temp F_DX() {
    return get_x64_reg(x64_RDX);
}
Temp_temp F_FP() {
    return get_x64_reg(x64_RBP);
}
Temp_temp F_SP() {
    return get_x64_reg(x64_RSP);
}
Temp_temp F_RV() {
    return get_x64_reg(x64_RAX);
}
Temp_temp F_AX() {
    return get_x64_reg(x64_RAX);
}
Temp_temp F_Keep_Regs(int i)
{
    // x86-64 architecture use rdi, rsi, rdx, rcx, r8, r9 to pass first 6 formal parameters
    // Thus returning constant register number
    return get_x64_reg(i);
}

Temp_tempList all_regs = NULL;

Temp_tempList F_all_regs()
{
    if (!all_regs) {
        all_regs = Temp_TempLists(
            get_x64_reg(x64_RDI),
            get_x64_reg(x64_RSI),
            get_x64_reg(x64_RDX),
            get_x64_reg(x64_RCX),
            get_x64_reg(x64_R8),
            get_x64_reg(x64_R9),
            get_x64_reg(x64_RBP),
            get_x64_reg(x64_RSP),
            get_x64_reg(x64_RAX),
            get_x64_reg(x64_RBX),
            get_x64_reg(x64_R10),
            get_x64_reg(x64_R11),
            get_x64_reg(x64_R12),
            get_x64_reg(x64_R13),
            get_x64_reg(x64_R14),
            get_x64_reg(x64_R15),
            NULL
        );
    }
    return all_regs;
}

Temp_tempList caller_save_regs = NULL;

Temp_tempList F_caller_keep_regs()
{
    if (!caller_save_regs) {
        caller_save_regs = Temp_TempLists(
            get_x64_reg(x64_RAX),
            get_x64_reg(x64_RCX),
            get_x64_reg(x64_RDX),
            get_x64_reg(x64_RSI),
            get_x64_reg(x64_RDI),
            get_x64_reg(x64_R8),
            get_x64_reg(x64_R9),
            get_x64_reg(x64_R10),
            get_x64_reg(x64_R11),
            NULL
        );
    }
    return caller_save_regs;
}

Temp_tempList callee_save_regs = NULL;

Temp_tempList F_callee_keep_regs()
{
    if (!callee_save_regs) {
        callee_save_regs = Temp_TempLists(
            get_x64_reg(x64_RBX),
            get_x64_reg(x64_R12),
            get_x64_reg(x64_R13),
            get_x64_reg(x64_R14),
            get_x64_reg(x64_R15),
            NULL
        );
    }
    return callee_save_regs;
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
T_exp F_externalCall(char *s, T_expList args, F_accessList accs) {
    // cdcel need to 
    return T_Call(T_Name(Temp_namedlabel(s)), args, accs, NULL);
}

int callee_saved(Temp_temp reg)
{
    if (
        reg->num == x64_RBX ||
        reg->num == x64_R12 ||
        reg->num == x64_R13 ||
        reg->num == x64_R14 ||
        reg->num == x64_R15
    ) {
        return TRUE;
    }
    return FALSE;
}

void set_callee_regs(Temp_temp r, E_map m, char* callee_saved)
{
    char* s = NULL;
    if (r->num >= 6 && r->num <= 10) {
        callee_saved[r->num - 6] = 1;
    } else {
        s = Temp_look(m, r);
        if (s == x64_reg_names[x64_RBX]) {
            callee_saved[x64_RBX - 6] = 1;
        } else if (s == x64_reg_names[x64_R12]) {
            callee_saved[x64_R12 - 6] = 1;
        } else if (s == x64_reg_names[x64_R13]) {
            callee_saved[x64_R13 - 6] = 1;
        } else if (s == x64_reg_names[x64_R14]) {
            callee_saved[x64_R14 - 6] = 1;
        } else if (s == x64_reg_names[x64_R15]) {
            callee_saved[x64_R15 - 6] = 1;
        }
    }
}

void scan_callee_regs(AS_instrList body, E_map m, char* callee_saved)
{
    char* s = NULL;
    AS_instr i = NULL;
    Temp_tempList dst = NULL;
    while (body) {
        i = body->head;
        switch (i->kind) {
            case I_OPER:
                if (
                    // Instructions change regs: sub,pop,mov,imul,add
                    // Instructions do not write callee saved regs: push,jne,jmp,jle,jle,jle,jge,jg,je,idiv,cmp,call
                    i->u.OPER.assem[0] == 'a' || 
                    i->u.OPER.assem[0] == 'm' || 
                    i->u.OPER.assem[0] == 's' ||
                    (i->u.OPER.assem[0] == 'i' && i->u.OPER.assem[1] == 'm') ||
                    (i->u.OPER.assem[0] == 'p' && i->u.OPER.assem[1] == 'o')
                ) {
                    dst = i->u.OPER.dst;
                    while (dst) {
                        set_callee_regs(dst->head, m, callee_saved);
                        dst = dst->tail;
                    }
                }
                break;
            case I_MOVE:
                if (i->u.OPER.assem[0] != '#') {
                    dst = i->u.OPER.dst; 
                    while (dst) {
                        set_callee_regs(dst->head, m, callee_saved);
                        dst = dst->tail;
                    }
                }
                break;
        }
        body = body->tail;
    }
}

AS_proc F_procEntryExit(F_frame frame, AS_instrList body, E_map m) 
{
    AS_instrList procEntry = NULL, procExit = NULL, callee_push = NULL, callee_pop = NULL;
    char callee_saved[5] = {0};
    int i;

    scan_callee_regs(body, m, callee_saved);

    for (i = 0; i < 5; i++) {
        if (callee_saved[i]) {
            callee_push = AS_InstrList(
                AS_Oper("push `s0\n", NULL, Temp_TempLists(get_x64_reg(i + 6), NULL), NULL),
                callee_push
            );
        }
    }
    for (i = 4; i >= 0; i--) {
        if (callee_saved[i]) {
            callee_pop = AS_InstrList(
                AS_Oper("pop `s0\n", NULL, Temp_TempLists(get_x64_reg(i + 6), NULL), NULL),
                callee_pop
            );
        }
    }
    procEntry = AS_InstrLists(
        AS_Label(get_heap_str("%s:\n", Temp_labelstring(frame->name)), frame->name),
        AS_Oper("push `s0\n", NULL, Temp_TempLists(F_FP(), NULL), NULL),
        AS_Move("mov `d0, `s0\n", Temp_TempLists(F_FP(), NULL), Temp_TempLists(F_SP(), NULL)),
        NULL
    );
    if (frame->local_count) {
        procEntry = AS_splice(
            procEntry,
            AS_InstrLists(
                AS_Oper(
                    get_heap_str("sub `s0, %d\n", frame->local_count * F_WORD_SIZE),
                    Temp_TempList(F_SP(), NULL),
                    Temp_TempList(F_SP(), NULL),
                    NULL
                ),
                NULL
            )
        );
    }
    procExit = AS_InstrLists(
        AS_Move("mov `d0, `s0\n", Temp_TempLists(F_SP(), NULL), Temp_TempLists(F_FP(), NULL)),
        AS_Oper("pop `s0\n", NULL, Temp_TempLists(F_FP(), NULL), NULL),
        AS_Oper("ret\n", NULL, NULL, NULL),
        NULL
    );
    // callee_push & callee_pop may be NULL, thus use -1 as terminator
    body = AS_splices(
        procEntry,
        callee_push,
        body,
        callee_pop,
        procExit,
        -1L // Declare 64 bit constant of -1
    );
    return AS_Proc(get_heap_str("; PROCEDURE %s START\n", S_name(frame->fun_name)), body, get_heap_str("; %s END\n", S_name(frame->fun_name)));
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
F_access F_InFrame(int offset)
{
    F_access a = checked_malloc(sizeof(*a));
    a->kind = F_inFrame;
    a->u.offset = offset;
    return a;
}

F_access F_InReg(Temp_temp reg)
{
    F_access a = checked_malloc(sizeof(*a));
    a->kind = F_inReg;
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
F_access F_GetAccess(int *regCount, int *memCount, int escape)
{
    if(*regCount < F_KEEP && !escape) {
        (*regCount)++;
        return F_InReg(F_Keep_Regs(*regCount - 1));
    } else {
        (*memCount)++;
        return F_InFrame(((*memCount) - 1) * F_WORD_SIZE);
    }
}
F_frame F_newFrame(Temp_label name, U_boolList formals, S_symbol fun_name)
{
    F_frame f = checked_malloc(sizeof(*f));
    f->fun_name = fun_name;
    f->name = name;
    f->formals = NULL;
    f->locals = NULL;
    f->local_count = 0;
    f->inReg_count = 0;
    f->local_frame_count = 1;
    f->inFrame_count = 2; 
    // `rbp + 0` and `rbp + 8`
    /* 
        In x64:
        1. push formals (including rbp)
        2. call
        3. push rbp
        4. rsp->rbp
        while rbp is the frame pointer:
        ...
        rbp -  8: first local variable
        rbp     : caller rbp
        rbp +  8: return addrss
        rbp + 16: static link (parent frame pointer, may or may not be caller's rbp)
        rbp + 24: first non-escape arg
        ...
    */
    while (formals) {
        f->formals = F_AccessList(
            F_GetAccess(&(f->inReg_count), &(f->inFrame_count), formals->head), 
            f->formals
        );
        formals = formals->tail;
    }

    return f;
}

F_access F_allocLocal(F_frame f, int escape)
{
    F_access a = NULL;
    if (escape) {
        a = F_InFrame(-(f->local_frame_count) * F_WORD_SIZE);
        f->inFrame_count++;
        f->local_frame_count++;
    } else {
        a = F_InReg(Temp_newtemp());
        f->inReg_count++;
    }
    f->local_count++;
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