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
   Hence use static tokens as replacements for constant string literals in the code in a local scope, as follows:
   static const n_token::t_token & z_label( generate_safe_static_const_t_interned(n_token::t_token("some text")) );
   @see below on caveats in Static Initialization

   @note It is intentionally inconvenient to convert to std::string.  Allowing automatic conversion would allow
   the compiler to create std::string temporaries and give up the benefits of the t_token.  Do not
   make a std::string conversion operator.

   @par @b Static Initialization and Deinitializtion @b
   Static initialization problems can occur when a static file local variable from one compilation unit/file is
   accessed in a function called from another compilation unit/file during static initialization/de-initialization.
   The initialization order is not specified in the C++ spec and hence potentially random. There are 3 ways to fix
   it for 3 different use cases, outlined below.

   @li 1. Create a non-inlined function that allocates (via new) a static pointer to a t_token and return a reference to it.
   This is used in z_empty() and is rock solid and never fails.  As long as it is not inlined the static allocation is executed once
   when control passes over the line.  It is never de-allocated so it never faces the static de-initialization problem.  The downside
   is that is always results in the overhead of a function call.  It is necessary for creating default parameters in function
   declarations.

   @li 2. Use new to allocate a static pointer in a local scope block.  This is also rock solid and works for everything except
   creaing default parameters in function declarations. This has the slight drawback of the pointer overhead.
   static t_token *z_empty = new t_token("", false);

	@li 3.	Lastly create a static object in a local scope.  This is avoids the pointer dereference, but risks a static de-initalization
	core dump if z_some_string is deallocated before its last use.  If you are sure that this function is never called by destructors
	outside of the compilation unit/file that it is created in, then this method is correct, but a little risky
	static const t_token z_some_string("some_string", false);

 */

class t_token : public  n_interned::t_interned_token<std::string> {
public:
	typedef n_interned::t_interned_token<std::string> t_base;

	t_token() : t_base(z_empty()){}
	explicit t_token(std::string const & a , bool is_ref_counted = true) : t_base(a, is_ref_counted) {}
	t_token(t_token const & a) : t_base(a) {}
	t_token  & operator=(t_token const & a) { this->t_base::operator=(a); return *this;}

	/** Empty string token in a form suitable for use as a default value safe from static initialization errors */
	static const t_token & z_empty();
	/**Return a default interned object suitable as a default value which won't cause a static initialization error */
	template <char T_defchar>
	static t_token const & default_value();

	inline bool empty() const {return *this == z_empty() ;}

	std::string const & str() const {return **this;}

	char const * c_str() const {return (**this).c_str();}   /// todo remove this kluge for lua.cpp

	/**These are slow comparison functions.  Prefer t_token == t_token which is fast */
	friend inline bool operator==(t_token const &a, char const *b){ return (*a) == b;}
	friend inline bool operator==(char const *b, t_token const &a){ return b == (*a);}
	friend inline bool operator!=(t_token const &a, char const *b){ return !(a == b); }
	friend inline bool operator!=(char const *b, t_token const &a){ return !(b == a); }

	friend std::ostream & operator<<(std::ostream &out, t_token const & a){
		return out << (*a); }
};

/** Do not inline this enforces static initialization order */
template <char T_defval>
t_token const & t_token::default_value() {
	//This is NOTa memory leak
	//It is static so it is only allocated once and not de-allocated until the program terminates.
	//If changed to a static reference it may cause a static de-initialization
	//core dump when the destructor for z_empty is called here before its last use in another file.
	//Do not remove the = new t_token(std::string(1, T_defval), false); part
	static t_token *z_defval = new t_token(std::string(1, T_defval), false);
	return *z_defval;
}

inline std::string operator+(const n_token::t_token &a, const std::string &b) { return (*a) + b; }
inline std::string operator+(const std::string &a, const n_token::t_token &b) { return a + (*b); }
inline std::string operator+(const n_token::t_token &a, const char *b) { return (*a) + b; }
inline std::string operator+(const char *a, const n_token::t_token &b) { return a + (*b); }
inline std::string operator+(const n_token::t_token &a, const n_token::t_token &b) { return (*a) + (*b); }

/** These 2 macros create a default parameter value for functions that is static initialization and de-initialization safe
	There is no other way as templates don't allow string literals
	For DEFAULT_TOKEN_HEADER(blah, blah_value)
	The function declaration will be
	const t_token & blah();
 */
#define DEFAULT_TOKEN_HEADER(name, value) \
	n_token::t_token const & name();

#define DEFAULT_TOKEN_BODY(name, value) \
	static n_token::t_token const & name() {									\
	/*This is NOTa memory leak											\
	It is static so it is only allocated once and not de-allocated until the program terminates. \
	If changed to a static reference it may cause a static de-initialization \
	core dump when the destructor for z_empty is called here before its last use in another file. \
	Do not remove the = new t_token(std::string(1, T_defval), false); part  */ \
		static n_token::t_token *z_defval = new n_token::t_token(value, false);	\
		return *z_defval;												\
}




}//end namepace

#endif
