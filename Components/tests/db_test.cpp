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

void SelectTest(QSqlQuery &q) {
	auto s = Select(f_id::Value, f_name::Value)
						.from(t_achivements_list::Value)
						.where(Condition(AE::f_id::Value,"=",2)
						);

	s.exec(q);
	bool first = q.first();
	bool valid = q.isValid();

	DEBUG("Result 0: %s\n", printable(q.value(0)));
	DEBUG("Result 1: %s\n", printable(q.value(1)));
}
void UpdateTest(QSqlQuery &q) {
	auto u = Update(t_achivements_list::Value)
						.set(Condition(f_visible::Value,"=",0))
						.where(Condition(f_id::Value,"=",1));
	QList<Condition> defaultConditions;
	defaultConditions.append(Condition(f_user::Value,"=",1));
	defaultConditions.append(Condition(f_project::Value,"=",1));
	u.addWhereConditions(defaultConditions);

	DEBUG("u expression: %s\n", u.expression().toUtf8().data());
	//	u.exec(q);
	DEBUG("last error: %s\n", q.lastError().text().toUtf8().data());
	//	QVariant v = QVariant::fromValue(s);
	//	DEBUG("Type of v: %d; own type: %d\n", v.type(), s.variantType());
	//	DEBUG("Select Variant to string: %s\n", v.toString().toUtf8().data());
}
void InsertIntoTest(QSqlQuery &q) {
	auto i = InsertInto(t_users::Value)
					.keys(f_name::Value)
					.values("TestUser1");

	DEBUG("INSERT INTO expression: %s\n", i.expression().toUtf8().data());

	auto i1 = InsertInto(t_users::Value)
					.values("TestUser1");

	DEBUG("INSERT INTO expression: %s\n", i1.expression().toUtf8().data());

	auto i2 = InsertInto(t_users::Value)
					.keys(f_id::Value, f_name::Value, f_passwd::Value)
					.values(1, "TestUser2", "lkskjt");

	DEBUG("INSERT INTO expression: %s\n", i2.expression().toUtf8().data());

	auto i3 = InsertInto(t_users::Value)
					.keys(f_name::Value, f_passwd::Value)
					.values("TestUser2", "sdstetwst");

	//	i3.exec(q);
}
void CreateTableTest(QSqlQuery &q) {
	QString testTable1 = "TestTable1";
	auto c = CreateTable(testTable1);
	c.add(FieldInfo("custom_id", dtype::INTEGER, "PRIMARY KEY"));
	c.add(FieldInfo("f1", dtype::DATETIME));
	c.add(FieldInfo("f2", dtype::STRING));
	c.add(FieldInfo("f3", dtype::INTEGER));
	c.exec(q);

	QString testTable2 = "TestTable2";
	auto c1 = CreateTable(testTable2);
	c1.add(FieldInfo("custom_id", dtype::INTEGER, "PRIMARY KEY"));
	c1.add(FieldInfo("f4", dtype::DATETIME));
	c1.add(FieldInfo("f5", dtype::STRING));
	c1.add(FieldInfo("f6", dtype::INTEGER));
	c1.add(ForeignKey("f6", Reference(t_users::Value, f_id::Value)));
	c1.exec(q);

	q.exec("DROP TABLE " + testTable1);
	q.exec("DROP TABLE " + testTable2);
}

void SqlClassesTest() {
	const char lc[] = "UTF-8";
	DEBUG("Setting locale: %s\n", lc);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName(lc));
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "action_db");
	db.setDatabaseName(QString(g_achivements_path::Value) + "/" + g_dbName::Value);
	if (!db.open()) {
		DEBUG_ERR("Unable to open database. An error occurred while opening the connection: %s\n", qPrintable(db.lastError().text()));
	}
	QSqlQuery q("", db);
//	SelectTest(q);
//	UpdateTest(q);
//	InsertIntoTest(q);
	CreateTableTest(q);
}

int main (int argc, char ** argv)
{
	DEBUG("db_start\n");
	SqlClassesTest();
	DEBUG("db_finished\n");
	return 0;
}
