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
		//char* const* argv = NULL;
		//vector<char* const*> multipleArgv;
		vector<const char*> commands;
		comParse comPr;
		do {
			cerr << "$ " << flush;
			
			string input = "";
			getline(cin, input);	// Get command

			commands = comPr.parseLine(input);
		} while (commands.size() == 1); // if empty line, repeat req for command. sz 1 = just NULL delim.

		//int startIndx = 0; // Semicolon, multiple args
		for (unsigned int i = 0; i < commands.size() - 1; i++) {
			if (strcmp(commands.at(i), ";") == 0) cerr << "; at " << i << endl;
			else if (strcmp(commands.at(i), "&") == 0) cerr << "& at " << i << endl;
			else if (strcmp(commands.at(i), "|") == 0) cerr << "| at " << i << endl;
		}

		//char ex[5] = "exit";
		//if (strcmp(argv[0], ex) == 0 && comPr.size() == 1) return 0; // EXIT command

		//int pid = fork();
		//if (pid == -1) {	// Error in fork
		//	perror("error in fork");
		//	exit(-1);
		//} else if (pid == 0) {	// Child Process
		//	if (-1 == execvp(argv[0], argv)) {
		//		perror("error in execvp");
		//		exit(-1);
		//	}
		//}
	}

	return 0;
}

