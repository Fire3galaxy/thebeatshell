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
			kp_delim[7] = {'&', '|', ';', '#', '<', '>', '\0'};
		char_separator<char> charSep(rm_delim, kp_delim); // should ignore only whitespace
		tokenizer parse(com, charSep);

		// add every token to vector<string> words
		// special exception for redirection sym: if #>, combine tokens
		int countIR = 0, countOR = 0;
		for (tokenizer::iterator it = parse.begin(); it != parse.end(); it++) {
			if (*it == "<") {
				int inpInd = -1;
				
				// if more than 1 '<', find respective index of token
				for (int i = 0; i <= countIR; i++) inpInd = com.find_first_of('<', inpInd + 1);

				if (inpInd == -1) {
					cerr << "find_first_of\n";
					exit(-1);
				}
				
				if (!words.empty() && (inpInd - 1 < 0 || isdigit(com.at(inpInd - 1)))) {
					if (inpInd - 2 < 0  || isspace(com.at(inpInd - 2)))
						words.back() += *it;
					else words.push_back(*it);
				} else words.push_back(*it);

				countIR++;
			} else if (*it == ">") {
				int inpInd = -1;
				
				for (int i = 0; i <= countOR; i++) inpInd = com.find_first_of('>', inpInd + 1);

				if (inpInd == -1) {
					cerr << "find_first_of\n";
					exit(-1);
				}
				
				if (!words.empty() && (inpInd - 1 < 0 || isdigit(com.at(inpInd - 1)))) { 	// #> syntax
					if (inpInd - 2 < 0  || isspace(com.at(inpInd - 2)))
						words.back() += *it;
					else words.push_back(*it);
				} else if (!words.empty() && inpInd - 1 >= 0 && com.at(inpInd - 1) == '>') {	// >> syntax
					words.back() += *it;
				} else words.push_back(*it);

				countOR++;
			} else words.push_back(*it);
		}


		bool hashtag = false;
		commands.resize(words.size());
		// if string has hashtag, remove all subsequent strings
		// else add string to c_string equiv return vector
		for (unsigned int i = 0; i < words.size() && !hashtag; i++) { // In condition: if hashtag = true, stop loop
			if (words.at(i) == "#") {
				// Remove all subsequent words and current word from arg list
				words.erase(words.begin() + i, words.end());
				commands.resize(i);

				hashtag = true;
			} else {
				commands.at(i) = new char[words.at(i).size() + 1]; // Prepare space
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
