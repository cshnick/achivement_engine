#include "SQL.h"
#include "Conventions.h"
#include <QtCore>

namespace Wrap_Sql {

static QString joinVariantMap(const QVariantList &l) {
	QString result;
	int cnt = 0;
	for (auto iter = l.begin(); iter != l.end(); ++iter,++cnt) {
		QVariant v = (*iter);
		QString vs = v.toString();

		if (v.type() != QVariant::Bool &&
				v.type() != QVariant::Int &&
				v.type() != QVariant::LongLong &&
				v.type() != QVariant::Double) {
			vs = "'" + vs + "'"; //Quote if not numeric
		}
		result += vs;
		if (cnt != l.count() - 1) result += ",";
	}

	return result;
}

QString InsertInto::expression() const {
	QString exp;
	exp.append("INSERT INTO ");
	//Append fields
	QString tn = m_table; //tn: table name
	exp.append(m_table);
	//Append keys
	QString kl = m_keys.join(","); //kl: key list
	if (!kl.isEmpty()) {
		exp.append(" (");
		exp.append(kl);
		exp.append(")");
	} else {
		DEBUG("Insert into: empty fields list. Make sure you've been fitted all values\n");
	}
	//Append values
	QString vl = joinVariantMap(m_values); //vl: value list
	if (!vl.isEmpty()) {
		exp.append(" VALUES (");
		exp.append(vl);
		exp.append(")");
	} else {
		DEBUG_ERR("INSERT INTO: Values list is empty!\n");
	}

	return exp;
}
bool InsertInto::exec() {
	return false;
}
QSqlQuery InsertInto::exec(QSqlQuery &q) {
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

InsertInto::InsertInto(const QString &p_tableName) {
	m_table = p_tableName;
}

InsertInto &InsertInto::keys(const QString &p_k1) {
	m_keys.append(p_k1);
	return *this;
}
InsertInto &InsertInto::keys(const QString &p_k1, const QString &p_k2) {
	keys(p_k1);
	m_keys.append(p_k2);
	return *this;
}
InsertInto &InsertInto::keys(const QString &p_k1, const QString &p_k2, const QString &p_k3){
	keys(p_k1, p_k2);
	m_keys.append(p_k3);
	return *this;
}
InsertInto &InsertInto::keys(const QString &p_k1, const QString &p_k2, const QString &p_k3, const QString &p_k4){
	keys(p_k1, p_k2, p_k3);
	m_keys.append(p_k4);
	return *this;
}
InsertInto &InsertInto::keys(const QString &p_k1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5){
	keys(p_k1, p_k2, p_k3, p_k4);
	m_keys.append(p_k5);
	return *this;
}
InsertInto &InsertInto::keys(const QString &p_k1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5, const QString &p_k6) {
	keys(p_k1, p_k2, p_k3, p_k4, p_k5);
	m_keys.append(p_k6);
	return *this;
}

InsertInto &InsertInto::values(const QVariant &p_v1) {
	m_values.append(p_v1);
	return *this;
}
InsertInto &InsertInto::values(const QVariant &p_v1, const QVariant &p_v2) {
	values(p_v1);
	m_values.append(p_v2);
	return *this;
}
InsertInto &InsertInto::values(const QVariant &p_v1, const QVariant &p_v2, const QVariant &p_v3){
	values(p_v1, p_v2);
	m_values.append(p_v3);
	return *this;
}
InsertInto &InsertInto::values(const QVariant &p_v1, const QVariant &p_v2, const QVariant &p_v3, const QVariant &p_v4){
	values(p_v1, p_v2, p_v3);
	m_values.append(p_v4);
	return *this;
}
InsertInto &InsertInto::values(const QVariant &p_v1, const QVariant &p_v2, const QVariant &p_v3, const QVariant &p_v4, const QVariant &p_v5){
	values(p_v1, p_v2, p_v3, p_v4);
	m_values.append(p_v5);
	return *this;
}
InsertInto &InsertInto::values(const QVariant &p_v1, const QVariant &p_v2, const QVariant &p_v3, const QVariant &p_v4, const QVariant &p_v5, const QVariant &p_v6) {
	values(p_v1, p_v2, p_v3, p_v4, p_v5);
	m_values.append(p_v6);
	return *this;
}
InsertInto &InsertInto::append(const QString &key, const QVariant &value) {
	if (m_keys.count() != m_values.count()) {
		DEBUG_ERR("Can't append a KVP to container with different KV capacity."
				"Query can only consist of empty keys or keys count "
				"has to be the same as values count\n");
		return *this;
	}
	m_keys.append(key);
	m_values.append(value);

	return *this;
}
InsertInto &InsertInto::append(const QVariantMap &keyValues) {
	if (m_keys.count() != m_values.count()) {
		DEBUG_ERR("Can't append a KVP map to container with different KV capacity."
				"Query can only consist of empty keys or keys count "
				"has to be the same as values count\n");
		return *this;
	}
	for (auto iter = keyValues.begin(); iter != keyValues.end(); iter++) {
		m_keys.append(iter.key());
		m_values.append(iter.value());
	}

	return *this;
}
InsertInto &InsertInto::appendValue(const QVariant &val) {
	if (m_keys.count() > 0) {
		DEBUG_ERR("Can't append values to non empty keys container."
				"Query can only consist of empty keys or keys count "
				"has to be the same as values count\n");
		return *this;
	}

	m_values.append(val);
	return *this;
}
InsertInto &InsertInto::appendValues(const QVariantList &vl) {
	if (m_keys.count() > 0) {
		DEBUG_ERR("Can't append values to non empty keys container."
				"Query can only consist of empty keys or keys count "
				"has to be the same as values count\n");
		return *this;
	}

	m_values.append(vl);
	return *this;
}


} //namespace Wrap_Sql
