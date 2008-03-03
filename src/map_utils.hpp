#ifndef MAP_UTILS_HPP_INCLUDED
#define MAP_UTILS_HPP_INCLUDED

#include <map>

template<typename K, typename V>
const V& map_get_value_default(const std::map<K,V>& m, const K& key, const V& val) {
	typename std::map<K,V>::const_iterator i = m.find(key);
	if(i != m.end()) {
		return i->second;
	} else {
		return val;
	}
}

#endif
