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

#define MIN_COLUMN_WIDTH 3 // Learned from GNU: .. and space b/w columns
#define MAX(arg1, arg2) arg1 > arg2 ? arg1 : arg2

void print_many_per_lines(std::vector<char*> files);
int get_columns_num();

void print_many_per_line(std::vector<char*> files) {
	std::sort(files.begin(), files.end()); // Alphabetical

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
		columnMaxWidth.clear();

		// count how many chars per row, compare to possible amount.
		// i -> goes through rows
		// j -> goes through columns
		// k -> actual index of files
		unsigned int i = 0, j = 0, k = 0;

		for (i = 0; i < num_rows; i++) {
			int lineLength = 0;
			for (j = 0, k = i; j < num_columns && k < files.size(); 
					j++, k += num_rows) {
				//length of lines in chars
				lineLength += strlen(files.at(j)); 

				// record widths of columns based on longest file name
				if (strlen(files.at(j)) > columnMaxWidth.at(i))
					columnMaxWidth.at(i) = strlen(files.at(j));

				// space in between file names
				if (j != num_columns - 1) {
					lineLength += 1; 
					(columnMaxWidth.at(i) ) ++;
				}
			}
			// File names too long for curr column count, loop again with 
			// diff column and row count
			if (lineLength > charLineCount) {  // Risk of being really inefficient. 
				num_columns -= 1;
				num_rows = files.size() / num_columns + 
					(files.size() % num_columns ? 1 : 0);
				foundROWbyCOLUMN = false;
				break;
			}
		}
	}
	
	int k = 0; // for column width
	for (unsigned int i = 0; i < num_rows; i++) {
		for (unsigned int j = i ; j < num_columns && j < files.size(); j += num_rows) {
			std::cout << std::setw( columnMaxWidth.at(k) ); // diff column, diff width
			std::cout << files.at(j); // files + ' ' considered in columnsMaxWidth

			k++;
		}
		std::cout << std::endl;
		k = 0;
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
