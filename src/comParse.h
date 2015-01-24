#ifndef COMPARSE_H
#define COMPARSE_H

#include <boost/tokenizer.hpp>
#include <iterator>

using namespace std;
using namespace boost;

class comParse {
private:
	vector<string> words;
	vector<char*> tokens;

public:
	char** parseLine(string com) {
		words.clear(); // Reuse of function requires empty arrays
		tokens.clear(); // Need for argc int requires keeping array

		typedef tokenizer<char_separator<char> > tokenizer;

		char delim[3] = {' ', '\t', '\0'};
		char_separator<char> charSep(delim); // should ignore only whitespace
		tokenizer parse(com, charSep);

		for (tokenizer::iterator it = parse.begin(); it != parse.end(); it++)
			words.push_back(*it);
		
		//char* const argv[] = new char[tokens.size() + 1];

		//for (int i = 0; i < tokens.size(); i++)
			//argv[i] = tokens.at(i).c_str();
		for (vector<string>::iterator it = words.begin(); it != words.end(); it++)
			tokens.push_back( &(it->at(0)) );

		return tokens.data();
	}
	int size() {
		return tokens.size();
	}
};

#endif
