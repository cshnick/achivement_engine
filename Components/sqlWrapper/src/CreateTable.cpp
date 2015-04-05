#include "SQL.h"
#include "Conventions.h"
#include <QtCore>

#define SQL_FORMAT_EXPRESSION getenv("SQL_FORMAT_EXPRESSION")

namespace Wrap_Sql {

CreateTable::CreateTable(const QString &p_tableName) : m_tableName(p_tableName) {
}
QString CreateTable::expression() const {
	QString result;
	result.append("CREATE TABLE ");
	result.append(m_tableName);
	result.append(SQL_FORMAT_EXPRESSION ? "\n" : " ");
	result.append("(");
	result.append(SQL_FORMAT_EXPRESSION ? "\n" : "");
	for (auto iter = m_fields.begin(); iter != m_fields.end(); ++iter) {
		result.append(SQL_FORMAT_EXPRESSION ? "\t" : " ");
		FieldInfo ifo = *iter;
		result.append(ifo.expr());
		result.append(iter != m_fields.end() - 1  || m_foreignKeys.count() ? "," : "");
		result.append(SQL_FORMAT_EXPRESSION ? "\n" : "");
	}
	for (auto iter = m_foreignKeys.begin(); iter != m_foreignKeys.end(); ++iter) {
		result.append(SQL_FORMAT_EXPRESSION ? "\t" : " ");
		ForeignKey fk = *iter;
		result.append(fk.expr());
		result.append(iter != m_foreignKeys.end() - 1 ? "," : "");
		result.append(SQL_FORMAT_EXPRESSION ? "\n" : "");
	}
	result.append(")");
	result.append(SQL_FORMAT_EXPRESSION ? "\n" : "");

	return result;
}
QSqlQuery CreateTable::exec(QSqlQuery &q) {
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
 bool CreateTable::exec() {
	return false;
}
CreateTable &CreateTable::add(const FieldInfo &p_fi) {
	m_fields.append(p_fi);
	return *this;

}
CreateTable &CreateTable::add(const QList<FieldInfo> &p_fis) {
	m_fields.append(p_fis);
	return *this;
}
CreateTable &CreateTable::add(const ForeignKey &p_fk) {
	m_foreignKeys.append(p_fk);
	return *this;

}
CreateTable &CreateTable::add(const QList<ForeignKey> &p_fks) {
	m_foreignKeys.append(p_fks);
	return *this;
}

} //namespace Wrap_Sql
