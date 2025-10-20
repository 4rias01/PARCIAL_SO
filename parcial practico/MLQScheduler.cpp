#include "MLQScheduler.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <climits>
#include <iomanip>

MLQScheduler::MLQScheduler(int currentTime) {
    this->currentTime = currentTime;
}

void MLQScheduler::readFile(string filename) {
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Error: no se pudo abrir el archivo: " << filename << endl;
        return;
    }

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        // Separar por punto y coma
        stringstream ss(line);
        string name, bt, at, q, pr;

        getline(ss, name, ';');
        getline(ss, bt, ';');
        getline(ss, at, ';');
        getline(ss, q, ';');
        getline(ss, pr, ';');

        // Quitar espacios
        name.erase(remove(name.begin(), name.end(), ' '), name.end());
        bt.erase(remove(bt.begin(), bt.end(), ' '), bt.end());
        at.erase(remove(at.begin(), at.end(), ' '), at.end());
        q.erase(remove(q.begin(), q.end(), ' '), q.end());
        pr.erase(remove(pr.begin(), pr.end(), ' '), pr.end());

        Process* p = new Process(name, stoi(bt), stoi(at), stoi(q), stoi(pr));
        processes.push_back(p);
        cout << "Leído proceso: " << name << " BT=" << bt << " AT=" << at << " Q=" << q << " P=" << pr << endl;
    }

    file.close();
    // No inicializamos las colas aquí; construiremos la lista global en run() para soportar preempción entre colas
}

void MLQScheduler::run() {
    // Implementación preemptiva entre niveles (cola 1 > cola 2 > cola 3)
    // Construir lista global de llegadas (ordenada por arrivalTime, en empate por name)
    deque<Process*> globalIncoming;
    for (auto p : processes) globalIncoming.push_back(p);
    sort(globalIncoming.begin(), globalIncoming.end(), [](Process* a, Process* b) {
        if (a->getArrivalTime() != b->getArrivalTime()) return a->getArrivalTime() < b->getArrivalTime();
        return a->getName() < b->getName();
    });

    // Asegurar que las colas estén vacías al inicio
    queue1.clear(); queue2.clear(); queue3.clear();

    const int q1_quantum = 3;
    const int q2_quantum = 5;

    // Ejecutar hasta que no queden procesos en ninguna cola ni por llegar
    while (!globalIncoming.empty() || !queue1.empty() || !queue2.empty() || !queue3.empty()) {
        // Mover llegadas actuales a sus colas
        while (!globalIncoming.empty() && globalIncoming.front()->getArrivalTime() <= currentTime) {
            Process* p = globalIncoming.front(); globalIncoming.pop_front();
            if (p->getQueueLevel() == 1) queue1.push_back(p);
            else if (p->getQueueLevel() == 2) queue2.push_back(p);
            else queue3.push_back(p);
            cout << "Llegada: " << p->getName() << " AT=" << p->getArrivalTime() << " Q=" << p->getQueueLevel() << endl;
        }

        // Si no hay procesos listos, avanzar al siguiente arrival
        if (queue1.empty() && queue2.empty() && queue3.empty()) {
            if (!globalIncoming.empty()) {
                currentTime = max(currentTime, globalIncoming.front()->getArrivalTime());
                continue;
            } else break;
        }

        // Prioridad: cola1 > cola2 > cola3
        if (!queue1.empty()) {
            // Usar proceso-lista estable para RR en Q1: tomar snapshot de la cola y procesar
            vector<Process*> procList;
            while (!queue1.empty()) { procList.push_back(queue1.front()); queue1.pop_front(); }

            size_t idx = 0;
            while (idx < procList.size()) {
                Process* p = procList[idx];
                if (p->getRemainingTime() == 0) { idx++; continue; }
                if (p->getRemainingTime() == p->getBurstTime() && p->getResponseTime() == -1) p->setResponseTime(currentTime);

                int exec = min(q1_quantum, p->getRemainingTime());
                p->setRemainingTime(p->getRemainingTime() - exec);
                currentTime += exec;
                cout << "[Q1] Ejecutado " << p->getName() << " por " << exec << " -> t=" << currentTime << endl;

                // Después de ejecutar, reencolar el proceso incompleto al final de la lista
                if (p->getRemainingTime() > 0) {
                    procList.push_back(p);
                } else {
                    p->setCompletionTime(currentTime);
                    int wt = p->getCompletionTime() - p->getArrivalTime() - p->getBurstTime();
                    p->setWaitingTime(wt);
                    cout << "[Q1] Completado " << p->getName() << " CT=" << p->getCompletionTime() << " WT=" << wt << " RT=" << p->getResponseTime() << endl;
                }

                // Procesar llegadas que ocurrieron durante la ejecución y añadirlas
                while (!globalIncoming.empty() && globalIncoming.front()->getArrivalTime() <= currentTime) {
                    Process* np = globalIncoming.front(); globalIncoming.pop_front();
                    if (np->getQueueLevel() == 1) {
                        procList.push_back(np); // nuevas llegadas de Q1 van al final de la lista
                    } else if (np->getQueueLevel() == 2) {
                        queue2.push_back(np);
                    } else {
                        queue3.push_back(np);
                    }
                    cout << "Llegada: " << np->getName() << " AT=" << np->getArrivalTime() << " Q=" << np->getQueueLevel() << endl;
                }

                idx++;
            }

            // Cualquier proceso restante en procList (no completados) devolverlos a queue1 en orden
            for (auto &rp : procList) if (rp->getRemainingTime() > 0) queue1.push_back(rp);
            continue;
        }

        if (!queue2.empty()) {
            // Similar al manejo de Q1 pero teniendo en cuenta preempción por llegadas a Q1
            vector<Process*> procList;
            while (!queue2.empty()) { procList.push_back(queue2.front()); queue2.pop_front(); }

            size_t idx = 0;
            while (idx < procList.size()) {
                Process* p = procList[idx];
                if (p->getRemainingTime() == 0) { idx++; continue; }
                if (p->getRemainingTime() == p->getBurstTime() && p->getResponseTime() == -1) p->setResponseTime(currentTime);

                int plannedExec = min(q2_quantum, p->getRemainingTime());
                int exec = plannedExec;

                // Si hay una llegada de Q1 dentro de la ventana, ajustar exec
                if (!globalIncoming.empty()) {
                    int windowEnd = currentTime + plannedExec;
                    for (auto it = globalIncoming.begin(); it != globalIncoming.end(); ++it) {
                        int at = (*it)->getArrivalTime();
                        if (at <= currentTime) continue;
                        if (at >= windowEnd) break;
                        if ((*it)->getQueueLevel() == 1) { exec = at - currentTime; break; }
                    }
                }

                if (exec > 0) {
                    p->setRemainingTime(p->getRemainingTime() - exec);
                    currentTime += exec;
                    cout << "[Q2] Ejecutado " << p->getName() << " por " << exec << " -> t=" << currentTime << endl;
                } else {
                    // Llegada inmediata de Q1: no ejecutar este proceso ahora
                    // Devolver el proceso al frente de la lista y salir para procesar Q1
                    // (colocar de nuevo en procList en la misma posición y romper)
                    // No incrementamos idx
                    break;
                }

                // Después de ejecutar, reencolar incompletos al final de la lista
                if (p->getRemainingTime() > 0) procList.push_back(p);
                else {
                    p->setCompletionTime(currentTime);
                    int wt = p->getCompletionTime() - p->getArrivalTime() - p->getBurstTime();
                    p->setWaitingTime(wt);
                    cout << "[Q2] Completado " << p->getName() << " CT=" << p->getCompletionTime() << " WT=" << wt << " RT=" << p->getResponseTime() << endl;
                }

                // Procesar llegadas que ocurrieron durante la ejecución y añadirlas
                while (!globalIncoming.empty() && globalIncoming.front()->getArrivalTime() <= currentTime) {
                    Process* np = globalIncoming.front(); globalIncoming.pop_front();
                    if (np->getQueueLevel() == 1) {
                        queue1.push_back(np);
                    } else if (np->getQueueLevel() == 2) {
                        procList.push_back(np);
                    } else {
                        queue3.push_back(np);
                    }
                    cout << "Llegada: " << np->getName() << " AT=" << np->getArrivalTime() << " Q=" << np->getQueueLevel() << endl;
                }

                idx++;
                // Si durante la ejecución apareció una llegada de Q1 que hemos añadido a queue1,
                // preferimos salir y procesar Q1 antes de seguir con Q2
                if (!queue1.empty()) break;
            }

            // Devolver los procesos no completados en orden a queue2
            for (auto &rp : procList) if (rp->getRemainingTime() > 0) queue2.push_back(rp);

            // Si aparecieron procesos en Q1, dejaremos que el bucle principal los procese ahora
            continue;
        }

        // Cola 3 (FIFO) — ejecutable pero susceptible de preempción por llegadas a Q1/Q2
        if (!queue3.empty()) {
            Process* p = queue3.front(); queue3.pop_front();
            if (p->getRemainingTime() == p->getBurstTime() && p->getResponseTime() == -1) p->setResponseTime(currentTime);

            int plannedExec = p->getRemainingTime();
            int exec = plannedExec;

            // Buscar la primera llegada dentro de la ventana que pertenezca a Q1 o Q2 (mayor prioridad)
            if (!globalIncoming.empty()) {
                int windowEnd = currentTime + plannedExec;
                for (auto it = globalIncoming.begin(); it != globalIncoming.end(); ++it) {
                    int at = (*it)->getArrivalTime();
                    if (at <= currentTime) continue;
                    if (at >= windowEnd) break;
                    int qlvl = (*it)->getQueueLevel();
                    if (qlvl == 1 || qlvl == 2) {
                        exec = at - currentTime;
                        break;
                    }
                }
            }

            if (exec > 0) {
                p->setRemainingTime(p->getRemainingTime() - exec);
                currentTime += exec;
                cout << "[Q3] Ejecutado " << p->getName() << " por " << exec << " -> t=" << currentTime << endl;
            } else {
                // Hay una llegada inmediata de mayor prioridad; reencolar al frente y procesar llegadas
                queue3.push_front(p);
                continue;
            }

            if (p->getRemainingTime() == 0) {
                p->setCompletionTime(currentTime);
                int wt = p->getCompletionTime() - p->getArrivalTime() - p->getBurstTime();
                p->setWaitingTime(wt);
                cout << "[Q3] Completado " << p->getName() << " CT=" << p->getCompletionTime() << " WT=" << wt << " RT=" << p->getResponseTime() << endl;
            } else {
                // preemptado por llegada a Q1/Q2: dejarlo al frente para respetar FIFO
                queue3.push_front(p);
            }
            continue;
        }
    }
}

void MLQScheduler::deleteInstances() {
    for (auto process : processes) {
        delete process;
    }
    processes.clear();
    queue1.clear();
    queue2.clear();
    queue3.clear();
}

void MLQScheduler::printReport() {
    // Imprimir en el formato solicitado
    std::cout << "# archivo: mlq001.txt\n";
    std::cout << "# etiqueta: BT; AT; Q; Pr; WT; CT; RT; TAT\n";

    double sumWT = 0.0, sumCT = 0.0, sumRT = 0.0, sumTAT = 0.0;
    int n = 0;

    for (auto p : processes) {
        int WT = p->getWaitingTime();
        int CT = p->getCompletionTime();
        int RT = p->getResponseTime();
        int TAT = p->getTurnaroundTime();
        std::cout << p->getName() << ";" << p->getBurstTime() << ";" << p->getArrivalTime() << ";" << p->getQueueLevel() << ";" << p->getPriority() << "; "
                  << WT << "; " << CT << "; " << RT << "; " << TAT << "\n";

        sumWT += WT;
        sumCT += CT;
        sumRT += RT;
        sumTAT += TAT;
        n++;
    }

    if (n > 0) {
        double avgWT = sumWT / n;
        double avgCT = sumCT / n;
        double avgRT = sumRT / n;
        double avgTAT = sumTAT / n;
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "\nWT=" << avgWT << "; CT=" << avgCT << "; RT=" << avgRT << "; TAT=" << avgTAT << ";\n";
    }
}