#include "EngineImpl.h"

#include <QtCore>
#include <QtSql>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <cstdlib>

#include <iostream>

#define VERBOSE getenv("VERBOSE") != NULL
#define EXEC_AND_REPORT_COND \
		if (!q.exec()) { \
		SQL_ERR( "last error: %s, executed query: %s\n", qPrintable(q.lastError().text()), qPrintable(q.executedQuery()) ); \
			return; \
		} \

static const QString g_dbName = "/home/ilia/.local/share/action_engine/ae.db";
static const QString s_sessions = "Sessions";
static const QString s_actions = "Actions";
static const QString f_id = "id";
static const QString f_start = "Start";
static const QString f_finish = "Finish";
static const QString f_session = "Session";

__attribute__((constructor)) static void initialize_db_path() {
//	QString dirname = QFileInfo(g_dbName).dir().absolutePath();
//	if (VERBOSE) {
//		fprintf(stdout, "Retreiving dir name: %s", qPrintable(dirname));
//	}
//	if (!QFile::exists(dirname)) {
//		QDir().mkpath(dirname);
//	}
}

namespace AE {

class EngineImplPrivate {
public:
	EngineImplPrivate(EngineImpl *p_q) :
		q(p_q),
		m_session_id(-1)
	{
		if (VERBOSE) {
			DEBUG("Initializing database\nSetting database name: %s", qPrintable(g_dbName));
		}
		m_db = QSqlDatabase::addDatabase("QSQLITE", "action_db");
		m_db.setDatabaseName(g_dbName);
		if (!m_db.open()) {
			DEBUG_ERR("Unable to open database. An error occurred while opening the connection: %s\n", qPrintable(m_db.lastError().text()));
		}
		addDefaultTables();
	}
	bool checkDB() {
		if (!m_db.isOpen()) {
			DEBUG_ERR( "last error: %s\n", qPrintable(m_db.lastError().text()) );
			return false;
		}
		return true;
	}
	void begin() {
		if (!checkDB()) return;
		QSqlQuery q("", m_db);

		q.prepare(QString("INSERT INTO %1 (%2) VALUES (?)")
				.arg(s_sessions)
				.arg(f_start));
		q.bindValue(0, QDateTime::currentDateTime());
		EXEC_AND_REPORT_COND;
		m_session_id = q.lastInsertId().toInt();
	}
	void end() {
		if (!checkDB()) return;
		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			DEBUG_ERR("m_session_id is negtive. Called end several times?\n");
		}
		q.prepare(QString("UPDATE %1 SET %2 = :fin_time WHERE %3 = :id")
				.arg(s_sessions)
				.arg(f_finish)
				.arg(f_id));
		q.bindValue(":fin_time", QDateTime::currentDateTime());
		q.bindValue(":id", m_session_id);
		EXEC_AND_REPORT_COND;
		m_session_id = -1;
	}
	void addAction(const action_params &p_actions) {
		if (!checkDB()) return;
		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			DEBUG_ERR("m_session_id is negtive. Haven't called begin method for engine?\n");
		}
		QSqlRecord rec = m_db.record(s_actions);
		if (!rec.isEmpty()) {
			std::map<std::string, std::string>::const_iterator i = p_actions.begin();
			std::map<std::string, std::string>::const_iterator end = p_actions.end();
			QStringList kl, vl; //Store keys and values
			while (i != end) {
				QString nm = QString::fromStdString(i->first);
				QString v = QString::fromStdString(i->second);
				kl << nm;
				vl << QString("'%1'").arg(v);
				//Create non existent fields
				if (!rec.contains(nm)) {
					q.prepare(QString("ALTER TABLE %1 ADD %2 STRING")
							.arg(s_actions)
							.arg(nm));
					EXEC_AND_REPORT_COND;
				}
				i++;
			}

			q.clear();
			q = QSqlQuery("", m_db);
			// Prepare strings to INSERT statement
			QString kp = kl.join(",");
			QString vp = vl.join(",");
			q.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)")
					.arg(s_actions)
					.arg(kp)
					.arg(vp));
			EXEC_AND_REPORT_COND;

			SQL_DEBUG("Rec is not empty\n");
			SQL_DEBUG("Fields names are:\n");
			for (int i = 0; i < rec.count(); i++) {
				SQL_DEBUG("\t%s;\n", qPrintable(rec.fieldName(i)));
			}
			SQL_DEBUG("Finished checking field names\n");
		}
	}

private:
	void addDefaultTables() {
		QSqlQuery q("", m_db);
		QString tl = s_sessions;
		if (!m_db.tables().contains(tl)) {
			if (!q.exec(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 DATETIME)")
					.arg(tl)
					.arg(f_id)
					.arg(f_start)
					.arg(f_finish))) {
				SQL_ERR("last error: %s, executed query: %s\n", qPrintable(q.lastError().text()), qPrintable(q.executedQuery()));
			}
		}
		tl = s_actions;
		if (!m_db.tables().contains(tl)) {
			if (!q.exec(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY)")
					.arg(tl)
					.arg(f_id))) {
				SQL_ERR("last error: %s\n", qPrintable(q.lastError().text()));
			}
		}
	}

private:
	friend class EngineImpl;
	EngineImpl *q;

	QSqlDatabase m_db;
	QSqlRecord m_record;
	int m_session_id;
};

EngineImpl::EngineImpl()
: p(new EngineImplPrivate(this))
{
}
void EngineImpl::begin()
{
	p->begin();
}
void EngineImpl::end()
{
	p->end();
}
void EngineImpl::addAction(const action_params &p_actions)
{
	std::map<std::string, std::string>::const_iterator i = p_actions.begin();
	std::map<std::string, std::string>::const_iterator end = p_actions.end();
	while (i != end) {
		std::cout << i->first << " : " << i->second << std::endl;
		++i;
	}
	p->addAction(p_actions);
}
EngineImpl::~EngineImpl()
{
	if (p) delete p;
}

} //namespace AE
