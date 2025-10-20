#include "FIFOScheduler.h"
#include <iostream>
#include <algorithm>

using namespace std;

FIFOScheduler::FIFOScheduler(int currentTime, deque<Process*> processes) {
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

void FIFOScheduler::run() {
    // Ready queue: procesos que han llegado y están esperando ejecución (FIFO)
    deque<Process*> readyQueue;

    // Añadir inicialmente los procesos que ya han llegado
    while (!incomingProcesses.empty() && incomingProcesses.front()->getArrivalTime() <= currentTime) {
        readyQueue.push_back(incomingProcesses.front());
        incomingProcesses.pop_front();
    }

    // Ejecutar hasta que no queden procesos listos ni por llegar
    while (!readyQueue.empty() || !incomingProcesses.empty()) {
        // Si no hay procesos listos, avanzar tiempo al próximo arrival
        if (readyQueue.empty()) {
            currentTime = max(currentTime, incomingProcesses.front()->getArrivalTime());
            while (!incomingProcesses.empty() && incomingProcesses.front()->getArrivalTime() <= currentTime) {
                readyQueue.push_back(incomingProcesses.front());
                incomingProcesses.pop_front();
            }
        }

        if (readyQueue.empty()) continue; // seguridad

        // FIFO: sacar el primer proceso y ejecutarlo completamente
        Process* p = readyQueue.front();
        readyQueue.pop_front();

        // Si por alguna razón aún no ha llegado, avanzar
        if (p->getArrivalTime() > currentTime) currentTime = p->getArrivalTime();

        // Registrar response time si es la primera ejecución
        if (p->getRemainingTime() == p->getBurstTime() && p->getResponseTime() == -1) {
            p->setResponseTime(currentTime);
        }

        // Ejecutar hasta completar
        int execTime = p->getRemainingTime();
        currentTime += execTime;
        p->setRemainingTime(0);

        // Fijar métricas de terminación
        p->setCompletionTime(currentTime);
        int wt = p->getCompletionTime() - p->getArrivalTime() - p->getBurstTime();
        p->setWaitingTime(wt);

        // Durante la ejecución han podido llegar más procesos: añadirlos al readyQueue
        while (!incomingProcesses.empty() && incomingProcesses.front()->getArrivalTime() <= currentTime) {
            readyQueue.push_back(incomingProcesses.front());
            incomingProcesses.pop_front();
        }
    }
}

int FIFOScheduler::getCurrentTime() {
    return currentTime;
}