/*
   Copyright (C) 2009 - 2010 by Chris Hopman <cjhopman@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CT_MATH_HPP_INCLUDED
#define CT_MATH_HPP_INCLUDED

/** calculates num / den and the remainder of that division **/
template <unsigned num, unsigned den>
struct ct_div {
	enum {
		quot = num / den,
		rem = num % den
	};
};

/** calculates the gcd of a and b **/
template <unsigned a, unsigned b>
struct ct_gcd {
	enum { value = ct_gcd<b, a % b>::value };
};

template <unsigned a>
struct ct_gcd<a, 0> {
	enum { value = a };
};

/** calculates the least common multiple of a and b **/
template <unsigned a, unsigned b>
struct ct_lcm {
	enum { value = a / ct_gcd<a, b>::value * b };
};


template <unsigned b, unsigned quot, unsigned rem>
struct ct_lmgte_impl {
	enum { value = b * (quot + 1) };
};

template <unsigned b, unsigned quot>
struct ct_lmgte_impl<b, quot, 0> {
	enum { value = b * quot };
};

/** calculates the least multiple of b that is greater than or equal to a **/
template <unsigned a, unsigned b>
struct ct_lmgte {
	enum { value = ct_lmgte_impl<b, ct_div<a, b>::quot, ct_div<a, b>::rem>::value };
};

template <typename T>
struct strideof_helper {
	T t;
	char a;
};

/**
 *  calculates the stride of some type T. That is, if char* p points to a properly aligned
 *  address for an object of type T, then (generally) p + i * strideof<T>::value will
 *  be properly aligned for an object of type T for any integer i.
 **/
template <typename T, bool = sizeof(T) != sizeof(strideof_helper<T>)>
struct strideof {
	enum { value = sizeof(strideof_helper<T>) - sizeof(T) };
};

template <typename T>
struct strideof<T, false> {
	enum { value = sizeof(T) };
};

#endif

