#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "canon.h"
#include "env.h"

typedef struct expRefList_ *expRefList;
struct expRefList_ {
    T_exp *head;
    expRefList tail;
};

/* local function prototypes */
T_stm do_stm(T_stm stm);
struct stmExp do_exp(T_exp exp);
C_stmListList mkBlocks(T_stmList stms, Temp_label done);
T_stmList getNext(void);

expRefList ExpRefList(T_exp *head, expRefList tail)
{
    expRefList p = (expRefList)checked_malloc(sizeof *p);
    p->head = head;
    p->tail = tail;
    return p;
}

int isNop(T_stm x)
{
    return x == NULL || (x->kind == T_EXP && x->u.EXP->kind == T_CONST);
}

T_stm seq(T_stm x, T_stm y)
{
    if (isNop(x)) {
        return y;
    }
    if (isNop(y)) {
        return x;
    }
    return T_Seq(x, y);
}

int commute(T_stm x, T_exp y)
{
    // T_NAME in expression returns the address of that assembly lable
    // it is a constant
    return isNop(x) || y->kind == T_CONST || y->kind == T_NAME;
}

struct stmExp {
    T_stm s;
    T_exp e;
};

T_stm reorder(expRefList rlist)
{
    // For input of e1,e2,...,en 
    // produce a list of s1,s2,...,sn,e'1,1',...,en' that equivlent by calling do_exp for each ei
    // si need to be commute with e(i-1), thus need to start from the end of the list
    // and recursively call self
    if (rlist == NULL) {
        return NULL; /* nop */
    } else {
        T_stm s = reorder(rlist->tail);
        struct stmExp hd = do_exp(*rlist->head);
        if (commute(s, hd.e)) {
            *rlist->head = hd.e;
            return seq(hd.s, s);
        } else {
            Temp_temp t = Temp_newtemp();
            *rlist->head = T_Temp(t);
            return seq(hd.s, seq(T_Move(T_Temp(t), hd.e), s));
        }
    }
}

expRefList get_call_rlist(T_exp exp)
{
    expRefList rlist, curr;
    T_expList args = exp->u.CALL.args;
    rlist = ExpRefList(&exp->u.CALL.fun, NULL);
    curr = rlist;
    for (; args; args = args->tail) {
        curr = curr->tail = ExpRefList(&args->head, NULL);
    }
    return rlist;
}

struct stmExp StmExp(T_stm stm, T_exp exp)
{
    struct stmExp x;
    x.s = stm;
    x.e = exp;
    return x;
}

struct stmExp do_exp(T_exp exp)
{
    // return `exp` equivalents, that is 
    //      1. first exectue `do_exp(exp).s` 
    //      2. then `do_exp(exp).e` will result in the same value
    switch (exp->kind) {
        case T_CONST:
        case T_NAME:
        case T_TEMP: 
            return (struct stmExp) {
                .s = NULL,
                .e = exp
            };
        case T_MEM: {
            struct stmExp ret = do_exp(exp->u.MEM);
            exp->u.MEM = ret.e;
            ret.e = exp;
            return ret;
        }
        case T_ESEQ: {
            T_stm s = do_stm(exp->u.ESEQ.stm);
            struct stmExp ret = do_exp(exp->u.ESEQ.exp);
            ret.s = seq(s, ret.s);
            return ret;
        }
        case T_CALL: {
            T_stm s = reorder(get_call_rlist(exp));
            // For case like `T_Binop(PLUS, T_Call(), T_Call())`
            // The latter call exp will re-write the RV result from formar call
            // Thus must save it in an temprary reg
            Temp_temp t = Temp_newtemp();
            s = seq(
                s,
                T_Move(T_Temp(t), exp)
            );
            return (struct stmExp) {
                .s = s,
                .e = T_Temp(t)
            };
        }
        case T_BINOP:
            return (struct stmExp) {
                .s = reorder(ExpRefList(&exp->u.BINOP.left, ExpRefList(&exp->u.BINOP.right, NULL))),
                .e = exp
            };
        default:
            assert(0);
    }
}

T_stm do_stm(T_stm stm)
{
    switch (stm->kind) {
        case T_SEQ:
            return seq(
                do_stm(stm->u.SEQ.left), 
                do_stm(stm->u.SEQ.right)
            );
        case T_JUMP:{
            struct stmExp ret = do_exp(stm->u.JUMP.exp);
            stm->u.JUMP.exp = ret.e;
            return seq(ret.s, stm);
        }
        case T_CJUMP:
            return seq(
                reorder(ExpRefList(&stm->u.CJUMP.left, ExpRefList(&stm->u.CJUMP.right, NULL))),
                stm
            );
        case T_MOVE:
            // If dst is T_TEMP, it will be written by the src anyway ,thus trivial
            if (stm->u.MOVE.dst->kind == T_TEMP && stm->u.MOVE.src->kind == T_CALL) {
                // Though T_Call() will effect RV
                // but in this case T_Temp() will not commute with RV
                return seq(
                    reorder(get_call_rlist(stm->u.MOVE.src)), 
                    stm
                );
            } else if (stm->u.MOVE.dst->kind == T_TEMP) {
                return seq(
                    reorder(ExpRefList(&stm->u.MOVE.src, NULL)), 
                    stm
                );
            } else if (stm->u.MOVE.dst->kind == T_MEM) {
                return seq(
                    reorder(ExpRefList(&stm->u.MOVE.dst->u.MEM, ExpRefList(&stm->u.MOVE.src, NULL))),
                    stm
                );
            } else if (stm->u.MOVE.dst->kind == T_ESEQ) {
                // It is equivalent for the following:
                //      1. T_Move(T_Eseq(stm, exp1), exp2)
                //      2. T_Seq(stm, T_Move(exp1, exp2))
                T_stm s = stm->u.MOVE.dst->u.ESEQ.stm;
                stm->u.MOVE.dst = stm->u.MOVE.dst->u.ESEQ.exp;
                return do_stm(T_Seq(s, stm));
            }
            // Only lvalue can be assigned, which all will be T_MEM or T_TEMP
            assert(0); /* dst should be temp or mem only */
        case T_EXP:
            if (stm->u.EXP->kind == T_CALL) {
                return seq(
                    reorder(get_call_rlist(stm->u.EXP)), 
                    stm
                );
            } else {
                return seq(
                    reorder(ExpRefList(&stm->u.EXP, NULL)), 
                    stm
                );
            }
        case T_LABEL:
            return stm;
        default:
            assert(0);
    }
}

/* linear gets rid of the top-level SEQ's, producing a list */
T_stmList linear(T_stm stm, T_stmList right)
{
    if (stm->kind == T_SEQ) {
        return linear(stm->u.SEQ.left, linear(stm->u.SEQ.right, right));
    } else {
        return T_StmList(stm, right);
    }
}

/* 
From an arbitrary Tree statement, produce a list of cleaned trees satisfying the following properties:
    1.  No SEQ's or ESEQ's
    2.  The parent of every CALL is an EXP(..) or a MOVE(TEMP t,..)
*/
T_stmList C_linearize(T_stm stm)
{
    return linear(do_stm(stm), NULL);
}

C_stmListList StmListList(T_stmList head, C_stmListList tail)
{
    C_stmListList p = (C_stmListList)checked_malloc(sizeof *p);
    p->head = head;
    p->tail = tail;
    return p;
}

C_stmListList* handle_stm_list(C_stmListList* ll_tail, T_stmList** l_tail, T_stmList* l_head)
{
    T_stmList tmp;
    // Adding stm listlist
    (*ll_tail) = StmListList((*l_head), NULL);
    // Break stm list
    tmp = (**l_tail)->tail;
    (**l_tail)->tail = NULL;
    (*l_head) = tmp;
    (*l_tail) = l_head;
    return &(*ll_tail)->tail;
}
/* 
basicBlocks : Tree.stm list -> (Tree.stm list list * Tree.label)
From a list of cleaned trees, produce a list of basic blocks satisfying the following properties:
    1.  and 2. as above;
    3.  Every block begins with a LABEL;
    4.  A LABEL appears only at the beginning of a block;
    5.  Any JUMP or CJUMP is the last stm in a block;
    6.  Every block ends with a JUMP or CJUMP;
Also produce the "label" to which control will be passed upon exit.
*/
struct C_block C_basicBlocks(T_stmList stmList)
{
    struct C_block b = {
        .label = Temp_newlabel(),
        .stmLists = NULL
    };
    C_stmListList* ll_tail = &b.stmLists;
    T_stmList l_head = NULL;
    T_stmList* l_tail = NULL;
    l_head = stmList;
    l_tail = &stmList;

    if (stmList == NULL) {
        return b;
    }
    if (stmList->head->kind != T_LABEL) {
        stmList = T_StmList(T_Label(Temp_newlabel()), stmList);
        l_head = stmList;
    }
    // Skip first lable 
    l_tail = &(*l_tail)->tail;

    while (TRUE) {
        if ((*l_tail) == NULL) {
            (*l_tail) = T_StmList(T_Jump(T_Name(b.label), Temp_LabelList(b.label, NULL)), NULL);
            ll_tail = handle_stm_list(ll_tail, &l_tail, &l_head); 
            break; 
        } else if((*l_tail)->head->kind == T_JUMP || (*l_tail)->head->kind == T_CJUMP) {
            ll_tail = handle_stm_list(ll_tail, &l_tail, &l_head);
            if ((*l_tail)->head->kind != T_LABEL) {
                (*l_tail) = T_StmList(T_Label(Temp_newlabel()), (*l_tail));
            }
            l_tail = &(*l_tail)->tail;
        } else if ((*l_tail)->head->kind == T_LABEL) {
            Temp_label lab = (*l_tail)->head->u.LABEL;
            (*l_tail) = T_StmList(T_Jump(T_Name(lab), Temp_LabelList(lab, NULL)), (*l_tail));
            ll_tail = handle_stm_list(ll_tail, &l_tail, &l_head);
            l_tail = &(*l_tail)->tail;
        } else {
            l_tail = &(*l_tail)->tail;
        }
    }

    return b;
}

E_map block_env;
struct C_block global_block;

T_stmList getLast(T_stmList list)
{
    T_stmList last = list;
    while (last->tail->tail) {
        last = last->tail;
    }
    return last;
}

void trace(T_stmList list)
{
    T_stmList last = getLast(list);
    T_stm lab = list->head;
    T_stm s = last->tail->head;
    S_enter(block_env, lab->u.LABEL, NULL);
    if (s->kind == T_JUMP) {
        T_stmList target = (T_stmList)S_look(block_env, s->u.JUMP.jumps->head);
        if (!s->u.JUMP.jumps->tail && target) {
            // If only one jump target, merge the 2 lists removing JUMP stm
            last->tail = target;
            trace(target);
        } else {
            /* merge and keep JUMP stm */
            last->tail->tail = getNext();
        }
    } else if (s->kind == T_CJUMP) {
        /* we want false label to follow CJUMP */
        T_stmList true = (T_stmList)S_look(block_env, s->u.CJUMP.true);
        T_stmList false = (T_stmList)S_look(block_env, s->u.CJUMP.false);
        if (false) {
            // merge false block if exists
            last->tail->tail = false;
            trace(false);
        } else if (true) { 
            Temp_label tmp;
            tmp = s->u.CJUMP.false;
            s->u.CJUMP.false = s->u.CJUMP.true;
            s->u.CJUMP.true = tmp;
            s->u.CJUMP.op = T_notRel(s->u.CJUMP.op);
            last->tail->tail = true;
            trace(true);
        } else {
            Temp_label new_false = Temp_newlabel();
            Temp_label old_false = s->u.CJUMP.false;
            s->u.CJUMP.false = new_false;
            last->tail->tail = T_StmList(
                T_Label(new_false), 
                T_StmList(
                    T_Jump(T_Name(old_false), Temp_LabelList(old_false, NULL)),
                    getNext()
                )
            );
        }
    } else {
        assert(0);
    }
}

/* get the next block from the list of stmLists, using only those that have
 * not been traced yet */
T_stmList getNext()
{
    if (!global_block.stmLists) {
        return T_StmList(T_Label(global_block.label), NULL);
    } else {
        T_stmList s = global_block.stmLists->head;
        if (S_look(block_env, s->head->u.LABEL)) {
            /* label exists in the table */
            trace(s);
            return s;
        } else {
            global_block.stmLists = global_block.stmLists->tail;
            return getNext();
        }
    }
}
/* 
traceSchedule : Tree.stm list list * Tree.label -> Tree.stm list
From a list of basic blocks satisfying properties 1-6, along with an "exit" label, produce a list of stms such that:
    1. and 2. as above;
    7. Every CJUMP(_,t,f) is immediately followed by LABEL f.
The blocks are reordered to satisfy property 7; 
also in this reordering as many JUMP(T.NAME(lab)) statements as possible are eliminated by falling through into T.LABEL(lab).
*/
T_stmList C_traceSchedule(struct C_block b)
{
    C_stmListList sList;
    block_env = E_empty_env();
    global_block = b;

    for (sList = global_block.stmLists; sList; sList = sList->tail) {
        S_enter(block_env, sList->head->head->u.LABEL, sList->head);
    }

    return getNext();
}


#pragma region Sample illustration for do_stm, do_exp and reorder
/*
T_Exp(
    T_Binop(T_plus,
        T_Mem(T_Name(t)),
        T_Eseq(
            T_Move(
                T_Mem(T_Name(t)),
                T_Const(2)
            ),
            T_Mem(T_Name(t))
        )
    )
)

process:
do_stm(T_exp(...)){
    reorder({&T_binop(...),NULL}){
        hd = do_exp(T_binop(...)){
            reorder({&T_men(T_name("a")), &T_eseq(...), NULL}){
                hd = do_exp(T_men(T_name("a"))){
                    reorder({&T_name("a"), NULL}){
                        hd = do_exp(T_name("a")) {
                            return {
                                s = T_exp(T_const(0)),
                                e = T_name("a")
                            }
                        }
                        s = reorder(NULL){
                            return T_exp(T_const(0))
                        }
                        *&T_name("a") = T_name("a")
                        return {T_exp(T_const(0))}
                    }
                    return {
                        s = T_exp(T_const(0)),
                        e = T_men(T_name("a"))
                    }
                }
                s = reorder({&T_eseq(...), NULL}){
                    hd = do_exp(T_eseq(...)){
                        x = do_exp(T_men(T_name("a"))){
                            reorder({&T_name("a"), NULL}){
                                hd = do_exp(T_name("a")) {
                                    return {
                                        s = T_exp(T_const(0)),
                                        e = T_name("a")
                                    }
                                }
                                s = reorder(NULL){
                                    return T_exp(T_const(0))
                                }
                                *&T_name("a") = T_name("a")
                                return T_exp(T_const(0))
                            }
                            return {
                                s = T_exp(T_const(0)),
                                e = T_men(T_name("a"))
                            }
                        }
                        do_stm(T_mov(...)){
                            reorder({&T_men(T_name("a")), &T_const(2), NULL}){
                                hd = do_exp(T_men(T_name("a"))){
                                    reorder({&T_name("a"), NULL}){
                                        hd = do_exp(T_name("a")) {
                                            return {
                                                s = T_exp(T_const(0)),
                                                e = T_name("a")
                                            }
                                        }
                                        s = reorder(NULL){
                                            return T_exp(T_const(0))
                                        }
                                        *&T_name("a") = T_name("a")
                                        return T_exp(T_const(0))
                                    }
                                    return {
                                        s = T_exp(T_const(0)),
                                        e = T_men(T_name("a"))
                                    }
                                }
                                s = reorder({&T_const(2), NULL}){
                                    hd = do_exp(T_const(2)){
                                        return {
                                            s = T_exp(T_const(0)),
                                            e = T_const(2)
                                        }
                                    }
                                    s = reorder({NULL}){
                                        return T_exp(T_const(0))
                                    }
                                    *&T_const(2) = T_const(2)
                                    return T_exp(T_const(0))
                                }
                                return T_exp(T_const(0))
                            }
                            return T_mov(...)
                        }
                        return {
                            s = T_mov(...),
                            e = T_men(T_name("a"))
                        }
                    }
                    s = reorder({NULL}){
                        return T_exp(T_const(0))
                    }
                    &T_eseq(...) = T_men(T_name("a"))
                    return T_mov(...)
                }
                return {T_mov(T_Temp(t), T_men(T_name("a"))), T_mov(...)}
            }
            return {
                s = {T_mov(T_Temp(t), T_men(T_name("a"))), T_mov(...)}
                e = T_binop(...)
            }
        }
        s = reorder({NULL}){
            return T_exp(T_const(0))
        }
        *&T_binop(...) = T_binop(...)
        return {T_mov(T_Temp(t), T_men(T_name("a"))), T_mov(...)}
    }
    return {T_mov(T_Temp(t), T_men(T_name("a"))), T_mov(...), T_binop(...)}
}
*/
#pragma endregion