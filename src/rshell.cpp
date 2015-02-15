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
		char* const* argv = NULL;
		vector<char**> args; // FIXME delete later
		comParse cp;
		do {
			cerr << "$ " << flush;
			
			string command = "";
			getline(cin, command);	// Get command

			args = cp.parseLine(command); // FIXME used to be argv here
		} while (argv == NULL); // if empty line, repeat req for command

		//char ex[5] = {'e','x','i','t','\0'};
		//if (strcmp(argv[0], ex) == 0 && cp.size() == 1) return 0; // EXIT command

		for (unsigned int i = 0; i < args.size(); i++) 
			for (unsigned int j = 0; args.at(i)[j] != '\0'; j++)
				cerr << args.at(i)[j] << endl;

		int pid = fork();
		if (pid == -1) {	// Error in fork
			perror("error in fork");
			exit(-1);
		} else if (pid == 0) {	// Child Process
			if (-1 == execvp(argv[0], argv)) {
				perror("error in execvp");
				exit(-1);
			}
		}
	}

	return 0;
}

