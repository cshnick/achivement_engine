#include "Conventions.h"
#include <QtCore>
#include <QtSql>
#include <QMetaType>

#include "SQL.h"

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

using namespace Wrap_Sql;
QString SelectToString(const Select &d)
{
    return d.expression();
}

class cl1 {
public:
	QString toString() const {return val;}
private:
	QString val;
};
Q_DECLARE_METATYPE(cl1)


void SqlClassesTest() {
	auto s = Select(f_id::Value, f_name::Value).from(t_actions::Value, t_users::Value, t_projects::Value);
	QVariant v = QVariant::fromValue(s);
	DEBUG("Type of v: %d; own type: %d\n", v.type(), s.variantType());
	int tp = QMetaType::type("Wrap_Sql::Select");
	const char* nm = v.typeName();
	DEBUG("Tp is %d; type name: %s\n", tp, nm);
//	QMetaType::registerConverter(&SelectToString);
//	QMetaType::registerConverter(&Select::expression);
//	QMetaType::registerConverter<cl1, QString>(&cl1::toString);
	QMetaType::registerConverter(&cl1::toString);
	DEBUG("Select expression result: %s\n", s.expression().toUtf8().data());
}

int main (int argc, char ** argv)
{
	DEBUG("db_start\n");
	SqlClassesTest();
	DEBUG("db_finished\n");
	return 0;
}
