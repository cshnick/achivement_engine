#include "SQL.h"
#include "Conventions.h"
#include <QtCore>

__attribute__((constructor))
static void initialize_types() {
	QMetaType::registerConverter(&Wrap_Sql::Select::expression);
}

namespace Wrap_Sql {

namespace Private {
QString joinConditionMap(const QVariantMap &p_map) {
	if (p_map.isEmpty()) {
		return QString();
	}

	QStringList zip_l;
	for (auto n = p_map.begin(); n != p_map.end(); ++n) {
		zip_l.append(n.key() + "=" + n.value().toString());
	}

	return zip_l.join(" AND ");
}
} //namespace Private

QString Select::expression() const{
	QString exp;
	exp.append("SELECT ");
	//Append fields
	QString sf = "*"; //string for fields
	QString lf = m_fields.join(",");
	if (!lf.isEmpty()) sf = lf;
	exp.append(sf);
	exp.append(" FROM ");
	//Append tables
	QString st = m_tables.join(","); //string for tables
	if (st.isEmpty()) {
		DEBUG_ERR("No tables for select, the query will not be executed\n");
	}
	exp.append(st);
	//Append conditions
	QString cm = Private::joinConditionMap(m_conditions); //conditions map
	if (!cm.isEmpty()) {
		exp.append(" WHERE ");
		exp.append(cm);
	}

	return exp;
}
bool Select:: exec() {
	return false;
}
Select::Select() {

}
Select::Select(const QString &p_f1) {
	m_fields.append(p_f1);
}
Select::Select(const QString &p_f1, const QString &p_f2)
: Select(p_f1) {
	m_fields.append(p_f2);
}
Select::Select(const QString &p_f1, const QString &p_f2, const QString &p_f3)
: Select(p_f1, p_f2) {
	m_fields.append(p_f3);
}
Select::Select(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4)
: Select(p_f1, p_f2, p_f3) {
	m_fields.append(p_f4);
}
Select::Select(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5)
: Select(p_f1, p_f2, p_f3, p_f4) {
	m_fields.append(p_f5);
}
Select::Select(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5, const QString &p_f6)
: Select(p_f1, p_f2, p_f3, p_f4, p_f5) {
	m_fields.append(p_f6);
}

Select& Select::from(const QString &p_f1) {
	m_tables.append(p_f1);
	return *this;
}
Select& Select::from(const QString &p_f1, const QString &p_f2) {
	from(p_f1);
	m_tables.append(p_f2);
	return *this;
}
Select& Select::from(const QString &p_f1, const QString &p_f2, const QString &p_f3){
	from(p_f1, p_f2);
	m_tables.append(p_f3);
	return *this;
}
Select& Select::from(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4){
	from(p_f1, p_f2, p_f3);
	m_tables.append(p_f4);
	return *this;
}
Select& Select::from(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5){
	from(p_f1, p_f2, p_f3, p_f4);
	m_tables.append(p_f5);
	return *this;
}
Select& Select::from(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5, const QString &p_f6){
	from(p_f1, p_f2, p_f3, p_f4, p_f5);
	m_tables.append(p_f6);
	return *this;
}

Select& Select::where(const QString &p_f1) {
	m_tables.append(p_f1);
	return *this;
}
Select& Select::where(const QString &p_f1, const QString &p_f2) {
	from(p_f1);
	m_tables.append(p_f2);
	return *this;
}
Select& Select::where(const QString &p_f1, const QString &p_f2, const QString &p_f3){
	from(p_f1, p_f2);
	m_tables.append(p_f3);
	return *this;
}
Select& Select::where(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4){
	from(p_f1, p_f2, p_f3);
	m_tables.append(p_f4);
	return *this;
}
Select& Select::where(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5){
	from(p_f1, p_f2, p_f3, p_f4);
	m_tables.append(p_f5);
	return *this;
}
Select& Select::where(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5, const QString &p_f6){
	from(p_f1, p_f2, p_f3, p_f4, p_f5);
	m_tables.append(p_f6);
	return *this;
}

Select::~Select() {

}


int main(int argc, char **argv)
{
	return 0;
}

} //namespace Wrap_Sql
