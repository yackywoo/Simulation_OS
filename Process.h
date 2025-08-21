//Jacky Qiu
//----------------------------------
#pragma once
#include <vector>
#include <unordered_set>

class Process {
    public:
        int PID_;
        unsigned long long size_;
        int priority_;
        int currentDisk_;
        Process* parent_;

        Process ();
        Process (const int PID, unsigned long long size, int priority, Process* parent);
    
        std::unordered_set<Process*> childrenProcesses_;
        std::unordered_set<Process*> zombieProcesses_;
};
