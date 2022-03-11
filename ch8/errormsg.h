#pragma once
#include "util.h"

typedef char bool;

extern bool EM_anyErrors;
extern int EM_tokPos;
extern int charPos;

void EM_error(A_pos pos, char *message,...);
void EM_reset(char * filename);