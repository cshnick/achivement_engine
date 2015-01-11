#ifndef ENGINE_IMPL_H
#define ENGINE_IMPL_H

#include <string>
#include <map>

#include "Engine.h"

namespace AE {

class EngineImplPrivate;
class EngineImpl : public Engine {
public:
	EngineImpl();
	void begin();
	void end();
	void addAction(const action_params &p_actions);
	~EngineImpl();

private:
	friend class EngineImplPrivate;
	EngineImplPrivate *p;
};
}

#endif //ENGINE_IMPL_H
