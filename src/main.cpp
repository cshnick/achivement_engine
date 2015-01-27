#include <stdio.h>
#include "EngineImpl.h"
#include "Conventions.h"
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

#include <QtCore>

static void getWords(std::vector<std::string> &words) {
	std::string line;
	std::ifstream myfile ("../achivement_engine/files/word_set");
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
//	c *cl = new d;
//	cl->printme();
	DEBUG("Main start\n");

	getWords(g_words);
	const char lc[] = "UTF-8";
	DEBUG("Setting locale: %s\n", lc);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	action_params params_map;
	params_map["string_val"] = variant(getRandomWord());
	params_map["int_val"] = variant(1);
	params_map["float_val"] = variant(100.65123f);
	params_map["datetime_val"] = variant(dateTime(215000));

	action_params params_map1;
	params_map1["string_val"] = variant(getRandomWord());
	params_map1["int_val"] = variant(1);
	params_map1["float_val"] = variant(100.65123f);
	params_map1["datetime_val"] = variant(dateTime(215000));

	Engine *e = new EngineImpl();
	e->begin();
	e->addAction(params_map);
	e->addAction(params_map1);
	e->end();

	DEBUG("Main finished\n");
	return 0;
}
