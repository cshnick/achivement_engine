#ifndef ENGINE_IMPL_H
#define ENGINE_IMPL_H

#include <string>
#include <map>
#include <vector>

#include "Engine.h"

#define dbg_fprintf(stream, message, ...) fprintf(stream, message __VA_ARGS__)
#define SQL_ERR(...) dbg_fprintf(stderr, "SQL: ", __VA_ARGS__)
#define SQL_DEBUG(...) dbg_fprintf(stdout, "SQL: ", __VA_ARGS__)
#define DEBUG_ERR(...) dbg_fprintf(stderr, "ERR: ", __VA_ARGS__)
#define DEBUG(...) dbg_fprintf(stdout, "DBG: ", __VA_ARGS__)

namespace AE {

class CalcVarDelegateBase  {
public:
	CalcVarDelegateBase() {}
	virtual std::string typeStr() const = 0;
	virtual std::string varName() const = 0;
	virtual void refresh() = 0;
	virtual variant var() const;
	virtual ~CalcVarDelegateBase() {}
};

class DelegateContainer {
public:
	DelegateContainer() {}
	virtual void addContext(void *context) = 0;
	virtual std::vector<CalcVarDelegateBase*> delegates() = 0;
	virtual ~DelegateContainer() {}
};

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
} //namespace AE



#endif //ENGINE_IMPL_H
