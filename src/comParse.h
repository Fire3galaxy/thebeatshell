#ifndef COMPARSE_H
#define COMPARSE_H

#include <boost/tokenizer.hpp>
#include <iterator>
#include <string.h>

using namespace std;
using namespace boost;

class comParse {
private:
	vector<char*> commands;
public:
	const vector<char*>& parseLine(const string com) {
		commands.clear();
		vector<string> words;

		typedef tokenizer<char_separator<char> > tokenizer;

		char rm_delim[4] = {' ', '\t', '\n', '\0'}, // null is delim, not char for filtering
			kp_delim[5] = {'&', '|', ';', '#', '\0'};
		char_separator<char> charSep(rm_delim, kp_delim); // should ignore only whitespace
		tokenizer parse(com, charSep);

		// add every token to vector<string> words
		for (tokenizer::iterator it = parse.begin(); it != parse.end(); it++)
			words.push_back(*it);

		bool hashtag = false;
		commands.resize(words.size());
		// if string has hashtag, remove all subsequent strings
		// else add string to c_string equiv return vector
		for (unsigned int i = 0; i < words.size() && !hashtag; i++) { // In condition: if hashtag = true, stop loop
			if (words.at(i) == "#") {
				// Remove all subsequent words and current word from arg list
				words.erase(words.begin() + i, words.end());

				hashtag = true;
			} else {
				commands.at(i) = new char[words.at(i).size()]; // Prepare space
				strcpy(commands.at(i), words.at(i).c_str()); // Copy!
			}
		}

		commands.push_back(NULL); // when using data(), need null delim
				
		return commands;
	}

	int size() {
		return commands.size();
	}

	void deleteCStrings(vector<char*>& commands) {
		for (unsigned int i = 0; i < commands.size(); i++) 
			if (commands.at(i) != NULL) 
				delete[] commands.at(i);
	}
};

#endif
