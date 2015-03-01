#include "Engine.h"
#include "unistd.h"

#ifndef NDEBUG
#define dbg_fprintf(stream, message, ...) if (getenv("DEBUG")) fprintf(stream, message __DATE__ ":" __TIME__ "\t" __VA_ARGS__)
#else //NDEBUG
#define dbg_fprintf(stream, message, ...)
#endif //NDEBUG
#define SQL_ERR(...) dbg_fprintf(stderr, "SQL: ", __VA_ARGS__)
#define SQL_DEBUG(...) dbg_fprintf(stdout, "SQL: ", __VA_ARGS__)
#define DEBUG_ERR(...) dbg_fprintf(stderr, "ERR: ", __VA_ARGS__)
#define DEBUG(...) dbg_fprintf(stdout, "DBG: ", __VA_ARGS__)

#define NO_IMPL_REPORT DEBUG("%s: %d NOT IMPLEMENTED\n", __PRETTY_FUNCTION__, __LINE__)

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
