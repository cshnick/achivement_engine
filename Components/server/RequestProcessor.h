#ifndef COMPONENTS_SERVER_REQUESTPROCESSOR_H_
#define COMPONENTS_SERVER_REQUESTPROCESSOR_H_

#include "http_server.h"

namespace Network {
class RequestProcessorBase {
public:
	virtual ~RequestProcessorBase() {}
	virtual void operator()(IHttpRequestPtr req) = 0;
};
} //namespace Network




#endif /* COMPONENTS_SERVER_REQUESTPROCESSOR_H_ */
