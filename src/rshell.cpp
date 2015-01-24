#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include "comParse.h"

using namespace boost;
using namespace std;

int main() {
	cout << "$ " << flush;
	
	string command = "";
	getline(cin, command);	// Get command

	comParse cp;
	char* const* argv = cp.parseLine(command); // Parse line

	int pid = fork();
	if (pid == -1) {	// Error in fork
		perror("error in fork");
		exit(-1);
	} else if (pid == 0) {	// Child Process
		if (-1 == execvp(argv[0], argv)) {
			perror("error in execvp");
			exit(-1);
		}

		exit(1); // should never happen, since child should exec
	} 
	
	if (-1 == wait(0)) {	// Parent Process
		perror("error in wait");
		exit(-1);
	}

	return 0;
}

