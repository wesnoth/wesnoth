/* $Id$ */
/*
   Copyright (C) 2004 - 2011 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

/**
 * @file
 */

#include <string>
#include <vector>
#include <iosfwd>
//@todo when C++0x is supported switch to #include <unordered_map>
#include <boost/unordered_map.hpp>

//debug
#include <iostream> //std::cerr

#include "utils/interned.hpp"

namespace n_token {

/**
   @class t_token
   @brief t_token is an object created from a string that allows fast O(1) equality comparison and copying.
   A unique copy of the string is available as a const reference.
   It is typically reference counted unless explicitly requested otherwise upon construction (i.e for static strings)

   The copying of tokens is faster than the creation of tokens, because a reference object isn't created.  
   Hence use static tokens as replacements for constant string literals in the code, as follows:
   static const t_token z_some_string("some_string", false);

   @note It is intentionally inconvenient to convert to std::string.  Allowing automatic conversion would allow
   the compiler to create std::string temporaries and give up the benefits of the t_token.  Do not
   make a std::string conversion operator.

 */

class t_token : public  n_interned::t_interned_token<std::string> {
public:
	typedef n_interned::t_interned_token<std::string> t_base;

	t_token() : t_base(z_empty()){} 
	explicit t_token(std::string const & a , bool is_ref_counted = true) : t_base(a, is_ref_counted) {}
	t_token(t_token const & a) : t_base(a) {}
	t_token  & operator=(t_token const & a) { this->t_base::operator=(a); return *this;}

	///Empty string token in a form suitable for use as a default value safe from static initialization errors
	static const t_token & z_empty();
	///Return a default interned object suitable as a default value which won't cause a static initialization error
	template <char T_defchar>
	static t_token const & default_value();
		
	inline bool empty() const {return *this == z_empty() ;}

	char const * c_str() const {return static_cast<std::string const &>(*this).c_str();}   /// todo remove this kluge for lua.cpp

	///These are slow comparison functions.  Prefer t_token == t_token which is fast
	friend inline bool operator==(t_token const &a, char const *b){ return static_cast<std::string const &>(a) == b;}
	friend inline bool operator==(char const *b, t_token const &a){ return b == static_cast<std::string const &>(a);}
	friend inline bool operator!=(t_token const &a, char const *b){ return !(a == b); }
	friend inline bool operator!=(char const *b, t_token const &a){ return !(b == a); }

	friend std::ostream & operator<<(std::ostream &out, t_token const & a){
		return out << static_cast<std::string const &>(a); }
};

/// Do not inline this enforces static initialization order
template <char T_defval>
const t_token& t_token::default_value() {
	static t_token z_defval(std::string(1, T_defval), false);
	return z_defval;
}

inline std::string operator+(const n_token::t_token &a, const std::string &b) { return static_cast<std::string const &>(a) + b; }
inline std::string operator+(const std::string &a, const n_token::t_token &b) { return a + static_cast<std::string const &>(b); }
inline std::string operator+(const n_token::t_token &a, const char *b) { return static_cast<std::string const &>(a) + b; }
inline std::string operator+(const char *a, const n_token::t_token &b) { return a + static_cast<std::string const &>(b); }
inline std::string operator+(const n_token::t_token &a, const n_token::t_token &b) { 
	return static_cast<std::string const &>(a) + static_cast<std::string const &>(b); }

}//end namepace

#endif
