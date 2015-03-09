#ifndef LS_H
#define LS_H

#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>

#include <iostream>
#include <string.h>
#include <algorithm>
#include <assert.h>
#include <iomanip>
#include <ctype.h>
#include <sstream>

#define MIN_COLUMN_WIDTH 3 // Learned from GNU: . and 2 spaces b/w columns
#define MAX(arg1, arg2) arg1 > arg2 ? arg1 : arg2
#define MANY_PER_LINE 0
#define LONG_FORM 1
#define permis(filedeets, right, c) if ( (filedeets & right) != 0) putchar(c); \
	else putchar('-')

void print_files(std::vector<char*> files, std::vector<std::pair<char*, unsigned char> > types, int format);
void print_many_per_line(std::vector<char*> files, std::vector<std::pair<char*, unsigned char> > types);
void print_long_format(std::vector<char*> files, std::vector<std::pair<char*, unsigned char> > types);
int get_columns_num();
bool cstringLS_cmp(char* a, char* b);
int indexFirstCharOfName(char* s);

void print_files(std::vector<char*> files, std::vector<std::pair<char*, unsigned char> > types, int format) {
	if (format == MANY_PER_LINE) print_many_per_line(files, types);
	else if (format == LONG_FORM) print_long_format(files, types);
}
	
bool cstringLS_cmp(char* a, char* b) { // for sort function. Ignore case.
	return strcasecmp(a + indexFirstCharOfName(a), b + indexFirstCharOfName(b)) <= 0;
}

bool cstringLS_cmp2(std::pair<char*, unsigned char> a, std::pair<char*, unsigned char> b) { // for sort function. Ignore case.
	return strcasecmp(a.first + indexFirstCharOfName(a.first), b.first + indexFirstCharOfName(b.first)) <= 0;
}

// If file starts with ..<NAME>, sort by name, not by dots
int indexFirstCharOfName(char* s) {
	for (unsigned int i = 0; i < strlen(s); i++) 
		if (s[i] != '.') return i;

	return 0;
}

void print_test(std::vector<char*> files) {
	std::sort(files.begin(), files.end(), cstringLS_cmp); // Alphabetical, ignore case

	for (std::vector<char*>::iterator it = files.begin(); it != files.end();
			it++)
		std::cout << *it << std::endl;
}

void print_many_per_line(std::vector<char*> files, std::vector<std::pair<char*, unsigned char> > types) {
	std::sort(files.begin(), files.end(), cstringLS_cmp); // Alphabetical, ignore case
	std::sort(types.begin(), types.end(), cstringLS_cmp2); // Alphabetical, ignore case

	// learned from GNU (lab3) ls command, need at least 1 column,
	// need additional row if file.size / num_columns has remainder
	int charLineCount = get_columns_num();

	unsigned int num_columns = MAX(1, charLineCount / MIN_COLUMN_WIDTH); 
	unsigned int num_rows = files.size() / num_columns + 
		(files.size() % num_columns ? 1 : 0);

	
	bool foundROWbyCOLUMN = false;
	// record width of each column while counting
	std::vector<unsigned int> columnMaxWidth(num_columns, 0); 
	
	while (!foundROWbyCOLUMN) {
		// If column count or row count needs readjustment, set to false and
		// try again
		foundROWbyCOLUMN = true; 
		// if column and row are reset, reset the width vector too
		columnMaxWidth.assign(num_columns, 0);

		// count how many chars per row, compare to possible amount.
		// i -> goes through rows
		// j -> goes through columns
		// k -> actual index of files
		unsigned int i = 0, j = 0, k = 0;

		for (i = 0; i < num_rows; i++) {
			int lineLength = 0;
			for (j = 0, k = i; j < num_columns && k < files.size() 
					&& lineLength <= charLineCount; j++, k += num_rows) {

				// record widths of columns based on longest file name
				if (strlen(files.at(k)) > columnMaxWidth.at(j))
					columnMaxWidth.at(j) = strlen(files.at(k));

				//length of lines in chars
				if (columnMaxWidth.at(j) == 0) lineLength += strlen(files.at(k)); 
				else lineLength += columnMaxWidth.at(j);

				// space in between file names
				if (j != num_columns - 1) lineLength += 2; 
			}

			// If too many columns in initial guess (extremely likely)
			// and only need 1 row, set num_columns to files.size()
			if (num_columns > j) num_columns = j; 
			
			// File names too long for curr column count, loop again with 
			// diff column and row count
			if (lineLength > charLineCount) {  // Risk of being really inefficient. 
				num_columns--;
				num_rows = files.size() / num_columns + 
					(files.size() % num_columns ? 1 : 0);
				foundROWbyCOLUMN = false;
				break;
			}
		}
	}
	
	unsigned int i = 0, j = 0, k = 0; // for column width
	std::cout << std::left; 
	for (i = 0; i < num_rows; i++) {
		for (j = 0, k = i ; j < num_columns && k < files.size(); j++, k += num_rows) {
			std::string color;
			//if (DT_DIR == types.at(j).second) color.append("\x1b[32m");
			if (DT_REG == types.at(j).second) color.append("\x1b[31m");

			int columnWidth = columnMaxWidth.at(j) + (j != num_columns ? 2 : 0);
			std::cout << color << std::setw(columnWidth); // diff column, diff width
			std::cout << files.at(k) << "\x1b[39;49m"; // files + ' ' considered in columnsMaxWidth
		}
		std::cout << std::endl;
	}

	return;
}

void print_long_format(std::vector<char*> files, std::vector<std::pair<char*, unsigned char> > types) {
	std::sort(files.begin(), files.end(), cstringLS_cmp); // Alphabetical, ignore case

	std::ostringstream oss;

	struct stat file_details;
	struct passwd* user;
	struct group* grp;
	struct tm* timeMod;

	unsigned long bitsizeWidth = 0;
	for (unsigned int i = 0; i < files.size(); i++) {
		if (-1 == stat(files.at(i), &file_details)) {
			perror("stat");
			exit(-1);
		}


		oss << file_details.st_size;
		if (bitsizeWidth < oss.str().size()) {
			bitsizeWidth = oss.str().size();
		}
		oss.str("");
	}
	
	for (unsigned int i = 0; i < files.size(); i++) {
		if (-1 == stat(files.at(i), &file_details)) {
			perror("stat");
			exit(-1);
		}

		permis(file_details.st_mode, S_IFDIR, 'd');
		permis(file_details.st_mode, S_IRUSR, 'r');
		permis(file_details.st_mode, S_IWUSR, 'w');
		permis(file_details.st_mode, S_IXUSR, 'x');
		permis(file_details.st_mode, S_IRGRP, 'r');
		permis(file_details.st_mode, S_IWGRP, 'w');
		permis(file_details.st_mode, S_IXGRP, 'x');
		permis(file_details.st_mode, S_IROTH, 'r');
		permis(file_details.st_mode, S_IWOTH, 'w');
		permis(file_details.st_mode, S_IXOTH, 'x');

		if (NULL == (user = getpwuid( file_details.st_uid ))) {
			perror("ls.h- getpwuid");
			exit(-1);
		}
		std::cout << ' ' << user->pw_name; // owner's username

		if (NULL == (grp = getgrgid( file_details.st_gid ))) {
			perror("ls.h- getgrgid");
			exit(-1);
		}
		std::cout << ' ' << grp->gr_name; // group's username

		std::cout << ' ' << std::setw(bitsizeWidth) << file_details.st_size; // bit size

		timeMod = localtime(&file_details.st_mtime); // last modified time
		char timeString[80];
		strftime(timeString, sizeof(timeString), "%b %e %R", timeMod);
		std::cout << ' ' << timeString;
		
		std::cout << ' ' << files.at(i) << std::endl;
	}
}

int get_columns_num() {
	struct winsize sz;

	if (-1 == (ioctl(STDOUT_FILENO, TIOCGWINSZ, &sz)) ) {
		perror("ls.h: ioctl");
		exit(-1);
	}

	return sz.ws_col;
}

#endif
