#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <map>


typedef std::map<std::string, std::string> action_params;

namespace AE {

enum varType {
	AE_VAR_STRING,
	AE_VAR_INT,
	AE_VAR_FLOAT
};

struct variant {
	variant(int p_var) : int_val(p_var), m_type(AE_VAR_INT) {
	}
	variant(float p_var) : float_val(p_var), m_type(AE_VAR_FLOAT){
	}
	variant(const std::string &p_var) : str_val(p_var), m_type(AE_VAR_STRING) {
	}

	int toInt(bool &ok) {ok = (m_type == AE_VAR_INT) ? true : false; return int_val;}
	float toFloat(bool &ok) {ok = (m_type == AE_VAR_FLOAT) ? true : false; return float_val;}
	std::string toString(bool &ok) {ok = (m_type == AE_VAR_STRING) ? true : false; return str_val;}

private:
	int int_val;
	std::string str_val;
	float float_val;
	int m_type;
};

class Engine {
public:
	virtual ~Engine() {}
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void addAction(const action_params &p_actions) = 0;
};

} //namespace AE

#endif //ENGINE_H
