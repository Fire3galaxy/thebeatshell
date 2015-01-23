#include <iostream>
#include <unistd.h>
#include <boost/tokenizer.hpp>
#include <iterator>

using namespace boost;
using namespace std;

int main() {
	std::cout << "$";
	std::string command;
	getline(std::cin, command);
	
	boost::tokenizer<> tok(command);
	tokenizer<>::iterator first = tok.begin(), second = tok.begin(), last = tok.end();
	second++;

	// exit, needs to account for commenting
	if (second == last && *first == "exit") // Only one wrod in string
		return 0;
	
	cout << "it did not exit" << endl;

	return -1;
}
