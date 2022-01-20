#pragma once

#include "util.h"

typedef struct Temp_tempList_ *Temp_tempList;
typedef struct Temp_labelList_ *Temp_labelList;

struct Temp_temp_ {
    int num;
};
struct Temp_tempList_ { 
    Temp_temp head; 
    Temp_tempList tail;
};
struct Temp_labelList_ { 
    Temp_label head; 
    Temp_labelList tail;
};

Temp_temp Temp_newtemp(void);
Temp_label Temp_newlabel(void);
Temp_label Temp_namedlabel(char* name);
Temp_tempList Temp_TempList(Temp_temp h, Temp_tempList t);
Temp_labelList Temp_LabelList(Temp_label h, Temp_labelList t);