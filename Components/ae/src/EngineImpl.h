#ifndef ENGINE_IMPL_H
#define ENGINE_IMPL_H

#include <string>
#include <map>
#include <vector>
#include "Engine.h"

namespace AE {

struct var_traits {
	std::string name;
	std::string alias;
	std::string type_str;
};

class CalcVarDelegateBase  {
public:
	CalcVarDelegateBase() {}
	virtual std::string typeStr() const = 0;
	virtual std::string varName() const = 0;
	virtual std::string varAlias() const = 0;
	virtual void refresh(const variant &p = variant()) = 0;
	virtual variant var() const = 0;
	virtual void addContext(void **v) = 0;
	virtual ~CalcVarDelegateBase() {}
};

class DelegateContainer {
public:
	DelegateContainer() {}
	virtual void addContext(void *context) = 0;
	virtual std::vector<CalcVarDelegateBase*> *delegates() = 0;
	virtual ~DelegateContainer() {}
};

class EngineImplPrivate;
class EngineImpl : public Engine {
public:
	EngineImpl();
	void begin();
	void end();
	void addAction(const action_params &p_actions);
	achievements_params take_ach_params();
	std::vector<var_traits> varMetas();

	~EngineImpl();

public:
	void dropTables();

private:
	friend class EngineImplPrivate;
	EngineImplPrivate *p;
};

} //namespace AE

#endif //ENGINE_IMPL_H
