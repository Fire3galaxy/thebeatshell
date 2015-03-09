#ifndef REDIRECT_H
#define REDIRECT

#include <vector>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#define I_REDIRECT 0
#define O_REDIRECT 1
#define ORD_APPEND 2
 
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

bool setRdct(char** argv, redirect& rdts);
bool redirection(char** argv, redirect& rdts, const int id);

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

#endif
