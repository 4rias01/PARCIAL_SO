#include "RoundRobinScheduler.h"
#include <iostream>
#include <algorithm>

using namespace std;

RoundRobinScheduler::RoundRobinScheduler(int timeQuantum, int currentTime, deque<Process*> processes) {
    this->timeQuantum = timeQuantum;
    this->currentTime = currentTime;

    // Separar procesos entrantes (ordenados por arrivalTime) y en caso de empate por name
    sort(processes.begin(), processes.end(), [](Process* a, Process* b) {
        if (a->getArrivalTime() != b->getArrivalTime()) return a->getArrivalTime() < b->getArrivalTime();
        return a->getName() < b->getName();
    });

    // Inicialmente, incomingProcesses contiene todos los procesos ordenados por llegada
    for (auto process : processes) {
        incomingProcesses.push_back(process);
    }
    // processList comienza vacío; iremos añadiendo procesos según llegan
}

void RoundRobinScheduler::run() {
    // Añadir al processList los procesos que ya han llegado al tiempo inicial
    while (!incomingProcesses.empty() && incomingProcesses.front()->getArrivalTime() <= currentTime) {
        processList.push_back(incomingProcesses.front());
        incomingProcesses.pop_front();
    }

    size_t currentIndex = 0; // índice del proceso a ejecutar en processList
    size_t completedCount = 0;

    // Ejecutar hasta que no queden procesos ni entrantes
    while (completedCount < processList.size() || !incomingProcesses.empty()) {
        // Si no hay procesos listos en la lista actual, avanzar tiempo y añadir entrantes
        if (processList.empty()) {
            // esperar al siguiente arribo
            currentTime = max(currentTime, incomingProcesses.front()->getArrivalTime());
            while (!incomingProcesses.empty() && incomingProcesses.front()->getArrivalTime() <= currentTime) {
                processList.push_back(incomingProcesses.front());
                incomingProcesses.pop_front();
            }
            continue;
        }

        if (currentIndex >= processList.size()) currentIndex = 0; // wrap around

        Process* p = processList[currentIndex];

        // Si p aún no ha llegado, avanzar al siguiente evento de llegada
        if (p->getArrivalTime() > currentTime) {
            int nextArrival = p->getArrivalTime();
            if (!incomingProcesses.empty()) nextArrival = min(nextArrival, incomingProcesses.front()->getArrivalTime());
            currentTime = nextArrival;
            // añadir entrantes que ya llegaron
            while (!incomingProcesses.empty() && incomingProcesses.front()->getArrivalTime() <= currentTime) {
                processList.push_back(incomingProcesses.front());
                incomingProcesses.pop_front();
            }
            continue;
        }

        // Si ya terminó, saltarlo
        if (p->getRemainingTime() == 0) {
            currentIndex++;
            continue;
        }

        // Ejecutar el proceso p por un quantum o hasta que termine
        if (p->getRemainingTime() == p->getBurstTime() && p->getResponseTime() == -1) {
            p->setResponseTime(currentTime);
        }

        int execTime = min(timeQuantum, p->getRemainingTime());
        p->setRemainingTime(p->getRemainingTime() - execTime);
        currentTime += execTime;

        // Añadir nuevos arribos que ocurrieron mientras se ejecutaba p
        while (!incomingProcesses.empty() && incomingProcesses.front()->getArrivalTime() <= currentTime) {
            processList.push_back(incomingProcesses.front());
            incomingProcesses.pop_front();
        }

        if (p->getRemainingTime() == 0) {
            p->setCompletionTime(currentTime);
            int wt = p->getCompletionTime() - p->getArrivalTime() - p->getBurstTime();
            p->setWaitingTime(wt);
            completedCount++;
        }

        // pasar al siguiente índice (circular)
        currentIndex++;
    }
}

int RoundRobinScheduler::getCurrentTime() {
    return currentTime;
}