#include <iostream>
#include <unistd.h>
#include <boost/tokenizer.hpp>
#include <iterator>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

using namespace boost;
using namespace std;

void executeCommand(tokenizer<> tok, string command);
void exitshell(tokenizer<> tok);

int main() {
	cout << "$ ";
	string command;
	getline(std::cin, command);
	
	tokenizer<> tok(command);

	executeCommand(tok, command);

	return -1;
}

void executeCommand(tokenizer<> tok, string command) {
	tokenizer<>::iterator first = tok.begin();

	int pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(1);
	} else if (pid == 0) {
		cout << "A child!" << endl;
		
		char c[3] = {""};
		char* const argv[1] = {c};
		execvp("pwd", argv);

		exit(0);
	} else {
		int childExecStatus = 0;

		if (-1 == wait(&childExecStatus)) {
			perror("wait"); // error with wait
			exit(-1);
		}
		if (childExecStatus == -1) {
			cerr << "last command"; // error with command from user
			exit(-1);
		}
	}
}

void exitshell(tokenizer<> tok) {
	tokenizer<>::iterator first = tok.begin(),
		 	second = tok.begin(), 
			last = tok.end(); // one-past-last
	second++; // should be one-past-last (one element in list) 

	// exit, FIXME: needs to account for commenting
	if (second == last && *first == "exit")	exit(0);
	
	cout << "it did not exit" << endl;
	return;
}
