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
		char const** argv = NULL;
		vector<const char*> commands;
		comParse comPr;
		do {
			cerr << "$ " << flush;
			
			string input = "";
			getline(cin, input);	// Get command

			commands = comPr.parseLine(input);
		} while (commands.size() == 1); // if empty line, repeat req for command. sz 1 = just NULL delim.

		int size1 = 0;
		for (unsigned int i = 0; i < commands.size(); i++) {
			if ( strcmp(commands.at(i), ";") == 0 ) {
				size1 = i;
				break;
			}
		}
		cout << size1;

		/* Confusing as heck, let me explain!
		 * char const* means pointer to a constant char
		 * so char const* const means a const pointer to a const char
		 * and char const* const* means a const pointer to a const char pointer!
		 * (Read declarations from right to left to make it make sense -
		 *  char const* = POINTER (*) to a CONST CHAR)
		 */
		//char const* const* c_arr = &commands[0]; // even if Not necessary: keep as really cool discovery!
		argv = &commands[0];
		argv[size1] = NULL;
		for (int i = 0; i < size1 + 1; i++)
			if (argv + i != NULL) cout << argv[i] << endl;
			else cout << "(null)" << endl;

		char ex[5] = "exit";
		if (strcmp(argv[0], ex) == 0 && comPr.size() == 1) return 0; // EXIT command

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

