#pragma once

#include "util.h"

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
Temp_temp Temp_tempstring(char* name);
Temp_label Temp_newlabel(void);
Temp_label Temp_namedlabel(char* name);
Temp_tempList Temp_TempList(Temp_temp h, Temp_tempList t);
Temp_tempList Temp_TempLists(Temp_temp temps, ...);
Temp_labelList Temp_LabelList(Temp_label h, Temp_labelList t);
char* Temp_labelstring(Temp_label s);

void Temp_layerMap(Temp_map* stack);
Temp_map Temp_empty(void);
void Temp_enter(Temp_map m, Temp_temp t, void* s);
void* Temp_look(Temp_map m, Temp_temp t);
Temp_map Temp_name();

Temp_tempList Temp_union(Temp_tempList a, Temp_tempList b);
Temp_tempList Temp_intersect(Temp_tempList a, Temp_tempList b);
Temp_tempList Temp_minus(Temp_tempList a, Temp_tempList b);
int Temp_equal(Temp_tempList a, Temp_tempList b);
