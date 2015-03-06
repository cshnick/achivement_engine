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
};
enum Operators {
	equal = 0
	, in = 1
};
QString strForOp(Operators op) {
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

class SqlBase {
public:
	virtual ~SqlBase() {}
	virtual int type() = 0;
	virtual QString expression() const = 0;
	virtual bool exec() = 0;
protected:
	QSqlQuery q;
};

class Condition {
public:
	Condition();
	Condition(const QString &key, Operators op, const QVariant &value)
		: key(key), op(op), val(value) {}
	Condition(const QString &key, const QString &str_op, const QVariant &value)
			: key(key), strOp(str_op), val(value) {}

public:
	QString key;
	Operators op = equal;
	QString strOp = "=";
	QVariant val;
};

namespace Private {
QString joinConditions(const QList<Condition> &p_conditions);
} //namespace Private

class Select: public SqlBase {
public:
	Select();
	Select(const QString &p_f1);
	Select(const QString &p_f1, const QString &p_f2);
	Select(const QString &p_f1, const QString &p_f2, const QString &p_f3);
	Select(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4);
	Select(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5);
	Select(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5, const QString &p_f6);

	Select &from(const QString &p_f1);
	Select &from(const QString &p_f1, const QString &p_f2);
	Select &from(const QString &p_f1, const QString &p_f2, const QString &p_f3);
	Select &from(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4);
	Select &from(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5);
	Select &from(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5, const QString &p_f6);

	Select &where(const Condition &p_f1);
	Select &where(const Condition &p_f1, const Condition &p_f2);
	Select &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3);
	Select &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4);
	Select &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5);
	Select &where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5, const Condition &p_f6);

	int type() {return (int)SQL_TYPES::Select;}
	int variantType() const {return QVariant::fromValue(*this).type();}
	QString variantName() {return QVariant::fromValue(*this).typeName();}
	QString expression() const;
	QString toString() const {return expression();}
	bool exec();
	~Select();

private:
	QStringList m_fields;
	QStringList m_tables;
	QList<Condition> m_conditions;
};

} // namespace Wrap_Sql
Q_DECLARE_METATYPE(Wrap_Sql::Select)

#endif //_COMPONENTS_SQL_WRAPPER_SWL_H
