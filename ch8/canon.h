#pragma once

#include "util.h"

typedef struct C_stmListList_ *C_stmListList;
struct C_block {
    C_stmListList stmLists;
    Temp_label label;
};
struct C_stmListList_ {
    T_stmList head;
    C_stmListList tail;
};

T_stmList C_linearize(T_stm stm);
struct C_block C_basicBlocks(T_stmList stmList);
T_stmList C_traceSchedule(struct C_block b);

