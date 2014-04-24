/*
   Copyright (C) 2011 - 2014 by Dmitry Kovalenko <nephro.wes@gmail.com>
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
#include <vector>
#include <boost/shared_ptr.hpp>

#include "lua/lualib.h"
#include "../../scripting/lua_api.hpp"
#include "config.hpp"
#include "../default/contexts.hpp"
#include "terrain_filter.hpp"
#include "resources.hpp"
#include "unit_advancements_aspect.hpp"

namespace ai {


class lua_object_base {

public:
	lua_object_base();
	virtual ~lua_object_base() {}

	virtual void store(lua_State* L, int n) = 0;
};

template <typename T>
class lua_object : public lua_object_base
{

public:

	lua_object()
		: value_()
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
	return boost::shared_ptr<bool>(new bool(luaW_toboolean(L, n)));
}

template <>
inline boost::shared_ptr<int> lua_object<int>::to_type(lua_State *L, int n)
{
	return boost::shared_ptr<int>(new int(lua_tointeger(L, n)));
}

template <>
inline boost::shared_ptr< std::vector<std::string> > lua_object< std::vector<std::string> >::to_type(lua_State *L, int n)
{
	boost::shared_ptr< std::vector<std::string> > v = boost::shared_ptr< std::vector<std::string> >(new std::vector<std::string>());
	int l = lua_rawlen(L, n);
	for (int i = 1; i < l + 1; ++i)
	{
		lua_pushinteger(L, i);
		lua_gettable(L, n);
		std::string  s = lua_tostring(L, -1);
		lua_settop(L, n);
		v->push_back(s);
	}

	return v;
}

template <>
inline boost::shared_ptr<config> lua_object<config>::to_type(lua_State *L, int n)
{
	boost::shared_ptr<config> cfg = boost::shared_ptr<config>(new config());
	luaW_toconfig(L, n, *cfg);
	return cfg;
}

template <>
inline boost::shared_ptr<terrain_filter> lua_object<terrain_filter>::to_type(lua_State *L, int n)
{
	// To Crab_: Is this part ok? I tested it, works fine
	boost::shared_ptr<config> cfg = boost::shared_ptr<config>(new config());
	boost::shared_ptr<vconfig> vcfg = boost::shared_ptr<vconfig>(new vconfig(*cfg));
	luaW_tovconfig(L, n, *vcfg);
	boost::shared_ptr<terrain_filter> tf = boost::shared_ptr<terrain_filter>(new terrain_filter(*vcfg, *resources::units));
	return tf;
}

template <>
inline boost::shared_ptr<std::vector<target> > lua_object< std::vector<target> >::to_type(lua_State *L, int n)
{
	boost::shared_ptr<std::vector<target> > targets = boost::shared_ptr<std::vector<target> >(new std::vector<target>());
	std::back_insert_iterator< std::vector<target> > tg(*targets);
	int l = lua_rawlen(L, n);

	for (int i = 1; i <= l; ++i)
	{
		lua_rawgeti(L, n, i); // st n + 1  TABLE @ N    table @ n + 1

		lua_pushstring(L, "loc"); // st n + 2
		lua_rawget(L, -2); // st n + 2

		lua_pushstring(L, "x"); // st n + 3
		lua_rawget(L, -2); // st n + 3
		int x = lua_tointeger(L, -1); // st n + 3
		lua_pop(L, 1); // st n + 2

		lua_pushstring(L, "y"); // st n + 3
		lua_rawget(L, -2); // st n + 3
		int y = lua_tointeger(L, -1); // st n + 3

		lua_pop(L, 2); // st n + 1

		lua_pushstring(L, "type"); // st n + 2
		lua_rawget(L, -2);  // st n + 2
		target::TYPE type = static_cast<target::TYPE>(lua_tointeger(L, -1));  // st n + 2
		lua_pop(L, 1); // st n + 1


		lua_pushstring(L, "value");
		lua_rawget(L, -2);
		int value = lua_tointeger(L, -1);

		map_location ml(x - 1, y - 1);

		*tg = target(ml, value, type);
	}

	lua_settop(L, n);
	return targets;
}

template <>
inline boost::shared_ptr<unit_advancements_aspect> lua_object<unit_advancements_aspect>::to_type(lua_State *L, int n)
{
	boost::shared_ptr<unit_advancements_aspect> uaa = boost::shared_ptr<unit_advancements_aspect>(new unit_advancements_aspect(L, n));
	return uaa;
}
} // end of namespace ai


#endif
