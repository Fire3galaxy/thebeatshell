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

int main(int argc, char** argv)
{
	bool IGNORE_DOT_AND_DOTDOT = true; // set to false for -a

	char dirName[] = ".";
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

	return 0;
}

