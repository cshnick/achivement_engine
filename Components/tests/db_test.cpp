#include "Conventions.h"
#include <QtCore>
#include <QtSql>

using namespace AE;
void smallTest()
{
	const char lc[] = "UTF-8";
	DEBUG("Setting locale: %s\n", lc);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName(lc));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName(lc));

	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "action_db");
	db.setDatabaseName(QString(g_achivements_path::Value) + "/" + g_dbName::Value);
	if (!db.open()) {
		DEBUG_ERR("Unable to open database. An error occurred while opening the connection: %s\n", qPrintable(db.lastError().text()));
	}

	QSqlQuery q("", db);
	if (!q.exec(QString("select count(*) from AchivementsDone where SessionId = 28"))) {
		DEBUG_ERR("Cannot execute query\n");
	}
	q.first();
	DEBUG("Result value is: %d\n", q.value(0).toInt());
	DEBUG("2nd attempt: \n");
	if (!q.exec(QString("select count(Id) from AchivementsDone where SessionId = 28"))) {
		q.bindValue(0, "Id");
		DEBUG_ERR("Cannot execute query\n");
	}
	q.first();
	DEBUG("Result value is: %d\n", q.value(0).toInt());
	DEBUG();
}

template<size_t n>
struct cn {
	char data[n+1];
};

// (2)
template<size_t n>
cn<n> magic(cn<n>);


void templateTest() {
	//static counter
	char (&GetCounterValue(void const *))[1];

	// (3) текущее значение счетчика
	DEBUG("sizeof value %d\n", sizeof(magic(cn<0>())) - 1); // 0

	// (4) «инкремент»
	cn<1> magic(cn<0>);

	// (5) текущее значение счетчика
	DEBUG("sizeof value %d\n", sizeof(magic(cn<0>())) - 1);
}

int main (int argc, char ** argv)
{
	DEBUG("db_start\n");
	templateTest();
	DEBUG("db_finished\n");
	return 0;
}
