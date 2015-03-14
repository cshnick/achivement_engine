#include "SQL.h"
#include "Conventions.h"
#include <QtCore>

namespace Wrap_Sql {

QString Update::expression() const {
	QString exp;
	exp.append("UPDATE ");
	//Append fields
	QString tn = m_table; //tn: table name
	exp.append(m_table);
	//Append set conditions
	QString scm = Condition::joinConditions(m_setConditions); //scm: set conditions map
	if (!scm.isEmpty()) {
		exp.append(" SET ");
		exp.append(scm);
	} else {
		DEBUG_ERR("Update: set conditions map is empty");
	}
	//Append where conditions
	QString wcm = Condition::joinConditions(m_whereConditions); //wcm: where conditions map
	if (!wcm.isEmpty()) {
		exp.append(" WHERE ");
		exp.append(wcm);
	} else {
		DEBUG_ERR("Update: where conditions map is empty");
	}

	return exp;
}
bool Update::exec() {
	return false;
}
QSqlQuery Update::exec(QSqlQuery &q) {
	q.prepare(expression());
	bool result = false;
	QTime ct = QTime::currentTime();
	result = q.exec();
	int msec_query = ct.msecsTo(QTime::currentTime());
	if (result) {
		SQL_DEBUG("Executed: %s;\t query length: %d\n", q.executedQuery().toUtf8().data(), msec_query);
	} else {
		SQL_ERR("Exec error: %s, query: %s;\t query length: %d\n", q.lastError().text().toUtf8().data(), q.executedQuery().toUtf8().data(), msec_query);
	}

	return q;
}

Update& Update::set(const Condition &p_f1) {
	m_setConditions.append(p_f1);
	return *this;
}
Update& Update::set(const Condition &p_f1, const Condition &p_f2) {
	where(p_f1);
	m_setConditions.append(p_f2);
	return *this;
}
Update& Update::set(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3){
	where(p_f1, p_f2);
	m_setConditions.append(p_f3);
	return *this;
}
Update& Update::set(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4){
	where(p_f1, p_f2, p_f3);
	m_setConditions.append(p_f4);
	return *this;
}
Update& Update::set(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5){
	where(p_f1, p_f2, p_f3, p_f4);
	m_setConditions.append(p_f5);
	return *this;
}
Update& Update::set(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5, const Condition &p_f6){
	where(p_f1, p_f2, p_f3, p_f4, p_f5);
	m_setConditions.append(p_f6);
	return *this;
}

Update& Update::where(const Condition &p_f1) {
	m_whereConditions.append(p_f1);
	return *this;
}
Update& Update::where(const Condition &p_f1, const Condition &p_f2) {
	where(p_f1);
	m_whereConditions.append(p_f2);
	return *this;
}
Update& Update::where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3){
	where(p_f1, p_f2);
	m_whereConditions.append(p_f3);
	return *this;
}
Update& Update::where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4){
	where(p_f1, p_f2, p_f3);
	m_whereConditions.append(p_f4);
	return *this;
}
Update& Update::where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5){
	where(p_f1, p_f2, p_f3, p_f4);
	m_whereConditions.append(p_f5);
	return *this;
}
Update& Update::where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5, const Condition &p_f6){
	where(p_f1, p_f2, p_f3, p_f4, p_f5);
	m_whereConditions.append(p_f6);
	return *this;
}

} //namespace Wrap_Sql
