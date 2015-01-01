/*
   Copyright (C) 2007 - 2015 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MAP_UTILS_HPP_INCLUDED
#define MAP_UTILS_HPP_INCLUDED

#include <map>
#include <stdexcept>

template<typename K, typename V>
const V& map_get_value_default(const std::map<K,V>& m, const K& key, const V& val) {
	typename std::map<K,V>::const_iterator i = m.find(key);
	if(i != m.end()) {
		return i->second;
	} else {
		return val;
	}
}

/**
 * Emulation for C++11's std::map::at().
 *
 * Acts like return map[key], but can be used on const map, and if the key
 * doesn't exist will throw an exception instead of adding the key.
 *
 * A non-official reference can be found here:
 * http://en.cppreference.com/w/cpp/container/map/at
 *
 * @note Didn't use template<class K, class V> since that has a problem when
 * deducting the type when the key is a std::string and the type send is a
 * character string, e.g. "foo". Letting the map deduct the K and V types works.
 *
 * @throw std::out_of_range       When the key is not in the map.
 *
 * @param map                     The map search into.
 * @param key                     The key to search for.
 *
 * @returns                       A copy of the value of key. @note C++11 uses
 *                                a reference, but it's not possible to create
 *                                a reference from an iterator.
 */
template<class M>
inline typename M::mapped_type at(
		  const M& map
		, const typename M::key_type& key)
{
	typename M::const_iterator itor = map.find(key);
	if(itor == map.end()) {
		throw std::out_of_range("Key »" + key + "« doesn't exist.");
	}

	return itor->second;
}

#endif
