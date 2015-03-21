#ifndef ENGINE_IMPL_H
#define ENGINE_IMPL_H

#include <string>
#include <map>
#include <vector>
#include "Engine.h"

class QIODevice;

namespace AE {

struct var_traits {
	std::string name;
	std::string alias;
	std::string type_str;
	std::string description;
};

class CalcVarDelegateBase  {
public:
	CalcVarDelegateBase() {}
	virtual std::string typeStr() const = 0;
	virtual std::string varName() const = 0;
	virtual std::string varAlias() const = 0;
	virtual std::string varDescription() const = 0;
	virtual void refresh(const variant &p = variant()) = 0;
	virtual variant var() const = 0;
	virtual ~CalcVarDelegateBase() {}
};

class DelegateContainer {
public:
	DelegateContainer() {}
	virtual void init(int id_user, int id_project) = 0;
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

	void addProject(const std::string &project);
	void addUser(const std::string &name, const std::string &passwd);
	bool init(const std::string &project, const std::string &name, const std::string &passwd = std::string());

	//Non inherited
	std::vector<var_traits> varMetas();
	bool achievementsToXml(QIODevice *stream, const std::string &user, const std::string &proj, bool showInvisible = false);
	bool updateAchievementsFromXml(QIODevice *stream, const std::string &user, const std::string &proj);
	bool hideAchievements(const std::vector<int> &ids,const std::string &user, const std::string &proj);



	~EngineImpl();

public:
	void dropTables();

private:
	friend class EngineImplPrivate;
	EngineImplPrivate *p;
};

} //namespace AE

#endif //ENGINE_IMPL_H
