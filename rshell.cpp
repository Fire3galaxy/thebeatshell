#include <iostream>
#include <unistd.h>

int main() {
	std::string command;
	getline(std::cin, command);

	std::cout << command << std::endl;

	return 0;
}
