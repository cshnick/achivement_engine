
#ifndef COMPONENTS_UTILS_CACHEDVECTOR_H_
#define COMPONENTS_UTILS_CACHEDVECTOR_H_

#include <vector>
#include <unordered_map>

namespace utils {

template <typename P, typename T>
class CachedVector;

template <typename P, typename T>
class Iter
{
public:
	Iter (const CachedVector<P, T>* p_vec, const typename std::vector<T>::const_iterator &p_iter)
	: _m_iter(p_iter)
	, _p_vec(p_vec)
	{}

	bool operator!= (const Iter& other) const {
		return this->_m_iter != other._m_iter;
	}
	int operator* () const;
	const Iter& operator++ () {
		++this->_m_iter;
		return *this;
	}

private:
	typename std::vector<T>::const_iterator _m_iter;
	const CachedVector<P, T> *_p_vec;
};

template <typename P, typename T>
class CachedVector
{
public:
	CachedVector () {}

	T get (int index) const
	{
		return m_vector.at(index);
	}
	T get (const P &key) {
		return m_map.find(key)->second;
	}
	Iter<P, T> begin() const {
		return Iter<P, T>( this, m_vector.begin());
    }
	Iter<P, T> end () const {
		return Iter<P, T>( this, m_vector.end());
	}

	void emplace (const P &key, const T &value)
	{
		auto it = m_map.find(key);
		if (it == m_map.end()) {
			m_map.emplace(key, value);
			m_vector.push_back(value);
		}
	}

private:
	std::vector<T> m_vector;
	std::unordered_map<P,T> m_map;
};

template <typename P, typename T>
int Iter<P, T>::operator* () const
{
	return *this->_m_iter;
}

class TestCached {
public:
	TestCached(int p_v);
	int val = 0;
};

}; //namespace utils

#endif /* COMPONENTS_UTILS_CACHEDVECTOR_H_ */
