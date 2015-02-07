#ifndef COMPARSE_H
#define COMPARSE_H

#include <boost/tokenizer.hpp>
#include <iterator>

using namespace std;
using namespace boost;

class comParse {
private:
	vector<string> words;
	vector<vector<char*> > tokens_lines;

public:
	vector<char**> parseLine(string com) {
		words.clear(); // Reuse of function requires empty arrays
		tokens_lines.clear(); // Need for argc int requires keeping array

		typedef tokenizer<char_separator<char> > tokenizer;

		char rm_delim[4] = {' ', '\t', '\n', '\0'},
			kp_delim[4] = {'&', '|', ';', '\0'};
		char_separator<char> charSep(rm_delim, kp_delim); // should ignore only whitespace
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
		
		
		vector<char*> n;
		for (vector<string>::iterator it = words.begin(); it != words.end(); it++) {
			size_t semicolon = it->find(";");
			if (semicolon != string::npos) {
				n.push_back(&it->at(0));
			} else {
				while (semicolon != string::npos) {
					string s1 = it->substr(0, semicolon); // replace curr word of iterator with s1, then put s2 in new vector. Repeat this if more than 1 semicolon.
					string s2 = it->substr(semicolon, it->size() - semicolon);
					tokens_lines.push_back(n);
					n.clear();
				}
			}
		}

		vector<char**> commands; // Due to parsing format, statements like
		for (vector< vector<char*> >::iterator it = tokens_lines.begin();
				it != tokens_lines.end(); it++) {
			char** c_ = NULL;
			if (!(it->empty())) { // convert vec <char*> to char** array
				c_ = it->data();
				c_[it->size()] = NULL; // issue with data() return because it doesn't null delimit well.
				commands.push_back(c_);
			}
		}
		
		return commands;
	}
};

#endif
