#pragma once
#include "ast.h"

// escape = TRUE means need to be in memory
// escape = FALSE means can be in register
// For case like:
//     function a() {
//         var x = 1;
//         function b() {
//             var x = 2;
//         }
//     }
// In this case, x not passed as parameter (pushed into stack or in any of parameter regs)
// the only way to access it is through frame pointer and recursivley upward seeking (in stack).
// So, we need to allocate it in memory (stack).

void Esc_findEscape(A_exp exp);