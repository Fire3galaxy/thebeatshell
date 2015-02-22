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

#define I_REDIRECT 0
#define O_REDIRECT 1
#define ORD_APPEND 2

struct redirect;
bool redirection(char** argv, redirect& rdts, const int id);
bool setRdct(char** argv, redirect& rdts);

struct redirect {
	// May now be irrelevant b/c rdctInd replaces it
	int indexIR;
	int indexOR;
	int indexPR;
	bool doI_Rdct; // input redirect
	bool doO_Rdct; // output redirect
	int savedP; // for duping 
	int pfd; // when opening new fd

	vector<int> v_ind,v_pfd,v_savedP;
	vector<pair<int,int> > v_opfd;

	vector<int*> currentFD;

	redirect() {
		indexOR = -1;
		indexIR = -1;
		indexPR = -1;
		
		doO_Rdct = false;
		doI_Rdct = false;
		savedP = -1;
		pfd = -1;
	}
	void closeCurrFDs() {
		if (!currentFD.empty()) {
			for (vector<int*>::iterator it = currentFD.begin(); 
					it != currentFD.end(); it++) {
				if (-1 == close(*(*it))) {
					perror("close");
					exit(-1);
				}
			}

			currentFD.clear();
		}
	}
	~redirect() {
		//closeCurrFDs();
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

		int timeout = 0;
		vector<char*> realcom; // command + arguments - input redirection & semicolon
		argv = &commands.at(0);
		redirect rdts;	// contains redirection flags and values

		for (unsigned int i = 0; i < commands.size() - 1; i++) { // NULL at end
			if ( strcmp(commands.at(i), ";") == 0) {
				delete[] commands.at(i);
				argv[i] = NULL;
				timeout = 1;

				break;
			} else if (strchr(commands.at(i), '<') != NULL) {
				int fd = -1;
				if (isdigit(commands.at(i)[0])) fd = commands.at(i)[0]; // num before operator

				delete[] commands.at(i);
				argv[i] = NULL;

				timeout = 2;	// do not push this or next arg
				rdts.v_opfd.push_back( pair<int,int>(I_REDIRECT, fd) ); // op, fd

				if ( !(i + 1 < commands.size()) ) {	// next arg must exist (filename)
					cerr << "rshell: syntax error at <";
					exit(-1);
				}
				rdts.v_ind.push_back(i + 1); // index
			} else if (strcmp(commands.at(i), ">") == 0
					|| strcmp(commands.at(i), "1>") == 0) {
				delete[] commands.at(i);
				argv[i] = NULL;

				timeout = 2;	// do not push this or next arg
				rdts.doO_Rdct = true;
				rdts.indexOR = i;
			} 

			if (timeout != 0) timeout--;
			else realcom.push_back(commands.at(i));
		}

		realcom.push_back(NULL);

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

		doExec = setRdct(argv, rdts);
		//if (rdts.doO_Rdct) doExec = redirection(argv, rdts, STDOUT_FILENO);
		//if (rdts.doE_Rdct) doExec = redirection(argv, rdts, STDERR_FILENO);

		argv = &realcom.at(0); // initialize to char**

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

		for (unsigned int i = 0; i < rdts.v_savedP.size(); i++) {
			if (-1 == dup2(rdts.v_savedP.at(i), rdts.v_pfd.at(i))) {	// Restore saved fds
				perror("dup2");
				exit(-1);
			}
		}

		comPr.deleteCStrings(commands);
		//rdts.closeCurrFDs();
	}

	return 0;
}


bool setRdct(char** argv, redirect& rdts) {
	bool doExec = true;
	for (unsigned int i = 0; i < rdts.v_opfd.size(); i++)
		doExec = redirection(argv, rdts, i) ? doExec : false; // if false, stay false
	return doExec;
}

// closes respective fd and opens file in its place
bool redirection(char** argv, redirect& rdts, const int id) {
	/*int* savedFD = rdts.savedP;
	int* newFD = rdts.pfd;
	int* index = rdts.indexPR;
	*/

	int savedFD,newFD;
	int index = rdts.v_ind.at(id);
	int FLAG;

	// 0 == INPUT, 1 == OUTPUT
	if (rdts.v_opfd.at(id).first == 0) {
		FLAG = O_RDONLY;
		if (rdts.v_opfd.at(id).second == -1) 
			rdts.v_opfd.at(id).second = 0; // DEFAULT for input operator
	} else if (rdts.v_opfd.at(id).first == 1) {
		FLAG = O_WRONLY | O_CREAT | O_TRUNC;
		if (rdts.v_opfd.at(id).second == -1) 
			rdts.v_opfd.at(id).second = 1; // DEFAULT for output operator
	} 

	// dup the fd
	if ( -1 == (savedFD = dup(rdts.v_opfd.at(id).second)) ) {
		perror("dup-redirect" + rdts.v_opfd.at(id).second);
		exit(-1);
	}
	rdts.v_savedP.push_back(savedFD); // Saving fd for dup2 later

	// close fd
	if ( -1 == close(rdts.v_opfd.at(id).second) ) {
		perror("close-redirect" + rdts.v_opfd.at(id).second);
		exit(-1);
	}

	// open file
	if ( -1 == (newFD = open(argv[index], FLAG)) ) { 
		if (errno == EACCES || errno == ENOENT) { 	//FIXME
			perror(argv[index]);
			return false;
		} else {
			perror("open-redirect" + rdts.v_opfd.at(id).second);
			exit(-1);
		}
	}
	rdts.v_pfd.push_back(newFD); // Saving new fd to close later

	return true;
}


