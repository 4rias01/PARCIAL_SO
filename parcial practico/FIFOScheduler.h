#ifndef FIFO_SCHEDULER_H
#define FIFO_SCHEDULER_H

#include "Process.h"
#include <queue>
#include <vector>
#include <deque>

using namespace std;

class FIFOScheduler
{
    public:
        FIFOScheduler(int currentTime, deque<Process*> processes);
        void run();
        int getCurrentTime();

    private:
        // incomingProcesses holds processes that haven't arrived yet (sorted by arrivalTime)
        deque<Process*> incomingProcesses;
        int currentTime;
};

#endif // FIFO_SCHEDULER_H