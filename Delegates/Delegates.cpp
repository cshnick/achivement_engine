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

//Printable variant for debug
const char *printable(const variant &v) {
	const char *result;
	switch (v.type()) {
	case AE_VAR_INT:
		result = QString("AE::variant(AE_VAR_INT, %2)").arg(v.toInt()).toUtf8().constData();
		break;
	case AE_VAR_DOUBLE:
		result = QString("AE::variant(AE_VAR_DOUBLE, %2)").arg(v.toDouble()).toUtf8().constData();
		break;
	case AE_VAR_STRING:
		result = QString("AE::variant(AE_VAR_STRING, %2)").arg(v.toString().c_str()).toUtf8().constData();
		break;
	case AE_VAR_DATETIME:
		result = QString("AE::variant(AE_VAR_DATETIME, %2)").arg(QDateTime::fromMSecsSinceEpoch((long long)v.toDateTime()).toString()).toUtf8().constData();
		break;
	default:
		break;
	}

	return result;
}
//Printable QVariant for debug
const char *printable(const QVariant &v) {
	const char *result;
	switch (v.type()) {
	case QVariant::Bool:
		result = QString("QVariant(Bool, %1)").arg(v.toBool()).toUtf8().constData();
		break;
	case QVariant::Int:
		result = QString("QVariant(Int, %1)").arg(v.toInt()).toUtf8().constData();
		break;
	case QVariant::Double:
		result = QString("QVariant(Double, %1)").arg(v.toDouble()).toUtf8().constData();
		break;
	case QVariant::String:
		result = QString("QVariant(String, %1)").arg(v.toString()).toUtf8().constData();
		break;
	case QVariant::DateTime:
		result = QString("QVariant(DateTime, %1)").arg(v.toDateTime().toString()).toUtf8().constData();
		break;
	default:
		result = QString("Invalid QVariant").toUtf8().constData();
		break;
	}

	return result;
}
QVariant fromAeVariant(const AE::variant &ae_val) {
	QVariant result;
	switch (ae_val.type()) {
	case AE_VAR_INT:
		result = QVariant(ae_val.toInt());
		break;
	case AE_VAR_DOUBLE:
		result = QVariant(ae_val.toDouble());
		break;
	case AE_VAR_STRING:
		result = QVariant(QString::fromStdString(ae_val.toString()));
		break;
	case AE_VAR_DATETIME:
		result = QVariant(QDateTime::fromMSecsSinceEpoch((long long)ae_val.toDateTime()));
		break;
	default:
		result = QVariant();
		break;
	}

	return result;
}

variant fromQVariant(const QVariant &q_val) {
	variant result;
	switch (q_val.type()) {
	case QVariant::Int:
	case QVariant::LongLong:
		result = variant(q_val.toInt());
		break;
	case QVariant::Double:
		result = variant(q_val.toDouble());
		break;
	case QVariant::String:
		result = variant(q_val.toString().toStdString());
		break;
	case QVariant::DateTime:
		result = variant(dateTime(q_val.toDateTime().toMSecsSinceEpoch()));
		break;
	}

	return result;
}

//SessionTime
BEGIN_DECLARE_DELEGATE(SessionTimeDelegate, "%st", "SessionTime", "Numeric");
void refresh(const variant &) {
	QSqlQuery q("", *m_db);
	q.prepare(QString("SELECT sum(%1) FROM %2 WHERE %3 = ("
			"SELECT %3 from (SELECT %3, max(%4) FROM %2)"
			")")
			.arg(f_actTime)
			.arg(t_actions)
			.arg(f_session_id)
			.arg(f_time)
	);
	EXEC_AND_REPORT_COND;
	if (q.first()) {
		m_var = fromQVariant(q.value(0));
	}
}
END_DECLARE_DELEGATE(SessionTimeDelegate)
BEGIN_DECLARE_DELEGATE(ActionTimeDelegate, "%at", "ActionTime", "Numeric");
void refresh(const variant &) {
	QSqlQuery q("", *m_db);
	q.prepare(QString("SELECT MAX(%1),%2 FROM %3")
			.arg(f_time)
			.arg(f_actTime)
			.arg(t_actions)
	);
	EXEC_AND_REPORT_COND;
	if (q.first()) {
		QVariant var = q.value(1);
		m_var = fromQVariant(var);
	}
}
END_DECLARE_DELEGATE(ActionTimeDelegate)
BEGIN_DECLARE_DELEGATE(AhivementsCount, "$", "AchivementsCount", "Achievements");
void refresh(const variant &p_id) {
	QSqlQuery q("", *m_db);
	int id = p_id.toInt();
	q.prepare(QString("SELECT COUNT(%1) FROM %2 WHERE %3 = ?")
			.arg(f_id)
			.arg(t_achivements_done)
			.arg(f_ach_id)
	);
	q.bindValue(0, id);
	EXEC_AND_REPORT_COND;
	if (q.first()) {
		QVariant var = q.value(0);
		if (!var.isValid()) {
			var = QVariant(0);
		}
		m_var = fromQVariant(var);
	}
}
END_DECLARE_DELEGATE(AhivementsCount)
BEGIN_DECLARE_DELEGATE(AllTimeSpent, "%spTime", "SpentTime", "Statistics");
void refresh(const variant &) {
	QSqlQuery q("", *m_db);
	q.prepare(QString("SELECT SUM(%1) FROM %2")
			.arg(f_actTime)
			.arg(t_actions)
	);
	EXEC_AND_REPORT_COND;
	if (q.first()) {
		QVariant var = q.value(0);
		if (!var.isValid()) {
			var = QVariant(0);
		}
		m_var = fromQVariant(var);
	}
}
END_DECLARE_DELEGATE(AllTimeSpent)
BEGIN_DECLARE_DELEGATE(ActionsCount, "%aCount", "ActionsCount", "Statistics");
void refresh(const variant &) {
	QSqlQuery q("", *m_db);
	q.prepare(QString("SELECT COUNT(%1) FROM %2")
			.arg(f_actTime)
			.arg(t_actions)
	);
	EXEC_AND_REPORT_COND;
	if (q.first()) {
		QVariant var = q.value(0);
		if (!var.isValid()) {
			var = QVariant(0);
		}
		m_var = fromQVariant(var);
	}
}
END_DECLARE_DELEGATE(ActionsCount)
BEGIN_DECLARE_DELEGATE(TrueActionsCount, "%taCount", "TrueActionsCount", "Statistics");
void refresh(const variant &) {
	QSqlQuery q("", *m_db);
	q.prepare(QString("SELECT COUNT(%1) FROM %2 WHERE %3 > 0")
			.arg(f_actTime)
			.arg(t_actions)
			.arg(QString::fromStdString(f_success))
	);
	EXEC_AND_REPORT_COND;
	if (q.first()) {
		QVariant var = q.value(0);
		if (!var.isValid()) {
			var = QVariant(0);
		}
		m_var = fromQVariant(var);
	}
}
END_DECLARE_DELEGATE(TrueActionsCount)
BEGIN_DECLARE_DELEGATE(FalseActionsCount, "%faCount", "FalseActionsCount", "Statistics");
void refresh(const variant &) {
	QSqlQuery q("", *m_db);
	q.prepare(QString("SELECT COUNT(%1) FROM %2 WHERE %3 = 0")
			.arg(f_actTime)
			.arg(t_actions)
			.arg(QString::fromStdString(f_success))
	);
	EXEC_AND_REPORT_COND;
	if (q.first()) {
		QVariant var = q.value(0);
		if (!var.isValid()) {
			var = QVariant(0);
		}
		m_var = fromQVariant(var);
	}
}
END_DECLARE_DELEGATE(FalseActionCount)

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

extern "C" DelegateContainer *loadFactory() {return new DCDB;}


} //namespace AE
