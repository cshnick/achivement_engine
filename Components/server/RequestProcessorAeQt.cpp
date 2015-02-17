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
		m_callbacks[AE::n_achievement_list_path::Value] = &RequestProcessorAeQtHelper::processAhievementList;
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
			wr.writeEndElement();
		}
		wr.writeEndElement();
		wr.writeEndDocument();

		req->SetResponseString(res.toStdString());
	}
	void processAhievementList(IHttpRequestPtr req) {
		QBuffer buf;
		buf.open(QIODevice::WriteOnly);
		AE::EngineImpl().achievementsToXml(&buf);
//		PRINT_IF_VERBOSE("Response data: %s", buf.data().data());
//		std::string test_str(buf.data().data());
//		PRINT_IF_VERBOSE("Response std data: %s", test_str.c_str());
		req->SetResponseAttr(Network::Http::Response::Header::ContentType::Value, "text/xml; charset=utf-8");
		req->SetResponseString(buf.data().data());
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

