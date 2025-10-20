#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include "Process.h"
#include <queue>
#include <vector>
#include <deque>

using namespace std;

class RoundRobinScheduler
{
    public:
        RoundRobinScheduler(int timeQuantum, int currentTime, deque<Process*> processes);
        void run();
        int getCurrentTime();

    private:
        // Use a stable list: processes remain in their positions; arrivals are appended
        std::vector<Process*> processList;
        // incomingProcesses holds processes that haven't arrived yet (sorted by arrivalTime)
        deque<Process*> incomingProcesses;
        int timeQuantum;
        int currentTime;
};

#endif // ROUND_ROBIN_H