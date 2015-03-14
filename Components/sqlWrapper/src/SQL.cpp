#include "SQL.h"
#include "Conventions.h"
#include <QtCore>

__attribute__((constructor))
static void initialize_types() {
	QMetaType::registerConverter(&Wrap_Sql::Select::expression);
}

namespace Wrap_Sql {

namespace Private {
QString joinConditions(const QList<Condition> &p_conditions) {
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
	QString cm = Private::joinConditions(m_conditions); //conditions map
	if (!cm.isEmpty()) {
		exp.append(" WHERE ");
		exp.append(cm);
	}
	QString om = m_orders.join(","); //orders map
	if (!om.isEmpty()) {
		exp.append(" ORDER BY ");
		exp.append(om);
	}
	QString gm = m_groups.join(","); // groups map
	if (!gm.isEmpty()) {
		exp.append(" GROUP BY ");
		exp.append(gm);
	}

	return exp;
}
bool Select::exec() {return false;}
QSqlQuery Select::exec(QSqlQuery &q) {
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

Select& Select::where(const Condition &p_f1) {
	m_conditions.append(p_f1);
	return *this;
}
Select& Select::where(const Condition &p_f1, const Condition &p_f2) {
	where(p_f1);
	m_conditions.append(p_f2);
	return *this;
}
Select& Select::where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3){
	where(p_f1, p_f2);
	m_conditions.append(p_f3);
	return *this;
}
Select& Select::where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4){
	where(p_f1, p_f2, p_f3);
	m_conditions.append(p_f4);
	return *this;
}
Select& Select::where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5){
	where(p_f1, p_f2, p_f3, p_f4);
	m_conditions.append(p_f5);
	return *this;
}
Select& Select::where(const Condition &p_f1, const Condition &p_f2, const Condition &p_f3, const Condition &p_f4, const Condition &p_f5, const Condition &p_f6){
	where(p_f1, p_f2, p_f3, p_f4, p_f5);
	m_conditions.append(p_f6);
	return *this;
}

Select& Select::orderBy(const QString &p_f1) {
	m_orders.append(p_f1);
	return *this;
}
Select& Select::orderBy(const QString &p_f1, const QString &p_f2) {
	orderBy(p_f1);
	m_orders.append(p_f2);
	return *this;
}
Select& Select::orderBy(const QString &p_f1, const QString &p_f2, const QString &p_f3){
	orderBy(p_f1, p_f2);
	m_orders.append(p_f3);
	return *this;
}
Select& Select::orderBy(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4){
	orderBy(p_f1, p_f2, p_f3);
	m_orders.append(p_f4);
	return *this;
}
Select& Select::orderBy(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5){
	orderBy(p_f1, p_f2, p_f3, p_f4);
	m_orders.append(p_f5);
	return *this;
}
Select& Select::orderBy(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5, const QString &p_f6){
	orderBy(p_f1, p_f2, p_f3, p_f4, p_f5);
	m_orders.append(p_f6);
	return *this;
}

Select& Select::groupBy(const QString &p_f1) {
	m_groups.append(p_f1);
	return *this;
}
Select& Select::groupBy(const QString &p_f1, const QString &p_f2) {
	groupBy(p_f1);
	m_groups.append(p_f2);
	return *this;
}
Select& Select::groupBy(const QString &p_f1, const QString &p_f2, const QString &p_f3){
	groupBy(p_f1, p_f2);
	m_groups.append(p_f3);
	return *this;
}
Select& Select::groupBy(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4){
	groupBy(p_f1, p_f2, p_f3);
	m_groups.append(p_f4);
	return *this;
}
Select& Select::groupBy(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5){
	groupBy(p_f1, p_f2, p_f3, p_f4);
	m_groups.append(p_f5);
	return *this;
}
Select& Select::groupBy(const QString &p_f1, const QString &p_f2, const QString &p_f3, const QString &p_f4, const QString &p_f5, const QString &p_f6){
	groupBy(p_f1, p_f2, p_f3, p_f4, p_f5);
	m_groups.append(p_f6);
	return *this;
}

Select::~Select() {

}


int main(int argc, char **argv)
{
	return 0;
}

} //namespace Wrap_Sql
