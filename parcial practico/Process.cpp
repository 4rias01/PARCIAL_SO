#include "Process.h"

Process::Process() {
    this->name = "";
    this->arrivalTime = 0;
    this->burstTime = 0;
    this->remainingTime = 0;
    this->queueLevel = 0;
    this->priority = 0;

    this->waitingTime = 0;
    this->responseTime = -1; // -1 indicates response time not set yet
    this->completionTime = 0;
    this->turnaroundTime = 0;
}

Process::Process(string name, int burstTime, int arrivalTime) {
    this->name = name;
    this->arrivalTime = arrivalTime;
    this->burstTime = burstTime;
    this->remainingTime = burstTime; // Initially, remaining time is equal to burst time
    this->queueLevel = 0;
    this->priority = 0;

    this->waitingTime = 0;
    this->responseTime = -1; // -1 indicates response time not set yet
    this->completionTime = 0;
    this->turnaroundTime = 0;
}

Process::Process(string name, int burstTime, int arrivalTime, int queueLevel, int priority) {
    this->name = name;
    this->arrivalTime = arrivalTime;
    this->burstTime = burstTime;
    this->remainingTime = burstTime; // Initially, remaining time is equal to burst time
    this->queueLevel = queueLevel;
    this->priority = priority;

    this->waitingTime = 0;
    this->responseTime = -1; // -1 indicates response time not set yet
    this->completionTime = 0;
    this->turnaroundTime = 0;
}

string Process::getName() {
    return this->name;
}

int Process::getArrivalTime() {
    return this->arrivalTime;
}

int Process::getBurstTime() {
    return this->burstTime;
}

int Process::getRemainingTime() {
    return this->remainingTime;
}

int Process::getPriority() {
    return this->priority;
}

int Process::getQueueLevel() {
    return this->queueLevel;
}

int Process::getWaitingTime() {
    return this->waitingTime;
}

int Process::getResponseTime() {
    return this->responseTime;
}

int Process::getCompletionTime() {
    return this->completionTime;
}

int Process::getTurnaroundTime() {
    return this->turnaroundTime;
}

void Process::setRemainingTime(int time) {
    this->remainingTime = time;
}

void Process::setWaitingTime(int time) {
    this->waitingTime = time;
}

void Process::setResponseTime(int time) {
    this->responseTime = time;
}

void Process::setCompletionTime(int time) {
    this->completionTime = time;
    calculateTurnaroundTime();
}

void Process::calculateTurnaroundTime() {
    this->turnaroundTime = this->completionTime - this->arrivalTime;
}