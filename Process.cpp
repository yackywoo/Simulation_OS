//Jacky Qiu
//----------------------------------
#include "Process.h"

//default constructor
Process::Process() : 
    PID_{-1},
    size_{0},
    priority_{-1},
    currentDisk_{-1},
    parent_{nullptr} {
}

//parameter constructor
Process::Process(const int PID, unsigned long long size, int priority, Process* parent) :
    PID_{PID},
    size_{size},
    priority_{priority},
    currentDisk_{-1},
    parent_{parent} {
}

