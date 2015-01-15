#include <stdio.h>
#include "EngineImpl.h"
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

static void getWords(std::vector<std::string> &words) {
	std::string line;
	std::ifstream myfile ("../files/word_set");
	if (myfile.is_open()) {
		while ( getline (myfile,line, ' ') ) {
			if (line.length() > 3) {
				words.push_back(line);
			}
		}
		myfile.close();
	} else DEBUG_ERR("Unable to open file");

}
static std::vector<std::string> g_words;

std::string getRandomWord() {
	std::string result;
	std::srand(std::time(0)); // use current time as seed for random generator
	int ri = std::rand() % g_words.size();
	result = g_words.at(ri);

	return result;
}

using namespace AE;

int main (int argc, char ** argv)
{
	getWords(g_words);

	action_params params_map;
	params_map["String value"] = AE::variant(getRandomWord());
	params_map["Int value"] = AE::variant(1);
	params_map["Float value"] = AE::variant(100.65123f);
	params_map["Datetime value"] = AE::variant("215000");

	Engine *e = new EngineImpl();
	e->begin();
	e->addAction(params_map);
	e->end();

	printf("main start\n");
	return 0;
}
