/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file util.hpp
//! Templates and utility-routines for strings and numbers.

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "global.hpp"
#include <cmath>
#include <map>
#include <sstream>

// instead of playing with VC++'s crazy definitions of min and max,
// just define our own
//! Replacement for VC++'s definitions of min and max.
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

//! Guarantees portable results for division by 100; round towards 0
inline int div100rounded(int num) {
	return (num < 0) ? -(((-num) + 49) / 100) : (num + 49) / 100;
}

//! round (base_damage * bonus / divisor) to the closest integer,
//! but up or down towards base_damage
inline int round_damage(int base_damage, int bonus, int divisor) {
	const int rounding = divisor / 2 - (bonus < divisor ? 0 : 1);
	return maximum<int>(1, (base_damage * bonus + rounding) / divisor);
}

// not guaranteed to have exactly the same result on different platforms
inline int round_double(double d) {
#ifdef HAVE_ROUND
	return static_cast<int>(std::round(d)); //surprisingly, not implemented everywhere
#else
	return static_cast<int>((d >= 0.0)? std::floor(d + 0.5) : std::ceil(d - 0.5));
#endif
}

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

template<>
int lexical_cast<int, const std::string&>(const std::string& a);

template<>
int lexical_cast<int, const char*>(const char* a);

template<>
int lexical_cast_default<int, const std::string&>(const std::string& a, int def);

template<>
int lexical_cast_default<int, const char*>(const char* a, int def);

template<typename From>
std::string str_cast(From a)
{
	return lexical_cast<std::string,From>(a);
}

template<typename To, typename From>
To lexical_cast_in_range(From a, To def, To min, To max)
{
	To res;
	std::stringstream str;

	if(!(str << a && str >> res)) {
		return def;
	} else {
		if(res < min) {
			return min;
		}
		if(res > max) {
			return max;
		}
		return res;
	}
}

inline bool chars_equal_insensitive(char a, char b) { return tolower(a) == tolower(b); }
inline bool chars_less_insensitive(char a, char b) { return tolower(a) < tolower(b); }

//! A definition of 'push_back' for strings,
// since some implementations don't support string::push_back
template<typename T, typename C>
void push_back(T& str, C c)
{
	str.resize(str.size()+1);
	str[str.size()-1] = c;
}

#ifdef __GNUC__
#define LIKELY(a)    __builtin_expect((a),1) // Tells GCC to optimize code so that if is likely to happen
#define UNLIKELY(a)  __builtin_expect((a),0) // Tells GCC to optimize code so that if is unlikely to happen
#else
#define LIKELY(a)    a
#define UNLIKELY(a)  a
#endif


#if 1
# include <SDL_types.h>
typedef Sint32 fixed_t;
# define fxp_shift 8
# define fxp_base (1 << fxp_shift)

//! IN: float or int - OUT: fixed_t
# define ftofxp(x) (fixed_t((x) * fxp_base))

//! IN: unsigned and fixed_t - OUT: unsigned
# define fxpmult(x,y) (((x)*(y)) >> fxp_shift)

//! IN: unsigned and int - OUT: fixed_t
# define fxpdiv(x,y) (((x) << fxp_shift) / (y))

//! IN: fixed_t - OUT: int
# define fxptoi(x) ( ((x)>0) ? ((x) >> fxp_shift) : (-((-(x)) >> fxp_shift)) )

#else
typedef float fixed_t;
# define ftofxp(x) (x)
# define fxpmult(x,y) ((x)*(y))
# define fxpdiv(x,y) (static_cast<float>(x) / static_cast<float>(y))
# define fxptoi(x) ( static_cast<int>(x) )
#endif

#endif
