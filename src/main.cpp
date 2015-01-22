#include <stdio.h>
#include "EngineImpl.h"
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
	std::ifstream myfile ("../engine/files/word_set");
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

class c {
public:
	c() {}
	virtual void printme() = 0;
	virtual ~c() {;}
};

class d: public c {
public:
	d(int pv = 0) {var = pv;}
	virtual void printme() {DEBUG("Print\n");}
private:
	int var;
};

int main (int argc, char ** argv)
{
	c *cl = new d;
	cl->printme();

	getWords(g_words);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	action_params params_map;
	params_map["string_val"] = variant(getRandomWord());
	params_map["int_val"] = variant(1);
	params_map["float_val"] = variant(100.65123f);
	params_map["datetime_val"] = variant(dateTime(215000));

	Engine *e = new EngineImpl();
	e->begin();
	e->addAction(params_map);
	e->end();

	printf("main start\n");
	return 0;
}
