#pragma once

#include "util.h"

struct Temp_temp_ {
    int num;
};

Temp_temp Temp_newtemp(void);
Temp_label Temp_newlabel(void);
Temp_label Temp_namedlabel(char* name);