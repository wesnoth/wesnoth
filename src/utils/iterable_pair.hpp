/*
 Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
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

#pragma once

#include <utility>
#include <iterator>

namespace std { // Some cases don't work if not in std namespace

template<typename T>
inline T begin(const std::pair<T, T>& p) {
	return p.first;
}

template<typename T>
inline T end(const std::pair<T, T>& p) {
	return p.second;
}

// TODO: Is there a way to enforce that this is called only for const_iterators?
template<typename T>
inline T cbegin(const std::pair<T, T>& p) {
	return p.first;
}

template<typename T>
inline T cend(const std::pair<T, T>& p) {
	return p.second;
}

template<typename T>
inline std::reverse_iterator<T> rbegin(const std::pair<T, T>& p) {
	return std::reverse_iterator<T>(p.second);
}

template<typename T>
inline std::reverse_iterator<T> rend(const std::pair<T, T>& p) {
	return std::reverse_iterator<T>(p.first);
}

// TODO: Is there a way to enforce that this is called only for const_iterators?
template<typename T>
inline std::reverse_iterator<T> crbegin(const std::pair<T, T>& p) {
	return std::reverse_iterator<T>(p.second);
}

template<typename T>
inline std::reverse_iterator<T> crend(const std::pair<T, T>& p) {
	return std::reverse_iterator<T>(p.first);
}

}
