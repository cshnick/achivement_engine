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

static const QString g_dbName = "/home/ilia/.local/share/action_engine/ae.db";
static const QString s_sessions = "Sessions";
static const QString s_actions = "Actions";
static const QString f_id = "id";
static const QString f_start = "Start";
static const QString f_finish = "Finish";

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
			fprintf(stdout, "Initializing database\nSetting database name: %s", qPrintable(g_dbName));
		}
		m_db = QSqlDatabase::addDatabase("QSQLITE", "action_db");
		m_db.setDatabaseName(g_dbName);
		if (!m_db.open()) {
			fprintf(stderr, "Unable to open database. An error occurred while opening the connection: %s\n", qPrintable(m_db.lastError().text()));
		}
		QSqlQuery q("", m_db);
		QString tl = s_sessions;
		if (!m_db.tables().contains(tl)) {
			if (!q.exec("create table " + tl + " (id integer primary key, Start datetime, Finish datetime)")) {
				fprintf(stderr, "last error: %s, executed query: %s\n", qPrintable(q.lastError().text()), qPrintable(q.executedQuery()));
			}
		}
		tl = s_actions;
		if (!m_db.tables().contains(tl)) {
			if (!q.exec("create table " + tl + " (id integer primary key)")) {
				fprintf(stderr, "last error: %s\n", qPrintable(q.lastError().text()));
			}
		}
	}
	bool checkDB() {
		if (!m_db.isOpen()) {
			fprintf(stderr, "last error: %s\n", qPrintable(m_db.lastError().text()));
			return false;
		}
		return true;
	}
	void begin() {
		if (!checkDB()) return;
		QSqlQuery q("", m_db);

		q.prepare("insert into " + s_sessions + "(Start) values (?)");
		q.bindValue(0, QDateTime::currentDateTime());
		if (!q.exec()) {
			fprintf(stderr, "last error: %s, executed query: %s\n", qPrintable(q.lastError().text()), qPrintable(q.executedQuery()));
			return;
		}
		m_session_id = q.lastInsertId().toInt();
	}
	void end() {
		if (!checkDB()) return;
		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			fprintf(stderr, "m_session_id is negtive. Called end several times?\n");
		}
		q.prepare("uptate table " + s_sessions +
				" set " + f_finish + " = ? where " + f_id + " = ?");
		q.bindValue(0, QDateTime::currentDateTime());
		q.bindValue(1, m_session_id);
		if (!q.exec()) {
			fprintf(stderr, "last error: %s, executed query: %s\n", qPrintable(q.lastError().text()), qPrintable(q.executedQuery()));
			return;
		}
		m_session_id = -1;
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
}
EngineImpl::~EngineImpl()
{
	if (p) delete p;
}

} //namespace AE
