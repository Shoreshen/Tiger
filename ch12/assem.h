#include "util.h"

typedef struct AS_targets_ *AS_targets;
typedef struct AS_instr_ *AS_instr;
struct AS_targets_ {
	Temp_labelList labels;
};
struct AS_instr_ {
	enum {
		I_OPER,
		I_LABEL,
		I_MOVE //Has to be the form of `mov reg, reg`
	} kind;
	union {
		struct {
			char *assem;
			Temp_tempList dst;
			Temp_tempList src;
			AS_targets jumps;
		} OPER;
		struct {
			char *assem;
			Temp_label label;
		} LABEL;
		struct {
			char *assem;
			Temp_tempList dst;
			Temp_tempList src;
		} MOVE;
	} u;
};
struct AS_proc_ {
	char *prolog;
	AS_instrList body;
	char *epilog;
};
struct AS_instrList_ {
	AS_instr head;
	AS_instrList tail;
};

AS_targets AS_Targets(Temp_labelList labels);
AS_instr AS_Oper(char *a, Temp_tempList d, Temp_tempList s, AS_targets j);
AS_instr AS_Label(char *a, Temp_label label);
AS_instr AS_Move(char *a, Temp_tempList d, Temp_tempList s);
void AS_print(FILE *out, AS_instr i, E_map m);
AS_instrList AS_InstrList(AS_instr head, AS_instrList tail);
AS_instrList AS_InstrLists(AS_instr instrs, ...);
AS_instrList AS_splice(AS_instrList a, AS_instrList b);
AS_instrList AS_splices(AS_instrList instrlists, ...);
void AS_printInstrList(FILE *out, AS_instrList iList, E_map m);
AS_proc AS_Proc(char *p, AS_instrList b, char *e);

AS_instrList AS_instrUnion(AS_instrList ta, AS_instrList tb);
AS_instrList AS_instrMinus(AS_instrList ta, AS_instrList tb);
AS_instrList AS_instrIntersect(AS_instrList ta, AS_instrList tb);
int AS_instrInList(AS_instr i, AS_instrList il);
void AS_clearList(AS_instrList il);

void AS_InsertAfter(AS_instrList il, AS_instr i);
void AS_RmAfter(AS_instrList il);
Temp_tempList AS_def(AS_instr i);
Temp_tempList AS_use(AS_instr i);