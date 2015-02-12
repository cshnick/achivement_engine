#ifndef COMPONENTS_SERVER_REQUESTPROCESSORAEQT_H_
#define COMPONENTS_SERVER_REQUESTPROCESSORAEQT_H_

#include <RequestProcessor.h>
#include <mutex>

class RequestProcessorAeQtPrivate;
class RequestProcessorAeQt: public Network::RequestProcessorBase {
public:
	RequestProcessorAeQt(std::mutex *p_mtx);
	virtual ~RequestProcessorAeQt();

	void operator()(Network::IHttpRequestPtr req);
private:
	friend class RequestProcessorAeQtPrivate;
	RequestProcessorAeQtPrivate *p;
};

#endif /* COMPONENTS_SERVER_REQUESTPROCESSORAEQT_H_ */
