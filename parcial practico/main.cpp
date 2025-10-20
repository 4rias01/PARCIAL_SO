#include <iostream>
#include <fstream>
#include <iomanip>
#include "MLQScheduler.h"

int main(int argc, char* argv[]) {
	// Si se pasa un archivo como argumento, usarlo; si no, crear un archivo de ejemplo
	std::string inputFile;
	if (argc > 1) {
		inputFile = argv[1];
		std::cout << "Usando archivo de entrada: " << inputFile << "\n";
	} else {
		// Crear un archivo de entrada de ejemplo
		inputFile = "procesos/processes.txt";
		std::ofstream out(inputFile);
		// Formato: name;burst;arrival;queue;priority
		out << "A;6;0;1;5\n";
		out << "B;9;0;1;4\n";
		out << "C;10;0;2;3\n";
		out << "D;15;0;2;3\n";
		out << "E;8;0;3;2\n";
		out.close();
		std::cout << "Archivo de ejemplo creado: " << inputFile << "\n";
	}

	MLQScheduler mlq(0);
	mlq.readFile(inputFile);
	mlq.run();

	// Dejar que MLQScheduler imprima el informe final
	mlq.printReport();

	mlq.deleteInstances();
	return 0;
}