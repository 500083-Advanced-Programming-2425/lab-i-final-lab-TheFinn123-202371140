// #######################################################################################
// ##                                                                                   ##
// ##  This main.cpp file WILL BE REPLACED during the marking procedure.                ##
// ##  Therefore all of your implementation code MUST be in the Solution class,         ##
// ##  or any other new classes that you create and use inside the Solution class.      ##
// ##                                                                                   ##
// #######################################################################################

#include <iostream>
#include <iomanip>
#include <chrono>

#include "Solution.h"

using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::duration_cast;

int main(int, char**) {

	Solution solver;

	// Timing BuildNetwork

	const auto buildStart = high_resolution_clock::now();
	const auto buildSuccess = solver.buildNetwork("Users.csv", "Friendships.csv");
	const auto buildEnd = high_resolution_clock::now();
	const auto buildTime = std::max(duration_cast<microseconds>(buildEnd - buildStart).count(), 1LL);

	if (buildSuccess) {
		std::cout << std::fixed << std::setprecision(1) << "BuildNetwork - " << buildTime << " microseconds" << std::endl;
	}
	else {
		std::cout << "\n*** Error *** BuildNetwork" << std::endl;
	}

	// Timing processing of individual commands

	std::ifstream fin("Commands.txt");
	while (!fin.eof()) {
		std::string command;
		std::getline(fin, command);

		const auto cmdStart = high_resolution_clock::now();
		const auto cmdSuccess = solver.processCommand(command);
		const auto cmdEnd = high_resolution_clock::now();
		const auto cmdTime = std::max(duration_cast<microseconds>(cmdEnd - cmdStart).count(), 1LL);

		if (cmdSuccess) {
			std::cout << std::fixed << std::setprecision(1) << command << " - " << cmdTime << " microseconds" << std::endl;
		}
		else {
			std::cout << "\n*** Error *** " << command << std::endl;
		}
	}

	std::cin.get(); // wait for a key press
}
