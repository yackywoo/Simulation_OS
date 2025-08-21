//Jacky Qiu
//----------------------------------
#pragma once
#include <string>
#include <vector>
#include <queue>
#include <tuple>
#include <list>
#include <unordered_set>
#include "Process.h"

//FOR DISK
struct FileReadRequest {
    int  PID{0};
    std::string fileName{""};
};

//FOR RAM
struct MemoryItem {
    unsigned long long itemAddress;
    unsigned long long itemSize;
    int PID; 
};
using MemoryUse = std::vector<MemoryItem>;

//FOR CPU / PROCESS CLASS
constexpr int NO_PROCESS{-1};

class SimOS {
    public: 
        //OS, RAM, CPU functions
        SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned long long sizeOfOS);
        bool NewProcess( unsigned long long size, int priority );
        bool SimFork();
        void SimExit();
        void SimWait();
        int GetCPU();
        std::vector<int> GetReadyQueue();
        MemoryUse GetMemory();
        
        //Disk functions
        void DiskReadRequest( int diskNumber, std::string fileName );
        void DiskJobCompleted( int diskNumber );
        FileReadRequest GetDisk( int diskNumber );
        std::queue<FileReadRequest> GetDiskQueue( int diskNumber );

    private:
        //OS data members
        int numberOfDisks_;
        unsigned long long amountOfRAM_;
        unsigned long long sizeOfOS_;
        bool OSadded_;
        
        //Process management
        int trackPID_;
        std::list<Process> processList;
        std::unordered_set<Process*> waitingParents;
        Process* currentProcess;
        void updateCurrProcess();
        bool parentFork();
        void removeFromRAM(int PID);
        void removeFromScheduler(Process* ptr);
        void removeFromProcessList(Process* ptr);
        void removeFromDisk(Process* ptr);
        void removeFromDiskQueue(Process* ptr);
        void removeFromAnyDisk(Process* ptr);
        void killFamilyTree(Process* ptr);      //recursive family killer

        //RAM management
        MemoryUse RAM_; 
        unsigned long long remainingRAM_;
        bool fitInRAM(unsigned long long size);
        int findWorstFitIndex();

        //CPU scheduling using maxHeap
        //Tuple is (priority, PID) order
        std::priority_queue<std::tuple<int, Process*>> Scheduler;  

        //Disk management
        std::vector<std::queue<std::tuple<FileReadRequest,Process*>>> waitingQueueInDisk;
        std::vector<std::tuple<FileReadRequest,Process*>> currProcessInDisk;
};

