/* $Id$ */
/*
   Copyright (C) 2007 - 2012 by David White <dave.net>
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
