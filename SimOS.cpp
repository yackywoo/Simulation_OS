//Jacky Qiu
//----------------------------------
#include "SimOS.h"

SimOS::SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned long long sizeOfOS) : 
    numberOfDisks_{numberOfDisks},
    amountOfRAM_{amountOfRAM},
    sizeOfOS_{sizeOfOS},
    OSadded_{false},
    trackPID_{0},
    currentProcess{nullptr},
    remainingRAM_{amountOfRAM},
    currProcessInDisk{static_cast<size_t>(numberOfDisks)},
    waitingQueueInDisk{static_cast<size_t>(numberOfDisks)} {
        
    OSadded_ = NewProcess(sizeOfOS_, 0);
}

bool SimOS::NewProcess( unsigned long long size, int priority ) {
    //OS case
    if (OSadded_ == false && trackPID_ == 0 && size == sizeOfOS_ && fitInRAM(size)) {
        Process newProcess (trackPID_, size, priority, nullptr);
        processList.push_back(newProcess);
        Scheduler.push({priority, &processList.back()});
        updateCurrProcess();
        return true;
    }
    
    //non OS case
    if (OSadded_ && fitInRAM(size) && !RAM_.empty()) {
        Process newProcess (trackPID_, size, priority, nullptr);
        processList.push_back(newProcess);
        Scheduler.push({priority, &processList.back()});
        updateCurrProcess();
        return true;
    }
    
    return false;
}

bool SimOS::fitInRAM(unsigned long long size) {
    //first process (OS) case
    if (!OSadded_ && RAM_.empty() && size <= amountOfRAM_) {
        RAM_.push_back({0, sizeOfOS_, ++trackPID_});
        remainingRAM_ -= size;
        return true;
    }
    //process too large
    if (size > remainingRAM_) {
        return false;
    } 

    int worstFit = findWorstFitIndex();
    if (worstFit > 0) {
        MemoryItem neighbor = RAM_[worstFit-1];

        //new address = neighborAddress + neighborSize
        auto newAddress = neighbor.itemAddress + neighbor.itemSize;
        MemoryItem newProcess {newAddress, size, ++trackPID_};
        RAM_.insert(RAM_.begin() + worstFit, newProcess);
        remainingRAM_ -= size;

        return true;
    } 
        
    return false;
}

void SimOS::updateCurrProcess() {
    if (!Scheduler.empty()) {

        auto [nextPriority, ptrNextProcess] = Scheduler.top();

        //no current process
        if (!currentProcess) {
            currentProcess = ptrNextProcess;
            Scheduler.pop();
            return;
        }

        //next process GREATER THAN priority of current case
        if (nextPriority > currentProcess->priority_) {
            Scheduler.pop();
            //reschedule current process if real process
            if (currentProcess->PID_ != NO_PROCESS) {
                Scheduler.push({currentProcess->priority_, currentProcess});
            }
            currentProcess = ptrNextProcess;
            return;
        }
        //next process LESS THAN or EQUAL TO priority of current case -> do nothing
    }
}

bool SimOS::parentFork() {
    if (currentProcess->PID_ == 1 || currentProcess->PID_ == NO_PROCESS || !currentProcess) {
        return false;
    }
    auto parentProcess = currentProcess;
    bool childFitsInRAM = fitInRAM(parentProcess->size_);
    if (childFitsInRAM) {
        //create child process with parent's PID
        Process childProcess (trackPID_, parentProcess->size_, parentProcess->priority_, parentProcess);
        processList.push_back(childProcess);
        Scheduler.push({childProcess.priority_, &processList.back()});
        parentProcess->childrenProcesses_.insert(&processList.back());
        return true;
    }
    return false;
}

int SimOS::findWorstFitIndex() {
    unsigned long long maxHoleSize = 0;
    int worstFitIndex = 0;
    //RAM will never be empty since OS always running assuming it was successfully added to RAM
    for (int i = 1; i < RAM_.size(); ++i) {
        auto prevMemItem = RAM_[i-1];
        auto currMemItem = RAM_[i];
        auto prevAddrEnd = prevMemItem.itemAddress + prevMemItem.itemSize;

        //contiguous process case (no hole)
        if (prevAddrEnd == currMemItem.itemAddress) {
            continue;
        //non-contiguous process case
        } else {
            auto holeSize = currMemItem.itemAddress - prevAddrEnd;
            if (holeSize > maxHoleSize) {
                maxHoleSize = holeSize;
                worstFitIndex = i;
            }
        }
    }
    //check hole (from last process) TO (end of RAM)
    auto lastMemItem = RAM_.back();
    auto lastAddrEnd = lastMemItem.itemAddress + lastMemItem.itemSize;
    auto lastHole = amountOfRAM_ - lastAddrEnd;
    
    if (lastHole > maxHoleSize) {
        return RAM_.size();
    }

    //ELSE (lastHole <= maxHoleSize) THEN use earlier index
    return worstFitIndex;
}

bool SimOS::SimFork() {
    if (OSadded_ == false || currentProcess->PID_ == 1 || currentProcess->PID_ == NO_PROCESS || !currentProcess) {
        return false;
    }
    updateCurrProcess();
    bool childCreated = parentFork();
    if (childCreated) {
        updateCurrProcess();
        return true;
    }
    return false;
}

void SimOS::killFamilyTree(Process* ptr) {
    //base case
    if (ptr == nullptr) {
        return;
    }

    for (auto childrenPtrs : (ptr->childrenProcesses_)) {
        //keep traversing down family tree
        killFamilyTree(childrenPtrs);
        //kill children & grandchildren
        removeFromRAM(childrenPtrs->PID_);
        removeFromScheduler(childrenPtrs);
        removeFromAnyDisk(childrenPtrs);
        removeFromProcessList(childrenPtrs);
    }
    ptr->childrenProcesses_.clear();
}

void SimOS::SimExit() {
    if (OSadded_ == false || currentProcess->PID_ == 1 || currentProcess->PID_ == NO_PROCESS || !currentProcess) {
        return;
    }
    updateCurrProcess();
    bool isChild = currentProcess->parent_ != nullptr;
    bool isParent = !currentProcess->childrenProcesses_.empty();
    if (isChild) {
        auto child = currentProcess;
        auto parent = child->parent_;
        bool waitingParentExists = waitingParents.count(parent);
        if (waitingParentExists) {
            //waiting parent + child exit case
 
            parent->childrenProcesses_.erase(child);
            //remove child process object and clean up
            removeFromRAM(child->PID_);
            removeFromScheduler(child);
            removeFromAnyDisk(child);
            removeFromProcessList(child);

            //parent gets out of waiting
            waitingParents.erase(parent);
            Scheduler.push({parent->priority_, parent});
            
            currentProcess = nullptr;
            updateCurrProcess();
            return;
        } else if (!waitingParentExists) {
            //non-waiting parent + child exit case (ZOMBIE)
            
            //create zombie process
            parent->childrenProcesses_.erase(child);
            parent->zombieProcesses_.insert(child);
            //remove child process from RAM and start next process
            removeFromRAM(child->PID_);
            removeFromScheduler(child);
            removeFromAnyDisk(child);
            
            currentProcess = nullptr;
            updateCurrProcess();
            return;
        }
    } else if (isParent) {
        //parent exit -> kill all children & grandchildren then exit (ORPHAN)
        auto parent = currentProcess;
        killFamilyTree(parent);
        
        //remove parent
        removeFromRAM(parent->PID_);
        removeFromScheduler(parent);
        removeFromAnyDisk(parent);
        removeFromProcessList(parent);
        waitingParents.erase(parent);
        
        currentProcess = nullptr;
        updateCurrProcess();
        return;
    } else {
        //non-parent , non-child case
        removeFromRAM(currentProcess->PID_);
        removeFromScheduler(currentProcess);
        removeFromAnyDisk(currentProcess);
        removeFromProcessList(currentProcess);
        
        currentProcess = nullptr;
        updateCurrProcess();
    }
}

void SimOS::removeFromScheduler(Process* ptr) {
    std::priority_queue<std::tuple<int, Process*>> newScheduler;

    while (!Scheduler.empty()) {
        auto [nextPriority, ptrNextProcess] = Scheduler.top();
        if (ptrNextProcess != ptr) {
            newScheduler.push({nextPriority, ptrNextProcess});
        }
        Scheduler.pop();
    }
    Scheduler = std::move(newScheduler);   
}

void SimOS::removeFromProcessList(Process* ptr) {
    for (auto listptr = processList.begin(); listptr != processList.end(); ++listptr) {
        Process* addressOfProcess = &(*listptr);
        if (addressOfProcess == ptr) { 
            processList.erase(listptr);
            return;
        }
    }
}

void SimOS::removeFromRAM(int PID) {
    for (auto ptr = RAM_.begin(); ptr != RAM_.end(); ++ptr) {
        if (ptr->PID == PID) {
            remainingRAM_ += ptr->itemSize;
            RAM_.erase(ptr);
            return;
        }
    }
}

void SimOS::removeFromAnyDisk(Process* ptr) {
    removeFromDisk(ptr);
    removeFromDiskQueue(ptr);
    ptr->currentDisk_ = -1;
}

void SimOS::removeFromDisk(Process* ptr) {
    if (ptr == nullptr) {
        return;
    }
    
    int currDisk = ptr->currentDisk_;
    if (currDisk == -1) {
        return;
    }

    //get ptr of process using current disk
    auto currProcInDisk = std::get<1>(currProcessInDisk[currDisk]);
    if (ptr == currProcInDisk) {
        currProcessInDisk[currDisk] = {FileReadRequest{}, nullptr};
        
        //load next process if non-empty queue
        if (!waitingQueueInDisk[currDisk].empty()) {
            auto [nextRequest, nextProcessPtr] = waitingQueueInDisk[currDisk].front();
            waitingQueueInDisk[currDisk].pop();
            currProcessInDisk[currDisk] = {nextRequest, nextProcessPtr};
        }
    }
}

void SimOS::removeFromDiskQueue(Process* ptr) {
    if (ptr == nullptr) {
        return;
    }
    
    int currDisk = ptr->currentDisk_;
    if (currDisk == -1) {
        return;
    }

    std::queue<std::tuple<FileReadRequest,Process*>>* ptrToWaitingQ = &waitingQueueInDisk[currDisk];
    std::queue<std::tuple<FileReadRequest,Process*>> replacementQueue;
    while (!ptrToWaitingQ->empty()) {
        auto [request, ptrProcess] = ptrToWaitingQ->front();
        ptrToWaitingQ->pop();
        if (ptrProcess != ptr) {
            replacementQueue.push({request, ptrProcess});
        }
    }
    //replace waiting queue 
    waitingQueueInDisk[currDisk] = replacementQueue;
}

void SimOS::SimWait() {
    //if not parent or invalid process, do nothing
    if (OSadded_ == false || currentProcess->PID_ == 1 || currentProcess->PID_ == NO_PROCESS || !currentProcess || currentProcess->childrenProcesses_.empty()) {
        return;
    } 

    auto parent = currentProcess;
    if (parent->zombieProcesses_.empty()) {
        //no zombie processes case -> parent waits
        waitingParents.insert(parent);
        currentProcess = nullptr;
        updateCurrProcess();
    } else {
        //zombies exist case
        auto zombieSet = parent->zombieProcesses_;
        Process* zombieProcess = *(zombieSet.begin());
        //clean up zombie process remanents
        parent->zombieProcesses_.erase(zombieProcess);
        removeFromProcessList(zombieProcess);
    }
}

int SimOS::GetCPU() {
    if (OSadded_ == false) {
        return NO_PROCESS;
    }

    updateCurrProcess();
    return currentProcess->PID_;
}

std::vector<int> SimOS::GetReadyQueue() {
    if (OSadded_ == false || Scheduler.empty()) {
        return {};
    }
    
    auto schedulerCopy = Scheduler;
    std::vector<int> readyQ (schedulerCopy.size()); 
    int i = 0;
    while (!schedulerCopy.empty()) {
        auto nextProcess = std::get<1>(schedulerCopy.top());
        readyQ[i++] = nextProcess->PID_;

        schedulerCopy.pop();
    }
    return readyQ;
}

MemoryUse SimOS::GetMemory() {
    if (OSadded_ == false) {
        return {};
    }

    return RAM_;
}

void SimOS::DiskReadRequest( int diskNumber, std::string fileName ) {
    if (!currentProcess || !OSadded_ || currentProcess->PID_ == 1 || currentProcess->PID_ == NO_PROCESS || diskNumber >= numberOfDisks_) {
        return;
    } 
    updateCurrProcess();
    
    //first check if disk already being used
    FileReadRequest requestMade {currentProcess->PID_, fileName};
    bool noCurrProcessInDisk = std::get<1>(currProcessInDisk[diskNumber]) == nullptr;
    if (noCurrProcessInDisk) {
        //make current process run in disk
        currProcessInDisk[diskNumber] = {requestMade, currentProcess};
    } else {
        waitingQueueInDisk[diskNumber].push({requestMade, currentProcess});
    }

    currentProcess->currentDisk_ = diskNumber;
    //start next process
    currentProcess = nullptr;
    updateCurrProcess();
}

void SimOS::DiskJobCompleted( int diskNumber ) {
    if (OSadded_ == false || diskNumber >= numberOfDisks_) {
        return;
    } 
    //only complete job if disk is busy
    bool noCurrProcessInDisk = std::get<1>(currProcessInDisk[diskNumber]) == nullptr;
    if (noCurrProcessInDisk) {
        return;
    }

    //get finished request, and clear
    auto [finishedRequest, finishedProcessPtr] = currProcessInDisk[diskNumber];
    finishedProcessPtr->currentDisk_ = -1;
    currProcessInDisk[diskNumber] = {FileReadRequest{}, nullptr};
    
    //load next process from queue if not empty queue
    if (!waitingQueueInDisk[diskNumber].empty()) {
        auto [nextRequest, nextProcessPtr] = waitingQueueInDisk[diskNumber].front();
        waitingQueueInDisk[diskNumber].pop();
        
        currProcessInDisk[diskNumber] = {nextRequest, nextProcessPtr};
    }

    //add finished process to sched and update current process
    Scheduler.push({finishedProcessPtr->priority_, finishedProcessPtr});
    updateCurrProcess();
}

FileReadRequest SimOS::GetDisk(int diskNumber) {
    if (OSadded_ == false || diskNumber >= numberOfDisks_) {
        return FileReadRequest{};
    } 
    //only check request if non-empty
    bool noCurrProcessInDisk = std::get<1>(currProcessInDisk[diskNumber]) == nullptr ;
    if (noCurrProcessInDisk) {
        return FileReadRequest{};
    }

    auto currentRequest = std::get<0>(currProcessInDisk[diskNumber]);
    return currentRequest;
}

std::queue<FileReadRequest> SimOS::GetDiskQueue( int diskNumber ) {
    if (OSadded_ == false || diskNumber >= numberOfDisks_) {
        return {};
    }
    //only send copy if queue is non-empty
    bool noWaitingProcesses = waitingQueueInDisk[diskNumber].empty();
    if (noWaitingProcesses) {
        return {};
    }

    auto queueCopy = waitingQueueInDisk[diskNumber];
    std::queue<FileReadRequest> result;
    while (!queueCopy.empty()) {
        auto [readRequest, ignore] = queueCopy.front();
        queueCopy.pop();
        result.push(readRequest);
    }

    return result;
}
