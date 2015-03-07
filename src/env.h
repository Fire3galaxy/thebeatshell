#ifndef ENV_H
#define ENV_H

#include <stdlib.h>
#include <boost/tokenizer.hpp>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <iterator>

class env {
private:
	string scope; // To keep returned values in scope
public:
	const string getpath(const char* program) {
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

		for (tokenizer::iterator it = parse.begin(); it != parse.end(); it++) {
			DIR* dirp = opendir(it->c_str());
			if (dirp == NULL) {
				//perror("opendir");	// Waiting for Mike Izbicki to move this to end of prog
				continue; 	// directory may not exist
			}

			dirent* ent;
			errno = 0; // if readdir returns NULL, check errno after while loop
			while (NULL != (ent = readdir(dirp))) {
				if (0 == strcmp(ent->d_name, program)) {
					if (-1 == closedir(dirp)) {
						perror("close");
						exit(-1);
					}
					return *it;
				}
			}

			if (errno != 0) {
				perror("readdir");
				exit(-1);
			}

			if (-1 == closedir(dirp)) {
				perror("close");
				exit(-1);
			}
		}

		return "";
	}
};

#endif
