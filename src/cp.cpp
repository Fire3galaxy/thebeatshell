/* C++ interface for program benchmark timer management. */
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unistd.h>

// extern "C" int gettimeofday(timeval *tp, void *tzp);
// extern "C" int getrusage(int who, struct rusage *rusag);
class Timer
{
public:
int start();
int elapsedWallclockTime (double &);
int elapsedUserTime (double &);
int elapsedSystemTime (double &);
int elapsedTime (double &wc, double &us, double &system);
private:
rusage old_us_time;
rusage new_us_time;
timeval old_wc_time;
timeval new_wc_time;
};
int
Timer::start()
{
if (gettimeofday (&this->old_wc_time, 0) == -1
|| getrusage (RUSAGE_SELF, &this->old_us_time) == -1)
return -1;
else
return 0;
}
int
Timer::elapsedWallclockTime (double &wc)
{
if (gettimeofday (&this->new_wc_time, 0) == -1)
return -1;
wc = (this->new_wc_time.tv_sec - this->old_wc_time.tv_sec)
+ (this->new_wc_time.tv_usec - this->old_wc_time.tv_usec) / 1000000.0;
return 0;
}
int
Timer::elapsedUserTime (double &ut)
{
	if (getrusage (RUSAGE_SELF, &this->new_us_time) == -1)
		return -1;
	ut = (this->new_us_time.ru_utime.tv_sec - this->old_us_time.ru_utime.tv_sec)
		+ ((this->new_us_time.ru_utime.tv_usec
		- this->old_us_time.ru_utime.tv_usec) / 1000000.0);
	return 0;
}
int
Timer::elapsedSystemTime (double &st)
{
if (getrusage (RUSAGE_SELF, &this->new_us_time) == -1)
return -1;
st = (this->new_us_time.ru_stime.tv_sec - this->old_us_time.ru_stime.tv_sec)
+ ((this->new_us_time.ru_stime.tv_usec
- this->old_us_time.ru_stime.tv_usec) / 1000000.0);
return 0;
}
int
Timer::elapsedTime (double &wallclock, double &user_time, double &system_time)
{
if (this->elapsedWallclockTime (wallclock) == -1)
return -1;
else
{
if (getrusage (RUSAGE_SELF, &this->new_us_time) == -1)
return -1;
user_time = (this->new_us_time.ru_utime.tv_sec
- this->old_us_time.ru_utime.tv_sec)
+ ((this->new_us_time.ru_utime.tv_usec
- this->old_us_time.ru_utime.tv_usec) / 1000000.0);
system_time = (this->new_us_time.ru_stime.tv_sec
- this->old_us_time.ru_stime.tv_sec)
+ ((this->new_us_time.ru_stime.tv_usec
- this->old_us_time.ru_stime.tv_usec) / 1000000.0);
return 0;
}
}
/* Example of use
#include "Timer.h"
#include <iostream>
int main()
{
	Timer t;
	double eTime;
	t.start();
	for (int i=0, j; i<1000000000; i++)
		j++;
	t.elapsedUserTime(eTime);
	cout << eTime << endl;
}
*/

void copyByFStream(char* input, char* output);
void copyByReadByte(char* input, char*output);
void copyByReadBuf(char* input, char*output);

int main(int argc, char** argv) {
	if (argc > 4) {
		std::cout << "cp: insufficient number of args" << std::endl;
		exit(-1);
	}
	if (argc < 3) { 
		std::cout << "cp: insufficient number of args" << std::endl;
		exit(-1);
	}

	struct stat buf;
	if (stat(argv[2], &buf) != -1) {
		perror("output file exists");
		exit(-1);
	}

	if (argc == 3) copyByFStream(argv[1], argv[2]);
	else if (argc == 4) {
		Timer t1;
		double eTime, sysTime, wallTime;
		t1.start();

		copyByFStream(argv[1], argv[2]);

		t1.elapsedUserTime(eTime);
		t1.elapsedSystemTime(sysTime);
		t1.elapsedWallclockTime(wallTime);
		std::cout << "user runtime get/put: " << eTime << std::endl;
		std::cout << "system runtime get/put: " << sysTime << std::endl;
		std::cout << "wallclock runtime get/put: " << wallTime << std::endl;
		std::cout << std::endl;

		Timer t2;
		double eTime2, sysTime2, wallTime2;
		t2.start();
		copyByReadByte(argv[1], argv[2]);
		
		t2.elapsedUserTime(eTime2);
		t2.elapsedSystemTime(sysTime2);
		t2.elapsedWallclockTime(wallTime2);
		std::cout << "user runtime read/write (byte): " << eTime2 << std::endl;
		std::cout << "system runtime read/write (byte): " << sysTime2 << std::endl;
		std::cout << "wallclock runtime read/write (byte): " << wallTime2 << std::endl;
		std::cout << std::endl;

		Timer t3;
		double eTime3, sysTime3, wallTime3;
		t3.start();
		copyByReadByte(argv[1], argv[2]);
		
		t3.elapsedUserTime(eTime3);
		t3.elapsedSystemTime(sysTime3);
		t3.elapsedWallclockTime(wallTime3);
		std::cout << "user runtime read/write (buffer): " << eTime3 << std::endl;
		std::cout << "system runtime read/write (buffer): " << sysTime3 << std::endl;
		std::cout << "wallclock runtime read/write (buffer): " << wallTime3 << std::endl;
		std::cout << std::endl;
	} 

	return 0;
}

void copyByFStream(char* input, char* output) {
	std::ifstream ifs(input);
	std::ofstream ofs(output);

	if (!ifs.is_open()) {
		std::cout << "input file not found" << std::endl;
		exit(-1);
	} 

	// METHOD 1
	char c;
	while (ifs.get(c)) // ifstream and ofstream
		ofs.put(c);

	ifs.close();
	ofs.close();
}

void copyByReadByte(char* input, char* output) {
	int fdIn;
	if (-1 == (fdIn = open(input,O_RDONLY))) {
		perror("open failed on input file");
		exit(-1);
	}
	
	int fdOut;
	if (-1 == (fdOut = open(output,O_RDWR))) {
		perror("open failed on output file");
		exit(-1);
	}


	// METHOD 2
	char cbuf[1];
	int size;
	while ((size = read(fdIn, cbuf, 1)) != 0) {
		if (-1 == size) {
			perror("read for input fail");
			exit(-1);
		}

		if (-1 == (size = write(fdOut, cbuf, 1))) {
			perror("write to output fail");
			exit(-1);
		}
	}
	
	close(fdIn);
	close(fdOut);

}

void copyByReadBuf(char* input, char*output) {
	int fdIn;
	if (-1 == (fdIn = open(input,O_RDONLY))) {
		perror("open failed on input file");
		exit(-1);
	}
	
	int fdOut;
	if (-1 == (fdOut = open(output,O_RDWR))) {
		perror("open failed on output file");
		exit(-1);
	}


	// METHOD 3
	char cbuf2[BUFSIZ + 1];
	int size;
	while ((size = read(fdIn, cbuf2, BUFSIZ)) != 0) {
		if (-1 == size) {
			perror("read for input fail"); exit(-1); }

		if (-1 == (size = write(fdOut, cbuf2, size))) {
			perror("write to output fail");
			exit(-1);
		}
	}

	close(fdIn);
	close(fdOut);
}
