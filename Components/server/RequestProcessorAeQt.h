#ifndef COMPONENTS_SERVER_REQUESTPROCESSORAEQT_H_
#define COMPONENTS_SERVER_REQUESTPROCESSORAEQT_H_

#include <RequestProcessor.h>
#include <mutex>
#include <memory>

class RequestProcessorAeQtPrivate;
class RequestProcessorAeQt: public Network::RequestProcessorBase {
public:
	RequestProcessorAeQt(std::mutex *p_mtx);
	~RequestProcessorAeQt();

	void operator()(Network::IHttpRequestPtr req);

private:
	std::mutex *m_mutex;
};

#endif /* COMPONENTS_SERVER_REQUESTPROCESSORAEQT_H_ */
