#ifndef _COMPONENTS_SQL_WRAPPER_SWL_H
#define _COMPONENTS_SQL_WRAPPER_SWL_H

#include <QtCore>
#include <QtSql>


namespace Wrap_Sql {
namespace Private {
QString joinConditionMap(const QVariantMap &p_map);
} //namespace Private

enum class SQL_TYPES {
	Undefined = 0
			,Select = 1
			,Insert = 2
			,Create = 4
};

class SqlBase {
public:
	virtual ~SqlBase() {}
	virtual int type() = 0;
	virtual QString expression() const = 0;
	virtual bool exec() = 0;
protected:
	QSqlQuery q;
};

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

	int type() {return (int)SQL_TYPES::Select;}
	int variantType() {return QVariant::fromValue(*this).type();}
	QString variantName() {return QVariant::fromValue(*this).typeName();}
	QString expression() const;
	QString toString() const {return expression();}
	bool exec();
	~Select();

private:
	QStringList m_fields;
	QStringList m_tables;
	QVariantMap m_conditions;
};

} // namespace Wrap_Sql
Q_DECLARE_METATYPE(Wrap_Sql::Select)

#endif //_COMPONENTS_SQL_WRAPPER_SWL_H
