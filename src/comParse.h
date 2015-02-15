#ifndef COMPARSE_H
#define COMPARSE_H

#include <boost/tokenizer.hpp>
#include <iterator>

using namespace std;
using namespace boost;

class comParse {
private:
	vector<string> words;
	vector<vector<char*> > separatedTokens; // keeps commands separated by operators in scope

public:
	vector<char**> parseLine(string com) {
		words.clear(); // Reuse of function requires empty arrays
		tokens_lines.clear(); // Need for argc int requires keeping array

		typedef tokenizer<char_separator<char> > tokenizer;

		char rm_delim[4] = {' ', '\t', '\n', '\0'},
			kp_delim[4] = {'&', '|', ';', '\0'};
		char_separator<char> charSep(rm_delim, kp_delim); // should ignore only whitespace
		tokenizer parse(com, charSep);

		// add every token to vector<string> words
		for (tokenizer::iterator it = parse.begin(); it != parse.end(); it++)
			words.push_back(*it);

		bool hashtag = false;
		// COMMENTS, if string has hashtag, remove everything else
		for (vector<string>::iterator it = words.begin(); 
				!hashtag && it != words.end(); it++) { // In condition: if hashtag = true, stop loop
			int a = it->find("#");

			if (a != string::npos) {
				it->erase(a, it->size()); // Remove all chars from #, onward

				// Remove all subsequent words and current word from arg list
				if ( a == 0 ) words.erase(it, words.end());
				// Remove all subsequent words
				else words.erase(it + 1, words.end());

				hashtag = true;
			}
		}
		
		
		// Semicolon search
		vector<char*> n;
		for (vector<string>::iterator it = words.begin(); it != words.end(); it++) {
			int indx = it->find(';');

			if (indx == string::npos) n.push_back(it->c_str()); // add normal word
			else {
				string before_sc = it->substr(0, indx); // substr, noninclusive 
				if ( !(before_sc.empty()) ) n.push_back(before_sc); // add word b/f ';'

				n.push_back(NULL); // data() turns vect<char*> into char**, so need NULL delim
				separatedTokens.push_back(n);

				n.clear(); // Begin next stream of tokens

				string after_sc = it->substr(indx + 1, string::npos); // substr, to end (npos means rest of string)

				// If other semicolons in string, repeat with string after_sc
				if (
				while ( (indx = after_sc.find(';')) != string::npos ) {
					before_sc = after_sc.substr(0, indx); // substr, noninclusive 
					if ( !(before_sc.empty()) ) n.push_back(before_sc); // add word b/f ';'

					n.push_back(NULL); // data() turns vect<char*> into char**, so need NULL delim
					separatedTokens.push_back(n);

					n.clear(); // Begin next stream of tokens
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

		for (unsigned int i = 0; i < commands.size(); i++)
			for (unsigned int j; commands.at(i)[j] != NULL; j++)
				cerr << commands.at(i)[j] << endl;
		
		return commands;
	}
};

#endif
