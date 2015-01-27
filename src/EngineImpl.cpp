#include "EngineImpl.h"

#include <QtCore>
#include <QtSql>
#include <QtXml>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "Conventions.h"
#include "dlfcn.h"
#include "ExpressionParser.h"

#ifdef ENABLE_TESTS
static QDateTime g_fakeCurrentTime = QDateTime::currentDateTime().addSecs(-60);
#endif //ENABLE_TESTS

typedef QVariant (*qv_func_t) (QVariant v1, QVariant v2);

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

class EngineImplPrivate {
public:
	EngineImplPrivate(EngineImpl *p_q) :
		q(p_q),
		m_session_id(-1)
	{
		PRINT_IF_VERBOSE("Initializing database. Setting database name: %s\n", qPrintable(g_dbName));

		m_db = QSqlDatabase::addDatabase("QSQLITE", "action_db");
		m_db.setDatabaseName(g_achivements_path + "/" + g_dbName);
		if (!m_db.open()) {
			DEBUG_ERR("Unable to open database. An error occurred while opening the connection: %s\n", qPrintable(m_db.lastError().text()));
		}
		addDefaultTables();
		synchroAvhivementsDb();
		fillAchivementsFromDB();
		fillCalcDelegatesMap();
	}
	DelegateContainer *loadCalcDelegatesContainer() {
		void *handle;
		handle = dlopen("./Delegates/libvar_calcs.so", RTLD_LAZY);
		if (!handle) {
			DEBUG_ERR("Error opening library file %s\n", dlerror());
			return 0;
		}
		dlerror();
		typedef DelegateContainer* (*loadLibrary_t)();
		loadLibrary_t loadLibrary = (loadLibrary_t)dlsym(handle, "loadFactory");
		char *error;
		if ((error = dlerror()) != NULL)  {
			DEBUG_ERR("Error reading symbol loadFactory, error text: %s\n", dlerror());
			return 0;
		}

		return loadLibrary();
	}
	void fillCalcDelegatesMap() {
		DelegateContainer *c = loadCalcDelegatesContainer();
		c->addContext((void*)this);
		PRINT_IF_VERBOSE("Filling calc's map...\n");
		for (auto iter = c->delegates()->begin(); iter != c->delegates()->end(); ++iter) {
			CalcVarDelegateBase *d = *iter;
			PRINT_IF_VERBOSE("\t[%s] = %s (%p);\n", d->varName().c_str(), d->varAlias().c_str(), d);
			m_calcVars[QString::fromStdString(d->varName())] = d;
		}
	}

	bool checkDB() {
		if (!m_db.isOpen()) {
			DEBUG_ERR( "last error: %s\n", qPrintable(m_db.lastError().text()) );
			return false;
		}
		return true;
	}

	void fillAchivementsFromDB() {
		 if (!checkDB()) return;
		 QSqlQuery q("", m_db);
		 q.prepare(QString("SELECT * FROM %1")
				 .arg(t_achivements_list));
		 EXEC_AND_REPORT_COND;
		 q.next();
		 while (q.isValid()) {
			 QSqlRecord r = q.record();
			 QVariantMap m;
			 for (int i = 0; i < r.count(); i++) {
				 m[r.fieldName(i)] = r.value(i);
			 }
			 m_achivements.append(m);
			 q.next();
		 }
	}

	void refreshCalcVars() {
		PRINT_IF_VERBOSE("Refreshing calc vars...\n");
		for (auto iter = m_calcVars.begin(); iter != m_calcVars.end(); ++iter) {
			CalcVarDelegateBase *d = iter.value();
			d->refresh();
			PRINT_IF_VERBOSE("\t[%s] = %s;\n", d->varName().c_str(), printable(d->var()));
		}
	}
	void checkAchivements() {
		for (int i = 0; i < m_achivements.count(); i++) {
			QVariantMap m = m_achivements[i];
			QString condition = m.value(f_condition).toString();
			if (parseCondition(condition)) {
				addAchivement(m);
			}
		}
	}
	bool parseCondition(const QString &str_cond) {
		PRINT_IF_VERBOSE("Parsing condition: %s\n", str_cond.toUtf8().constData());

		Node *condition_tree = ExpressionParser().parse(str_cond);
		QVariant result;
		if (!parseConditionTree(condition_tree, result)) {
			return false;
		}
		PRINT_IF_VERBOSE("Reporting condition result: %s\n", printable(result));
		return result.toInt();
	}
#define rr PRINT_IF_VERBOSE("next result %d, operator: %s\n", result.toInt(), qPrintable(str_op)); fflush(stdout);
	bool parseConditionTree(Node *condition_tree, QVariant &result) {
		QString str_op;
		QVariant var;
		foreach(Node *nd, condition_tree->children) {
			switch(static_cast<int>(nd->type)) {
			case Node::Identifier:
				var = vfi(nd->str);
				result = str_op.isNull() ? var : calc(str_op)(result, var);
//				rr;
				break;
			case Node::Number:
				var = nd->str.toInt();
				result = str_op.isNull() ? var : calc(str_op)(result, var);
//				rr;
				break;
			case Node::Operator:
				str_op = nd->str;
				break;
			case Node::AchCount:
				var = vfa(nd->str);
				result = str_op.isNull() ? var : calc(str_op)(result, var);
//				rr;
				break;
			case Node::Punctuator:
				break;
			case Node::CompExpression:
			case Node::OrExpression:
			case Node::AndExpression:
			case Node::AddExpression:
			case Node::MulExpression:
			case Node::Atom:
				if (!parseConditionTree(nd, var)) {
					return false;
				}
				result = str_op.isNull() ? var : calc(str_op)(result, var);
//				rr;
				break;
			default:
				return false;
			}
		}
		return true;
	}

	QVariant vfi(const QString &p_id) { //Qvariant from identifier
		QVariant result;
		CalcVarDelegateBase *delegate = m_calcVars.value(p_id);
		if (delegate) {
			delegate->refresh();
			result = fromAeVariant(delegate->var());
		}
		QString pIdTrimmed = p_id;
		pIdTrimmed.replace("%", "");
		PRINT_IF_VERBOSE("vfi for %s: %s\n", qPrintable(pIdTrimmed), printable(result));

		return result;
	}
	QVariant vfa(const QString &p_id) {
		QVariant result;
		QString pp_id = p_id;
		pp_id.remove('$');
		int iid = pp_id.toInt();
		CalcVarDelegateBase *delegate = m_calcVars.value("$");
		if (delegate) {
			delegate->refresh(variant(iid));
			result = fromAeVariant(delegate->var());
		}
		QString pIdTrimmed = p_id;
		pIdTrimmed.replace("%", "");
		PRINT_IF_VERBOSE("vfa for %s: %s\n", qPrintable(pIdTrimmed), printable(result));

		return result;
	}
	qv_func_t calc(const QString &op) {
	    if(op == "<") {
	        return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() < v2.toInt();};
	    } else if (op == ">") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() > v2.toInt();};
	    } else if (op == "<=") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() <= v2.toInt();};
	    } else if (op == ">=") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() >= v2.toInt();};
	    } else if (op == "!=") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1 != v2;};
	    } else if (op == "==") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1 == v2;};
	    } else if (op == "||") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() || v2.toInt();};
	    } else if (op == "&&") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() && v2.toInt();};
	    } else if (op == "+") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() + v2.toInt();};
	    } else if (op == "-") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() - v2.toInt();};
	    } else if (op == "*") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() * v2.toInt();};
	    } else if (op == "/") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() / v2.toInt();};
	    }

	    //By default return first value
	    return [](QVariant , QVariant ) -> QVariant {return QVariant();};
	}

	void addAchivement(const QVariantMap &m) {
		PRINT_IF_VERBOSE("Reached an achivement: %s!\n", qPrintable(m.value(f_name).toString()));
		addAchivementToDb(m);
	}
	void addAchivementToDb(const QVariantMap &m) {
		QSqlQuery q("", m_db);
		QStringList kl;
		kl << f_condition << f_description << f_name << f_time << f_ach_id << f_session_id;
		QString vl = "?,?,?,?,?,?";
		q.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)")
				.arg(t_achivements_done)
				.arg(kl.join(","))
				.arg(vl)
		);
		q.bindValue(0, m.value(f_condition));
		q.bindValue(1, m.value(f_description));
		q.bindValue(2, m.value(f_name));
		q.bindValue(3, currentTime());
		q.bindValue(4, m.value(f_id));
		q.bindValue(5, m_session_id);
		EXEC_AND_REPORT_COND;
	}
	void refreshAhivementsList() {
	}
	void begin() {
		if (!checkDB()) return;
		STAT_IF_VERBOSE;

		QSqlQuery q("", m_db);

		q.prepare(QString("INSERT INTO %1 (%2) VALUES (?)")
				.arg(t_sessions)
				.arg(f_start));
		q.bindValue(0, currentTime());
		EXEC_AND_REPORT_COND;
		m_session_id = q.lastInsertId().toInt();
		PRINT_IF_VERBOSE("Starting session: %d\n", m_session_id);
	}
	void end() {
		if (!checkDB()) return;
		STAT_IF_VERBOSE;

		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			DEBUG_ERR("m_session_id is negtive. Called end several times?\n");
		}
		q.prepare(QString("UPDATE %1 SET %2 = :fin_time WHERE %3 = :id")
				.arg(t_sessions)
				.arg(f_finish)
				.arg(f_id));
		q.bindValue(":fin_time", currentTime());
		q.bindValue(":id", m_session_id);
		EXEC_AND_REPORT_COND;
		m_session_id = -1;
	}
	void addAction(const action_params &p_actions) {
		STAT_IF_VERBOSE;
		addActionToDB(p_actions);
		checkAchivements();
	}
	void addActionToDB(const action_params &p_actions) {
		if (!checkDB()) return;
		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			DEBUG_ERR("m_session_id is negtive. Haven't called begin method for engine?\n");
		}
		//Insert new actions passed from the client
		QSqlRecord rec = m_db.record(t_actions);
		if (!rec.isEmpty()) {
			action_params::const_iterator i = p_actions.begin();
			action_params::const_iterator end = p_actions.end();
			QStringList kl, vl; //Store keys and values
			QList<QVariant> vvl;
			while (i != end) {
				QString nm = QString::fromStdString(i->first);
				AE::variant v = i->second;
				kl << nm;
				vl << "?";
				vvl << fromAeVariant(v);
				//Create non existent fields
				if (!rec.contains(nm)) {
					q.prepare(QString("ALTER TABLE %1 ADD %2 %3")
							.arg(t_actions)
							.arg(nm)
							.arg(QString::fromStdString(v.typeDBString()))
					);
					EXEC_AND_REPORT_COND;
				}
				i++;
			}
			q.clear();
			q = QSqlQuery("", m_db);

			QDateTime curTime = currentTime();
			int et = calculateElapsedSecsTo(curTime);
			// Prepare strings to INSERT statement
			QString kp = kl.join(",");
			QString vp = vl.join(",");
			kp.append(QString(",%1").arg(f_session_id));
			kp.append(QString(",%1").arg(f_time));
			kp.append(QString(",%1").arg(f_actTime));
			vp.append(",?"); //session id
			vp.append(",?"); //action time
			vp.append(",?"); //action time elapsed
			q.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)")
					.arg(t_actions)
					.arg(kp)
					.arg(vp));
			int k = 0;
			for (; k < vvl.count(); k++) {
				q.bindValue(k, vvl.at(k));
			}
			q.bindValue(k++, m_session_id);
			q.bindValue(k++, currentTime());
			q.bindValue(k++, et);
			EXEC_AND_REPORT_COND;

			SQL_DEBUG("Rec is not empty\n");
			SQL_DEBUG("Fields names are:\n");
			for (int i = 0; i < rec.count(); i++) {
				SQL_DEBUG("\t%s;\n", qPrintable(rec.fieldName(i)));
			}
			SQL_DEBUG("Finished checking field names\n");
		}
	}

	int calculateElapsedSecsTo(const QDateTime &ct) {
		QSqlQuery q("", m_db);
		//check previous time
		int result = -1;
		q.prepare(QString("SELECT MAX(%1) FROM %2 WHERE %3 = ?")
				.arg(f_actTime)
				.arg(t_actions)
				.arg(f_session_id)
		);
		q.bindValue(0, m_session_id);
		EXEC_AND_REPORT_COND;
		q.first();
		QDateTime dt = q.value(0).toDateTime();
		if (dt.isValid()) {
			SQL_DEBUG("DateTime value: %s, current time: %s\n", qPrintable(dt.toString()), qPrintable(ct.toString()));
			result = dt.secsTo(ct);
		} else { //Take time from session start time
			q.prepare(QString("SELECT %1 FROM %2 WHERE %3 = ?")
					.arg(f_start)
					.arg(t_sessions)
					.arg(f_id)
					);
			q.bindValue(0, m_session_id);
			EXEC_AND_REPORT_COND;
			q.first();
			QDateTime st = q.value(0).toDateTime();
			SQL_DEBUG("DateTime value: %s, current time: %s\n", qPrintable(st.toString()), qPrintable(ct.toString()));
			if (!st.isValid()) {
				DEBUG_ERR("Unable to retreive session start id... \n");
			}
			result = st.secsTo(ct);
		}

		return result;
	}

	static QDateTime currentTime() {
#ifdef ENABLE_TESTS
		qsrand(g_fakeCurrentTime.toMSecsSinceEpoch());
		g_fakeCurrentTime = g_fakeCurrentTime.addSecs(qrand() % 60);
		return g_fakeCurrentTime;
#else
		return QDateTime::currentDateTime();
#endif
	}

private:
	void addDefaultTables() {
		QSqlQuery q("", m_db);
		if (DROP_TABLES) {
			QString tables = QString("%1,%2,%3,%4")
					.arg(t_sessions)
					.arg(t_actions)
					.arg(t_achivements_list)
					.arg(t_achivements_done);
			QStringList tl = tables.split(',');
			for (int i = 0; i < tl.size(); i++) {
				q.prepare(QString("DROP TABLE %1")
						.arg(tl.at(i)));
				EXEC_AND_REPORT_COND;
			}
		}
		if (!m_db.tables().contains(t_sessions)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 DATETIME)")
					.arg(t_sessions)
					.arg(f_id)
					.arg(f_start)
					.arg(f_finish));
			EXEC_AND_REPORT_COND;
		}
		//Actions table
		if (!m_db.tables().contains(t_actions)) {
			q.prepare(QString("CREATE TABLE %1 ("
					"%2 INTEGER PRIMARY KEY, "
					"%3 STRING, "
					"%4 INTEGER, "
					"%6 DATETIME, "
					"%7 INTEGER, "
					"FOREIGN KEY(%4) REFERENCES %5(%2)"
					")")
					.arg(t_actions)
					.arg(f_id)
					.arg(f_name)
					.arg(f_session_id)
					.arg(t_sessions)
					.arg(f_time)
					.arg(f_actTime)
					);
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_achivements_list)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 STRING, %5 STRING, %6 STRING)")
					.arg(t_achivements_list)
					.arg(f_id)
					.arg(f_time)
					.arg(f_name)
					.arg(f_description)
					.arg(f_condition)
					);
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_achivements_done)) {
			q.prepare(QString("CREATE TABLE %1 "
					"(%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 STRING,"
					" %5 STRING, %6 STRING, %7 INTEGER, %8 INTEGER,"
					"FOREIGN KEY(%7) REFERENCES %9(%2)"
					")")
					.arg(t_achivements_done)
					.arg(f_id)
					.arg(f_time)
					.arg(f_name)
					.arg(f_description)
					.arg(f_condition)
					.arg(f_ach_id)
					.arg(f_session_id)
					.arg(t_achivements_list)
			);
			EXEC_AND_REPORT_COND;
		}
	}
	void synchroAvhivementsDb() {
		QFile ach_xml(g_achivements_path + "/" + g_achivementsFileName);
		if (!ach_xml.open(QIODevice::ReadOnly)) {
			DEBUG_ERR("Can't open %s for reading\n", qPrintable(g_achivements_path + "/" + g_achivementsFileName));
		}

		QList<QVariantMap> xml_rows;
		parseXmlRows(&ach_xml, xml_rows);

		QSqlQuery q("", m_db);
		for (int i = 0, cnt = 0; i < xml_rows.count(); i++, cnt=0) {
			QVariantMap mit = xml_rows.at(i);

			q.prepare(QString("SELECT %1 FROM %2 WHERE id=?")
					.arg(f_id)
					.arg(t_achivements_list));
			q.bindValue(0, mit.value(f_id).toInt());
			EXEC_AND_REPORT_COND;

			bool fst = q.first();
			if (!fst) { //No such a record
				QStringList kl = mit.keys();
				QStringList ptrn;
				for (int j = 0; j < mit.count(); j++) ptrn.append("?");
				q.prepare(QString("INSERT INTO %1 (%2) VALUES(%3)")
						.arg(t_achivements_list)
						.arg(kl.join(","))
						.arg(ptrn.join(","))
						);
				for (auto iter = mit.begin(); iter != mit.end(); iter++) {
					QVariant val = iter.value();
					if (iter.key() == f_id) {
						val = iter.value().toInt();
					}
					q.bindValue(cnt++, val);
				}
				EXEC_AND_REPORT_COND;
			} else {
				QString fields;
				QStringList kl = mit.keys();
				kl.append("");
				fields = kl.join(" = ?, ");
				fields = fields.remove(fields.lastIndexOf(","), 2); //Revmove trailing ', '
				q.prepare(QString("UPDATE %1 SET %2 WHERE %3 = ?")
						.arg(t_achivements_list)
						.arg(fields)
						.arg(f_id)
						);
				int ind = 0;
				for (auto iter = mit.begin(); iter != mit.end(); iter++) {
					QVariant val = iter.value();
					if (iter.key() == f_id) {
						ind = iter.value().toInt();
						val = ind;
					}
					q.bindValue(cnt++, val);
				}
				q.bindValue(cnt, QVariant::fromValue(ind));
				EXEC_AND_REPORT_COND;
			}
		}
	}

	bool parseXmlRows(QFile *p_file, QList<QVariantMap> &map) {
		QDomDocument doc;
		QString err_string;
		int err_line;
		int err_column;
		if (!doc.setContent(p_file, false, &err_string, &err_line, &err_column)) {
			DEBUG_ERR( "Can't set content for %s\n", qPrintable(p_file->fileName()) );
			p_file->close();
			return false;
		}
		QDomElement element = doc.firstChildElement().firstChildElement(tag_element);
		while (!element.isNull()) {
			QVariantMap dta;
			QDomElement elAttr = element.firstChildElement();
			while (!elAttr.isNull()) {
				//type conversion
				QVariant value;
				if (elAttr.tagName() == f_id) {
					int inttext = elAttr.text().toInt();
					value = inttext;
				} else {
					value = elAttr.text();
				}
				dta[elAttr.tagName()] = value;
				elAttr = elAttr.nextSiblingElement();
			}
			map.append(dta);
			element = element.nextSiblingElement(tag_element);
		}
		p_file->close();
		return true;
	}

private:
	friend class EngineImpl;
	EngineImpl *q;

	QSqlDatabase m_db;
	QSqlRecord m_record;
	int m_session_id;
	QList<QVariantMap> m_achivements;
	QMap<QString, CalcVarDelegateBase*> m_calcVars;
};

EngineImpl::EngineImpl()
: p(new EngineImplPrivate(this))
{
}
void EngineImpl::begin()
{
	p->begin();
}
void EngineImpl::end()
{
	p->end();
}
void EngineImpl::addAction(const action_params &p_actions)
{
	p->addAction(p_actions);
}
EngineImpl::~EngineImpl()
{
	if (p) delete p;
}

} //namespace AE
