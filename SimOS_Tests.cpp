#include <cassert>
#include <iostream>
#include "SimOS.h"
#define OS_SIZE 10'000'000'000
#define OS_DISKS 3
#define OS_RAM 64'000'000'000 //64GB RAM
#define DISK_0 0
#define DISK_1 1
#define DISK_2 2


bool notInRAM(SimOS& sim, const int PID, bool printerrors) {
    //check RAM
    bool result = true;
    auto RAM = sim.GetMemory();
    for (auto memItem : RAM) {
        if (memItem.PID == PID) {
            if (printerrors) {
                std::cout << "\t"<< PID << " EXISTS IN RAM" << std::endl;
            }
            result = false;
        }
    }
    return result;
}

bool notInCPU(SimOS& sim, const int PID, bool printerrors) {
    bool result = true;
    //check CPU
    if (sim.GetCPU() == PID) {
        if (printerrors) {
            std::cout << "\t"<< PID << " RUNNING IN CPU" << std::endl;
        }
        result = false;
    }
    return result;
}

bool notInReadyQ(SimOS& sim, const int PID, bool printerrors) {
    bool result = true;
    //check READY Q
    auto readyQ = sim.GetReadyQueue();
    for (auto process : readyQ) {
        if (process == PID) {
            if(printerrors) { 
                std::cout << "\t"<< PID << " EXISTS IN READY QUEUE" << std::endl;
            }
            result = false;
        }
    }
    return result;
}

bool notInDiskQ(SimOS& sim, const int PID, bool printerrors) {
    bool result = true;
    for (int i = 0 ; i < OS_DISKS ; ++i) {
        auto queueI = sim.GetDiskQueue(i);
        
        while (!queueI.empty()) {
            if (queueI.front().PID == PID) {
                if (printerrors) {
                    std::cout << "\t"<< PID << " EXISTS IN DISK QUEUE [" << i << "]" << std::endl;
                }
                result = false;
            }
            queueI.pop();
        }
    }
    return result;
}

bool notInDisk(SimOS& sim, const int PID, bool printerrors) {
    bool result = true;
    for (int i = 0 ; i < OS_DISKS ; ++i) {
        //check RUNNING DISK
        if (sim.GetDisk(i).PID == PID) {
            if (printerrors) {
                std::cout << "\t"<< PID << " RUNNING IN DISK [" << i << "]" << std::endl;
            }
            result = false;
        }
    }
    return result;
}

//DEBUG TOOL -> tells u where a procss EXISTS if it shouldnt
bool processDNE(SimOS& sim, const int PID, bool printerrors) {
    //assume process exists
    bool RAMcheck = notInRAM(sim, PID, printerrors);
    bool CPUcheck = notInCPU(sim, PID, printerrors);
    bool ReadyQcheck = notInReadyQ(sim, PID, printerrors);
    bool DiskQcheck = notInDiskQ(sim, PID, printerrors);
    bool Diskcheck = notInDisk(sim, PID, printerrors);
    
    //PID DNE return
    return RAMcheck && CPUcheck && ReadyQcheck && Diskcheck && DiskQcheck;
}

void emptyTests() {
    //maybe overkill for these?
    SimOS test (OS_DISKS, OS_RAM/100000, OS_SIZE); //OS OBJECT FAILS TO CREATE
    
    bool processCheck = false;
    bool readyQRAMcheck = true;
    bool existingDisksCheck = true;
    bool nonExistingDiskCheck = true;
    if (processCheck) {
        bool result = (
            (test.GetCPU() == NO_PROCESS) &&        //NO_PROCESS in CPU
            (test.NewProcess(1,1) == false) &&      //new process fails to create
            (test.SimFork() == false)               //fork fails 
        );
        if (result) {
            assert(result);
            std::cout << "EMPTY TEST 0: PASS" << std::endl;
        } else {
            std::cout << "EMPTY TEST 0: FAIL" << std::endl;
        }
    }
    if (readyQRAMcheck) {
        bool result = (
            (test.GetReadyQueue().empty()) &&       //readyQ empty
            (test.GetMemory().empty())              //RAM empty
        );
        if (result) {
            assert(result);
            std::cout << "EMPTY TEST 1: PASS" << std::endl;
        } else {
            std::cout << "EMPTY TEST 1: FAIL" << std::endl;
        }
    }
    if (existingDisksCheck) {
        bool result = (
            (test.GetDisk(0).PID == 0) && 
            (test.GetDisk(0).fileName == "") &&
            (test.GetDisk(1).PID == 0) &&
            (test.GetDisk(1).fileName == "") &&
            (test.GetDisk(2).PID == 0) &&
            (test.GetDisk(2).fileName == "") &&
            (test.GetDiskQueue(0).empty()) &&
            (test.GetDiskQueue(1).empty()) &&
            (test.GetDiskQueue(2).empty())             
        );
        if (result) {
            assert(result);
            std::cout << "EMPTY TEST 2: PASS" << std::endl;
        } else {
            std::cout << "EMPTY TEST 2: FAIL" << std::endl;
        }
    }
    if (nonExistingDiskCheck) {
        bool result = (
            (test.GetDisk(3).PID == 0) &&           //DISK NUMBER DNE
            (test.GetDisk(3).fileName == "") &&     //DISK NUMBER DNE
            (test.GetDiskQueue(3).empty())          //DISK NUMBER DNE
        );
        if (result) {
            assert(result);
            std::cout << "EMPTY TEST 3: PASS" << std::endl;
        } else {
            std::cout << "EMPTY TEST 3: FAIL" << std::endl;
        }
    }
    assert(processDNE(test,1,1));
}

void waitTests() {
    bool noChildrenTest = true;
    bool parentWaitChildRuns = true;
    bool parentWaitChildExits = true;
    bool parentWaitChildWaits = true;
    bool parentWaitKillsZombie = true;
    if (noChildrenTest) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000, 1000);            //2
        test.SimWait();                         //2 shouldn't get blocked -> keeps using CPU
        assert(test.GetCPU() == 2);

        auto readyQ = test.GetReadyQueue();     //only 1 in ready queue
        
        bool result = readyQ[0] == 1;
        
        if (result) {
            assert(result);
            std::cout << "WAIT TEST 1: PASS" << std::endl;
        } else {
            std::cout << "WAIT TEST 1: FAIL" << std::endl;
        }
    }
        
    if (parentWaitChildRuns) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000, 999);             //2 
        test.NewProcess(1000, 1000);            //3 parent
        test.SimFork();                         //4 child of 3
        while (test.GetCPU() != 3) {
            test.DiskReadRequest(0,"abc");
        }
        assert(test.GetCPU() == 3);
        test.SimWait();                         //3 is blocked
        assert(test.GetCPU() == 4);             //4 should start

        auto readyQ = test.GetReadyQueue();     //1 & 2 in ready queue
        std::unordered_set<int> checkMe (readyQ.begin(), readyQ.end());
        std::unordered_set<int> expected {1,2};
        assert(checkMe == expected);
        
        //ram should be ordered like this
        std::vector<int> orderedPID {1,2,3,4};
        auto RAM = test.GetMemory();
        int i = 0;
        bool result = true;
        for (auto memItem : RAM) {
            result = result && (memItem.PID == orderedPID[i]);
            assert(memItem.PID == orderedPID[i++]);
        }
        
        if (result) {
            assert(result);
            std::cout << "WAIT TEST 2: PASS" << std::endl;
        } else {
            std::cout << "WAIT TEST 2: FAIL" << std::endl;
        }
    }
        
    if (parentWaitChildExits) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000, 1000);            //2 parent
        test.NewProcess(1001, 999);             //3 
        test.SimFork();                         //4 child
        while(test.GetCPU() != 2) {
            test.DiskReadRequest(0,"abc");
        }
        
        assert(test.GetCPU() == 2);
        test.SimWait();                         //2 waits
        assert(test.GetCPU() == 4);
        
        //readyQ and RAM test BEFORE child exits
        auto readyQ = test.GetReadyQueue();     //1, 3 in ready queue
        std::unordered_set<int> checkMe (readyQ.begin(), readyQ.end());
        std::unordered_set<int> expected {1,3};
        assert(checkMe == expected);
        //ram should be ordered like this
        std::vector<int> orderedPID {1,2,3,4};
        auto RAM = test.GetMemory();
        int i = 0;
        bool result = true;
        for (auto memItem : RAM) {
            result = result && (memItem.PID == orderedPID[i]);
            assert(memItem.PID == orderedPID[i++]);
        }

        test.SimExit();                         //child exits
        assert(processDNE(test, 4, 1));         //make sure child DNE
        assert(test.GetCPU() == 2);             //parent resumes

        //readyQ and RAM test AFTER child exits
        auto readyQ2 = test.GetReadyQueue();     //1, 3 in ready queue
        std::unordered_set<int> checkMe2 (readyQ2.begin(), readyQ2.end());
        std::unordered_set<int> expected2 {1,3};
        assert(checkMe2 == expected2);
        //ram should be ordered like this
        std::vector<int> orderedPID2 {1,2,3};
        auto RAM2 = test.GetMemory();
        int i2 = 0;
        for (auto memItem : RAM2) {
            result = result && (memItem.PID == orderedPID2[i2]);
            assert(memItem.PID == orderedPID2[i2++]);
        }

        if (result) {
            assert(result);
            std::cout << "WAIT TEST 3: PASS" << std::endl;
        } else {
            std::cout << "WAIT TEST 3: FAIL" << std::endl;
        }
    }
    if (parentWaitChildWaits) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000, 1000);            //2 parent
        test.NewProcess(1001, 999);             //3 
        test.SimFork();                         //4 child
        assert(test.GetCPU() == 2);
        test.SimWait();                         //parent waits, child runs
        assert(test.GetCPU() == 4);
        test.SimWait();                         //child waits, but no children -> child keeps running
        
        //2 is blocked only 1 & 3 should be in ready queue and 4 in CPU
        auto readyQ = test.GetReadyQueue();
        std::unordered_set<int> checkMe (readyQ.begin(), readyQ.end());
        std::unordered_set<int> expected {1,3};
        assert(checkMe == expected);
        assert(test.GetCPU() == 4);

        //ram should be ordered like this
        std::vector<int> orderedPID {1,2,3,4};
        auto RAM = test.GetMemory();
        int i = 0;
        bool result = true;
        for (auto memItem : RAM) {
            result = result && (memItem.PID == orderedPID[i]);
            assert(memItem.PID == orderedPID[i++]);
        }
        
        if (result) {
            assert(result);
            std::cout << "WAIT TEST 4: PASS" << std::endl;
        } else {
            std::cout << "WAIT TEST 4: FAIL" << std::endl;
        }
    }
    if (parentWaitKillsZombie) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000, 1000);            //2 parent
        test.SimFork();                         //3 child
        test.NewProcess(1000, 999);             //4

        test.DiskReadRequest(0, "abc");         //kick out parent
        assert(test.GetCPU() == 3);             //child RUNS
        test.SimExit();                         //child exits -> 4 runs, 1 in ready-q 
        assert(processDNE(test, 3, 1));         //assert child DNE

        assert(test.GetCPU() == 4);

        test.NewProcess(500,996);               //5    half space of 3
        test.NewProcess(501,997);               //6
        test.NewProcess(500,998);               //7     half space of 3
        assert(test.GetCPU() == 4);

        test.DiskJobCompleted(0);               //parent comes back
        assert(test.GetCPU() == 2);
        test.SimWait();
        assert(test.GetCPU() == 2);             //kill zombie with wait -> parent stays running
        
        //ram should be ordered like this
        std::vector<int> orderedPID {1,2,4,5,6,7};
        auto RAM2 = test.GetMemory();
        int i2 = 0;
        bool result = true;
        for (auto memItem : RAM2) {
            result = result && (memItem.PID == orderedPID[i2]);
            assert(memItem.PID == orderedPID[i2++]);
        }

        auto readyQ = test.GetReadyQueue();
        std::unordered_set<int> checkMe (readyQ.begin(), readyQ.end());
        std::unordered_set<int> expected {1,4,5,6,7};
        result = (checkMe == expected) && result;         
        if (result) {
            assert(result);
            std::cout << "WAIT TEST 5: PASS" << std::endl;    
        } else {
            std::cout << "WAIT TEST 5: FAIL" << std::endl;
        }
    } 
}

void OStests() {
    SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
    test.SimFork();
    test.SimWait();
    test.SimExit();
    bool result = true;
    //only OS in memory
    auto RAM = test.GetMemory();
    for (auto memItem : RAM) {
        result = result && (memItem.PID == 1);
        assert(memItem.PID == 1);
    }
    //CPU only running OS & ready-q is empty
    assert(test.GetCPU() == 1);
    assert(test.GetReadyQueue().empty());
    
    //all disk requests should ignore
    test.DiskReadRequest(DISK_0, "abc");
    test.DiskReadRequest(DISK_1, "abcd");
    test.DiskReadRequest(DISK_2, "abcde");
    for (int i = 0 ; i < OS_DISKS ; ++i) {
        result = result && (test.GetDiskQueue(i).empty());
        assert(test.GetDiskQueue(i).empty());
    }
    if (result) {
        assert(result);
        std::cout << "OS TEST: PASS" << std::endl;
    } else {
        std::cout << "OS TEST: FAIL" << std::endl;
    }

}

void exitTests() {
    bool simpleExit = true;
    bool forkWaitExit = true;
    bool orphanExit = true;
    bool nestedOrphanExit = true;

    if (simpleExit) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1

        test.NewProcess(10000,1000);           //2
        test.NewProcess(10000,1000);           //3
        assert(test.GetCPU() == 2 || test.GetCPU() == 3);
        
        test.SimExit();
        assert(test.GetCPU() == 2 || test.GetCPU() == 3);
        
        test.SimExit();
        bool result = true;
        result = result && (processDNE(test,2,1) && processDNE(test,3,1));
        if (result) {
            assert(result);
            std::cout << "EXIT TEST 1: PASS" << std::endl;
        } else {
            std::cout << "EXIT TEST 1: FAIL" << std::endl;
        }
    }

    if (forkWaitExit) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000,1000);             //2 parent
        test.SimFork();                         //3 child
        
        assert(test.GetCPU() == 2);
        test.SimWait();                         //3 starts running
        assert(test.GetCPU() == 3);
        test.SimExit();                         //2 runs again
        
        bool result = true;
        result = result && (processDNE(test, 3, 1)) && (test.GetCPU() == 2);
        if (result) {
            assert(result);
            std::cout << "EXIT TEST 2: PASS" << std::endl;
        } else {
            std::cout << "EXIT TEST 2: FAIL" << std::endl;
        }
    }

    if (orphanExit) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000,1000);             //2 parent
        test.SimFork();                         //3 child 
        test.SimFork();                         //4 child
        test.SimFork();                         //5 child
        while(test.GetCPU()!=2) {
            test.DiskReadRequest(0,"abc");
        }
        assert(test.GetCPU()==2);
        test.SimExit();

        bool result = (
            processDNE(test,3,1) &&
            processDNE(test,4,1) && 
            processDNE(test,5,1)
        );

        if (result) {
            assert(result);
            std::cout << "EXIT TEST 3: PASS" << std::endl;
        } else {
            std::cout << "EXIT TEST 3: FAIL" << std::endl;
        }
    }

    if (nestedOrphanExit) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000,1000);             //2 parent
        test.SimFork();                         //3 child of 2
        test.SimFork();                         //4 child of 2
        
        test.DiskReadRequest(0,"abc");          //2 parent gets blocked

        assert(test.GetCPU() == 3 || test.GetCPU() == 4);   //3 or 4 runs
        test.SimFork();                         //5 grandchild of 2
        test.SimFork();                         //6 grandchild of 2
        test.SimFork();                         //7 grandchild of 2
        

        test.DiskJobCompleted(0);               //2 back in ready-queue
        while (test.GetCPU() != 2) {
            test.DiskReadRequest(0, "abc");
        }

        assert(test.GetCPU() == 2);
        test.SimExit();                         //parent exits -> MANY orphans created (3,4,5,6,7) & should be gone
        bool result = true;
        for (int i = 3; i < 8; ++i) {
            result = result && processDNE(test,i,1);
            assert(processDNE(test,i,1));       //assert processes (3,4,5,6,7) DNE since parent exited
        }

        if (result) {
            assert(result);
            std::cout << "EXIT TEST 4: PASS" << std::endl;
        } else {
            std::cout << "EXIT TEST 4: FAIL" << std::endl;
        }
    }
}

void diskTests() {
    bool ReadRequestWorks = true;
    bool ReadRequstJobComplete = true;
    if (ReadRequestWorks) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000,1000);             //2
        test.DiskReadRequest(0, "abc");         //2 goes read

        bool result = (
            test.GetCPU() == 1 &&
            test.GetDisk(0).PID == 2
        );

        if (result) {
            assert(result);
            std::cout << "DISK TEST 1: PASS" << std::endl;
        } else {
            std::cout << "DISK TEST 1: FAIL" << std::endl;
        }
    }

    if (ReadRequstJobComplete) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE); //1
        test.NewProcess(1000,1000);             //2
        test.DiskReadRequest(0, "abc");         //2 goes read
        test.NewProcess(1001,1001);             //3

        bool result = (
            test.GetCPU() == 3 &&               //3 higher priority than 1
            test.GetDisk(0).PID == 2            //2 using Disk 0
        );

        assert(result);

        test.DiskJobCompleted(0);               //2 comes back
        test.DiskReadRequest(0, "abc");         //3 goes read

        result = (
            result &&
            test.GetCPU() == 2 &&               //2 in CPU
            test.GetDisk(0).PID == 3 &&         //3 in Disk 0
            notInDiskQ(test, 3, 1) &&           //3 NOT in any Disk queue
            notInCPU(test,3,1)                  //3 NOT in CPU
        );

        if (result) {
            assert(result);
            std::cout << "DISK TEST 2: PASS" << std::endl;
        } else {
            std::cout << "DISK TEST 2: FAIL" << std::endl;
        }
    }
}

void RAMTests() {
    bool contiguousPID = true;
    bool worstFitTest = true;
    bool forkRAM = true;
    bool OOMtest = true;
    if (contiguousPID) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE);
        //contiguous PID test
        std::vector<int> PIDs {1,2,3,4,5,6};

        int testPriority = 10;
        int testSize = 1;
        for (int i = 2 ; i <= 6; i++) {
            test.NewProcess(testSize, testPriority);
            assert(test.GetCPU() == i);
            testSize += testSize;
            testPriority += testPriority;
        }
        auto RAMcopy = test.GetMemory();
        std::unordered_set<int> PIDs_RAM;
        int i = 0;
        bool result = true;
        for (auto items : RAMcopy) {
            result = (result && (items.PID == PIDs[i]));
            assert(items.PID == PIDs[i++]);
        }
        assert(test.GetCPU() == 6); 
        if (result) {
            assert(result);
            std::cout << "RAM TEST 1: PASS" << std::endl;
        } else {
            std::cout << "RAM TEST 1: FAIL" << std::endl;
        }
    }
    if (worstFitTest) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE);     //1

        test.NewProcess(26'000'000'000, 10000);     //2
        test.NewProcess(4'000'000'000, 9999);       //3
        //PID order should be : [ 1 2 3 ]

        assert(test.GetCPU() == 2);

        test.SimExit();                             //2 exits hole created between 1 & 3 -> worst fit on left of process 3
        //PID order should be : [ 1 3 ]

        test.NewProcess(2'100'000'000, 999);                    //4, placed between 1 and 3
        //PID order should be : [ 1 4* 3 ]
        test.NewProcess(3'000'000'000, 998);                    //5, placed after 3
        //PID order should be : [ 1 4 3 5* ]
        test.NewProcess(4'000'000'000, 997);                    //6, placed between 4 and 3
        //PID order should be : [ 1 4 6* 3 5 ]
        test.NewProcess(5'000'000'000, 996);                    //7, placed after 5
        //PID order should be : [ 1 4 6 3 5 7* ]

        assert(test.GetCPU() == 3);
        test.SimExit();                                         //3 EXITs, hole in favor of middle now
        //PID order should be : [ 1 4 6 5 7 ]                   
        test.NewProcess(7'900'000'001, 995);                    //8, hole if favor of end by 1 byte
        //PID order should be : [ 1 4 6 8* 5 7 ]
        
        test.NewProcess(3, 1);                                  //9, hole in favor of middle now
        //PID order should be : [ 1 4 6 8 5 7 9* ]

        test.NewProcess(1, 1);
        //PID order should be : [ 1 4 6 8 10* 5 7 9 ]           //10

        test.NewProcess(1, 1);
        //PID order should be : [ 1 4 6 8 10 11* 5 7 9 ]        //11

        test.NewProcess(1, 1);
        //PID order should be : [ 1 4 6 8 10 11 12* 5 7 9 ]     //12

        test.NewProcess(1, 1);
        //PID order should be : [ 1 4 6 8 10 11 12 5 7 9 13* ]  //13

        std::vector<int> orderedPID {1,4,6,8,10,11,12,5,7,9,13};
        auto RAM = test.GetMemory();
        int i = 0;
        bool result = true;
        for (auto memItem : RAM) {
            result = result && (memItem.PID == orderedPID[i]);
            assert(memItem.PID == orderedPID[i++]);
        }

        if (result) {
            assert(result);
            std::cout << "RAM TEST 2: PASS" << std::endl;
        } else {
            std::cout << "RAM TEST 2: FAIL" << std::endl;
        }
    }
    if (forkRAM) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE);     //1
        test.NewProcess(20'000'000'000, 500);       //2
        test.SimFork();                             //3

        test.NewProcess(1, 1);                      //4 

        while(test.GetCPU() == 2 || test.GetCPU() == 3) {
            test.SimExit();
        }

        assert(test.GetCPU() == 4);

        test.NewProcess(15'000'000'000, 500);       //5
        test.SimFork();                             //6 -> 10GB hole on left & 14GB hole on right
        //PID order should be : [ 1 5 6 4 ] 

        test.NewProcess(7'000'000'000, 100000);     //7 -> 10GB hole on left & 7GB hole on right
        //PID order should be : [ 1 5 6 4 7 ] 
        test.SimFork();                             //8 -> 3GB hole on left & 7GB hole on right
        //PID order should be : [ 1 5 6 8 4 7 ]

        std::vector<int> orderedPID {1,5,6,8,4,7};
        auto RAM = test.GetMemory();
        int i = 0;
        bool result = true;
        for (auto memItem : RAM) {
            result = result && (memItem.PID == orderedPID[i]);
            assert(memItem.PID == orderedPID[i++]);
        }

        if (result) {
            assert(result);
            std::cout << "RAM TEST 3: PASS" << std::endl;
        } else {
            std::cout << "RAM TEST 3: FAIL" << std::endl;
        }
    }
    if (OOMtest) {
        SimOS test (OS_DISKS, OS_RAM, OS_SIZE);     //1

        test.NewProcess(27'000'000'000, 100);       //2 (HALF CAPACITY)
        test.SimFork();                             //3 (MAX CAPACITY)

        test.SimWait();
        bool result = true;
        result = result && (!test.NewProcess(1,1)); //OOM 
        assert(result);

        auto memory = test.GetMemory();
        assert(memory.size() == 3);                 //3 processes in RAM

        test.SimExit();                             //2 or 3 exits

        result = result && (test.NewProcess(27'000'000'001, 100) == false); //fails OOM

        if (result) {
            assert(result);
            std::cout << "RAM TEST 4: PASS" << std::endl;
        } else {
            std::cout << "RAM TEST 4: FAIL" << std::endl;
        }
    }
}

int main() {
    //same priority process does NOT kick out current process in CPU
    OStests();      //1 test
    std::cout << "-----------------------" << std::endl;
    emptyTests();   //3 test
    std::cout << "-----------------------" << std::endl;    
    RAMTests();     //4 tests
    std::cout << "-----------------------" << std::endl;
    diskTests();    //2 tests
    std::cout << "-----------------------" << std::endl;
    exitTests();    //4 tests
    std::cout << "-----------------------" << std::endl;
    waitTests();    //5 tests 
    
}