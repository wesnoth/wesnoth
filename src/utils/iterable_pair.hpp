/*
 Copyright (C) 2014 - 2016 by David White <dave@whitevine.net>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */

/**
 * @file Enable range-for iteration over pairs of iterators.
 */

#ifndef UTILS_ITERABLE_PAIR_HPP_INCLUDED
#define UTILS_ITERABLE_PAIR_HPP_INCLUDED

#include <utility>

namespace std { // Some cases don't work if not in std namespace

template<typename T>
inline T begin(const std::pair<T, T>& p) {
	return p.first;
}

template<typename T>
inline T end(const std::pair<T, T>& p) {
	return p.second;
}

}

#endif
