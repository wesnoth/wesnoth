/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <map>

//instead of playing with VC++'s crazy definitions of min and max,
//just define our own
template<typename T>
T& minimum(T& a, T& b) { return a < b ? a : b; }

template<typename T>
const T& minimum(const T& a, const T& b) { return a < b ? a : b; }

template<typename T>
T& maximum(T& a, T& b) { return a < b ? b : a; }

template<typename T>
const T& maximum(const T& a, const T& b) { return a < b ? b : a; }

#endif
