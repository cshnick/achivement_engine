#ifndef CONVENTIONS_H
#define CONVENTIONS_H

#ifndef SWIG
#include <QtCore>
#endif //SWIG
#include "Engine.h"
#include "unistd.h"

#ifndef NDEBUG
#define dbg_fprintf(stream, message, ...) fprintf(stream, message __VA_ARGS__)
#else
#define dbg_fprintf(stream, message, ...)
#endif
#define SQL_ERR(...) dbg_fprintf(stderr, "SQL: ", __VA_ARGS__)
#define SQL_DEBUG(...) dbg_fprintf(stdout, "SQL: ", __VA_ARGS__)
#define DEBUG_ERR(...) dbg_fprintf(stderr, "ERR: ", __VA_ARGS__)
#define DEBUG(...) dbg_fprintf(stdout, "DBG: ", __VA_ARGS__)

#define EXEC_AND_REPORT_COND_RETURN \
		if (!q.exec()) { \
			SQL_ERR( "last error: %s, executed query: %s\n", qPrintable(q.lastError().text()), qPrintable(q.executedQuery()) ); \
			return; \
		} else { \
			SQL_DEBUG("Executed: %s\n", qPrintable(q.executedQuery())); \
		} \

#define EXEC_AND_REPORT_COND \
		QTime label = QTime::currentTime(); \
		bool res = q.exec(); \
		int msecs = label.msecsTo(QTime::currentTime()); \
		QString exQuery = q.executedQuery(); \
		if (VERBOSE) { \
			Q_FOREACH(QVariant val, q.boundValues().values()) { \
			QString pval = val.toString(); \
			if (val.type() != QVariant::Bool && val.type() != QVariant::Int && val.type() != QVariant::Double) { \
				pval = QString("'") + pval + "'"; \
			} \
			exQuery = exQuery.replace(exQuery.indexOf('?'), 1, pval); \
		} \
		} \
		if (!res) { \
			SQL_ERR( "last error: %s, executed query: %s; ", qPrintable(q.lastError().text()), qPrintable(exQuery) ); \
		} else { \
			SQL_DEBUG("Executed: %s; ", qPrintable(exQuery)); \
		} \
		dbg_fprintf(stdout, "", "Query length: %d\n", msecs); \

#define STAT_IF_VERBOSE \
		if (VERBOSE) { \
			DEBUG("Entering ('%s':%d) : %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
		} \

#define PRINT_IF_VERBOSE(...) \
		if (VERBOSE) { \
			DEBUG(__VA_ARGS__); \
		} \

#define VERBOSE getenv("VERBOSE") != NULL
#define DROP_TABLES getenv("AE_DROP_TABLES") != NULL

namespace AE {

#ifndef SWIG
const char *printable(const variant &v);
const char *printable(const QVariant &v);
QVariant fromAeVariant(const AE::variant &ae_val);
variant fromQVariant(const QVariant &q_val);
#endif //SWIG

char const g_achivements_path[] = "/home/ilia/.local/share/action_engine";
char const g_dbName[] = "ae.db";
char const g_achivementsFileName[] = "achivements.xml";
char const t_sessions[] = "Sessions";
char const t_actions[] = "Actions";
char const t_achivements_list[] = "AchivementsList";
char const t_achivements_done[] = "AchivementsDone";
char const f_id[] = "id";
char const f_ach_id[] = "AchivementId";
char const f_start[] = "Start";
char const f_finish[] = "Finish";
char const f_time[] = "Time";
char const f_actTime[] = "ActionTime";
char const f_session[] = "Session";
char const f_name[] = "Name";
char const f_session_id[] = "SessionId";
char const f_description[] = "Description";
char const f_condition[] = "Condition";
char const tag_element[] = "achivement";
char const tag_lastId[] = "lastId";

char const f_statement[] = "Statement";
char const f_result[] = "Result";
char const f_success[] = "Success";

} // namespace AE

#endif //CONVENTIONS_H
