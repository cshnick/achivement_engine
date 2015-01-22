#include "EngineImpl.h"
#include "Conventions.h"

#include <QtCore>
#include <QtSql>
#include <vector>


#define ADD_DELEGATE(name) m_delegates.push_back(new name(&m_db))

namespace AE {

class SessionTimeDelegate: public AE::CalcVarDelegateBase {
public:
	SessionTimeDelegate(QSqlDatabase *p_db = 0) : CalcVarDelegateBase(), m_db(p_db) {
		if (p_db) {

		}
		m_var = 1;
	}
	std::string typeStr() const {return "Numeric";}
	std::string varName() const {return "SessionTime";}
	variant var() const {return m_var;}
	void refresh() {;}
private:
	variant m_var;
	QSqlDatabase *m_db;
};
class ActionTimeDelegate: public AE::CalcVarDelegateBase {
public:
	ActionTimeDelegate(QSqlDatabase *p_db = 0) : CalcVarDelegateBase(), m_db(p_db) {
		if (p_db) {

		}
		m_var = 2;
	}
	std::string typeStr() const {return "Numeric";}
	std::string varName() const {return "ActionTime";}
	variant var() const {return m_var;}
	void refresh() {;}
private:
	variant m_var;
	QSqlDatabase *m_db;
};

class DCDB : public DelegateContainer {
public:
	void addContext(void *context) {
		m_context = (EngineImpl*)context;
	}
	std::vector<CalcVarDelegateBase*> delegates() {
		return m_delegates;
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
	EngineImpl *m_context;
	std::vector<CalcVarDelegateBase*> m_delegates;
	QSqlDatabase m_db;
};

extern "C" DelegateContainer *loadFactory() {return new DCDB;}


} //namespace AE
