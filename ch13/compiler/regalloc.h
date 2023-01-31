#pragma once
#include "util.h"
/* function prototype from regalloc.c */
struct RA_result {
    E_map coloring; 
    AS_instrList il;
};

struct RA_result RA_regAlloc(F_frame f, AS_instrList il);
