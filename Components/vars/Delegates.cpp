#include "EngineImpl.h"
#include "Conventions.h"

#include <QtCore>
#include <QtSql>
#include <vector>

#define ADD_DELEGATE(name) m_delegates.push_back(new name(&m_db))
#define BEGIN_DECLARE_DELEGATE(name, vName, vAlias, vType) \
		class name: public CalcVarDelegateBase { \
		public: \
		name(QSqlDatabase *p_db = 0) : CalcVarDelegateBase(), m_db(p_db) {} \
		variant var() const {return m_var;} \
		std::string varName() const {return vName;} \
		std::string varAlias() const {return vAlias;} \
		std::string typeStr() const {return vType;} \
		void addContext(void **p_context) {m_context = p_context;} \

#define END_DECLARE_DELEGATE(name) \
		private: \
		variant m_var; \
		QSqlDatabase *m_db; \
		int m_session_id = -1; \
		void **m_context; \
}; \

namespace AE {

//SessionTime
BEGIN_DECLARE_DELEGATE(SessionTimeDelegate, "$st", "SessionTime", "Numeric")
	;
	void refresh(const variant &) {
		QSqlQuery q("", *m_db);
		q.prepare(
				QString("SELECT sum(%1) FROM %2 WHERE %3 = ("
						"SELECT %3 from (SELECT %3, max(%4) FROM %2)"
						")").arg(f_actTime::Value).arg(t_actions::Value).arg(
						f_session_id::Value).arg(f_time::Value));
		EXEC_AND_REPORT_COND
		;
		if (q.first()) {
			m_var = fromQVariant(q.value(0));
		}
	}
END_DECLARE_DELEGATE(SessionTimeDelegate)
BEGIN_DECLARE_DELEGATE(ActionTimeDelegate, "$at", "ActionTime", "Numeric")
	;
	void refresh(const variant &) {
		QSqlQuery q("", *m_db);
		q.prepare(
				QString("SELECT MAX(%1),%2 FROM %3").arg(f_time::Value).arg(
						f_actTime::Value).arg(t_actions::Value));
		EXEC_AND_REPORT_COND
		;
		if (q.first()) {
			QVariant var = q.value(1);
			m_var = fromQVariant(var);
		}
	}
END_DECLARE_DELEGATE(ActionTimeDelegate)
BEGIN_DECLARE_DELEGATE(ActionRightDelegate, "$aright", "ActionRight", "Numeric")
	;
	void refresh(const variant &) {
		QSqlQuery q("", *m_db);
		q.prepare(
				QString("SELECT MAX(%1),%2 FROM %3").arg(f_time::Value).arg(
						QString::fromStdString(f_success::Value)).arg(
						t_actions::Value));
		EXEC_AND_REPORT_COND
		;
		if (q.first()) {
			QVariant var = q.value(1);
			m_var = fromQVariant(var);
		}
	}
END_DECLARE_DELEGATE(ActionRightDelegate)
BEGIN_DECLARE_DELEGATE(AhivementsCount, "$", "AchivementsCount", "Achievements")
	;
	void refresh(const variant &p_id) {
		QSqlQuery q("", *m_db);
		int id = p_id.toInt();
		q.prepare(
				QString("SELECT COUNT(%1) FROM %2 WHERE %3 = ?").arg(
						f_id::Value).arg(t_achivements_done::Value).arg(
						f_ach_id::Value));
		q.bindValue(0, id);
		EXEC_AND_REPORT_COND
		;
		if (q.first()) {
			QVariant var = q.value(0);
			if (!var.isValid()) {
				var = QVariant(0);
			}
			m_var = fromQVariant(var);
		}
	}
END_DECLARE_DELEGATE(AhivementsCount)
BEGIN_DECLARE_DELEGATE(AllTimeSpent, "$spTime", "SpentTime", "Statistics")
	;
	void refresh(const variant &) {
		QSqlQuery q("", *m_db);
		q.prepare(
				QString("SELECT SUM(%1) FROM %2").arg(f_actTime::Value).arg(
						t_actions::Value));
		EXEC_AND_REPORT_COND
		;
		if (q.first()) {
			QVariant var = q.value(0);
			if (!var.isValid()) {
				var = QVariant(0);
			}
			m_var = fromQVariant(var);
		}
	}
END_DECLARE_DELEGATE(AllTimeSpent)
BEGIN_DECLARE_DELEGATE(ActionsCount, "$aCount", "ActionsCount", "Statistics")
	;
	void refresh(const variant &) {
		QSqlQuery q("", *m_db);
		q.prepare(
				QString("SELECT COUNT(%1) FROM %2").arg(f_actTime::Value).arg(
						t_actions::Value));
		EXEC_AND_REPORT_COND
		;
		if (q.first()) {
			QVariant var = q.value(0);
			if (!var.isValid()) {
				var = QVariant(0);
			}
			m_var = fromQVariant(var);
		}
	}
END_DECLARE_DELEGATE(ActionsCount)
BEGIN_DECLARE_DELEGATE(TrueActionsCount, "$taCount", "TrueActionsCount", "Statistics")
	;
	void refresh(const variant &) {
		QSqlQuery q("", *m_db);
		q.prepare(
				QString("SELECT COUNT(%1) FROM %2 WHERE %3 > 0").arg(
						f_actTime::Value).arg(t_actions::Value).arg(
						QString::fromStdString(f_success::Value)));
		EXEC_AND_REPORT_COND
		;
		if (q.first()) {
			QVariant var = q.value(0);
			if (!var.isValid()) {
				var = QVariant(0);
			}
			m_var = fromQVariant(var);
			PRINT_IF_VERBOSE("Calculated %s = %d\n", varName().c_str(),
					var.toInt());
		}
	}
END_DECLARE_DELEGATE(TrueActionsCount)
BEGIN_DECLARE_DELEGATE(FalseActionsCount, "$faCount", "FalseActionsCount", "Statistics")
	;
	void refresh(const variant &) {
		QSqlQuery q("", *m_db);
		q.prepare(
				QString("SELECT COUNT(%1) FROM %2 WHERE %3 = 0").arg(
						f_actTime::Value).arg(t_actions::Value).arg(
						QString::fromStdString(f_success::Value)));
		EXEC_AND_REPORT_COND
		;
		if (q.first()) {
			QVariant var = q.value(0);
			if (!var.isValid()) {
				var = QVariant(0);
			}
			m_var = fromQVariant(var);
		}
	}
END_DECLARE_DELEGATE(FalseActionCount)

class DCDB: public DelegateContainer {
public:
	void addContext(void *context) {
		m_context = (int*) context;
	}
	std::vector<CalcVarDelegateBase*> *delegates() {
		return &m_delegates;
	}
	DCDB() :
			m_context(0) {
		DEBUG("Creating DCDB object\n");
		m_db = QSqlDatabase::addDatabase("QSQLITE",
				"DelegateContainer_action_db");
		m_db.setDatabaseName(
				QString(g_achivements_path::Value) + "/" + g_dbName::Value);
		if (!m_db.open()) {
			DEBUG_ERR(
					"Unable to open database. An error occurred while opening the connection: %s\n",
					qPrintable(m_db.lastError().text()));
		}

		ADD_DELEGATE(SessionTimeDelegate);
		ADD_DELEGATE(ActionTimeDelegate);
		ADD_DELEGATE(ActionRightDelegate);
		ADD_DELEGATE(AhivementsCount);
		ADD_DELEGATE(AllTimeSpent);
		ADD_DELEGATE(ActionsCount);
		ADD_DELEGATE(TrueActionsCount);
		ADD_DELEGATE(FalseActionsCount);

	}

private:
	int *m_context;
	std::vector<CalcVarDelegateBase*> m_delegates;
	QSqlDatabase m_db;
};

extern "C" DelegateContainer *loadFactory() {
	return new DCDB;
}

} //namespace AE
