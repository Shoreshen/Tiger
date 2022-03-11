#include "util.h"

typedef struct AS_targets_ *AS_targets;
typedef struct AS_instrList_ *AS_instrList;
typedef struct AS_instr_ *AS_instr;
typedef struct AS_proc_ *AS_proc;
struct AS_targets_ {
	Temp_labelList labels;
};
struct AS_instr_ {
	enum {
		I_OPER,
		I_LABEL,
		I_MOVE
	} kind;
	union {
		struct {
			char *assem;
			Temp_tempList dst, src;
			AS_targets jumps;
		} OPER;
		struct {
			char *assem;
			Temp_label label;
		} LABEL;
		struct {
			char *assem;
			Temp_tempList dst, src;
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
void AS_print(FILE *out, AS_instr i, E_stack m);
AS_instrList AS_InstrList(AS_instr head, AS_instrList tail);
AS_instrList AS_splice(AS_instrList a, AS_instrList b);
void AS_printInstrList(FILE *out, AS_instrList iList, E_stack m);
AS_proc AS_Proc(char *p, AS_instrList b, char *e);
