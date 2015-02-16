#ifndef COMPARSE_H
#define COMPARSE_H

#include <boost/tokenizer.hpp>
#include <iterator>
#include <string.h>

using namespace std;
using namespace boost;

class comParse {
private:
	vector<string> words;

public:
	vector<const char*> parseLine(const string com) {
		words.clear(); // Reuse of function requires empty arrays

		typedef tokenizer<char_separator<char> > tokenizer;

		char rm_delim[4] = {' ', '\t', '\n', '\0'}, // null is delim, not char for filtering
			kp_delim[5] = {'&', '|', ';', '#', '\0'};
		char_separator<char> charSep(rm_delim, kp_delim); // should ignore only whitespace
		tokenizer parse(com, charSep);

		// add every token to vector<string> words
		for (tokenizer::iterator it = parse.begin(); it != parse.end(); it++)
			words.push_back(*it);

		bool hashtag = false;
		vector<const char*> commands;
		// if string has hashtag, remove all subsequent strings
		// else add string to c_string equiv return vector
		for (vector<string>::iterator it = words.begin(); 
				!hashtag && it != words.end(); it++) { // In condition: if hashtag = true, stop loop
			if (*it == "#") {
				// Remove all subsequent words and current word from arg list
				words.erase(it, words.end());

				hashtag = true;
			} else commands.push_back(it->c_str());
		}

		commands.push_back(NULL); // when using data(), need null delim
				
		return commands;
	}

	int size() {
		return words.size();
	}
};

#endif
