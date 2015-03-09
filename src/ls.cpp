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

#define MANY_PER_LINE 0
#define LONG_FORM 1

struct FLAGS {
	bool IGNORE_DOT_AND_DOTDOT;
	bool long_format;
	bool many_per_line;
	bool recursive;
};

FLAGS all_flags;

void determine_arguments(int argc, char** argv, std::vector<std::string>& direc, std::vector<char>& flags);
void getFilesFromDirectory(DIR* dirp, std::vector<char*>& f, std::vector<std::pair<char*, unsigned char> >& t);
void scanFlags(const std::vector<char>& flags);
void printdir(const std::vector<char*> files, const std::vector<std::pair<char*, unsigned char> > types);
void recursive(std::vector<std::string>& direc, std::vector<char*> files, const char* dirName);

int main(int argc, char** argv)
{
	char DEFAULT[] = ".";
	std::vector<std::string> direc;
	std::vector<char> flags;

	all_flags.IGNORE_DOT_AND_DOTDOT = true; // May be changed by scanFlags()
	all_flags.many_per_line = true; // default

	determine_arguments(argc, argv, direc, flags); // sort arguments into directories and flags
	scanFlags(flags); // Trigger proper bools for flags

	unsigned int i = 0; // to enable loop if direc is not empty
	do {
		const char* dirName; // Get directory argument from direc or default
		if (direc.empty()) dirName = DEFAULT;
		else {
			dirName = direc.at(i).c_str();
			i++;
		}

		DIR *dirp = opendir(dirName);
		if (dirp == NULL) {
			perror("opendir");
			exit(-1);
		}
		
		std::vector<char*> files;
		std::vector<std::pair<char*, unsigned char> > filetypes;
		getFilesFromDirectory(dirp, files, filetypes);
		if (all_flags.recursive) recursive(direc, files, dirName);

		if (direc.size() > 1) { // Multiple directories -> output names too
			std::cout << dirName << ":" << std::endl;
			printdir(files, filetypes);
			if (i < direc.size()) std::cout << '\n'; // give newline if not last dir
		} else printdir(files, filetypes);

		if (-1 == (closedir(dirp))) {
			perror("closedir");
			exit(-1);
		}

	} while (i < direc.size());

	return 0;
}

void getFilesFromDirectory(DIR* dirp, std::vector<char*>& f, std::vector<std::pair<char*, unsigned char> >& t) {
	dirent *direntp;
	errno = 0;
	std::vector<char*> files;
	std::vector<std::pair<char*, unsigned char> > types;
	while ((direntp = readdir(dirp))) {
		if (direntp == NULL && errno != 0) {
			perror("readdir");
			exit(-1);
		}

		bool equalsDOT = strncmp(direntp->d_name, ".", 1) ? false : true; 
		bool equalsDOTDOT = strcmp(direntp->d_name, "..") ? false : true; 
		if (! all_flags.IGNORE_DOT_AND_DOTDOT || 
				(!equalsDOT  && !equalsDOTDOT )) {
			// use stat here to find attributes of file
			files.push_back(direntp->d_name);
			types.push_back(std::pair<char*, unsigned char>(direntp->d_name,
					direntp->d_type));
		}
	}

	f = files;
	t = types;
}

// takes arguments after ls and sorts them into directories and flags
void determine_arguments(int argc, char** argv, std::vector<std::string>& direc, std::vector<char>& flags) {
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && strlen(argv[i]) > 1) // flags = -<arguments>
			for (unsigned int j = 1; j < strlen(argv[i]); j++) {
				flags.push_back(argv[i][j]);
			}
		else direc.push_back(std::string(argv[i]));
	}

	return;
}

void scanFlags(const std::vector<char>& flags) {
	for (unsigned int i = 0; i < flags.size(); i++) {
		if (flags.at(i) == 'a') all_flags.IGNORE_DOT_AND_DOTDOT = false;
		else if (flags.at(i) == 'l') {
			all_flags.many_per_line = false;
			all_flags.long_format = true;
		} else if (flags.at(i) == 'R') 	all_flags.recursive = true;
		else {
			std::cout << "ls: invalid option -- '" << flags.at(i) << "'\n"
				<< "Try 'ls --help' for more information." << std::endl;
			exit(-1);
		}
	}
}

void printdir(const std::vector<char*> files, const std::vector<std::pair<char*, unsigned char> > types) {
	if (all_flags.many_per_line) print_files(files, types, MANY_PER_LINE);
	else if (all_flags.long_format) print_files(files, types, LONG_FORM);
}

void recursive(std::vector<std::string>& direc, std::vector<char*> files, const char* dirName) {
	std::sort(files.begin(), files.end());

	struct stat file_det;
	
	for (unsigned int i = 0; i < files.size(); i++) {
		std::string fullPath = dirName;
		fullPath += "/";
		fullPath += files.at(i);

		if (-1 == (stat(fullPath.c_str(), &file_det))) {
			perror("stat");
			exit(-1);
		}

		if ((file_det.st_mode & S_IFDIR) != 0) direc.push_back(fullPath.c_str());
	}
}
