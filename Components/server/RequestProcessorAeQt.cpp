#include <RequestProcessorAeQt.h>
#include <QtCore>
#include "Conventions.h"

class RequestProcessorAeQt;
class RequestProcessorAeQtPrivate {
	RequestProcessorAeQtPrivate(RequestProcessorAeQt *p_q) : q(p_q) {

	}
private:
	friend class RequestProcessorAeQt;
	RequestProcessorAeQt *q;
	std::mutex *m_mutex;
};

RequestProcessorAeQt::RequestProcessorAeQt(std::mutex *p_mtx)
   :p(new RequestProcessorAeQtPrivate(this))
{
}

RequestProcessorAeQt::~RequestProcessorAeQt() {
	delete p;
}

void RequestProcessorAeQt::operator()(Network::IHttpRequestPtr req) {
}

