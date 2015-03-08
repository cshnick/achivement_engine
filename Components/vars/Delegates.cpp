#include "EngineImpl.h"
#include "Conventions.h"

#include <QtCore>
#include <QtSql>
#include <vector>

#include <SQL.h>

using Wrap_Sql::Select;
using Wrap_Sql::Func;
using Wrap_Sql::Condition;

#define ADD_DELEGATE(name) m_delegates.push_back(new name(&m_db, m_user, m_project))
#define BEGIN_DECLARE_DELEGATE(name, vName, vAlias, vType) \
		class name: public CalcVarDelegateBase { \
		public: \
		name(QSqlDatabase *p_db, int p_user, int p_project) : CalcVarDelegateBase(), m_db(p_db), m_user(p_user), m_project(p_project) {} \
		variant var() const {return m_var;} \
		std::string varName() const {return vName;} \
		std::string varAlias() const {return vAlias;} \
		std::string typeStr() const {return vType;} \

#define END_DECLARE_DELEGATE(name) \
		private: \
		variant m_var; \
		QSqlDatabase *m_db; \
		int m_session_id = -1; \
		int m_user = -1; \
		int m_project =1; \
}; \

namespace AE {

namespace Private {
	template<int User, int Project>
	QList<Condition> commonConditions() {
		QList<Condition> res;
		res.append(Condition(AE::f_user::Value,"=",User));
		res.append(Condition(AE::f_project::Value,"=",Project));

		return res;
	}
} //namespace Private

//SessionTime
BEGIN_DECLARE_DELEGATE(SessionTimeDelegate, "$st", "SessionTime", "Numeric")
	;
	void refresh(const variant &) {
		QSqlQuery q("", *m_db);
		q.prepare(
				QString("SELECT sum(%1) FROM %2 WHERE %3 = ("
						"SELECT %3 from (SELECT %3, max(%4) FROM %2)"
						")")
						.arg(f_actTime::Value) //%1
						.arg(t_actions::Value) //%2
						.arg(f_session_id::Value) //%3
						.arg(f_time::Value) //%4
						);
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
		auto s = Select(Func("MAX",f_time::Value), f_actTime::Value)
				.from(t_actions::Value);
		QList<Condition> l = Private::commonConditions<m_user,m_project>();
//		s.addConditions(Private::commonConditions<m_user,m_project>());

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
	void init(int id_user, int id_project) {
		m_user = id_user;
		m_project = id_project;

		ADD_DELEGATE(SessionTimeDelegate);
		ADD_DELEGATE(ActionTimeDelegate);
		ADD_DELEGATE(ActionRightDelegate);
		ADD_DELEGATE(AhivementsCount);
		ADD_DELEGATE(AllTimeSpent);
		ADD_DELEGATE(ActionsCount);
		ADD_DELEGATE(TrueActionsCount);
		ADD_DELEGATE(FalseActionsCount);
	}
	std::vector<CalcVarDelegateBase*> *delegates() {
		return &m_delegates;
	}
	DCDB() {
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
	}
	~DCDB() {
		DEBUG("Removing CDDB object container\n");

		for (auto i = m_delegates.begin(); i != m_delegates.end(); ++i) {
			delete (*i);
			(*i) = nullptr;
		}

		QString connection;
		connection = m_db.connectionName();
		m_db.close();
		m_db = QSqlDatabase();
		m_db.removeDatabase(connection);
	}

private:
	int m_user = -1;
	int m_project = -1;
	std::vector<CalcVarDelegateBase*> m_delegates;
	QSqlDatabase m_db;
};

extern "C" DelegateContainer *loadFactory() {
	return new DCDB;
}
extern "C" void closeFactory(DelegateContainer *obj) {
	delete obj;
}

} //namespace AE
