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

#include <cmath>
#include <map>
#include <sstream>

//instead of playing with VC++'s crazy definitions of min and max,
//just define our own
template<typename T>
inline T& minimum(T& a, T& b) { return a < b ? a : b; }

template<typename T>
inline const T& minimum(const T& a, const T& b) { return a < b ? a : b; }

template<typename T>
inline T& maximum(T& a, T& b) { return a < b ? b : a; }

template<typename T>
inline const T& maximum(const T& a, const T& b) { return a < b ? b : a; }

template<typename T>
inline bool is_odd(T num) {
  int n = static_cast< int >(num);
  return static_cast< unsigned int >(n >= 0 ? n : -n) & 1;
}

template<typename T>
inline bool is_even(T num) { return !is_odd(num); }

struct bad_lexical_cast {};

template<typename To, typename From>
To lexical_cast(From a)
{
	To res;
	std::stringstream str;
	
	if(!(str << a && str >> res)) {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<typename To, typename From>
To lexical_cast_default(From a, To def=To())
{
	To res;
	std::stringstream str;
	
	if(!(str << a && str >> res)) {
		return def;
	} else {
		return res;
	}
}

template<typename From>
std::string str_cast(From a)
{
	return lexical_cast<std::string,From>(a);
}

inline bool chars_equal_insensitive(char a, char b) { return tolower(a) == tolower(b); }

//a definition of 'push_back' for strings, since some implementations
//don't support string::push_back
template<typename T, typename C>
void push_back(T& str, C c)
{
	str.resize(str.size()+1);
	str[str.size()-1] = c;
}

#if 1
# include <SDL_types.h>
typedef Sint32 fixed_t;
# define fxp_shift 8
# define fxp_base (1 << fxp_shift)

// IN: float or int - OUT: fixed_t
# define ftofxp(x) (fixed_t((x) * fxp_base))

// IN: unsigned and fixed_t - OUT: unsigned
# define fxpmult(x,y) (((x)*(y)) >> fxp_shift)

// IN: unsigned and int - OUT: fixed_t
# define fxpdiv(x,y) (((x) << fxp_shift) / (y))

// IN: fixed_t - OUT: int
# define fxptoi(x) ( ((x)>0) ? ((x) >> fxp_shift) : (-((-(x)) >> fxp_shift)) )

#else
typedef float fixed_t;
# define ftofxp(x) (x)
# define fxpmult(x,y) ((x)*(y))
# define fxpdiv(x,y) (static_cast<float>(x) / static_cast<float>(y))
# define fxptoi(x) ( static_cast<int>(x) )
#endif

#endif
