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
		char** argv = NULL;
		vector<char*> commands;
		comParse comPr;
		do {
			cerr << "$ " << flush;
			
			string input = "";
			getline(cin, input);	// Get command

			commands = comPr.parseLine(input);
		} while (commands.size() == 1); // if empty line, repeat req for command. sz 1 = just NULL delim.

		argv = &commands[0];

		for (unsigned int i = 0; i < commands.size() - 1; i++) { // NULL at end
			if ( strcmp(commands.at(i), ";") == 0 ) {
				argv[i] = NULL;
				break;
			}
		}

		/* Confusing as heck, let me explain!
		 * char const* means pointer to a constant char
		 * so char const* const means a const pointer to a const char
		 * and char const* const* means a const pointer to a const char pointer!
		 * (Read declarations from right to left to make it make sense -
		 *  char const* = POINTER (*) to a CONST CHAR)
		 */
		//char const* const* c_arr = &commands[0]; // even if Not necessary: keep as really cool discovery!

		char ex[5] = "exit";
		if (strcmp(argv[0], ex) == 0) {
			comPr.deleteCStrings(commands); // Has heap allocated memory, always should be dealt with
			return 0; // EXIT command
		}

		int pid = fork();
		if (pid == -1) {	// Error in fork
			perror("error in fork");
			comPr.deleteCStrings(commands);
			exit(-1);
		} else if (pid == 0) {	// Child Process
			if (-1 == execvp(argv[0], argv)) {
				if (errno == EACCES) { // Access denied
					perror(argv[0]);
				} else if (errno == ENOEXEC) { // Not Exec
					perror(argv[0]);
				} else if (errno == ENOENT) { // Does not exist
					perror(argv[0])
				} else {
					perror("error in execvp");
					exit(-1);
				}
			}
		} else if (pid > 0) { // parent!
			if (-1 == wait(0))
				perror("wait");
			if (errno != 0 && errno != EACCES && errno != ENOEXEC && errno != ENOENT)
				exit(-1);
		}

		comPr.deleteCStrings(commands);
	}

	return 0;
}

