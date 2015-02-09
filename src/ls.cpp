#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string.h>
#include <vector>

#include "ls.h"

/*
 * This is a BARE BONES example of how to use opendir/readdir/closedir.  Notice
 * that there is no error checking on these functions.  You MUST add error 
 * checking yourself.
 */

bool IGNORE_DOT_AND_DOTDOT;

void determine_arguments(int argc, char** argv, std::vector<char*>& direc, std::vector<char>& flags);
std::vector<char*> getFilesFromDirectory(DIR* dirp);

int main(int argc, char** argv)
{
	IGNORE_DOT_AND_DOTDOT = true; // set to false for -a
	char DEFAULT[] = ".";
	std::vector<char*> direc;
	std::vector<char> flags;
	
	determine_arguments(argc, argv, direc, flags); // sort arguments into directories and flags

	unsigned int i = 0; // to enable loop if direc is empty
	do {
		char* dirName; // Get directory argument from direc or default
		if (direc.empty()) dirName = DEFAULT;
		else dirName = direc.at(i);

		DIR *dirp = opendir(dirName);
		if (dirp == NULL) {
			perror("opendir");
			exit(-1);
		}
		
		std::vector<char*> files = getFilesFromDirectory(dirp);

		if (direc.size() > 1) { // Multiple directories -> output names too
			std::cout << dirName << ":" << std::endl;
			print_many_per_line(files);
			if (i < direc.size() - 1) std::cout << '\n'; // give newline if not last dir
		} else print_many_per_line(files);

		if (-1 == (closedir(dirp))) {
			perror("closedir");
			exit(-1);
		}

		i++;
	} while (i < direc.size());

	return 0;
}

std::vector<char*> getFilesFromDirectory(DIR* dirp) {
	dirent *direntp;
	errno = 0;
	std::vector<char*> files;
	while ((direntp = readdir(dirp))) {
		if (direntp == NULL && errno != 0) {
			perror("readdir");
			exit(-1);
		}

		bool equalsDOT = strncmp(direntp->d_name, ".", 1) ? false : true; 
		bool equalsDOTDOT = strcmp(direntp->d_name, "..") ? false : true; 
		if (! IGNORE_DOT_AND_DOTDOT || 
				(!equalsDOT  && !equalsDOTDOT )) {
			// use stat here to find attributes of file
			files.push_back(direntp->d_name);
		}
	}

	return files;
}

// takes arguments after ls and sorts them into directories and flags
void determine_arguments(int argc, char** argv, std::vector<char*>& direc, std::vector<char>& flags) {
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && strlen(argv[i]) > 1) // flags = -<arguments>
			for (unsigned int j = 1; j < strlen(argv[i]); j++)
				flags.push_back(argv[i][j]);
		else direc.push_back(argv[i]);
	}

	return;
}
