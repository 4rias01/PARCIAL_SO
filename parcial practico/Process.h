#ifndef PROCESS_H
#define PROCESS_H
#include <string>
#include <iostream>

using namespace std;

class Process {
    public:
        Process();  // Constructor por defecto
        Process(string name, int burstTime, int arrivalTime); // Constructor con parámetros para un proceso genérico
        Process(string name, int burstTime, int arrivalTime, int queueLevel, int priority); // Constructor con parámetros para un proceso con prioridad y nivel de cola
        
        string getName();
        int getArrivalTime();
        int getBurstTime();
        int getRemainingTime();
        int getPriority();
        int getQueueLevel();

        // New getters for computed times
        int getWaitingTime();
        int getResponseTime();
        int getCompletionTime();
        int getTurnaroundTime();

        void setRemainingTime(int time);
        void setWaitingTime(int time);
        void setResponseTime(int time);
        void setCompletionTime(int time);

    private:
        // Atributos del proceso
        string name;
        int burstTime; // BT
        int arrivalTime; // AT
        int remainingTime; // Tiempo restante
        int priority; // Px
        int queueLevel; // Qx

        // Atributos por calcular
        int waitingTime; // WT = TAT - BT
        int responseTime; // RT = momento en el que se ejecuta por primera vez
        int completionTime; // CT = momento en el que finaliza la ejecución
        int turnaroundTime; // TAT = CT - AT

        // Método para calcular el tiempo de respuesta
        void calculateTurnaroundTime();
};
#endif // PROCESS_H