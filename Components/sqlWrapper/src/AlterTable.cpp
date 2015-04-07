#include "SQL.h"
#include "Conventions.h"
#include <QtCore>
#include <iterator>

#define SQL_FORMAT_EXPRESSION getenv("SQL_FORMAT_EXPRESSION")

namespace Wrap_Sql {

AlterTable::AlterTable(const QString &p_tableName) : m_tableName(p_tableName) {
}
/**
 *  TODO Nothing to do in here from the moment since sqlite supports only single alter
 *  at a time. Think how to make it universal.
 */
QString AlterTable::expression() const {
	QString result;
	result.append("ALTER TABLE ");
	result.append(m_tableName);
	result.append(SQL_FORMAT_EXPRESSION ? "\n" : " ");
	result.append(actionString(m_action));
	result.append(" ");
//	result.append("(");
	result.append(SQL_FORMAT_EXPRESSION ? "\n" : "");
	auto action_fields = m_fields.equal_range((int)m_action);
	for (auto iter = action_fields.first; iter != action_fields.second; ++iter) {
		result.append(SQL_FORMAT_EXPRESSION ? "\t" : " ");
		FieldInfo ifo = iter->second;
		result.append(ifo.expr());
		result.append(std::next(iter) != m_fields.end() || m_foreignKeys.size() ? "," : "");
		result.append(SQL_FORMAT_EXPRESSION ? "\n" : "");
	}
	for (auto iter = m_foreignKeys.begin(); iter != m_foreignKeys.end(); ++iter) {
		result.append(SQL_FORMAT_EXPRESSION ? "\t" : " ");
	    Action ac = (Action)iter->first;
		ForeignKey fk = iter->second;
		result.append(fk.expr());
		result.append(std::next(iter) != m_foreignKeys.end() ? "," : "");
		result.append(SQL_FORMAT_EXPRESSION ? "\n" : "");
	}
//	result.append(")");
	result.append(SQL_FORMAT_EXPRESSION ? "\n" : "");

	return result;
}
QSqlQuery AlterTable::exec(QSqlQuery &q) {
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
 bool AlterTable::exec() {
	return false;
}
AlterTable &AlterTable::add(const FieldInfo &p_fi) {
	checkAction(Action::ADD);
	m_fields.emplace((int)Action::ADD, p_fi);
	if (p_fi.ref()) {
		m_foreignKeys.emplace((int)Action::ADD, ForeignKey(p_fi.name(), p_fi.ref()));
	}
	return *this;
}
AlterTable &AlterTable::add(const QList<FieldInfo> &p_fis) {
	for (auto iter = p_fis.begin(); iter != p_fis.end(); ++iter) {
		add(*iter);
	}
	return *this;
}
AlterTable &AlterTable::add(const ForeignKey &p_fk) {
	checkAction(Action::ADD);
	m_foreignKeys.emplace((int)Action::ADD, p_fk);
	return *this;

}
AlterTable &AlterTable::add(const QList<ForeignKey> &p_fks) {
	for (auto iter = p_fks.begin(); iter != p_fks.end(); ++iter) {
		add(*iter);
	}
	return *this;
}

} //namespace Wrap_Sql

