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
#include <mutex>
#include "Conventions.h"
#include "dlfcn.h"
#include "ExpressionParser.h"
#include "unistd.h"

static QDateTime g_fakeCurrentTime = QDateTime::currentDateTime();

#define DELEGATES_PATH getenv("AE_DELEGATES_PATH")
#define USE_FAKE_TIME getenv("FAKE_TIME")

typedef QVariant (*qv_func_t) (QVariant v1, QVariant v2);

namespace AE {

class EngineImplPrivate {
public:
	EngineImplPrivate(EngineImpl *p_q) :
		q(p_q),
		m_session_id(-1)
	{
		const char lc[] = "UTF-8";
		DEBUG("Setting locale: %s\n", lc);
		QTextCodec::setCodecForLocale(QTextCodec::codecForName(lc));
		QTextCodec::setCodecForCStrings(QTextCodec::codecForName(lc));
		if (DROP_TABLES) {
			refreshDB();
			dropTables();
		}
	}
	~EngineImplPrivate() {
		if (m_calc_vars_container) delete m_calc_vars_container;
		QString connection;
		connection = m_db.connectionName();
		m_db.close();
		m_db = QSqlDatabase();
		m_db.removeDatabase(connection);
		STAT_IF_VERBOSE;
	}
	DelegateContainer *loadCalcDelegatesContainer() {
		void *handle;
		char dpath[512];
		if (DELEGATES_PATH) {
			sprintf(dpath, "%s/libvar_calcs.so", DELEGATES_PATH);
		} else {
			sprintf(dpath, "./Calcs/libvar_calcs.so");
		}
		handle = dlopen(dpath, RTLD_LAZY);
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
		m_calc_vars_container = loadCalcDelegatesContainer();
		m_calc_vars_container->addContext((void*)this);
		PRINT_IF_VERBOSE("Filling calc's map...\n");
		for (auto iter = m_calc_vars_container->delegates()->begin(); iter != m_calc_vars_container->delegates()->end(); ++iter) {
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

	void fillAchivementsFromDB(bool reset = false) {
		 if (!checkDB()) return;
		 QSqlQuery q("", m_db);
		 q.prepare(QString("SELECT * FROM %1")
				 .arg(t_achivements_list::Value));
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
		STAT_IF_VERBOSE;
		for (int i = 0; i < m_achivements.count(); i++) {
			QVariantMap m = m_achivements[i];
			QString condition = m.value(f_condition::Value).toString();
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
			case Node::SqlExpression:
				var = vfs(nd->str);
				result = str_op.isNull() ? var : calc(str_op)(result, var);
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
	//variant for identifier
	QVariant vfi(const QString &p_id) { //Qvariant from identifier
		QVariant result;
		CalcVarDelegateBase *delegate = m_calcVars.value(p_id);
		if (delegate) {
			delegate->refresh();
			result = fromAeVariant(delegate->var());
		}
		const char *pid_ch = p_id.toUtf8().data();
		PRINT_IF_VERBOSE("vfi for %s: %s\n", pid_ch, printable(result));

		return result;
	}
	// variant for achievement count
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
	//variant for sql expression
	QVariant vfs(const QString &p_id) {
		QString str = p_id;
		str.remove("$sql{");
		str.remove("}");
		QSqlQuery q("", m_db);
		q.prepare(str);
		EXEC_AND_REPORT_COND;
		if (q.first()) {
			return q.value(0);
		}
		return QVariant();
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
	    } else if (op == "%") {
	    	return [](QVariant v1, QVariant v2) -> QVariant {return v1.toInt() % v2.toInt();};
	    }

	    //By default return first value
	    return [](QVariant , QVariant ) -> QVariant {return QVariant();};
	}

	void addAchivement(const QVariantMap &m) {
		PRINT_IF_VERBOSE("Reached an achivement: %s!\n", qPrintable(m.value(f_name::Value).toString()));
		addAchivementToDb(m);
		m_instant_achievements << m;
	}
	void addAchivementToDb(const QVariantMap &m) {
		QSqlQuery q("", m_db);
		QStringList kl;
		kl << f_condition::Value <<
				f_description::Value <<
				f_name::Value <<
				f_time::Value <<
				f_ach_id::Value <<
				f_session_id::Value;
		QString vl = "?,?,?,?,?,?";
		q.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)")
				.arg(t_achivements_done::Value)
				.arg(kl.join(","))
				.arg(vl)
		);
		q.bindValue(0, m.value(f_condition::Value));
		q.bindValue(1, m.value(f_description::Value));
		q.bindValue(2, m.value(f_name::Value));
		q.bindValue(3, currentTime());
		q.bindValue(4, m.value(f_id::Value));
		q.bindValue(5, m_session_id);
		EXEC_AND_REPORT_COND;
	}
	void refreshAhivementsList() {
	}
	bool testCredentials() {
		if (m_User.empty() || m_Project.empty()) {
			DEBUG_ERR("Username or project name is empty, process refused...\n");
		}
	}

	void begin() {
		if (!testCredentials()) {
			return;
		}

		initGlobal();
		if (!checkDB()) return;
		STAT_IF_VERBOSE;

		QSqlQuery q("", m_db);

		q.prepare(QString("INSERT INTO %1 (%2) VALUES (?)")
				.arg(t_sessions::Value)
				.arg(f_start::Value));
		q.bindValue(0, currentTime());
		EXEC_AND_REPORT_COND;
		m_session_id = q.lastInsertId().toInt();
		PRINT_IF_VERBOSE("Starting session: %d\n", m_session_id);
	}
	void end() {
		if (!testCredentials()) {
			return;
		}

		if (!checkDB()) return;
		STAT_IF_VERBOSE;

		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			DEBUG_ERR("m_session_id is negtive. Called end several times?\n");
		}
		q.prepare(QString("UPDATE %1 SET %2 = :fin_time WHERE %3 = :id")
				.arg(t_sessions::Value)
				.arg(f_finish::Value)
				.arg(f_id::Value));
		q.bindValue(":fin_time", currentTime());
		q.bindValue(":id", m_session_id);
		EXEC_AND_REPORT_COND;
		m_session_id = -1;
	}
	void addAction(const action_params &p_actions) {
		STAT_IF_VERBOSE;
		if (!testCredentials()) {
			return;
		}
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
		QSqlRecord rec = m_db.record(t_actions::Value);
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
							.arg(t_actions::Value)
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
			kp.append(QString(",%1").arg(f_session_id::Value));
			kp.append(QString(",%1").arg(f_time::Value));
			kp.append(QString(",%1").arg(f_actTime::Value));
			vp.append(",?"); //session id
			vp.append(",?"); //action time
			vp.append(",?"); //action time elapsed
			q.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)")
					.arg(t_actions::Value)
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
				.arg(f_time::Value)
				.arg(t_actions::Value)
				.arg(f_session_id::Value)
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
					.arg(f_start::Value)
					.arg(t_sessions::Value)
					.arg(f_id::Value)
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
		if (USE_FAKE_TIME) {
			g_fakeCurrentTime = g_fakeCurrentTime.addSecs(qrand() % 20);
			PRINT_IF_VERBOSE("Fake current time used...\n");
			return g_fakeCurrentTime;
		} else {
			PRINT_IF_VERBOSE("Real current time used...\n");
			return QDateTime::currentDateTime();
		}
	}

	achievements_params take_ach_params() {
		achievements_params apl;

		for (int i = 0; i < m_instant_achievements.count(); i++) {
			QVariantMap m = m_instant_achievements.at(i);

			action_params ap = toActionParams(m);
			apl.push_back(ap);
			PRINT_IF_VERBOSE("Reporting std string: %s\n", ap[f_description::Value].toString().c_str());
		}
		m_instant_achievements.clear();
		return apl;
	}

	void addProject(const std::string &project) {
		refreshDB();
		refreshTables();
		QSqlQuery q("", m_db);
		q.prepare(QString("SELECT * FROM %1 where %2 = ?")
				.arg(t_projects::Value)
				.arg(f_name::Value)
		);
		q.bindValue(0, QString::fromStdString(project));
		EXEC_AND_REPORT_COND;
		if (q.first()) {
			return;
		} else {
			q.prepare(QString("INSERT INTO %1 (%2) VALUES(?)")
					.arg(t_projects::Value)
					.arg(f_name::Value)
			);
			q.bindValue(0, QString::fromStdString(project));
			EXEC_AND_REPORT_COND;
		}
	}
	void addUser(const std::string &name, const std::string &passwd) {
		refreshDB();
		refreshTables();
		QSqlQuery q("", m_db);
		q.prepare(QString("SELECT * FROM %1 where %2 = ?")
				.arg(t_users::Value)
				.arg(f_name::Value)
				);
		q.bindValue(0, QString::fromStdString(name));
		EXEC_AND_REPORT_COND;
		if (q.first()) {
			return;
		} else {
			q.prepare(QString("INSERT INTO %1 (%2,%3) VALUES(?,?)")
					.arg(t_users::Value)
					.arg(f_name::Value)
					.arg(f_passwd::Value)
			);
			q.bindValue(0, QString::fromStdString(name));
			q.bindValue(1, QString::fromStdString(passwd));
			EXEC_AND_REPORT_COND;
		}
	}
	bool init(const std::string &project, const std::string &name, const std::string &passwd = std::string()) {
		initGlobal();
		NO_IMPL_REPORT;
		return false;
	}

	std::vector<var_traits> varMetas() {
		if (!m_calc_vars_container) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_calc_vars_container = loadCalcDelegatesContainer();
		}
		std::vector<var_traits> res;
		std::vector<CalcVarDelegateBase*> q_v = *m_calc_vars_container->delegates();
		for(auto d = q_v.begin(); d != q_v.end(); ++d) {
			var_traits vt;
			vt.name = (*d)->varAlias();
			vt.alias = (*d)->varName();
			vt.type_str = (*d)->typeStr();
			res.push_back(vt);
		}
		return res;
	}

	bool loadFromXml(QIODevice *stream) {
		//		QString xmlPath = QString(g_achivements_path::Value) + "/" + QString(g_achivementsFileName::Value);
		//		QFile ach_xml(xmlPath);
		//		if (!ach_xml.open(QIODevice::ReadOnly)) {
		//			DEBUG_ERR("Can't open %s for reading\n", qPrintable(xmlPath));
		//		}
		//
		synchroAvhivementsDb(stream);
		return true;
	}
	bool synchroAvhivementsDb(QIODevice *stream) {
		refreshDB();
		refreshTables();
		refreshCalcVarDelegates();

		QList<QVariantMap> xml_rows;
		parseXmlRows(stream, xml_rows);

		QSqlQuery q("", m_db);
		for (int i = 0, cnt = 0; i < xml_rows.count(); i++, cnt=0) {
			QVariantMap mit = xml_rows.at(i);

			q.prepare(QString("SELECT %1 FROM %2 WHERE id=?")
					.arg(f_id::Value)
					.arg(t_achivements_list::Value));
			q.bindValue(0, mit.value(f_id::Value).toInt());
			EXEC_AND_REPORT_COND;

			bool fst = q.first();
			if (!fst) { //No such a record
				QStringList kl = mit.keys();
				QStringList ptrn;
				for (int j = 0; j < mit.count(); j++) ptrn.append("?");
				q.prepare(QString("INSERT INTO %1 (%2) VALUES(%3)")
						.arg(t_achivements_list::Value)
						.arg(kl.join(","))
						.arg(ptrn.join(","))
						);
				for (auto iter = mit.begin(); iter != mit.end(); iter++) {
					QVariant val = iter.value();
					if (iter.key() == f_id::Value) {
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
						.arg(t_achivements_list::Value)
						.arg(fields)
						.arg(f_id::Value)
						);
				int ind = 0;
				for (auto iter = mit.begin(); iter != mit.end(); iter++) {
					QVariant val = iter.value();
					if (iter.key() == f_id::Value) {
						ind = iter.value().toInt();
						val = ind;
					}
					q.bindValue(cnt++, val);
				}
				q.bindValue(cnt, QVariant::fromValue(ind));
				EXEC_AND_REPORT_COND;
			}
		}

		refreshAchievements();
		return true;
	}

	bool achievementsToXml(QIODevice *stream) {
		refreshDB();
		refreshTables();
		if (!checkDB()) return false;

		QSqlQuery q("", m_db);

		q.prepare(QString("SELECT * FROM %1").arg(t_achivements_list::Value));
		EXEC_AND_REPORT_COND;
		q.first();
		if (!stream->isWritable()) {
			DEBUG_ERR("Can't write to stream...\n");
		}
		if (!stream->isOpen()) {
			stream->open(QIODevice::WriteOnly);
		}
		QXmlStreamWriter writer(stream);
		writer.setAutoFormatting(true);
		writer.writeStartDocument();
		writer.writeStartElement(AE::tag_root::Value);
		while (q.isValid()) {
			QSqlRecord rec = q.record();
			writer.writeStartElement(AE::tag_element::Value);
			writer.writeTextElement(f_description::Value, rec.value(f_description::Value).toString());
			writer.writeTextElement(f_condition::Value, rec.value(f_condition::Value).toString());
			writer.writeTextElement(f_name::Value, rec.value(f_name::Value).toString());
			writer.writeTextElement(f_id::Value, rec.value(f_id::Value).toString());
			writer.writeEndElement();
			q.next();
		}
		writer.writeEndElement();
		writer.writeEndDocument();
		stream->close();

		return true;
	}

private:
	void refreshDB() {
		PRINT_IF_VERBOSE("Initializing database. Setting database name: %s\n", qPrintable(g_dbName::Value));

		if (!m_db.isOpen()) {
			m_db = QSqlDatabase::addDatabase("QSQLITE", "action_db");
			m_db.setDatabaseName(QString(g_achivements_path::Value) + "/" + g_dbName::Value);
			if (!m_db.open()) {
				DEBUG_ERR("Unable to open database. An error occurred while opening the connection: %s\n", qPrintable(m_db.lastError().text()));
			}
		}
	}
	void refreshTables() {
		addDefaultTables();
	}
	void refreshAchievements(bool reset = false) {
		if (m_achivements.count() && !reset) {
			return;
		}
		fillAchivementsFromDB();
	}
	void refreshCalcVarDelegates(bool reset = false) {
		if (m_calc_vars_container) { //TODO implement reset
			return;
		}
		fillCalcDelegatesMap();
	}
	void initGlobal() {
		refreshDB();
		refreshTables();
		refreshAchievements();
		refreshCalcVarDelegates();
	}


	action_params toActionParams(const QVariantMap &m) {
		action_params res;
		for (auto iter = m.begin(); iter != m.end(); ++iter) {
			res[iter.key().toStdString()] = fromQVariant(iter.value());
		}

		return res;
	}

	void dropTables() {
		refreshDB();

		QSqlQuery q("", m_db);
		QStringList tablesToDrop;
		tablesToDrop
				<< t_sessions::Value
				<< t_actions::Value
				<< t_achivements_list::Value
				<< t_users::Value
				<< t_projects::Value
				<< t_achivements_done::Value;
		for (int i = 0; i < tablesToDrop.size(); i++) {
			q.prepare(QString("DROP TABLE %1")
					.arg(tablesToDrop.at(i)));
			EXEC_AND_REPORT_COND;
		}
	}

	void addDefaultTables() {
		QSqlQuery q("", m_db);
		if (!m_db.tables().contains(t_users::Value)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 STRING, %4 STRING)")
					.arg(t_users::Value)
					.arg(f_id::Value)
					.arg(f_name::Value)
					.arg(f_passwd::Value)
					);
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_projects::Value)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 STRING)")
					.arg(t_projects::Value)
					.arg(f_id::Value)
					.arg(f_name::Value));
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_sessions::Value)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 DATETIME, %5 INTEGER, %7 INTEGER"
					",FOREIGN KEY(%5) REFERENCES %6(%2)"
					",FOREIGN KEY(%7) REFERENCES %8(%2)"
					")")
					.arg(t_sessions::Value)
					.arg(f_id::Value)
					.arg(f_start::Value)
					.arg(f_finish::Value)
					.arg(f_user::Value)
					.arg(t_users::Value)
					.arg(f_project::Value)
					.arg(t_projects::Value)
					);
			EXEC_AND_REPORT_COND;
		}
		//Actions table
		if (!m_db.tables().contains(t_actions::Value)) {
			q.prepare(QString("CREATE TABLE %1 ("
					"%2 INTEGER PRIMARY KEY, "
					"%3 STRING,"
					"%4 INTEGER,"
					"%6 DATETIME,"
					"%7 INTEGER,"
					"%8 INTEGER,"
					"%10 INTEGER,"
					"FOREIGN KEY(%4) REFERENCES %5(%2)"
					",FOREIGN KEY(%8) REFERENCES %9(%2)"
					",FOREIGN KEY(%10) REFERENCES %11(%2)"
					")")
					.arg(t_actions::Value)
					.arg(f_id::Value)
					.arg(f_name::Value)
					.arg(f_session_id::Value)
					.arg(t_sessions::Value)
					.arg(f_time::Value)
					.arg(f_actTime::Value)
					.arg(f_user::Value)
					.arg(t_users::Value)
					.arg(f_project::Value)
					.arg(t_projects::Value)
					);
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_achivements_list::Value)) {
			q.prepare(QString("CREATE TABLE %1 (%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 STRING, %5 STRING, %6 STRING, %7 INTEGER, %9 INTEGER"
					",FOREIGN KEY(%7) REFERENCES %8(%2)"
					",FOREIGN KEY(%9) REFERENCES %10(%2)"
					")")
					.arg(t_achivements_list::Value)
					.arg(f_id::Value)
					.arg(f_time::Value)
					.arg(f_name::Value)
					.arg(f_description::Value)
					.arg(f_condition::Value)
					.arg(f_user::Value)
					.arg(t_users::Value)
					.arg(f_project::Value)
					.arg(t_projects::Value)
					);
			EXEC_AND_REPORT_COND;
		}
		if (!m_db.tables().contains(t_achivements_done::Value)) {
			q.prepare(QString("CREATE TABLE %1 "
					"(%2 INTEGER PRIMARY KEY, %3 DATETIME, %4 STRING,"
					" %5 STRING, %6 STRING, %7 INTEGER, %8 INTEGER,"
					"FOREIGN KEY(%7) REFERENCES %9(%2)"
					")")
					.arg(t_achivements_done::Value)
					.arg(f_id::Value)
					.arg(f_time::Value)
					.arg(f_name::Value)
					.arg(f_description::Value)
					.arg(f_condition::Value)
					.arg(f_ach_id::Value)
					.arg(f_session_id::Value)
					.arg(t_achivements_list::Value)
			);
			EXEC_AND_REPORT_COND;
		}
	}

	bool parseXmlRows(QIODevice *p_file, QList<QVariantMap> &map) {
		const QByteArray ba = p_file->readAll();
		QDomDocument doc;
		QString err_string;
		int err_line;
		int err_column;
		if (!doc.setContent(ba, false, &err_string, &err_line, &err_column)) {
			DEBUG_ERR( "Can't set content for stream\n" );
			p_file->close();
			return false;
		}
		QDomElement element = doc.firstChildElement().firstChildElement(tag_element::Value);
		while (!element.isNull()) {
			QVariantMap dta;
			QDomElement elAttr = element.firstChildElement();
			while (!elAttr.isNull()) {
				//type conversion
				QVariant value;
				if (elAttr.tagName() == f_id::Value) {
					int inttext = elAttr.text().toInt();
					value = inttext;
				} else {
					value = elAttr.text();
				}
				dta[elAttr.tagName()] = value;
				elAttr = elAttr.nextSiblingElement();
			}
			map.append(dta);
			element = element.nextSiblingElement(tag_element::Value);
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
	std::string m_Project;
	std::string m_User;
	QList<QVariantMap> m_achivements;
	QMap<QString, CalcVarDelegateBase*> m_calcVars;
	QList<QVariantMap> m_instant_achievements;
	DelegateContainer* m_calc_vars_container = nullptr;
	std::mutex m_mutex;
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
achievements_params EngineImpl::take_ach_params() {
	return p->take_ach_params();
}

void EngineImpl::addProject(const std::string &project) {
	p->addProject(project);
}
void EngineImpl::addUser(const std::string &name, const std::string &passwd) {
	p->addUser(name, passwd);
}
bool EngineImpl::init(const std::string &project, const std::string &name, const std::string &passwd) {
	return p->init(project, name, passwd);
}

std::vector<var_traits> EngineImpl::varMetas() {
	return p->varMetas();
}
bool EngineImpl::achievementsToXml(QIODevice *stream) {
	return p->achievementsToXml(stream);
}
bool EngineImpl::synchroAchievements(QIODevice *stream) {
	return p->synchroAvhivementsDb(stream);
}

EngineImpl::~EngineImpl()
{
	STAT_IF_VERBOSE;
	if (p) delete p;
}
void EngineImpl::dropTables() {
	p->dropTables();
}

} //namespace AE
