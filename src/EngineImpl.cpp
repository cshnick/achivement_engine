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
#define DROP_TABLES getenv("AE_DROP_TABLES") != NULL

#define EXEC_AND_REPORT_COND_RETURN \
		if (!q.exec()) { \
			SQL_ERR( "last error: %s, executed query: %s\n", qPrintable(q.lastError().text()), qPrintable(q.executedQuery()) ); \
			return; \
		} else { \
			SQL_DEBUG("Executed: %s\n", qPrintable(q.executedQuery())); \
		} \

#define EXEC_AND_REPORT_COND \
		if (!q.exec()) { \
			SQL_ERR( "last error: %s, executed query: %s\n", qPrintable(q.lastError().text()), qPrintable(q.executedQuery()) ); \
		} else { \
			SQL_DEBUG("Executed: %s\n", qPrintable(q.executedQuery())); \
		} \


#define STAT_IF_VERBOSE \
		if (VERBOSE) { \
			DEBUG("Entering ('%s':%d) : %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
		} \

#define PRINT_IF_VERBOSE(...) \
		if (VERBOSE) { \
			DEBUG(__VA_ARGS__); \
		} \


static const QString g_dbName = "/home/ilia/.local/share/action_engine/ae.db";
static const QString t_sessions = "Sessions";
static const QString t_actions = "Actions";
static const QString f_id = "id";
static const QString f_start = "Start";
static const QString f_finish = "Finish";
static const QString f_session = "Session";
static const QString f_name = "Name";
static const QString f_session_id = "SessionId";

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
		PRINT_IF_VERBOSE("Initializing database. Setting database name: %s\n", qPrintable(g_dbName));

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
		STAT_IF_VERBOSE;

		QSqlQuery q("", m_db);

		q.prepare(QString("INSERT INTO %1 (%2) VALUES (?)")
				.arg(t_sessions)
				.arg(f_start));
		q.bindValue(0, QDateTime::currentDateTime());
		EXEC_AND_REPORT_COND;
		m_session_id = q.lastInsertId().toInt();
	}
	void end() {
		if (!checkDB()) return;
		STAT_IF_VERBOSE;

		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			DEBUG_ERR("m_session_id is negtive. Called end several times?\n");
		}
		q.prepare(QString("UPDATE %1 SET %2 = :fin_time WHERE %3 = :id")
				.arg(t_sessions)
				.arg(f_finish)
				.arg(f_id));
		q.bindValue(":fin_time", QDateTime::currentDateTime());
		q.bindValue(":id", m_session_id);
		EXEC_AND_REPORT_COND;
		m_session_id = -1;
	}
	void addAction(const action_params &p_actions) {
		if (!checkDB()) return;
		STAT_IF_VERBOSE;
		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			DEBUG_ERR("m_session_id is negtive. Haven't called begin method for engine?\n");
		}
		QSqlRecord rec = m_db.record(t_actions);
		if (!rec.isEmpty()) {
			std::map<std::string, AE::variant>::const_iterator i = p_actions.begin();
			std::map<std::string, AE::variant>::const_iterator end = p_actions.end();
			QStringList kl, vl; //Store keys and values
			while (i != end) {
				QString nm = QString::fromStdString(i->first);
				QString v = QString::fromStdString(i->second.toString());
				kl << nm;
				vl << QString("'%1'").arg(v);
				//Create non existent fields
				if (!rec.contains(nm)) {
					q.prepare(QString("ALTER TABLE %1 ADD %2 STRING")
							.arg(t_actions)
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
			kp.append(QString(", %1").arg(f_session_id));
			vp.append(", :id");
			q.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)")
					.arg(t_actions)
					.arg(kp)
					.arg(vp));
			q.bindValue(":id", m_session_id);
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
		if (DROP_TABLES) {
			q.prepare(QString("DROP TABLE %1")
					.arg(t_sessions));
			EXEC_AND_REPORT_COND;
			q.prepare(QString("DROP TABLE %1")
					.arg(t_actions));
			EXEC_AND_REPORT_COND;
		}
		QString tl = t_sessions;
		if (!m_db.tables().contains(tl)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 DATETIME)")
					.arg(tl)
					.arg(f_id)
					.arg(f_start)
					.arg(f_finish));
			EXEC_AND_REPORT_COND;
		}
		tl = t_actions;
		if (!m_db.tables().contains(tl)) {
			q.prepare(QString("CREATE TABLE %1 ("
					"%2 INTEGER PRIMARY KEY, "
					"%3 STRING, "
					"%7 INTEGER, "
					"FOREIGN KEY(%4) REFERENCES %5(%6)"
					")")
					.arg(tl)
					.arg(f_id)
					.arg(f_name)
					.arg(f_session_id)
					.arg(t_sessions)
					.arg(f_id)
					.arg(f_session_id));
			EXEC_AND_REPORT_COND;
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
	std::map<std::string, AE::variant>::const_iterator i = p_actions.begin();
	std::map<std::string, AE::variant>::const_iterator end = p_actions.end();
	while (i != end) {
		std::cout << i->first << " : " << i->second.toString() << std::endl;
		++i;
	}
	p->addAction(p_actions);
}
EngineImpl::~EngineImpl()
{
	if (p) delete p;
}

} //namespace AE
