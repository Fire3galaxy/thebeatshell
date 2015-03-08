#ifndef DIGIT_H
#define DIGIT_H

#include <string.h>
#include <ctype.h>
#include <math.h>

using namespace std;

int toDigit(char const* c_num) {
	unsigned int size = strlen(c_num);
	int number = 0;

	for (unsigned int i = 0; i < size; i++) {
		if (isdigit(c_num[i]))
			number += (c_num[i] - 48) * pow(10, size - i - 1);
		else return -1;
	}
	
	return number;
}


#endif
