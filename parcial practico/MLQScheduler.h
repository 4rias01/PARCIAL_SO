#ifndef MLQ_SCHEDULER_H
#define MLQ_SCHEDULER_H


#include "Process.h"
#include "FIFOScheduler.h"
#include "RoundRobinScheduler.h"
#include <deque>
#include <vector>
#include <string>

using namespace std;

class MLQScheduler
{
    public:
        MLQScheduler(int currentTime);
        void readFile(string filename);
        void initializeQueues();
        void run();
        void deleteInstances();
    void printReport();
    const vector<Process*>& getProcesses() const { return processes; }

    private:
        RoundRobinScheduler* rr;
        RoundRobinScheduler* rr2;
        FIFOScheduler* fifoScheduler;

        deque<Process*> queue1; // Cola para el nivel 1
        deque<Process*> queue2; // Cola para el nivel 2
        deque<Process*> queue3; // Cola para el nivel 3

        vector<Process*> processes; // Todos los procesos para el resumen final

        int currentTime;
};

#endif // MLQ_SCHEDULER_H