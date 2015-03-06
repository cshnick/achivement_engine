#include "SQL.h"
#include "Conventions.h"
#include <QtCore>
#include <QtSql>
#include <QMetaType>


using namespace AE;
void smallTest()
{
	const char lc[] = "UTF-8";
	DEBUG("Setting locale: %s\n", lc);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName(lc));
//	QTextCodec::setCodecForCStrings(QTextCodec::codecForName(lc));

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

using namespace Wrap_Sql;

void SqlClassesTest() {
	auto s = Select(f_id::Value, f_name::Value).from(t_actions::Value, t_users::Value, t_projects::Value);
	QVariant v = QVariant::fromValue(s);
	DEBUG("Type of v: %d; own type: %d\n", v.type(), s.variantType());
	DEBUG("Select Variant to string: %s\n", v.toString().toUtf8().data());
}

int main (int argc, char ** argv)
{
	DEBUG("db_start\n");
	SqlClassesTest();
	DEBUG("db_finished\n");
	return 0;
}
