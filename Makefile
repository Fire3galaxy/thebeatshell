VPATH = src
CXXFLAGS = -ansi -pedantic -Wall -Werror -g
objects = rshell.o

all: $(objects) | bin
	g++ -o rshell rshell.o
	mv rshell bin

rshell: rshell.o | bin
	g++ $(CXXFLAGS) -o rshell rshell.o 
	mv rshell bin

rshell.o: comParse.h env.h

ls: ls.o | bin
	g++ $(CXXFLAGS) -o ls ls.o
	mv ls bin

ls.o: ls.h

bin:
	mkdir bin

clean:
	rm -fr rshell ls *.o bin
