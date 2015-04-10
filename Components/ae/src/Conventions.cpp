#include "Conventions.h"

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
	case QVariant::LongLong:
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
/**
 * Convert AE::variant to QVariant
 */
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
/**
 *  Convert QVariant to AE::variant
 */
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

} // namespace AE
