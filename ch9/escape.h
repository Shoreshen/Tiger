#pragma once
#include "ast.h"

// escape = TRUE means need to be in memory
// escape = FALSE means can be in register

void Esc_findEscape(A_exp exp);