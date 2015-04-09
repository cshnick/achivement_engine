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
#include "SQL.h"

static QDateTime g_fakeCurrentTime = QDateTime::currentDateTime();

#define DELEGATES_PATH getenv("AE_DELEGATES_PATH")
#define USE_FAKE_TIME getenv("FAKE_TIME")

typedef QVariant (*qv_func_t) (QVariant v1, QVariant v2);

using Wrap_Sql::Select;
using Wrap_Sql::Func;
using Wrap_Sql::Condition;
using Wrap_Sql::Update;
using Wrap_Sql::InsertInto;
using Wrap_Sql::CreateTable;
using Wrap_Sql::ForeignKey;
using Wrap_Sql::Reference;
using Wrap_Sql::FieldInfo;
using Wrap_Sql::dtype;
using Wrap_Sql::AlterTable;

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
//		QTextCodec::setCodecForCStrings(QTextCodec::codecForName(lc));
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
		m_calc_vars_container->init(m_Project, m_User);
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
		 auto s = Select().from(t_achivements_list::Value);
		 s.addCondition(Condition(f_visible::Value,"=",1));
		 s.addConditions(QList<Condition>()
				 << Condition(f_user::Value,"=",m_User)
				 << Condition(f_project::Value,"=",m_Project));

		 DEBUG("Select query expression: %s\n", s.expression().toUtf8().data());

		 s.exec(q);
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

		auto ins = InsertInto(t_achivements_done::Value);
		ins.append(f_condition::Value,   m.value(f_condition::Value));
		ins.append(f_description::Value, m.value(f_description::Value));
		ins.append(f_name::Value,        m.value(f_name::Value));
		ins.append(f_time::Value,        currentTime());
		ins.append(f_ach_id::Value,      m.value(f_id::Value));
		ins.append(f_session_id::Value,  m_session_id);
		ins.append(f_user::Value,        m_User);
		ins.append(f_project::Value,     m_Project);

		ins.exec(q);
	}
	void refreshAhivementsList() {
	}
	bool testCredentials() {
		if (m_User == -1 || m_Project == -1) {
			DEBUG_ERR("Username or project name is empty, process refused...\n");
		}
	}

	/**
	 *  \Brief Start session
	 *
	 *  Start session fill database start time
	 */
	void begin() {
		if (!testCredentials()) {
			throw AECommonErrorException("Username or project name is empty, process refused...");
			return;
		}

		if (!checkDB()) return;
		STAT_IF_VERBOSE;

		QSqlQuery q("", m_db);
		auto i = InsertInto(t_sessions::Value);
		i.append(f_start::Value, currentTime());
		i.append(f_user::Value, m_User);
		i.append(f_project::Value, m_Project);
		i.exec(q);
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
		auto u = Update(t_sessions::Value)
				.set(Condition(f_finish::Value, "=", currentTime()))
				.where(Condition(f_id::Value, "=", m_session_id));
		u.exec(q);
		m_session_id = -1;
	}
	void addAction(const action_params &p_actions) {
		STAT_IF_VERBOSE;
		if (!testCredentials()) {
			throw AECommonErrorException("Username or project name is empty, process refused...");
			return;
		}
		addActionToDB(p_actions);
		checkAchivements();
	}
	void addActionToDB(const action_params &p_actions) {
		if (!checkDB()) return;
		QSqlQuery q("", m_db);
		if (m_session_id == -1) {
			DEBUG_ERR("m_session_id is negative. Haven't called begin method for engine?\n");
			throw AECommonErrorException("m_session_id is negative. Haven't called begin method for engine?\n");
		}
		//Insert new actions passed from the client
		QSqlRecord rec = m_db.record(t_actions::Value);
		if (!rec.isEmpty()) {
			auto ins = InsertInto(t_actions::Value);
			for (auto iter = p_actions.begin(); iter != p_actions.end(); ++iter) {
				QString nm = QString::fromStdString(iter->first);
				AE::variant v = iter->second;
				ins.append(nm, fromAeVariant(v));
				//Create non existent fields
				if (!rec.contains(nm)) {
					auto a = AlterTable(t_actions::Value).add(FieldInfo(nm, v.typeDBString().c_str()));
					a.exec(q);
				}
			}
			QDateTime curTime = currentTime();
			int et = calculateElapsedSecsTo(curTime);
			ins.append(f_session_id::Value, m_session_id);
			ins.append(f_time::Value, curTime);
			ins.append(f_actTime::Value, et);
			ins.append(f_user::Value, m_User);
			ins.append(f_project::Value, m_Project);

			ins.exec(q);
		}
	}
	/**
	 * Calculate action time from previous action to current one
	 */
	int calculateElapsedSecsTo(const QDateTime &ct) {
		QSqlQuery q("", m_db);
		//check previous time from actions
		int result = -1;
		auto s = Select(Func("MAX", f_time::Value))
								.from(t_actions::Value)
								.where(Condition(f_session_id::Value, "=", m_session_id));
		s.addConditions(condUserProj(m_User, m_Project));
		s.exec(q);
		q.first();
		QDateTime dt = q.value(0).toDateTime();
		if (dt.isValid()) {
			SQL_DEBUG("DateTime value: %s, current time: %s\n", qPrintable(dt.toString()), qPrintable(ct.toString()));
			result = dt.secsTo(ct);
		} else { //! Take time from session start time
			auto s = Select(f_start::Value).from(t_sessions::Value).where(Condition(f_id::Value, "=", m_session_id));
			s.addConditions(condUserProj(m_User, m_Project));
			s.exec(q);
			q.first();
			QDateTime st = q.value(0).toDateTime();
			SQL_DEBUG("DateTime value: %s, current time: %s\n", qPrintable(st.toString()), qPrintable(ct.toString()));
			if (!st.isValid()) {
				DEBUG_ERR("Unable to retrieve session start id... \n");
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

	achievements_params sessionAchievements() {
		return achievements_params();
	}

	void addProject(const std::string &project) {
		idProject(project);
	}
	void addUser(const std::string &name, const std::string &passwd) {
		idUser(name, passwd);
	}
	bool init(const std::string &project, const std::string &name, const std::string &passwd = std::string()) {
		refreshDB();
		refreshTables();
		m_Project = idProject(project);
		m_User = idUser(name);
		refreshAchievements();
		refreshCalcVarDelegates();

		return true;
	}

	std::vector<var_traits> varMetas() {
		if (!m_calc_vars_container) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_calc_vars_container = loadCalcDelegatesContainer();
			m_calc_vars_container->init(m_Project, m_User);
		}
		std::vector<var_traits> res;
		std::vector<CalcVarDelegateBase*> q_v = *m_calc_vars_container->delegates();
		for(auto d = q_v.begin(); d != q_v.end(); ++d) {
			var_traits vt;
			vt.name = (*d)->varAlias();
			vt.alias = (*d)->varName();
			vt.type_str = (*d)->typeStr();
			vt.description = (*d)->varDescription();
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
		updateAchievementsFromXml(stream, "", "");
		return true;
	}
	int idUser(const std::string &name, const std::string &passwd = "") {
		if (name.empty()) {
			return -1;
		}
		refreshDB();
		refreshTables();
		QSqlQuery q("", m_db);
		auto s = Select(f_id::Value).from(t_users::Value).where(Condition(f_name::Value, "=", QString::fromStdString(name)));
		s.exec(q);
		if (q.first()) {
			return q.value(0).toInt();
		} else {
			auto i = InsertInto(t_users::Value);
			i.append(f_name::Value, QString::fromStdString(name));
			i.append(f_passwd::Value, QString::fromStdString(passwd));
			i.exec(q);

			return q.lastInsertId().toInt();
		}

		return -1;
	}
	int idProject(const std::string &name) {
		if (name.empty()) {
			return -1;
		}
		refreshDB();
		refreshTables();
		QSqlQuery q("", m_db);
		auto s = Select(f_id::Value).from(t_projects::Value).where(Condition(f_name::Value, "=", QString::fromStdString(name)));
		s.exec(q);
		if (q.first()) {
			return q.value(0).toInt();
		} else {
			auto i = InsertInto(t_projects::Value);
			i.append(f_name::Value, QString::fromStdString(name));
			i.exec(q);

			return q.lastInsertId().toInt();
		}

		return -1;
	}

	bool updateAchievementsFromXml(QIODevice *stream, const std::string &user, const std::string &proj) {
		refreshDB();
		refreshTables();
		refreshCalcVarDelegates();

		QList<QVariantMap> xml_rows;
		parseXmlRows(stream, xml_rows);

		int user_id = idUser(user);
		int proj_id = idProject(proj);

		QSqlQuery q("", m_db);
		for (int i = 0, cnt = 0; i < xml_rows.count(); i++, cnt=0) {
			QVariantMap mit = xml_rows.at(i);

			bool ok;
			int iid = mit.value(f_id::Value).toInt(&ok);
			bool invalidId = !ok || iid == -1;
			if (invalidId) {
				mit.remove(f_id::Value);
				iid = -1;
			}
			auto s = Select(f_id::Value)
					.from(t_achivements_list::Value)
					.where(Condition(f_id::Value,"=",iid));
			s.addConditions(condUserProj(user_id, proj_id));
			s.exec(q);

			//Insert
			bool fst = q.first();
			if (invalidId || !fst) {
				auto i = InsertInto(t_achivements_list::Value);
				i.append(mit);
				i.append(f_user::Value, user_id);
				i.append(f_project::Value, proj_id);
				i.exec(q);
			} else {
				auto u = Update(t_achivements_list::Value)
						.where(Condition(f_id::Value,"=",iid));
				for (auto iter = mit.begin(); iter != mit.end(); iter++) {
					u.addSetCondition(Condition(iter.key(),"=",iter.value()));
				}
				u.exec(q);
			}
		}

		refreshAchievements();
		return true;
	}

	bool achievementsToXml(QIODevice *stream, const std::string &user, const std::string &proj, bool showInvisible) {
		refreshDB();
		refreshTables();
		if (!checkDB()) return false;
		QSqlQuery q("", m_db);
		auto s = Select().from(t_achivements_list::Value);
		s.addConditions(condUserProj(idUser(user), idProject(proj)));
		s.exec(q);
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
			//Showing invisible is disabled and the item is invisible due to database field f_visible
			if (!showInvisible && !rec.value(f_visible::Value).toInt()) {
				q.next();
				continue;
			}
			writer.writeStartElement(AE::tag_element::Value);
			writer.writeTextElement(f_description::Value, rec.value(f_description::Value).toString());
			writer.writeTextElement(f_condition::Value, rec.value(f_condition::Value).toString());
			writer.writeTextElement(f_name::Value, rec.value(f_name::Value).toString());
			writer.writeTextElement(f_id::Value, rec.value(f_id::Value).toString());
			writer.writeTextElement(f_type::Value, rec.value(f_type::Value).toString());
			if (showInvisible) {
				writer.writeTextElement(f_visible::Value, rec.value(f_visible::Value).toString());
			}
			writer.writeEndElement();
			q.next();
		}
		writer.writeEndElement();
		writer.writeEndDocument();
		stream->close();

		return true;
	}
	bool hideAchievements(const std::vector<int> &ids,const std::string &user, const std::string &proj) {
		refreshDB();
		refreshTables();
		if (!checkDB()) return false;

		int userId = idUser(user);
		int projId = idProject(proj);

	    for (auto iter = ids.begin(); iter != ids.end(); iter++) {
	    	int id = *iter;
	    	auto s = Select().from(t_achivements_list::Value).where(Condition(f_id::Value,"=",id));
	    	s.addConditions(condUserProj(userId, projId));
	    	QSqlQuery q("", m_db);
	    	s.exec(q);
	    	q.first();
	        if (q.isValid()) {
	        	auto u = Update(t_achivements_list::Value)
	        			.set(Condition(f_visible::Value,"=",0))
						.where(Condition(f_id::Value,"=",id));
	        	u.addWhereConditions(condUserProj(userId, projId));
	        	u.exec(q);
	        }
	    }

		return true;
	}

private:
	void refreshDB() {
		if (!m_db.isOpen()) {
			char const db_driver[] = "QSQLITE";
			PRINT_IF_VERBOSE("Initializing %s database. Database name: %s\n", db_driver, qPrintable(g_dbName::Value));
			int atomic = atomic_cnt();
			QString connectionName = QString("action_db_%1").arg(atomic);
			PRINT_IF_VERBOSE("Connection name: %s\n", connectionName.toUtf8().data());
			m_db = QSqlDatabase::addDatabase("QSQLITE", QString("action_db_%1").arg(atomic));
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
	void initGlobal(int project, int name, const std::string &passwd = std::string()) {
		m_User = name;
		m_Project = project;
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
		/**
		 *	\brief Users table
		 */
		if (!m_db.tables().contains(t_users::Value)) {
			auto c = CreateTable(t_users::Value);
			c.add(FieldInfo(f_id::Value, dtype::INTEGER, "PRIMARY KEY"));
			c.add(FieldInfo(f_name::Value, dtype::STRING));
			c.add(FieldInfo(f_passwd::Value, dtype::STRING));
			c.exec(q);
		}
		/**
		 *	\brief Projects table
		 */
		if (!m_db.tables().contains(t_projects::Value)) {
			auto c = CreateTable(t_projects::Value);
			c.add(FieldInfo(f_id::Value, dtype::INTEGER, "PRIMARY KEY"));
			c.add(FieldInfo(f_name::Value, dtype::STRING));
		    c.exec(q);
		}
		/**
		 *	\brief Sessions table
		 */
		if (!m_db.tables().contains(t_sessions::Value)) {
			auto c = CreateTable(t_sessions::Value);
			c.add(FieldInfo(f_id::Value, dtype::INTEGER, "PRIMARY KEY"));
			c.add(FieldInfo(f_start::Value, dtype::DATETIME));
			c.add(FieldInfo(f_finish::Value, dtype::DATETIME));
			c.add(FieldInfo(f_user::Value, dtype::INTEGER).ForeignKey(Reference(t_users::Value, f_id::Value)));
			c.add(FieldInfo(f_project::Value, dtype::INTEGER).ForeignKey(Reference(t_projects::Value, f_id::Value)));
			c.exec(q);
		}
		/**
		 *	\brief Actions table
		 */
		if (!m_db.tables().contains(t_actions::Value)) {
			auto c = CreateTable(t_actions::Value);
			c.add(FieldInfo(f_id::Value, dtype::INTEGER, "PRIMARY KEY"));
			c.add(FieldInfo(f_name::Value, dtype::STRING));
			c.add(FieldInfo(f_time::Value, dtype::DATETIME));
			c.add(FieldInfo(f_actTime::Value, dtype::INTEGER));
			c.add(FieldInfo(f_session_id::Value, dtype::INTEGER).ForeignKey(Reference(t_sessions::Value, f_id::Value)));
			c.add(FieldInfo(f_user::Value, dtype::INTEGER).ForeignKey(Reference(t_users::Value, f_id::Value)));
			c.add(FieldInfo(f_project::Value, dtype::INTEGER).ForeignKey(Reference(t_projects::Value, f_id::Value)));
			c.exec(q);
		}
		/**
		 *	\brief Achievements table
		 */
		if (!m_db.tables().contains(t_achivements_list::Value)) {
			auto c = CreateTable(t_achivements_list::Value);
			c.add(FieldInfo(f_id::Value, dtype::INTEGER, "PRIMARY KEY"));
			c.add(FieldInfo(f_name::Value, dtype::STRING));
			c.add(FieldInfo(f_time::Value, dtype::DATETIME));
			c.add(FieldInfo(f_description::Value, dtype::STRING));
			c.add(FieldInfo(f_condition::Value, dtype::STRING));
			c.add(FieldInfo(f_visible::Value, dtype::INTEGER, "DEFAULT 1"));
			c.add(FieldInfo(f_type::Value, dtype::INTEGER));
			c.add(FieldInfo(f_user::Value, dtype::INTEGER).ForeignKey(Reference(t_users::Value, f_id::Value)));
			c.add(FieldInfo(f_project::Value, dtype::INTEGER).ForeignKey(Reference(t_projects::Value, f_id::Value)));
			c.exec(q);
		}
		/**
		 *	\brief Reached achievements table
		 */
		if (!m_db.tables().contains(t_achivements_done::Value)) {
			auto c = CreateTable(t_achivements_done::Value);
			c.add(FieldInfo(f_id::Value, dtype::INTEGER, "PRIMARY KEY"));
			c.add(FieldInfo(f_name::Value, dtype::STRING));
			c.add(FieldInfo(f_time::Value, dtype::DATETIME));
			c.add(FieldInfo(f_description::Value, dtype::STRING));
			c.add(FieldInfo(f_condition::Value, dtype::STRING));
			c.add(FieldInfo(f_ach_id::Value, dtype::INTEGER).ForeignKey(Reference(t_achivements_list::Value, f_id::Value)));
			c.add(FieldInfo(f_session_id::Value, dtype::INTEGER).ForeignKey(Reference(t_sessions::Value, f_id::Value)));
			c.add(FieldInfo(f_user::Value, dtype::INTEGER).ForeignKey(Reference(t_users::Value, f_id::Value)));
			c.add(FieldInfo(f_project::Value, dtype::INTEGER).ForeignKey(Reference(t_projects::Value, f_id::Value)));
			c.exec(q);
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

	QList<Condition> condUserProj(int user, int project) {
		return QList<Condition>()
				<< Condition(f_user::Value,"=",user)
				<< Condition(f_project::Value,"=",project)
				;
	}

	static int atomic_cnt() {
		volatile static int cnt = 0;
		__sync_fetch_and_add(&cnt, 1);
		return cnt;
	}

private:
	friend class EngineImpl;
	EngineImpl *q;

	QSqlDatabase m_db;
	QSqlRecord m_record;
	int m_session_id;
	int m_Project;
	int m_User;
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
achievements_params EngineImpl::session_achievements() {
	return p->sessionAchievements();
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
bool EngineImpl::achievementsToXml(QIODevice *stream, const std::string &user, const std::string &proj, bool showInvisible) {
	return p->achievementsToXml(stream, user, proj, showInvisible);
}
bool EngineImpl::updateAchievementsFromXml(QIODevice *stream, const std::string &user, const std::string &proj) {
	return p->updateAchievementsFromXml(stream, user, proj);
}
bool EngineImpl::hideAchievements(const std::vector<int> &ids,const std::string &user, const std::string &proj) {
	return p->hideAchievements(ids, user, proj);
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
