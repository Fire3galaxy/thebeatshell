#include <iostream>
#include <boost/tokenizer.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include <map>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <list>
#include "comParse.h"
#include "redirect.h"
#include "digit.h"

using namespace boost;
using namespace std;

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

// If first char is '~', replace with home var
string replaceTilde(const char* c) {
	string s = c;

	if (!s.empty()) {
		if (s.at(0) == '~') {
			char* HOME = getenv("HOME");
			if (HOME != NULL) 
				return s.replace(0, 1, HOME);
		}
	} else {
		char* HOME = getenv("HOME");
		if (HOME != NULL) 
			return (s = HOME);
	}

	return s;
}


int main() {
	list<int> stopped_pids;
	signal (SIGTSTP, input_sigHandler); // Ctrl C

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

			// For sake of cs100-runtests
			if (cin.fail()) exit(0);
		} while (commands.size() == 1); // if empty line, repeat req for command. sz 1 = just NULL delim.

		bool doExec = true; // If false, do not fork and exec command
		
		//for (unsigned int i = 0; i < commands.size() - 1; i++) { // NULL at end
		//	cout << commands.at(i) << ' ';
		//}
		//cout << endl;
		//exit(0);

		//MASSIVE SECTION OF CODE DEDICATED TO PARSING FOR SEMI, CONNECTORS, PIPES
		//-------------------------------------------------------------------------
		int timeout = 0;
		vector<char*> realcom; // command + arguments - input redirection & semicolon
		vector< vector<char*> > realcomsP;
		vector<bool> isPipe;
		bool setPipe = false; // pipe will only be made if needed
		argv = &commands.at(0);
		redirect rdts;	// contains redirection flags and values

		map<char**, string> conditionToRun; // For && and ||
		bool expectCommand = false;
		string condition = "";

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
				conditionToRun[&realcomsP.back().at(0)] = "";
				if (expectCommand) {
					conditionToRun[&realcomsP.back().at(0)] = condition;
					expectCommand = false;
					condition = "";
				}

			} else if ( strchr(c, '&') != NULL) {
				if (0 != strcmp(c, "&&") || realcom.empty()) {
					cerr << "rshell: syntax error at " << c << "\n";
					doExec = false;
					break;
				}
				delete[] commands.at(i);
				argv[i] = NULL;
				timeout = 1;

				realcom.push_back(NULL);
				realcomsP.push_back(realcom);
				realcom.clear();
				isPipe.push_back(false);
				conditionToRun[&realcomsP.back().at(0)] = "";
				if (expectCommand) {
					conditionToRun[&realcomsP.back().at(0)] = condition;
					expectCommand = false;
					condition = "";
				}
				expectCommand = true;
				condition = "&&";
			} else if (strcmp(c, "||") == 0) {
				if (realcom.empty()) {
					cerr << "rshell: syntax error at " << c << "\n";
					doExec = false;
					break;
				}
				delete[] commands.at(i);
				argv[i] = NULL;
				timeout = 1;

				realcom.push_back(NULL);
				realcomsP.push_back(realcom);
				realcom.clear();
				isPipe.push_back(false);
				conditionToRun[&realcomsP.back().at(0)] = "";
				if (expectCommand) {
					conditionToRun[&realcomsP.back().at(0)] = condition;
					expectCommand = false;
					condition = "";
				}
				expectCommand = true;
				condition = "||";
			} else if (strcmp(c, "|") == 0) {
				if (realcom.empty()) {
					cerr << "rshell: syntax error at " << c << "\n";
					doExec = false;
					break;
				}
				delete[] commands.at(i);
				argv[i] = NULL;
				timeout = 1;

				realcom.push_back(NULL);
				realcomsP.push_back(realcom);
				realcom.clear();
				isPipe.push_back(true);

				setPipe = true;
				conditionToRun[&realcomsP.back().at(0)] = "";
				if (expectCommand) {
					conditionToRun[&realcomsP.back().at(0)] = condition;
					expectCommand = false;
					condition = "";
				}
				expectCommand = true; // should have something to pipe to
				condition = "|";
			} else if (strchr(c, '<') != NULL) {
				if (realcom.empty()) {
					cerr << "rshell: syntax error at " << c << "\n";
					doExec = false;
					break;
				}
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
				if (realcom.empty()) {
					cerr << "rshell: syntax error at " << c << "\n";
					doExec = false;
					break;
				}
				int fd = -1;
				// FIXME: Currently assumes 1 digit fd. K for assignment.
				// Convert char to int with - 48
				if (isdigit(commands.at(i)[0])) fd = commands.at(i)[0] - 48; // num before operator

				if (strstr(commands.at(i), ">>>") != NULL) { // 3 or more '>'
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
					cerr << "rshell: syntax error at >\n";
					exit(-1);
				}
				rdts.v_ind.push_back(i + 1); // index
			} 

			if (timeout != 0) timeout--;
			else realcom.push_back(commands.at(i));
		}

		if (!realcom.empty()) {
			realcom.push_back(NULL);
			realcomsP.push_back(realcom);
			isPipe.push_back(false);
			conditionToRun[&realcomsP.back().at(0)] = "";
			if (expectCommand) {
				conditionToRun[&realcomsP.back().at(0)] = condition;
				expectCommand = false;
				condition = "";
			}
		} else {
			if (expectCommand) {
				cerr << "rshell: missing arg after " << condition << endl;
				doExec = false;
			}
		}


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

		vector<int> retStat(realcomsP.size());

		signal(SIGINT, quit_sigHandler);

		for (unsigned int i = 0; doExec && i < realcomsP.size(); i++) {
			argv = &realcomsP.at(i).at(0); // initialize to char**
			
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
				// cd/fg (Do in parent process)
				if (0 == strcmp(argv[0], "cd") || 0 == strcmp(argv[0], "fg")) exit(0);

				// &&
				if (conditionToRun[argv] == "&&" && retStat.at(i - 1) != 0) exit(-1);
				// ||
				if (conditionToRun[argv] == "||" && retStat.at(i - 1) == 0) exit(-1);

				// Default signals for ctrl z, ctrl c
				signal(SIGINT, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);


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


				exit(0);
			} else if (pid > 0) { // parent!

				int status;

				// Ctrl Z: WUNTRACED is an option that lets the parent wait for processes that
				// have stopped too, not just terminated.
				// If stopped, add its pid to a queue of stopped pids and use kill(pid, SIGCONT) 
				// to continue it if user enters fg or bg (don't wait if bg, but keep process in queue)
				if (-1 == waitpid(pid, &status, WUNTRACED))
					perror("wait");
				if (errno != 0 && errno != EACCES && errno != ENOEXEC && errno != ENOENT)
					exit(-1);
				if (WIFSTOPPED(status)) {
					stopped_pids.push_back(pid);
					cout << "[" << stopped_pids.size() << "]+ Stopped\t\t" << argv[0] << endl;
				}
				if (WIFEXITED(status)) retStat.at(i) = WEXITSTATUS(status);
				if (WIFSIGNALED(status)) // Ctrl C stops entire line of command
					if (WTERMSIG(status) == SIGINT) break;

				// cd
				if (0 == strcmp(argv[0], "cd") && !(isPipe.at(i)) && !resetPipe ) { // cd: PARENT PROCESS
					string newDir = realcomsP.at(i).size() > 2 ? // Supports ~/ and no argument
						replaceTilde(argv[1]) : replaceTilde("");

					if (-1 == chdir(newDir.c_str()))
						perror("rshell: cd");
				}

				// fg
				int waiting_pid = -1;
				if (0 == strcmp(argv[0], "fg")) {
					if (stopped_pids.empty()) {
						cerr << "rshell: fg: " << argv[1] << ": no such job";
						exit(-1);
					}
					int stoppedID = stopped_pids.size() ? stopped_pids.size() : 1; // default, most recent or 1
					int sizePids = stopped_pids.size();

					//cout << endl << "stoppedID = " << stoppedID << endl
					//	<< "sizePids = " << sizePids << endl;

					// If fg has argument (id of stopped process)
					// vector composed of <fg> <arg> <null> (at least 3)
					if (realcomsP.at(i).size() > 2) {
						if (-1 != (stoppedID = toDigit(argv[1])) && sizePids >= stoppedID) {

							// get element at stoppedID - 1
							list<int>::iterator it = stopped_pids.begin(); 
							for (int i = 0; i < stoppedID - 1; i++, it++);
							int pidStopped = *it;

							// continue process, wait
							if (-1 != kill(pidStopped, SIGCONT)) {
								waiting_pid = pidStopped;
								stopped_pids.erase(it);
							} else perror("rshell: fg"); // No such job
							
						} else cerr << "rshell: fg: " << argv[1] << ": no such job";
					} 
					
					// Default fg
					else {
						// continue process, wait
						if (-1 != kill(stopped_pids.back(), SIGCONT)) {
							waiting_pid = stopped_pids.back();
							stopped_pids.pop_back();
						} else {
							string s = "rshell: fg";
							s +=  argv[1];
							perror(s.c_str()); // No such job
						}
					}
				}

				if (waiting_pid != -1) {
					if (-1 == waitpid(waiting_pid, &status, WUNTRACED))
						perror("wait");
					if (WIFSTOPPED(status)) {
						stopped_pids.push_back(waiting_pid);
						cout << "[" << stopped_pids.size() << "]+ Stopped\t\t" << argv[0] << endl;
					}
				}
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

