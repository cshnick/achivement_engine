#ifndef ENGINE_IMPL_H
#define ENGINE_IMPL_H

#include <string>
#include <map>

#include "Engine.h"

#define dbg_fprintf(stream, message, ...) fprintf(stream, message __VA_ARGS__)
#define SQL_ERR(...) dbg_fprintf(stderr, "SQL: ", __VA_ARGS__)
#define DEBUG_ERR(...) dbg_fprintf(stderr, "ERR: ", __VA_ARGS__)
#define DEBUG(...) dbg_fprintf(stderr, "DBG: ", __VA_ARGS__)

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
