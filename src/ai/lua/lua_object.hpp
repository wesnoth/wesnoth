/*
   Copyright (C) 2011 by Dmitry Kovalenko <nephro.wes@gmail.com>
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
 * Lua object(value) wrapper implementation
 */


#ifndef LUA_OBJECT_HPP_INCLUDED
#define LUA_OBJECT_HPP_INCLUDED

#include <string>
#include <boost/shared_ptr.hpp>

#include "lua/lualib.h"

namespace ai {

class lua_object_base {

public:
	lua_object_base();
	virtual void store(lua_State* L, int n) = 0;
};

template <typename T>
class lua_object : public lua_object_base
{

public:

	lua_object()
	{
		// empty
	}

	boost::shared_ptr<T> get()
	{ 
		return value_;
	}
	
	void store(lua_State* L, int n)
	{
		this->value_ = boost::shared_ptr<T>(to_type(L, n));
	}

protected:

	// A group of functions that deal with the translation of the results to C++
	boost::shared_ptr<T> to_type(lua_State *, int) 
	{
		return boost::shared_ptr<T>();
	}

	boost::shared_ptr<T> value_;
};

template <>
inline boost::shared_ptr<double> lua_object<double>::to_type(lua_State *L, int n) 
{
	return boost::shared_ptr<double>(new double(lua_tonumber(L, n)));
}

template <>
inline boost::shared_ptr<std::string> lua_object<std::string>::to_type(lua_State *L, int n) 
{
	return boost::shared_ptr<std::string>(new std::string(lua_tostring(L, n)));
}

template <>
inline boost::shared_ptr<bool> lua_object<bool>::to_type(lua_State *L, int n) 
{
	return boost::shared_ptr<bool>(new bool(lua_toboolean(L, n)));
}
	
template <>
inline boost::shared_ptr<int> lua_object<int>::to_type(lua_State *L, int n) 
{
	return boost::shared_ptr<int>(new int(lua_tonumber(L, n)));
}

} // end of namespace ai


#endif