#ifndef LS_H
#define LS_H

#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>

void print_many_per_lines(std::vector<char[]> files);
int find_column_size();

void print_many_per_lines(std::vector<char[]> files) {
	return;
}

int find_column_size() {
	struct winsize sz;

	if (-1 == (ioctl(STDOUT_FILENO, TIOCGWINSZ, &sz)) ) {
		perror("ls.h- ioctl");
		exit(-1);
	}

	return sz.ws_col;
}

#endif
