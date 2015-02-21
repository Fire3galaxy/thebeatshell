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
#include <sys/stat.h>
#include <fcntl.h>

using namespace boost;
using namespace std;

struct redirect {
	bool doIRdct; // input redirect
	int indexIR;
	int savedSTDIN; // for duping STDIN and saving it
	int wfd; // when opening new fd in 0 for writing

	redirect() {
		doIRdct = false;
		savedSTDIN = -1;
		wfd = -1;
		indexIR = -1;
	}
};

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
		redirect rdts;

		for (unsigned int i = 0; i < commands.size() - 1; i++) { // NULL at end
			if ( strcmp(commands.at(i), ";") == 0 ) {
				delete[] commands.at(i);
				argv[i] = NULL;

				break;
			} else if (strcmp(commands.at(i), "<") == 0) {
				delete[] commands.at(i);
				argv[i] = NULL;

				rdts.doIRdct = true;
				rdts.indexIR = i;
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

		bool doExec = true; // If false, do not fork and exec command

		char ex[5] = "exit";
		if (strcmp(argv[0], ex) == 0) {
			comPr.deleteCStrings(commands); // Has heap allocated memory, always should be dealt with
			return 0; // EXIT command
		}

		// INPUT REDIRECT
		if (rdts.doIRdct) {
		 	// dup the STDIN
			if ( -1 == (rdts.savedSTDIN = dup(STDIN_FILENO)) ) {
				perror("dup-input redirect");
				exit(-1);
			}

			// close STDIN
			if ( -1 == close(STDIN_FILENO) ) {
				perror("close-input redirect");
				exit(-1);
			}

			// open file
			if ( -1 == (rdts.wfd = open(argv[rdts.indexIR + 1], S_IRUSR)) ) { 
				if (errno == EACCES || errno == ENOENT) { 	//FIXME
					perror(argv[rdts.indexIR + 1]); 	// < filename syntax
					doExec = false; 
				} else {
					perror("open-input redirect");
					exit(-1);
				}

				if (rdts.wfd != 0) {
					cerr << "Wrong place!";
					exit(-1);
				}
			}
		}


		if (doExec) {
			int pid = fork();
			if (pid == -1) {	// Error in fork
				perror("error in fork");
				comPr.deleteCStrings(commands);
				exit(-1);
			} else if (pid == 0) {	// Child Process
				if (-1 == execvp(argv[0], argv)) {
					// All children need to exit! (forgot this)
					if (errno == EACCES) { // Access denied
						perror(argv[0]);
						exit(-1);
					} else if (errno == ENOEXEC) { // Not Exec
						perror(argv[0]);
						exit(-1);
					} else if (errno == ENOENT) { // Does not exist
						perror(argv[0]);
						exit(-1);
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
		}

		if (rdts.doIRdct) dup2(rdts.savedSTDIN, rdts.wfd);	// Restore stdin

		comPr.deleteCStrings(commands);
	}

	return 0;
}

