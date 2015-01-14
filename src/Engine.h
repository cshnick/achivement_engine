#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <map>

namespace AE {
class variant;
}
typedef std::map<std::string, AE::variant> action_params;

namespace AE {

enum varType {
	AE_VAR_INVALID = 0x00,
	AE_VAR_STRING,
	AE_VAR_INT,
	AE_VAR_FLOAT,
	AE_VAR_DATETIME
};

class dateTime {
public:
	dateTime() : m_value(-1) {}
	dateTime(int p_var) : m_value(p_var) {}
    operator int() {return m_value;}
private:
	int m_value;
};

class variant {
public:
	variant() : m_type(AE_VAR_INVALID) {
	}
	variant(int p_var) : int_val(p_var), m_type(AE_VAR_INT) {
	}
	variant(float p_var) : float_val(p_var), m_type(AE_VAR_FLOAT){
	}
	variant(const std::string &p_var) : str_val(p_var), m_type(AE_VAR_STRING) {
	}
	variant(const dateTime &p_var) : dt_val(p_var), m_type(AE_VAR_DATETIME) {
	}
	int type() const {return m_type;}

	int toInt(bool *ok = 0) const {if (ok) *ok = (m_type == AE_VAR_INT) ? true : false; return int_val;}
	float toFloat(bool *ok = 0) const {if (ok) *ok = (m_type == AE_VAR_FLOAT) ? true : false; return float_val;}
	std::string toString(bool *ok = 0) const {if (ok) *ok = (m_type == AE_VAR_STRING) ? true : false; return str_val;}
	dateTime toDateTime(bool *ok = 0) const {if (ok) *ok = (m_type == AE_VAR_DATETIME) ? true : false; return dt_val;}

private:
	int int_val = -1;
	std::string str_val;
	float float_val = -1;
	dateTime dt_val;
	int m_type = AE_VAR_INVALID;
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
