/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file util.hpp
 *  Templates and utility-routines for strings and numbers.
 */

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <string>
#include <sstream>
#include <iostream>
#include <boost/type_traits.hpp>

struct bad_lexical_cast {};

template<typename To, typename From>
To lexical_cast(From a)
{
#ifdef LOG
	std::cerr << "Lexical cast generic\n";
#endif
	To res;
	std::stringstream str;

	if(!(str << a && str >> res)) {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}

template<>
int lexical_cast<int, const std::string&>(const std::string& a);

template<>
int lexical_cast<int, const char*>(const char* a);

namespace implementation {

template<typename To, typename From>
To default_lexical_cast(From a)
{
#ifdef THROW
		throw std::string("default");
#endif
#ifdef LOG
	std::cerr << "default\n";
#endif
	To res;
	std::stringstream str;

	if(!(str << a && str >> res)) {
		throw bad_lexical_cast();
	} else {
		return res;
	}
}



template<typename To, typename From>
struct tlexical_cast
{
	To operator()(From a)
	{
#ifdef LOG
		std::cerr << "NEW Generic\n";
#endif
		return default_lexical_cast<To,
				typename boost::add_reference<
				typename boost::add_const<From>::type>::type>(a);
	}
};

template<typename To, typename From>
struct tlexical_cast<To, From*>
{
	To operator()(From* a)
	{
#ifdef LOG
		std::cerr << "NEW pointer\n";
#endif
		return default_lexical_cast<To, const From*>(a);
	}
};

template<typename From>
struct tlexical_cast<std::string, From>
{
	template <class T>
	std::string cast(T a, const boost::true_type&)
	{
#ifdef THROW
		throw std::string("cast To = std::string - true_type");
#endif
#ifdef LOG
		std::cerr << "cast To = std::string - true_type";
#endif
		std::stringstream sstr;
		sstr << a;
		return sstr.str();
	}

	template <class T>
	std::string cast(T a, const boost::false_type&)
	{
#ifdef THROW
		throw std::string("cast To = std::string - false_type");
#endif
#ifdef LOG
		std::cerr << "cast To = std::string - false_type";
#endif
		return default_lexical_cast<std::string, T>(a);
	}

	std::string operator()(From a)
	{
#ifdef LOG
		std::cerr << "cast To = std::string\b";
#endif
		return this->cast<From>(a, boost::is_integral<From>());
	}
};


/*
template<typename To>
struct tlexical_cast<To, const std::string&>
{
	To operator()(const std::string& str)
	{
#ifdef LOG
		std::cerr << "NEW Specialized\n";
#endif
		return lexical_cast_from_string<From>(a, boost::is_integral<From>());
	}
};
*/

} // namespace implementation


template<typename To, typename From>
To NEW_lexical_cast(From a)
{
#ifdef LOG
	std::cerr << "NEW Lexical cast\n";
#endif
	return implementation::tlexical_cast<To, From>().operator()(a);

}
#endif
