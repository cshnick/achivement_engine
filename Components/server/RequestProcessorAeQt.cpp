#include <RequestProcessorAeQt.h>
#include <QtCore>
#include <QtXml>
#include "Conventions.h"
#include "http_request.h"
#include "http_headers.h"
#include "http_content_type.h"
#include "EngineImpl.h"

#include <iostream>
#include <sstream>
#include <mutex>
#include <memory>
#include <map>
#include <functional>

using namespace Network;

class RequestProcessorAeQt;
class RequestProcessorAeQtHelper {
public:
	typedef void (RequestProcessorAeQtHelper::*__req_func_t)(IHttpRequestPtr);
	typedef std::map<std::string, __req_func_t> callback_map;
	RequestProcessorAeQtHelper(RequestProcessorAeQt *p_q, std::mutex *p_mutex) : q(p_q), m_mutex(p_mutex) {
		m_callbacks[AE::n_tables_path::Value] 			= &RequestProcessorAeQtHelper::processMeta;
		m_callbacks[AE::n_achievement_list_path_get::Value] = &RequestProcessorAeQtHelper::processAhievementListGet;
		m_callbacks[AE::n_achievement_list_path_send::Value] = &RequestProcessorAeQtHelper::processAhievementListSend;
	}
	void processMeta(IHttpRequestPtr req) {
		QString res;
		QXmlStreamWriter wr(&res);
		wr.setAutoFormatting(true);

		wr.writeStartDocument();
		wr.writeStartElement(AE::tag_root::Value);
		AE::conv_map m;
		AE::fillConventions(m);
		for (auto i = m.begin(); i != m.end(); ++i) {
			wr.writeStartElement(AE::tag_element::Value);
			wr.writeTextElement(AE::tag_name::Value, i->first);
			wr.writeTextElement(AE::tag_value::Value, i->second);
			wr.writeTextElement(AE::tag_type_str::Value, AE::val_type_sql::Value);
			wr.writeEndElement();
		}
		m_mutex->lock();
		std::vector<AE::var_traits> v = AE::EngineImpl().varMetas();
		m_mutex->unlock();
		for (auto j = v.begin(); j != v.end(); ++j) {
			wr.writeStartElement(AE::tag_element::Value);
			wr.writeTextElement(AE::tag_name::Value, j->name.c_str());
			wr.writeTextElement(AE::tag_value::Value, j->alias.c_str());
			wr.writeTextElement(AE::tag_type_str::Value, j->type_str.c_str());
			wr.writeTextElement(AE::tag_description::Value, j->description.c_str());
			wr.writeEndElement();
		}
		wr.writeEndElement();
		wr.writeEndDocument();

		req->SetResponseString(res.toStdString());
	}
	void processAhievementListGet(IHttpRequestPtr req) {
		if (req->GetRequestType() == IHttpRequest::Type::POST) {
			DEBUG("Request post\n");
			int len  = req->GetContentSize();
			char req_params[len + 1];
			memset(req_params, '\0', len + 1);
			req->GetContent((void*)req_params, len, 1);
			if (req_params) {
				QString str = QString::fromUtf8(req_params, len);
				QMap<QString, QString> m = parseParams(str);
				std::string user = m.value(AE::tag_user::Value).toStdString();
				std::string proj = m.value(AE::tag_project::Value).toStdString();
				QBuffer buf;
				buf.open(QIODevice::WriteOnly);
				AE::EngineImpl().achievementsToXml(&buf, user, proj);
				req->SetResponseAttr(Network::Http::Response::Header::ContentType::Value, "text/xml; charset=utf-8");
				req->SetResponseString(buf.data().data());
			}
		}
	}
	void processAhievementListSend(IHttpRequestPtr req) {
		if (req->GetRequestType() == IHttpRequest::Type::POST) {
			DEBUG("Request post\n");
			int len  = req->GetContentSize();
			char buf[len + 1];
			memset(buf, '\0', len + 1);
			STAT_IF_VERBOSE;
			DEBUG("SERVER REPLY: Buffer: %s\n", buf);
			req->GetContent((void*)buf, len, 1);
			if (buf) {
				QString str = QString::fromUtf8(buf, len);
				QMap<QString, QString> m = parseParams(str);
				std::string user = m.value(AE::tag_user::Value).toStdString();
				std::string proj = m.value(AE::tag_project::Value).toStdString();
				//Hide removed achievements
				QString removed = m.value(AE::tag_removed::Value);
			    QJsonDocument jsDoc = QJsonDocument::fromJson(removed.toUtf8());
			    if (jsDoc.isArray()) {
			    	std::vector<int> arr_id;
			    	QJsonArray jsArr = jsDoc.array();
			    	for (auto iter = jsArr.begin(); iter != jsArr.end(); ++iter) {
			    		QJsonValue vref = *iter;
			    		QVariant v = vref.toVariant();
			    		if (v.type()  == QVariant::Map) {
			    			QVariantMap iter_map = v.value<QVariantMap>();
			    			int iter_id = iter_map.value(AE::f_id::Value).toInt();
			    			arr_id.push_back(iter_id);
			    		}
			    	}
			    	AE::EngineImpl().hideAchievements(arr_id, user, proj);
			    }
			    //Parse content
				QString content(m.value(AE::tag_content::Value));
				QBuffer b;
				b.setData(content.toUtf8().data(), content.toUtf8().length());
				b.open(QIODevice::ReadOnly);
				AE::EngineImpl().updateAchievementsFromXml(&b, user, proj);

				QBuffer buf_response;
				buf_response.open(QIODevice::WriteOnly);
				AE::EngineImpl().achievementsToXml(&buf_response, user, proj);
				req->SetResponseAttr(Network::Http::Response::Header::ContentType::Value, "text/xml; charset=utf-8");
				req->SetResponseString(buf_response.data().data());
				req->SetResponseCode(200);
			} else {
				req->SetResponseCode(204);
			}
		}
	}

	QMap<QString, QString> parseParams(const QString &str) {
		QMap<QString, QString> res;
		QStringList l = str.split("&***");
		Q_FOREACH(QString n_p, l) {
			QStringList p = n_p.split("=***");
			res[p.first()] = p.last();
		}
		return res;
	}

	void execForString(const std::string &str_exp, IHttpRequestPtr req) {
		auto i = m_callbacks.find(str_exp);
		if (i != m_callbacks.end()) {
			__req_func_t callable = i->second;
			(this->*callable)(req);
		}
	}
	void processRequestMain(Network::IHttpRequestPtr req) {
		std::string Path = req->GetPath();
		execForString(Path, req);

		DEBUG("Request path: %s\n", Path.c_str());
	}
	~RequestProcessorAeQtHelper() {
	}

private:
	friend class RequestProcessorAeQt;
	RequestProcessorAeQt *q;
	std::mutex *m_mutex = nullptr;
	callback_map m_callbacks;
};

RequestProcessorAeQt::RequestProcessorAeQt(std::mutex *p_mtx)
{
	m_mutex = p_mtx;
}

RequestProcessorAeQt::~RequestProcessorAeQt() {
}

void RequestProcessorAeQt::operator()(Network::IHttpRequestPtr req) {
	RequestProcessorAeQtHelper(this, m_mutex).processRequestMain(req);
}

