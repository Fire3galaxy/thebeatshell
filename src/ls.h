#ifndef LS_H
#define LS_H

#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <string.h>
#include <algorithm>
#include <assert.h>
#include <iomanip>
#include <ctype.h>

#define MIN_COLUMN_WIDTH 3 // Learned from GNU: . and 2 spaces b/w columns
#define MAX(arg1, arg2) arg1 > arg2 ? arg1 : arg2

void print_many_per_lines(std::vector<char*> files);
int get_columns_num();
bool cstringLS_cmp(char* a, char* b);
int indexFirstCharOfName(char* s);
	
bool cstringLS_cmp(char* a, char* b) { // for sort function. Ignore case.
	return strcasecmp(a + indexFirstCharOfName(a), b + indexFirstCharOfName(b)) <= 0;
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

void print_many_per_line(std::vector<char*> files) {
	std::sort(files.begin(), files.end(), cstringLS_cmp); // Alphabetical, ignore case

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
			int columnWidth = columnMaxWidth.at(j) + (j != num_columns ? 2 : 0);
			std::cout << std::setw(columnWidth); // diff column, diff width
			std::cout << files.at(k); // files + ' ' considered in columnsMaxWidth
		}
		std::cout << std::endl;
	}

	return;
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
