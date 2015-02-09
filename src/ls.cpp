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
void determine_arguments(int argc, char** argv, std::vector<char*>& direc, std::vector<char>& flags);

int main(int argc, char** argv)
{
	bool IGNORE_DOT_AND_DOTDOT = true; // set to false for -a
	char DEFAULT[] = ".";
	std::vector<char*> direc;
	std::vector<char> flags;
	
	determine_arguments(argc, argv, direc, flags); // sort arguments into directories and flags

	do {
		char* dirName; // Get directory argument from direc or default
		if (direc.empty()) dirName = DEFAULT;
		else dirName = direc.at(0);

		DIR *dirp = opendir(dirName);
		if (dirp == NULL) {
			perror("opendir");
			exit(-1);
		}
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

		print_many_per_line(files);

		if (-1 == (closedir(dirp))) {
			perror("closedir");
			exit(-1);
		}
	} while (i < direc.size());

	return 0;
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
