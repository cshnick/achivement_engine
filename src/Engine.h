#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <map>

typedef std::map<std::string, std::string> action_params;

namespace AE {

class Engine {
public:
	virtual ~Engine() {}
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void addAction(const action_params &p_actions) = 0;
};

} //namespace AE

#endif //ENGINE_H
