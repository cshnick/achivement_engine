#include "EngineImpl.h"
#include "Conventions.h"

#include <QtCore>
#include <QtSql>
#include <vector>

#define ADD_DELEGATE(name) m_delegates.push_back(new name(&m_db))

namespace AE {

class SessionTimeDelegate: public CalcVarDelegateBase {
public:
	SessionTimeDelegate(QSqlDatabase *p_db = 0) : CalcVarDelegateBase(), m_db(p_db) {
		if (p_db) {

		}
		m_var = 1000;
	}
	std::string typeStr() const {return "Numeric";}
	std::string varName() const {return "%st";}
	std::string varAlias() const {return "SessionTime";}
	variant var() const {return m_var;}
	void refresh() {
		QSqlQuery q("", *m_db);
		q.prepare(QString("SELECT MAX(%1) FROM %2"));
	}
private:
	variant m_var;
	QSqlDatabase *m_db;
	int m_session_id = -1;
};
class ActionTimeDelegate: public CalcVarDelegateBase {
public:
	ActionTimeDelegate(QSqlDatabase *p_db = 0) : CalcVarDelegateBase(), m_db(p_db) {
		if (p_db) {

		}
		m_var = 3;
	}
	std::string typeStr() const {return "Numeric";}
	std::string varName() const {return "%at";}
	std::string varAlias() const {return "ActionTime";}
	variant var() const {return m_var;}
	void refresh() {
		QSqlQuery q("", *m_db);

	}
private:
	variant m_var;
	QSqlDatabase *m_db;
	int m_session_id = -1;
};

class DCDB : public DelegateContainer {
public:
	void addContext(void *context) {
		m_context = (int*)context;
	}
	std::vector<CalcVarDelegateBase*> *delegates() {
		return &m_delegates;
	}
	DCDB() : m_context(0) {
		DEBUG("Creating DCDB object\n");
		m_db = QSqlDatabase::addDatabase("QSQLITE", "DelegateContainer_action_db");
		m_db.setDatabaseName(g_achivements_path + "/" + g_dbName);
		if (!m_db.open()) {
			DEBUG_ERR("Unable to open database. An error occurred while opening the connection: %s\n", qPrintable(m_db.lastError().text()));
		}

		ADD_DELEGATE(SessionTimeDelegate);
		ADD_DELEGATE(ActionTimeDelegate);
	}

private:
	int *m_context;
	std::vector<CalcVarDelegateBase*> m_delegates;
	QSqlDatabase m_db;
};

extern "C" DelegateContainer *loadFactory() {return new DCDB;}


} //namespace AE
