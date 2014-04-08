/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * @file
 * Template for arrays.
 */

#ifndef ARRAY_HPP_INCLUDED
#define ARRAY_HPP_INCLUDED

#include <algorithm>

namespace util
{

template<typename T,size_t N>
class array
{
public:
	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;

	array() {}
	array(const T& o)
	{
		std::fill(begin(),end(),o);
	}

	iterator begin() { return a; }
	iterator end() { return a + N; }

	const_iterator begin() const { return a; }
	const_iterator end() const { return a + N; }

	reference operator[](size_type n) { return a[n]; }
	const_reference operator[](size_type n) const { return a[n]; }

	reference front() { return a[0]; }
	reference back() { return a[N-1]; }

	const_reference front() const { return a[0]; }
	const_reference back() const { return a[N-1]; }

	size_type size() const { return N; }

	bool empty() const { return size() == 0; }

	T* data() { return a; }
	const T* data() const { return a; }

private:
	T a[N];
};

} // end namespace util

#endif

