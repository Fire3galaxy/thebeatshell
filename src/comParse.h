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

		char delim[4] = {' ', '\t', '\n', '\0'};
		char_separator<char> charSep(delim); // should ignore only whitespace
		tokenizer parse(com, charSep);

		for (tokenizer::iterator it = parse.begin(); it != parse.end(); it++)
			words.push_back(*it);

		bool hashtag = false;
		// COMMENTS, if string has hashtag, remove everything else
		for (vector<string>::iterator it = words.begin(); 
				!hashtag && it != words.end(); it++) {
			size_t a = it->find("#");

			if (a != string::npos) {
				it->erase(a, string::npos); // Remove all chars from #, onward

				// Remove all subsequent words and current word from arg list
				if (a == 0 && !words.empty()) words.erase(it, words.end());
				// Remove all subsequent words
				else words.erase(it + 1, words.end());

				hashtag = true;
			}
		}
		
		for (vector<string>::iterator it = words.begin(); it != words.end(); it++)
			tokens.push_back( &(it->at(0)) );

		char** c_ = NULL;
		if (!tokens.empty()) { // convert vec <char*> to char** array
			c_ = tokens.data();
			c_[tokens.size()] = NULL; // issue with data() return because it doesn't null delimit well.
		}

		return c_;
	}
	int size() {
		return tokens.size();
	}
};

#endif
