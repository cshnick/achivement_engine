#include <RequestProcessorAeQt.h>
#include <QtCore>
#include <QtXml>
#include "Conventions.h"
#include "http_request.h"
#include "http_headers.h"
#include "http_content_type.h"

#include <iostream>
#include <sstream>
#include <mutex>

using namespace Network;

class RequestProcessorAeQt;
class RequestProcessorAeQtPrivate {
	RequestProcessorAeQtPrivate(RequestProcessorAeQt *p_q) : q(p_q) {

	}
	void processRequestMain(Network::IHttpRequestPtr req) {
		std::string Path = req->GetPath();
		std::string responseStr = "";

		DEBUG("Request path: %s\n", Path.c_str());
		if (Path == AE::n_tables_path::Value) {
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
				wr.writeEndElement();
			}
			wr.writeEndElement();
			wr.writeEndDocument();

			responseStr = res.toStdString();
			DEBUG("Result string: %s\n", responseStr.c_str());
		}

//		Path = "./test_content" + Path + (Path == "/" ? "index.html" : std::string());
		{
			std::stringstream Io;
			Io << "Path: " << Path << std::endl
					<< Http::Request::Header::Host::Name << ": "
					<< req->GetHeaderAttr(Http::Request::Header::Host::Value) << std::endl
					<< Http::Request::Header::Referer::Name << ": "
					<< req->GetHeaderAttr(Http::Request::Header::Referer::Value) << std::endl;
			std::lock_guard<std::mutex> Lock(*m_mutex);
			std::cout << Io.str() << std::endl;
		}
		req->SetResponseAttr(Http::Response::Header::Server::Value, "For private uses only");
//		req->SetResponseAttr(Http::Response::Header::ContentType::Value,
//				Http::Content::TypeFromFileName(Path));
//		req->SetResponseFile(Path);
		req->SetResponseString(responseStr);
	}

private:
	friend class RequestProcessorAeQt;
	RequestProcessorAeQt *q;
	std::mutex *m_mutex;
};

RequestProcessorAeQt::RequestProcessorAeQt(std::mutex *p_mtx)
   :p(new RequestProcessorAeQtPrivate(this))
{
	p->m_mutex  = p_mtx;
}

RequestProcessorAeQt::~RequestProcessorAeQt() {
	delete p;
}

void RequestProcessorAeQt::operator()(Network::IHttpRequestPtr req) {
	p->processRequestMain(req);
}

