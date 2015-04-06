#ifndef _COMPONENTS_SQL_WRAPPER_SWL_H
#define _COMPONENTS_SQL_WRAPPER_SWL_H

#include <QtCore>
#include <QtSql>


namespace Wrap_Sql {

enum class SQL_TYPES {
	Undefined = 0
			,Select = 1
			,Insert = 2
			,Create = 4
			,Update = 8
};

enum dtype {
	Undefined = 0
			, INTEGER =  1
			, STRING = 2
			, DATETIME = 4
};

enum Operators {
	equal = 0
	, in = 1
};

class SqlBase {
public:
	virtual ~SqlBase() {}
	virtual int type() const = 0;
	virtual QString expression() const = 0;
	virtual bool exec() = 0;
protected:
	QSqlQuery q;
};

// Foreign key reference
class Reference {
public:
	Reference() {}
	Reference(const QString &p_table, const QString &p_field): m_table(p_table), m_field(p_field) {}
	QString expr() {return m_table + "(" + m_field + ")";}

	QString table() const {return m_table;}
	QString field() const {return m_field;}

	operator bool() {
		return !m_table.isEmpty() && !m_field.isEmpty();
	}

private:
	QString m_table;
	QString m_field;
};

class ForeignKey {
public:
	ForeignKey(const QString &p_field, const Reference &p_ref) : m_fieldName(p_field), m_ref(p_ref) {}
	QString expr() {return "FOREIGN KEY (" + m_fieldName + ") REFERENCES " + m_ref.expr();}
private:
	QString m_fieldName;
	Reference m_ref;
};

class FieldInfo {
	friend class CreateTable;
public:
	static QString typeStr(dtype tp) {
		switch (static_cast<int>(tp)) {
		case DATETIME:
			return "DATETIME";
			break;
		case STRING:
			return "STRING";
			break;
		case INTEGER:
			return "INTEGER";
		default:
			return "UNDEFINED";
		}

		return "UNDEFINED";
	}

	FieldInfo(const QString &p_name, dtype p_tp, const QString &p_addInfo = QString())
	: m_name(p_name)
	, m_type(p_tp)
	, m_addInfo(p_addInfo)
    {
	}
	QString expr() {
		return m_name + " " + typeStr(m_type) + " " + m_addInfo;
	}
	FieldInfo &ForeignKey(const Reference &ref) {
		m_ref = ref;
		return *this;
	}
	Reference ref() const {return m_ref;}
	QString name() const {return m_name;}

private:
	QString m_name;
	dtype m_type;
	QString m_addInfo; //additional information like  primary key and so one
	Reference m_ref;
};

class Condition {
public:
	Condition();
	Condition(const QString &key, Operators op, const QVariant &value)
		: key(key), op(op), val(value) {}
	Condition(const QString &key, const QString &str_op, const QVariant &value)
	: key(key), strOp(str_op), val(value) {}
	static QString strForOp(Operators op) {
		switch (static_cast<int>(op)) {
		case in:
			return "IN";
			break;
		case equal:
		default:
			return "=";
			break;
		}
		return QString();
	}
	static QString joinConditions(const QString &sep, const QList<Condition> &p_conditions) {
		if (p_conditions.isEmpty()) {
			return QString();
		}

		QStringList zip_l;
		for (auto n = p_conditions.begin(); n != p_conditions.end(); ++n) {
			QString decorated = (*n).val.toString();
			switch(static_cast<int>((*n).val.type())) {
			case QVariant::String:
			case QVariant::DateTime:
				decorated = "'" + decorated + "'";
				break;
			case QVariant::UserType:
				decorated = "(" + decorated + ")";
				break;
			}
			zip_l.append((*n).key + (*n).strOp + decorated);
		}

		return zip_l.join(" " + sep + " ");
	}

public:
	QString key;
	Operators op = equal;
	QString strOp = "=";
	QVariant val;
};

class Func {
public:
	Func();
	Func(const QString &name, const QString &field) : m_func(name), m_field(field) {}
	operator QString() {
		return m_func + "(" + m_field + ")";
	}
private:
    QString m_func;
    QString m_field;
};

class Select: public SqlBase {
public:
	Select();
	Select(const QString &p_f1);
	Select(const QString &p_f1, const QString &p_k2);
	Select(const QString &p_f1, const QString &p_k2, const QString &p_k3);
	Select(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4);
	Select(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5);
	Select(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5, const QString &p_f6);

	Select &from(const QString &p_f1);
	Select &from(const QString &p_f1, const QString &p_k2);
	Select &from(const QString &p_f1, const QString &p_k2, const QString &p_k3);
	Select &from(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4);
	Select &from(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5);
	Select &from(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5, const QString &p_f6);

	Select &where(const Condition &p_f1);
	Select &where(const Condition &p_f1, const Condition &p_f2);
	Select &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3);
	Select &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4);
	Select &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5);
	Select &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5, const Condition &p_f6);

	Select &orderBy(const QString &p_f1);
	Select &orderBy(const QString &p_f1, const QString &p_k2);
	Select &orderBy(const QString &p_f1, const QString &p_k2, const QString &p_k3);
	Select &orderBy(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4);
	Select &orderBy(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5);
	Select &orderBy(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5, const QString &p_f6);

	Select &groupBy(const QString &p_f1);
	Select &groupBy(const QString &p_f1, const QString &p_k2);
	Select &groupBy(const QString &p_f1, const QString &p_k2, const QString &p_k3);
	Select &groupBy(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4);
	Select &groupBy(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5);
	Select &groupBy(const QString &p_f1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5, const QString &p_f6);

	void addCondition(const Condition &c) {
		m_conditions.append(c);
	}
	void addConditions(const QList<Condition> &lc) {
		m_conditions.append(lc);
	}

	int type() const {return (int)SQL_TYPES::Select;}
	int variantType() const {return QVariant::fromValue(*this).type();}

	QString variantName() {return QVariant::fromValue(*this).typeName();}
	QString expression() const;
	QString toString() const {return expression();}
	bool exec();
	QSqlQuery exec(QSqlQuery &q);
	~Select();

private:
	QStringList m_fields;
	QStringList m_tables;
	QStringList m_orders;
	QStringList m_groups;
	QList<Condition> m_conditions;
};

class Update: public SqlBase {
public:
	int type() const {return (int)SQL_TYPES::Update;}
	QString expression() const;
	bool exec();
	QSqlQuery exec(QSqlQuery &q);

	Update(const QString &p_table) : m_table(p_table) {}

	Update &set(const Condition &p_f1);
	Update &set(const Condition &p_f1, const Condition &p_f2);
	Update &set(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3);
	Update &set(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4);
	Update &set(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5);
	Update &set(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5, const Condition &p_f6);

	Update &where(const Condition &p_f1);
	Update &where(const Condition &p_f1, const Condition &p_f2);
	Update &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3);
	Update &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4);
	Update &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5);
	Update &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5, const Condition &p_f6);

	void addSetCondition(const Condition &c) {
		m_setConditions.append(c);
	}
	void addSetConditions(const QList<Condition> &lc) {
		m_setConditions.append(lc);
	}
	void addWhereCondition(const Condition &c) {
		m_whereConditions.append(c);
	}
	void addWhereConditions(const QList<Condition> &lc) {
		m_whereConditions.append(lc);
	}

private:
	QStringList m_fields;
	QString m_table;
	QList<Condition> m_setConditions;
	QList<Condition> m_whereConditions;

};

class InsertInto: public SqlBase {
public:
	InsertInto(const QString &p_tableName);
	int type() const {return (int)SQL_TYPES::Insert;}
	virtual QString expression() const;
	virtual bool exec();
	QSqlQuery exec(QSqlQuery &q);

	InsertInto &keys(const QString &p_k1);
	InsertInto &keys(const QString &p_k1, const QString &p_k2);
	InsertInto &keys(const QString &p_k1, const QString &p_k2, const QString &p_k3);
	InsertInto &keys(const QString &p_k1, const QString &p_k2, const QString &p_k3, const QString &p_k4);
	InsertInto &keys(const QString &p_k1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5);
	InsertInto &keys(const QString &p_k1, const QString &p_k2, const QString &p_k3, const QString &p_k4, const QString &p_k5, const QString &p_k6);

	InsertInto &values(const QVariant &p_v1);
	InsertInto &values(const QVariant &p_v1, const QVariant &p_v2);
	InsertInto &values(const QVariant &p_v1, const QVariant &p_v2, const QVariant &p_v3);
	InsertInto &values(const QVariant &p_v1, const QVariant &p_v2, const QVariant &p_v3, const QVariant &p_v4);
	InsertInto &values(const QVariant &p_v1, const QVariant &p_v2, const QVariant &p_v3, const QVariant &p_v4, const QVariant &p_v5);
	InsertInto &values(const QVariant &p_v1, const QVariant &p_v2, const QVariant &p_v3, const QVariant &p_v4, const QVariant &p_v5, const QVariant &p_v6);

	InsertInto &append(const QString &key, const QVariant &value);
	InsertInto &append(const QVariantMap &keyValues);
	InsertInto &appendValue(const QVariant &val);
	InsertInto &appendValues(const QVariantList &vl);

private:
	QString m_table;
	QStringList m_keys;
	QVariantList m_values;
};

class CreateTable : public SqlBase {
public:


	CreateTable(const QString &p_tableName);
	int type() const {return (int)SQL_TYPES::Create;}
	QString expression() const;
	bool exec();
	QSqlQuery exec(QSqlQuery &q);

	CreateTable &add(const FieldInfo &p_fi);
	CreateTable &add(const QList<FieldInfo> &p_fis);
	CreateTable &add(const ForeignKey &p_fk);
	CreateTable &add(const QList<ForeignKey> &p_fks);

private:
	QString m_tableName;
	QList<FieldInfo> m_fields;
	QList<ForeignKey> m_foreignKeys;
};

} // namespace Wrap_Sql
Q_DECLARE_METATYPE(Wrap_Sql::Select)

#endif //_COMPONENTS_SQL_WRAPPER_SWL_H
