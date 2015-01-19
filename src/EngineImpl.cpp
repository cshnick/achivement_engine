#include "EngineImpl.h"

#include <QtCore>
#include <QtSql>
#include <QtXml>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <cstdlib>
#include <iostream>
#include "Conventions.h"

#ifdef ENABLE_TESTS
static QDateTime g_fakeCurrentTime;
#endif //ENABLE_TESTS

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
		m_db.setDatabaseName(g_achivements_path + "/" + g_dbName);
		if (!m_db.open()) {
			DEBUG_ERR("Unable to open database. An error occurred while opening the connection: %s\n", qPrintable(m_db.lastError().text()));
		}
		addDefaultTables();
		synchroAvhivementsDb();
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
			action_params::const_iterator i = p_actions.begin();
			action_params::const_iterator end = p_actions.end();
			QStringList kl, vl; //Store keys and values
			QList<QVariant> vvl;
			while (i != end) {
				QString nm = QString::fromStdString(i->first);
				AE::variant v = i->second;
				kl << nm;
				vl << "?";
				vvl << fromAeVariant(v);
				//Create non existent fields
				if (!rec.contains(nm)) {
					q.prepare(QString("ALTER TABLE %1 ADD %2 %3")
							.arg(t_actions)
							.arg(nm)
							.arg(QString::fromStdString(v.typeDBString()))
							);
					EXEC_AND_REPORT_COND;
				}
				i++;
			}

			q.clear();
			q = QSqlQuery("", m_db);
			// Prepare strings to INSERT statement
			QString kp = kl.join(",");
			QString vp = vl.join(",");
			kp.append(QString(",%1").arg(f_session_id));
			vp.append(",?");
			q.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)")
					.arg(t_actions)
					.arg(kp)
					.arg(vp));
			int k = 0;
			for (; k < vvl.count(); k++) {
				q.bindValue(k, vvl.at(k));
			}
			q.bindValue(k, m_session_id);
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
			QString tables = QString("%1,%2,%3,%4")
					.arg(t_sessions)
					.arg(t_actions)
					.arg(t_achivements_list)
					.arg(t_achivements_done);
			QStringList tl = tables.split(',');
			for (int i = 0; i < tl.size(); i++) {
				q.prepare(QString("DROP TABLE %1")
						.arg(tl.at(i)));
				EXEC_AND_REPORT_COND;
			}
		}
		if (!m_db.tables().contains(t_sessions)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 DATETIME)")
					.arg(t_sessions)
					.arg(f_id)
					.arg(f_start)
					.arg(f_finish));
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_actions)) {
			q.prepare(QString("CREATE TABLE %1 ("
					"%2 INTEGER PRIMARY KEY, "
					"%3 STRING, "
					"%7 INTEGER, "
					"FOREIGN KEY(%4) REFERENCES %5(%6)"
					")")
					.arg(t_actions)
					.arg(f_id)
					.arg(f_name)
					.arg(f_session_id)
					.arg(t_sessions)
					.arg(f_id)
					.arg(f_session_id));
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_achivements_list)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 STRING, %5 STRING, %6 STRING)")
					.arg(t_achivements_list)
					.arg(f_id)
					.arg(f_time)
					.arg(f_name)
					.arg(f_description)
					.arg(f_condition)
					);
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_achivements_done)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 STRING, %5 STRING, %6 STRING)")
					.arg(t_achivements_done)
					.arg(f_id)
					.arg(f_time)
					.arg(f_name)
					.arg(f_description)
					.arg(f_condition)
			);
			EXEC_AND_REPORT_COND;
		}
	}
	void synchroAvhivementsDb() {
		QFile ach_xml(g_achivements_path + "/" + g_achivementsFileName);
		if (!ach_xml.open(QIODevice::ReadOnly)) {
			DEBUG_ERR("Can't open %s for reading\n", qPrintable(g_achivements_path + "/" + g_achivementsFileName));
		}

		QList<QVariantMap> xml_rows;
		parseXmlRows(&ach_xml, xml_rows);

		QSqlQuery q("", m_db);
		for (int i = 0; i < xml_rows.count(); i++) {
			QVariantMap mit = xml_rows.at(i);

			q.prepare(QString("SELECT id FROM %1 WHERE id=?")
					.arg(t_achivements_list));
			q.bindValue(0, mit.value(f_id).toInt());
			EXEC_AND_REPORT_COND;

			bool fst = q.first();
			if (!fst) { //No such a record
				QStringList kl = mit.keys();
				QStringList ptrn;
				for (int j = 0; j < mit.count(); j++) ptrn.append("?");
				q.prepare(QString("INSERT into %1 (%2) values(%3)")
						.arg(t_achivements_list)
						.arg(kl.join(","))
						.arg(ptrn.join(","))
						);
				int cnt = 0;
				for (auto iter = mit.begin(); iter != mit.end(); iter++) {
					QVariant val = iter.value();
					if (iter.key() == f_id) {
						val = iter.value().toInt();
					}
					q.bindValue(cnt++, val);
				}
				EXEC_AND_REPORT_COND;
			}
			int sz = q.size();
		}
	}

	bool parseXmlRows(QFile *p_file, QList<QVariantMap> &map) {
		QDomDocument doc;
		QString err_string;
		int err_line;
		int err_column;
		if (!doc.setContent(p_file, false, &err_string, &err_line, &err_column)) {
			DEBUG_ERR( "Can't set content for %s\n", qPrintable(p_file->fileName()) );
			p_file->close();
			return false;
		}
		QDomElement element = doc.firstChildElement().firstChildElement(tag_element);
		while (!element.isNull()) {
			QVariantMap dta;
			QDomElement elAttr = element.firstChildElement();
			while (!elAttr.isNull()) {
				//type conversion
				QVariant value;
				if (elAttr.tagName() == f_id) {
					int inttext = elAttr.text().toInt();
					value = inttext;
				} else {
					value = elAttr.text();
				}
				dta[elAttr.tagName()] = value;
				elAttr = elAttr.nextSiblingElement();
			}
			map.append(dta);
			element = element.nextSiblingElement(tag_element);
		}
		p_file->close();
		return true;
	}

	QVariant fromAeVariant(const AE::variant &ae_val) {
		QVariant result;
		switch (ae_val.type()) {
		case AE_VAR_INT:
			result = QVariant(ae_val.toInt());
			break;
		case AE_VAR_FLOAT:
			result = QVariant(ae_val.toFloat());
			break;
		case AE_VAR_STRING:
			result = QVariant(QString::fromStdString(ae_val.toString()));
			break;
		case AE_VAR_DATETIME:
			result = QVariant(QDateTime::fromMSecsSinceEpoch(ae_val.toDateTime()));
			break;
		default:
			break;
		}

		return result;
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
	p->addAction(p_actions);
}
EngineImpl::~EngineImpl()
{
	if (p) delete p;
}

} //namespace AE
