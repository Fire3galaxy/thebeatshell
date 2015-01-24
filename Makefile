VPATH = src
CXXFLAGS = -ansi -pedantic -Wall -Werror
objects = rshell.o

all: $(objects) | bin
	g++ -o rshell $(objects)
	mv rshell bin

rshell: $(objects) | bin
	g++ -o rshell $(objects)
	mv rshell bin

rshell.o: comParse.h

bin:
	mkdir bin

clean:
	rm -fr rshell *.o bin
