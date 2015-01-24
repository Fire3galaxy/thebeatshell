#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include "comParse.h"
#include <cstring>
#include <unistd.h>

using namespace boost;
using namespace std;

int main() {
	while (true) {
		cerr << "$ " << flush;
		
		string command = "";
		getline(cin, command);	// Get command

		comParse cp;
		vector<vector<char*> > args_v = cp.parseLine(command); // Parse line

		for (unsigned int i = 0; i < args_v.size(); i++) {
			char** argv = args_v.at(i).data();
			argv[args_v.at(i).size()] = '\0';

			char ex[5] = {'e','x','i','t','\0'};
			if (strcmp(argv[0], ex) == 0 && args_v.at(i).size() == 1) return 0; // EXIT command

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
			
			int status = -1;
			if (-1 == waitpid(pid, &status, 0)) {	// Parent Process
				perror("error in wait");
				exit(-1);
			}
		}
	}

	return 0;
}

