/*
	Copyright (C) 2011 - 2024
	by Dmitry Kovalenko <nephro.wes@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#pragma once

#include "config.hpp"
#include "log.hpp"
#include "lua/lua.h"
#include "map/location.hpp"
#include "resources.hpp"
#include "scripting/lua_common.hpp"
#include "terrain/filter.hpp"
#include "variable.hpp"
#include "ai/default/contexts.hpp"
#include "ai/lua/aspect_advancements.hpp"

#include <iterator>
#include <string>
#include <vector>

static lg::log_domain log_scripting_lua("scripting/lua");
#define ERR_OBJ_LUA LOG_STREAM(err, log_scripting_lua)

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

	lua_object(const T& init)
		: value_(std::make_shared<T>(init))
	{
		// empty
	}

	std::shared_ptr<T> get()
	{
		return value_;
	}

	void store(lua_State* L, int n)
	{
		this->value_ = to_type(L, lua_absindex(L, n));
	}

	void push(lua_State* L)
	{
		from_type(L, this->value_);
	}

protected:

	// A group of functions that deal with the translation of the results to C++
	std::shared_ptr<T> to_type(lua_State *, int)
	{
		return std::shared_ptr<T>();
	}

	// A group of functions that deal with the translations of values back to Lua
	void from_type(lua_State* L, std::shared_ptr<T>)
	{
		lua_pushliteral(L, "Unsupported AI aspect type for Lua!");
		lua_error(L);
	}

	std::shared_ptr<T> value_;
};

template <>
inline std::shared_ptr<double> lua_object<double>::to_type(lua_State *L, int n)
{
	return std::make_shared<double>(lua_tonumber(L, n));
}

template <>
inline void lua_object<double>::from_type(lua_State *L, std::shared_ptr<double> value)
{
	if(value) lua_pushnumber(L, *value);
	else lua_pushnil(L);
}

template <>
inline std::shared_ptr<std::string> lua_object<std::string>::to_type(lua_State *L, int n)
{
	return std::make_shared<std::string>(lua_tostring(L, n));
}

template <>
inline void lua_object<utils::variant<bool, std::vector<std::string>>>::from_type(lua_State *L, std::shared_ptr<utils::variant<bool, std::vector<std::string>>> value)
{
	if(value) {
		// TODO: this is is duplicated as a helper function in ai/lua/core.cpp
		utils::visit(
			[L](const auto& v) {
				if constexpr(utils::decayed_is_same<bool, decltype(v)>) {
					lua_pushboolean(L, v);
				} else {
					lua_createtable(L, v.size(), 0);
					for(const std::string& str : v) {
						lua_pushlstring(L, str.c_str(), str.size());
						lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
					}
				}
			},
			*value);
	} else lua_pushnil(L);
}

template <>
inline std::shared_ptr< utils::variant<bool, std::vector<std::string>> > lua_object< utils::variant<bool, std::vector<std::string>> >::to_type(lua_State *L, int n)
{
	if (lua_isboolean(L, n)) {
		return std::make_shared<utils::variant<bool, std::vector<std::string>>>(luaW_toboolean(L, n));
	} else {
		auto v = std::make_shared<std::vector<std::string>>();
		int l = lua_rawlen(L, n);
		for (int i = 1; i < l + 1; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, n);
			std::string s = lua_tostring(L, -1);
			lua_settop(L, n);
			v->push_back(s);
		}

		return std::make_shared<utils::variant<bool, std::vector<std::string>>>(*v);
	}
}

template <>
inline void lua_object<std::string>::from_type(lua_State *L, std::shared_ptr<std::string> value)
{
	if(value) lua_pushlstring(L, value->c_str(), value->size());
	else lua_pushnil(L);
}

template <>
inline std::shared_ptr<bool> lua_object<bool>::to_type(lua_State *L, int n)
{
	return std::make_shared<bool>(luaW_toboolean(L, n));
}

template <>
inline void lua_object<bool>::from_type(lua_State *L, std::shared_ptr<bool> value)
{
	if(value) lua_pushboolean(L, *value);
	else lua_pushnil(L);
}

template <>
inline std::shared_ptr<int> lua_object<int>::to_type(lua_State *L, int n)
{
	return std::make_shared<int>(static_cast<int>(lua_tointeger(L, n)));
}

template <>
inline void lua_object<int>::from_type(lua_State *L, std::shared_ptr<int> value)
{
	if(value) lua_pushnumber(L, *value);
	else lua_pushnil(L);
}

template <>
inline std::shared_ptr< std::vector<std::string> > lua_object< std::vector<std::string> >::to_type(lua_State *L, int n)
{
	auto v = std::make_shared<std::vector<std::string>>();
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
inline void lua_object< std::vector<std::string> >::from_type(lua_State *L, std::shared_ptr< std::vector<std::string> > value)
{
	if(value) {
		lua_createtable(L, value->size(), 0);
		for(const std::string& str : *value) {
			lua_pushlstring(L, str.c_str(), str.size());
			lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
		}
	} else lua_pushnil(L);
}

template <>
inline std::shared_ptr<config> lua_object<config>::to_type(lua_State *L, int n)
{
	auto cfg = std::make_shared<config>();
	luaW_toconfig(L, n, *cfg);
	return cfg;
}

template <>
inline void lua_object<config>::from_type(lua_State *L, std::shared_ptr<config> value)
{
	if(value) luaW_pushconfig(L, *value);
	else lua_pushnil(L);
}

template <>
inline std::shared_ptr<terrain_filter> lua_object<terrain_filter>::to_type(lua_State *L, int n)
{
	auto cfg = std::make_shared<config>();
	auto vcfg = std::make_shared<vconfig>(*cfg);
	if (!luaW_tovconfig(L, n, *vcfg)) {
		cfg->add_child("not");
	}
	vcfg->make_safe();
	return std::make_shared<terrain_filter>(*vcfg, resources::filter_con, false);
}

template <>
inline void lua_object<terrain_filter>::from_type(lua_State *L, std::shared_ptr<terrain_filter> value)
{
	if(value) {
		std::set<map_location> locs;
		value->get_locations(locs);
		lua_createtable(L, locs.size(), 0);
		for(const map_location& loc : locs) {
			luaW_pushlocation(L, loc);
			lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
		}
	} else lua_pushnil(L);
}

template <>
inline std::shared_ptr<std::vector<target> > lua_object< std::vector<target> >::to_type(lua_State *L, int n)
{
	auto targets = std::make_shared<std::vector<target>>();
	int l = lua_rawlen(L, n);

	for (int i = 1; i <= l; ++i)
	{
		lua_rawgeti(L, n, i); // st n + 1  TABLE @ N    table @ n + 1

		lua_pushstring(L, "loc"); // st n + 2
		lua_rawget(L, -2); // st n + 2

		lua_pushstring(L, "x"); // st n + 3
		lua_rawget(L, -2); // st n + 3
		int x = static_cast<int>(lua_tointeger(L, -1)); // st n + 3
		lua_pop(L, 1); // st n + 2

		lua_pushstring(L, "y"); // st n + 3
		lua_rawget(L, -2); // st n + 3
		int y = static_cast<int>(lua_tointeger(L, -1)); // st n + 3

		lua_pop(L, 2); // st n + 1

		lua_pushstring(L, "type"); // st n + 2
		lua_rawget(L, -2);  // st n + 2
		std::optional<ai_target::type> type = ai_target::type::xplicit;
		if(lua_isnumber(L, -1)) {
			int target = static_cast<int>(lua_tointeger(L, -1));
			type = ai_target::get_enum(target);  // st n + 2
			if(!type) {
				ERR_OBJ_LUA << "Failed to convert ai target type of " << target << ", skipping.";
				continue;
			}
		} else if(lua_isstring(L, -1)) {
			std::string target = lua_tostring(L, -1);
			type = ai_target::get_enum(target);  // st n + 2
			if(!type) {
				ERR_OBJ_LUA << "Failed to convert ai target type of " << target << ", skipping.";
				continue;
			}
		}
		lua_pop(L, 1); // st n + 1

		lua_pushstring(L, "value");
		lua_rawget(L, -2);
		int value = static_cast<int>(lua_tointeger(L, -1));

		map_location ml(x, y, wml_loc());

		targets->emplace_back(ml, value, *type);
	}

	lua_settop(L, n);
	return targets;
}

template <>
inline std::shared_ptr<unit_advancements_aspect> lua_object<unit_advancements_aspect>::to_type(lua_State *L, int n)
{
	return std::make_shared<unit_advancements_aspect>(L, n);
}

// This one is too complex to define in the header.
struct aspect_attacks_lua_filter;
template <>
std::shared_ptr<aspect_attacks_lua_filter> lua_object<aspect_attacks_lua_filter>::to_type(lua_State *L, int n);
} // end of namespace ai
