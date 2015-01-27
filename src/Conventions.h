#ifndef CONVENTIONS_H
#define CONVENTIONS_H

#include <QtCore>
#include "Engine.h"

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
		dbg_fprintf("Query length: %d\n", msecs); \

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
const char *printable(const variant &v);
const char *printable(const QVariant &v);
QVariant fromAeVariant(const AE::variant &ae_val);
variant fromQVariant(const QVariant &q_val);
} // namespace AE

static const QString g_achivements_path = "/home/ilia/.local/share/action_engine";
static const QString g_dbName = "ae.db";
static const QString g_achivementsFileName = "achivements.xml";
static const QString t_sessions = "Sessions";
static const QString t_actions = "Actions";
static const QString t_achivements_list = "AchivementsList";
static const QString t_achivements_done = "AchivementsDone";
static const QString f_id = "id";
static const QString f_ach_id = "AchivementId";
static const QString f_start = "Start";
static const QString f_finish = "Finish";
static const QString f_time = "Time";
static const QString f_actTime = "ActionTime";
static const QString f_session = "Session";
static const QString f_name = "Name";
static const QString f_session_id = "SessionId";
static const QString f_description = "Description";
static const QString f_condition = "Condition";
static const QString tag_element = "achivement";
static const QString tag_lastId = "lastId";

#endif //CONVENTIONS_H
