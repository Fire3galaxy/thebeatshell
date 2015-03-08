#include <iostream>
#include <boost/tokenizer.hpp>
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
#include <signal.h>

using namespace boost;
using namespace std;

#define I_REDIRECT 0
#define O_REDIRECT 1
#define ORD_APPEND 2
 
void input_sigHandler(int param) {
	if (param == SIGINT) {
		char currentDir[512];
		if (NULL == (getcwd(currentDir, 512))) { // Currently outputs error that directory is too long. Don't like this FIXME
			perror("getcwd");
			currentDir[0] = '\0';
		}
		cout << '\n' << currentDir << " $ " << flush;
	}
	if (param == SIGTSTP) {}
}

void quit_sigHandler(int param) {
	if (param == SIGINT) {
		cout << endl;
	}
}

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
};

int main() {
	signal (SIGTSTP, input_sigHandler);
	while (true) {
		signal(SIGINT, input_sigHandler);
		char** argv = NULL;
		vector<char*> commands;
		comParse comPr;

		char currentDir[512]; // Current Working Directory
		if (NULL == (getcwd(currentDir, 512))) { // Currently outputs error that directory is too long. Don't like this FIXME
			perror("getcwd");
			currentDir[0] = '\0';
		}

		do {
			cout << currentDir << " $ " << flush;
			
			string input = "";
			getline(cin, input);	// Get command

			commands = comPr.parseLine(input);
		} while (commands.size() == 1); // if empty line, repeat req for command. sz 1 = just NULL delim.

		bool doExec = true; // If false, do not fork and exec command

		//MASSIVE SECTION OF CODE DEDICATED TO PARSING FOR SEMI, PIPES
		//-------------------------------------------------------------------------
		int timeout = 0;
		vector<char*> realcom; // command + arguments - input redirection & semicolon
		vector< vector<char*> > realcomsP;
		vector<bool> isPipe;
		bool setPipe = false; // pipe will only be made if needed
		argv = &commands.at(0);
		redirect rdts;	// contains redirection flags and values

		for (unsigned int i = 0; i < commands.size() - 1; i++) { // NULL at end
			char* c = commands.at(i);
			// if semicolon or pipe, split command into two
			if ( strcmp(commands.at(i), ";") == 0) {
				delete[] commands.at(i);
				argv[i] = NULL;
				timeout = 1;

				realcom.push_back(NULL);
				realcomsP.push_back(realcom);
				realcom.clear();
				isPipe.push_back(false);
			} else if (strcmp(c, "|") == 0) {
				delete[] commands.at(i);
				argv[i] = NULL;
				timeout = 1;

				realcom.push_back(NULL);
				realcomsP.push_back(realcom);
				realcom.clear();
				isPipe.push_back(true);

				setPipe = true;
			} else if (strchr(c, '<') != NULL) {
				int fd = -1;
				// FIXME: Currently assumes 1 digit fd. K for assignment.
				// Convert char to int with - 48
				if (isdigit(commands.at(i)[0])) fd = commands.at(i)[0] - 48; // num before operator

				delete[] commands.at(i);
				argv[i] = NULL;

				timeout = 2;	// do not push this or next arg
				rdts.v_opfd.push_back( pair<int,int>(I_REDIRECT, fd) ); // op, fd

				if ( !(i + 1 < commands.size()) ) {	// next arg must exist (filename)
					cerr << "rshell: syntax error at <\n";
					doExec = false;
					break;
				}
				rdts.v_ind.push_back(i + 1); // index
			} else if (strchr(c, '>') != NULL) {
				int fd = -1;
				// FIXME: Currently assumes 1 digit fd. K for assignment.
				// Convert char to int with - 48
				if (isdigit(commands.at(i)[0])) fd = commands.at(i)[0] - 48; // num before operator

				if (strstr(commands.at(i), ">>>") != NULL) {
					cerr << "rshell: syntax error at >\n";
					doExec = false;
					break;
				}
				else if (strstr(commands.at(i), ">>") != NULL) 
					rdts.v_opfd.push_back( pair<int,int>(ORD_APPEND, fd) ); // op, fd
				else {
					rdts.v_opfd.push_back( pair<int,int>(O_REDIRECT, fd) ); // op, fd
//					c = commands.at(i - 1);
//					if (i - 1 < 0 || strcmp( c, ":" ) == 0) // should truncate file if :>
//						realcom.pop_back();
				}

				delete[] commands.at(i);
				argv[i] = NULL;

				timeout = 2;	// do not push this or next arg

				if ( !(i + 1 < commands.size() - 1) ) {	// next arg must exist (filename)
					cerr << "rshell: syntax error at <";
					exit(-1);
				}
				rdts.v_ind.push_back(i + 1); // index
			} 

			if (timeout != 0) timeout--;
			else realcom.push_back(commands.at(i));
		}

		realcom.push_back(NULL);
		realcomsP.push_back(realcom);
		isPipe.push_back(false);

		// Test produces weird results but gdb says contents are correct...wth?
		//for (unsigned int i = 0; i < realcomsP.size(); i++) {
		//	cerr << i << ": ";
		//	for (unsigned int j = 0; j < realcomsP.at(i).size(); j++) {
		//		cerr << realcomsP.at(1).at(j) << ' ';
		//	}
		//	cerr << endl;
		//}
		//exit(0);

		/* Confusing as heck, let me explain!
		 * char const* means pointer to a constant char
		 * so char const* const means a const pointer to a const char
		 * and char const* const* means a const pointer to a const char pointer!
		 * (Read declarations from right to left to make it make sense -
		 *  char const* = POINTER (*) to a CONST CHAR)
		 */
		//char const* const* c_arr = &commands[0]; // even if Not necessary: keep as really cool discovery!

		if (doExec != false) doExec = setRdct(argv, rdts);
		//if (rdts.doO_Rdct) doExec = redirection(argv, rdts, STDOUT_FILENO);
		//if (rdts.doE_Rdct) doExec = redirection(argv, rdts, STDERR_FILENO);
		char ex[5] = "exit";
		int fdpipe[2];
		int savefdpipe[2];
		bool resetPipe = false;

		if (setPipe) {
			if (-1 == pipe(fdpipe)) {
				perror("pipe");
				exit(-1);
			}
		}

		signal(SIGINT, quit_sigHandler);

		for (unsigned int i = 0; doExec && i < realcomsP.size(); i++) {
			argv = &realcomsP.at(i).at(0); // initialize to char**

			if (strcmp(argv[0], ex) == 0) {
				comPr.deleteCStrings(commands); // Has heap allocated memory, always should be dealt with
				return 0; // EXIT command
			}

			int pid = fork();
			if (pid == -1) {	// Error in fork
				perror("error in fork");
				comPr.deleteCStrings(commands);
				exit(-1);
			} else if (pid == 0) {	// child Process
				signal(SIGINT, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);

				if (0 == strcmp(argv[0], "cd")) exit(0); // cd: PARENT PROCESS

				// If piping, this command outputs to pipe
				if (isPipe.at(i)) {
					// move output's fd
					if (-1 == (savefdpipe[1] = dup(STDOUT_FILENO))) {
						perror("dup w/ pipe");
						exit(-1);
					}
					// place pipe in output
					if (-1 == dup2(fdpipe[1], STDOUT_FILENO)) {
						perror("dup w/ pipe");
						exit(-1);
					}
				}

				char* cpathvar = getenv("PATH");
				if (cpathvar == NULL) {
					perror("getenv");
					exit(-1);
				}

				string pathvar = cpathvar;
				typedef boost::tokenizer<char_separator<char> > tokenizer;
				char rm_delim[2] = {':', '\0'}; // null is delim, not char for filtering
				char_separator<char> charSep(rm_delim); // should ignore only whitespace
				tokenizer parse(pathvar, charSep);
				
				errno = 0;

				for (tokenizer::iterator it = parse.begin(); it != parse.end(); it++) {
					string s = *it + "/";
					s += argv[0];
					if (-1 != execv(s.c_str(), argv)) {
						break;
					}
				}

				if (errno != 0) { // perror for execv
					// All children need to exit! (forgot this)
					if (errno == EACCES) { // Access denied
						perror(argv[0]);
					} else if (errno == ENOEXEC) { // Not Executable
						perror(argv[0]);
					} else if (errno == ENOENT) { // Does not exist
						perror(argv[0]);
					} else {
						perror("error in execv");
					}

					exit(-1);
				}

				// if last command piped, next command's input is from pipe
				if (isPipe.at(i)) {
					// put output back in place
					if (-1 == dup2(savefdpipe[1],STDOUT_FILENO)) {
						perror("dup w/ pipe");
						exit(-1);
					}
					// close newly dup'ed output
					if (-1 == (close(savefdpipe[1]))) {
						perror("close w/ pipe");
						exit(-1);
					}
					// save input
					if (-1 == (savefdpipe[0] = dup(STDIN_FILENO))) {
						perror("dup w/ pipe");
						exit(-1);
					}
					// put pipe into input
					if (-1 == dup2(fdpipe[0],STDIN_FILENO)) {
						perror("dup w/ pipe");
						exit(-1);
					}

					resetPipe = true;
				} else if (resetPipe) {
					// put input back in place
					if (-1 == dup2(savefdpipe[0],STDIN_FILENO)) {
						perror("dup w/ pipe");
						exit(-1);
					}
					// close newly dup'ed input
					if (-1 == (close(savefdpipe[0]))) {
						perror("dup w/ pipe");
						exit(-1);
					}

					resetPipe = false;
				}

				exit(0);
			} else if (pid > 0) { // parent!
				// Ctrl Z: WUNTRACED is an option that lets the parent wait for processes that
				// have stopped too, not just terminated.
				// If stopped, add its pid to a queue of stopped pids and use kill(pid, SIGCONT) 
				// to continue it if user enters fg or bg (don't wait if bg, but keep process in queue)
				if (-1 == waitpid(pid, 0, WUNTRACED))
					perror("wait");
				if (errno != 0 && errno != EACCES && errno != ENOEXEC && errno != ENOENT)
					exit(-1);
				if (0 == strcmp(argv[0], "cd")) { // cd: PARENT PROCESS
					if (-1 == chdir(argv[1])) {
						perror("rshell: cd");
					}
				}
			}
		}

		if (setPipe) {
			for (int i = 0; i < 2; i++) {
				if (-1 == close(fdpipe[i])) {
					perror("close pipe");
					exit(-1);
				}
			}
		}

		for (unsigned int i = 0; i < rdts.v_savedP.size(); i++) {
			if (-1 == dup2(rdts.v_savedP.at(i), rdts.v_pfd.at(i))) {	// Restore saved fds
				perror("dup2");
				exit(-1);
			}
			if (-1 == close(rdts.v_savedP.at(i))) {	// Restore saved fds
				perror("close");
				exit(-1);
			}
		}

		comPr.deleteCStrings(commands);
		//rdts.closeCurrFDs();
	}

	return 0;
}


bool setRdct(char** argv, redirect& rdts) {
	if (rdts.v_opfd.empty()) return true;

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
	if (rdts.v_opfd.at(id).first == I_REDIRECT) {
		FLAG = O_RDONLY;
		if (rdts.v_opfd.at(id).second == -1) 
			rdts.v_opfd.at(id).second = 0; // DEFAULT for input operator
	} else if (rdts.v_opfd.at(id).first == O_REDIRECT) {
		FLAG = O_WRONLY | O_CREAT | O_TRUNC;
		if (rdts.v_opfd.at(id).second == -1) 
			rdts.v_opfd.at(id).second = 1; // DEFAULT for output operator
	} else if (rdts.v_opfd.at(id).first == ORD_APPEND) {
		FLAG = O_WRONLY | O_CREAT | O_APPEND;
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
	// if create file, then give read write by default
	if ( -1 == (newFD = open(argv[index], FLAG, S_IRUSR | S_IWUSR)) ) {
		if (errno == EACCES || errno == ENOENT) { 	//FIXME
			perror(argv[index]);
			rdts.v_pfd.push_back(rdts.v_opfd.at(id).second); // Saving new fd to close later
			return false;
		} else {
			perror("open-redirect" + rdts.v_opfd.at(id).second);
			exit(-1);
		}
	}
	rdts.v_pfd.push_back(newFD); // Saving new fd to close later

	return true;
}


