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
#include <unordered_map>
#include "CachedVector.h"

#include <QtCore>

QDateTime g_fakeCurrentTime = QDateTime::currentDateTime();

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

action_params genAction() {
	qsrand(QDateTime::currentDateTime().toMSecsSinceEpoch());
	action_params res;
	int num1 = qrand() % 9;
	int num2 = qrand() % 9;

    int ri = num1*num2 + (num1 > 5 ? 1: 0);

	res[f_statement::Value] = variant(QString("%1x%2=").arg(num1).arg(num2).toStdString());
	res[f_result::Value] = variant(QString("%1").arg(ri).toStdString());
	res[f_success::Value] = variant(num1*num2==ri);

	return res;
}

void autoTest() {
	/**
	 *  \brief Generate EngineImpl interactions
	 */
	EngineImpl *e = new EngineImpl();
	for (int i = 0; i < 1; i++) {
		e->init("Таблица умножения", "Илья");
		e->begin();
		for (int j = 0; j < 3; j++) {
			e->addAction(genAction());
		}
		e->end();
	}
}

using namespace std;
class VectorMap {
public:
	VectorMap() {}
private:
	vector<int> v;
//	unordered_map<string, int> m;
};

class test1 {
public:
	test1(const std::string &m) : marker(m) {}
	test1(const test1 &v_o) {
		marker = v_o.marker;
	}
	test1 & operator=(test1 &&v_o) {
		DEBUG("operator = for &&; name -> %s\n", v_o.marker.c_str());
		return *this;
	}
	test1 & operator=(const test1 &v_o) {
		DEBUG("operator = for &; name -> %s\n", v_o.marker.c_str());
		return *this;
	}
    std::string marker;
};

test1 d1() {
//	return test1("& dima");
}

void DimaTest() {
	test1 t("init");
	t = test1("&& ref");
//	test1 t2("& ref");
	t = static_cast<const test1 &>(d1());
}

void CachedVectorTest() {
    utils::CachedVector<std::string ,int> v;

    v.emplace("first" ,1);
    v.emplace("second", 2);
    v.emplace("third", 3);

    for (int i: v) {
    	DEBUG("Next value: %d\n", i);
    }

    std::vector<std::string> keys = {"first", "third", "second"};
    for (std::string key: keys) {
//    	DEBUG("Search key: %s\n" , key.c_str());
//    	int n = v.get(key);

    	DEBUG("Val for key: %s -> %d\n", key.c_str(), v.get(key));
    }


//    for ( int i = 0; i < 100; i++ )
//    {
//        v.set( i , i );
//    }
//    for ( int i : v ) { cout << i << endl; }
}

void smallTest() {
	getWords(g_words);
	const char lc[] = "UTF-8";
	DEBUG("Setting locale: %s\n", lc);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
//	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

//	action_params params_map;
//	params_map["string_val"] = variant(getRandomWord());
//	params_map["int_val"] = variant(1);
//	params_map["float_val"] = variant(100.65123f);
//	params_map["datetime_val"] = variant(dateTime(215000));
//
//	action_params params_map1;
//	params_map1["string_val"] = variant(getRandomWord());
//	params_map1["int_val"] = variant(1);
//	params_map1["float_val"] = variant(100.65123f);
//	params_map1["datetime_val"] = variant(dateTime(215000));

	Engine *e = new EngineImpl();
//	e->begin();
//	e->addAction(params_map);
//	e->addAction(params_map1);
//	e->end();
}

void misc_test() {
	DEBUG("Sizeof int: %d\n", sizeof(int));
	DEBUG("Sizeof long: %d\n", sizeof(long));
	DEBUG("Sizeof double: %d\n", sizeof(double));
}

int main (int argc, char ** argv)
{
	setenv("FAKE_TIME", "1", 0);
	DEBUG("Main start\n");
	misc_test();
	DEBUG("Main finished\n");
	return 0;
}
