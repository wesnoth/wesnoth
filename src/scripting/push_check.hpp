/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "scripting/lua_common.hpp"
#include "scripting/lua_widget.hpp"

#include "lua/wrapper_lauxlib.h"
#include "tstring.hpp"
#include "map/location.hpp"
#include "variable.hpp"

#include <cassert>
#include <string_view>
#include <type_traits>

struct lua_index_raw {
	int index;
	lua_index_raw(int i) : index(i) {}
	lua_index_raw(lua_State* L) : index(lua_gettop(L)) {}
};

namespace lua_check_impl
{
	template<typename T, typename T2 = void>
	struct is_container : std::false_type {};

	template<typename T>
	struct is_container<T, std::void_t<
		typename std::decay_t<T>::value_type,
		typename std::decay_t<T>::iterator,
		typename std::decay_t<T>::size_type,
		typename std::decay_t<T>::reference>
	> : std::true_type {};

	template<class T, template<class> class U>
	inline constexpr bool is_instance_of_v = std::false_type{};

	template<template<class> class U, class V>
	inline constexpr bool is_instance_of_v<U<V>,U> = std::true_type{};

	template<typename T, typename T2 = void>
	struct is_map : std::false_type {};

	template<typename T>
	struct is_map<T, std::void_t<
		typename std::decay_t<T>::key_type,
		typename std::decay_t<T>::mapped_type>
	> : std::true_type {};

	template<typename T, typename T2 = void>
	struct is_pair : std::false_type {};

	template<typename T>
	struct is_pair<T, std::void_t<
		typename std::decay_t<T>::first_type,
		typename std::decay_t<T>::second_type>
	> : std::true_type {};

	template<typename T>
	std::enable_if_t<std::is_same_v<T, lua_index_raw>, lua_index_raw>
	lua_check(lua_State * L, int n)
	{
		return lua_index_raw{ lua_absindex(L, n) };
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, lua_index_raw>, lua_index_raw>
	lua_to_or_default(lua_State * L, int n, const T& /*def*/)
	{
		return lua_index_raw{ lua_absindex(L, n) };
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, lua_index_raw>, void>
	lua_push(lua_State * L, lua_index_raw n)
	{
		lua_pushvalue(L, n.index);
	}

	//std::string
	template<typename T>
	std::enable_if_t<std::is_same_v<T, std::string>, std::string>
	lua_check(lua_State *L, int n)
	{
		return std::string(luaW_tostring(L, n));
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, std::string>, std::string>
	lua_to_or_default(lua_State *L, int n, const T& def)
	{
		return std::string(luaW_tostring_or_default(L, n, def));
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, std::string>, void>
	lua_push(lua_State *L, const T& val)
	{
		lua_pushlstring(L, val.c_str(), val.size());
	}

	//std::string_view
	template<typename T>
	std::enable_if_t<std::is_same_v<T, std::string_view>, std::string_view>
	lua_check(lua_State *L, int n)
	{
		return luaW_tostring(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, std::string_view>, std::string_view>
	lua_to_or_default(lua_State *L, int n, const T& def)
	{
		return luaW_tostring_or_default(L, n, def);
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, std::string_view>, void>
	lua_push(lua_State *L, const T& val)
	{
		lua_pushlstring(L, val.data(), val.size());
	}

	//config
	template<typename T>
	std::enable_if_t<std::is_same_v<T, config>, config>
	lua_check(lua_State *L, int n)
	{
		return luaW_checkconfig(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, config>, config>
	lua_to_or_default(lua_State *L, int n, const T& def)
	{
		config cfg;
		return luaW_toconfig(L, n, cfg) ? cfg : def;
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, config>, void>
	lua_push(lua_State *L, const config& val)
	{
		luaW_pushconfig(L, val);
	}

	//vconfig
	template<typename T>
	std::enable_if_t<std::is_same_v<T, vconfig>, vconfig>
	lua_check(lua_State *L, int n)
	{
		return luaW_checkvconfig(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, vconfig>, vconfig>
	lua_to_or_default(lua_State *L, int n, const T& def)
	{
		vconfig cfg = vconfig::unconstructed_vconfig();
		return luaW_tovconfig(L, n, cfg) ? cfg : def;
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, vconfig>, void>
	lua_push(lua_State *L, const vconfig& val)
	{
		luaW_pushvconfig(L, val);
	}

	//location
	template<typename T>
	std::enable_if_t<std::is_same_v<T, map_location>, map_location>
	lua_check(lua_State *L, int n)
	{
		return luaW_checklocation(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, map_location>, map_location>
	lua_to_or_default(lua_State *L, int n, const T& def)
	{
		map_location res;
		if (!luaW_tolocation(L, n, res)) {
			return def;
		}
		return res;
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, map_location>, void>
	lua_push(lua_State *L, const map_location& val)
	{
		luaW_pushlocation(L, val);
	}

	//t_string
	template<typename T>
	std::enable_if_t<std::is_same_v<T, t_string>, t_string>
	lua_check(lua_State *L, int n)
	{
		return luaW_checktstring(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, t_string>, void>
	lua_push(lua_State *L, const t_string& val)
	{
		luaW_pushtstring(L, val);
	}

	//widget
	//lua_check for widget is not supported because lua_check returns by value
	template<typename T>
	std::enable_if_t<std::is_same_v<T, gui2::widget>, void>
	lua_push(lua_State *L, gui2::widget& val)
	{
		luaW_pushwidget(L, val);
	}

	//bool
	template<typename T>
	std::enable_if_t<std::is_same_v<T, bool>, bool>
	lua_check(lua_State *L, int n)
	{
		return luaW_toboolean(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, bool>, bool>
	lua_to_or_default(lua_State *L, int n, const T& /*def*/)
	{
		return luaW_toboolean(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_same_v<T, bool>, void>
	lua_push(lua_State *L, bool val)
	{
		lua_pushboolean(L, val);
	}

	//double, float
	template<typename T>
	std::enable_if_t<std::is_floating_point_v<T>, T>
	lua_check(lua_State *L, int n)
	{
		return luaL_checknumber(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_floating_point_v<T>, T>
	lua_to_or_default(lua_State *L, int n, const T& def)
	{
		int isnum;
		lua_Number d = lua_tonumberx(L, n, &isnum);
		if (!isnum) {
			return def;
		}
		return d;
	}
	template<typename T>
	std::enable_if_t<std::is_floating_point_v<T>, void>
	lua_push(lua_State *L, T val)
	{
		lua_pushnumber(L, val);
	}

	//integer types
	template<typename T>
	std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, T>
	lua_check(lua_State *L, int n)
	{
		return luaL_checkinteger(L, n);
	}
	template<typename T>
	std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, T>
	lua_to_or_default(lua_State *L, int n, const T& def)
	{
		int isnum;
		lua_Integer res = lua_tointegerx(L, n, &isnum);
		if (!isnum) {
			return def;
		}
		return res;
	}

	template<typename T>
	std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, void>
	lua_push(lua_State *L, T val)
	{
		lua_pushinteger(L, val);
	}

	//std::pair
	//Not sure if the not_<is_const> is required; only (maybe) if std::map matches is_container
	template<typename T>
	std::enable_if_t<is_pair<T>::value && !std::is_const_v<typename T::first_type>, T>
	lua_check(lua_State *L, int n)
	{
		T result;
		if (lua_istable(L, n)) {
			lua_rawgeti(L, n, 1);
			result.first = lua_check<const typename T::first_type&>(L, -1);
			lua_rawgeti(L, n, 2);
			result.second = lua_check<const typename T::second_type&>(L, -1);
			lua_pop(L, 2);
		}
		return result;
	}
	template<typename T>
	std::enable_if_t<is_pair<T>::value && !std::is_const_v<typename T::first_type>, void>
	lua_push(lua_State *L, const T& val)
	{
		lua_newtable(L);
		lua_push<const typename T::first_type&>(L, val.first);
		lua_rawseti(L, -2, 1);
		lua_push<const typename T::second_type&>(L, val.second);
		lua_rawseti(L, -2, 2);
	}

	//std::vector and similar but not std::string
	template<typename T>
	std::enable_if_t<is_container<T>::value && !std::is_same_v<T, std::string> && !std::is_same_v<T, std::string_view>, T>
	lua_check(lua_State * L, int n)
	{
		if (lua_istable(L, n))
		{
			T res;
			for (int i = 1, i_end = lua_rawlen(L, n); i <= i_end; ++i)
			{
				lua_rawgeti(L, n, i);
				// By using insert instead of push_back, it magically "just works" for sets too.
				res.insert(res.end(), lua_check_impl::lua_check<std::decay_t<typename T::reference>>(L, -1));
				lua_pop(L, 1);
			}
			return res;
		}
		else
		{
			luaL_argerror(L, n, "Table expected");
			throw "luaL_argerror returned"; //shouldn't happen, luaL_argerror always throws.
		}
	}

	//also accepts things like std::vector<int>() | std::adaptors::transformed(..)
	template<typename T>
	std::enable_if_t<
		is_container<T>::value && !std::is_same_v<T, std::string> && !std::is_same_v<T, std::string_view> && !is_map<T>::value
		, void
	>
	lua_push(lua_State * L, const T& list )
	{
		// NOTE: T might be some boost::iterator_range type where size might be < 0. (unfortunately in this case size() does not return T::size_type)
		assert(list.size() >= 0);
		lua_createtable(L, list.size(), 0);
		int i = 1;
		for(typename T::const_iterator iter = list.begin(); iter != list.end(); ++iter) {
			lua_check_impl::lua_push<std::decay_t<typename T::reference>>(L, *iter);
			lua_rawseti(L, -2, i++);
		}
	}

	//accepts std::map TODO: add a check function for that
	template<typename T>
	std::enable_if_t<is_map<T>::value, void>
	lua_push(lua_State * L, const T& map )
	{
		lua_newtable(L);
		for(const typename T::value_type& pair : map)
		{
			lua_check_impl::lua_push<std::decay_t<typename T::key_type>>(L, pair.first);
			lua_check_impl::lua_push<std::decay_t<typename T::mapped_type>>(L, pair.second);
			lua_settable(L, -3);
		}
	}

	// enum_base
	template<typename T>
	typename T::type
	lua_check(lua_State *L, int n)
	{
		std::string str = lua_check_impl::lua_check<std::string>(L, n);
		utils::optional<typename T::type> val = T::get_enum(str);
		if(!val) {
			luaL_argerror(L, n, ("cannot convert " + str + " to enum.").c_str());
		}
		return *val;
	}

	//optional
	template<typename T>
	std::enable_if_t<is_instance_of_v<T, utils::optional>, T>
	lua_check(lua_State *L, int n)
	{
		if(lua_isnoneornil(L, n)) {
			return T();
		}
		return lua_check_impl::lua_check<typename T::value_type>(L, n);
	}

	template<typename T>
	std::enable_if_t<is_instance_of_v<T, utils::optional>, void>
	lua_push(lua_State *L, const T& opt)
	{
		if(opt) {
			lua_check_impl::lua_push<typename T::value_type>(L, *opt);
		} else {
			lua_pushnil(L);
		}
	}
}

template<typename T>
std::decay_t<T> lua_check(lua_State *L, int n)
{
	//remove possible const& to make life easier for the impl namespace.
	return lua_check_impl::lua_check<std::decay_t<T>>(L, n);
}

template<typename T>
std::decay_t<T> lua_to_or_default(lua_State *L, int n, const T& def)
{
	//remove possible const& to make life easier for the impl namespace.
	return lua_check_impl::lua_to_or_default<std::decay_t<T>>(L, n, def);
}

template<typename T>
void lua_push(lua_State *L, const T& val)
{
	return lua_check_impl::lua_push<std::decay_t<T>>(L, val);
}

/**
 * returns t[k] where k is the table at index @a index and k is @a k or @a def if it is not convertible to the correct type.
 *
 */
template<typename T>
std::decay_t<T> luaW_table_get_def(lua_State *L, int index, std::string_view k,  const T& def)
{
	if(!lua_istable(L, index)) {
		luaL_argerror(L, index, "table expected");
	}
	if(index < 0) {
		//with the next lua_pushstring negative indicies will no longer be correct otherwise.
		--index;
	}
	lua_pushlstring(L, k.data(), k.size());
	lua_gettable(L, index);
	if(lua_isnoneornil(L, -1)) {
		lua_pop(L, 1);
		return def;
	}
	T res =  lua_check_impl::lua_to_or_default<std::decay_t<T>>(L, -1, def);
	lua_pop(L, 1);
	return res;
}


template<typename T>
void luaW_table_set(lua_State *L, int index, std::string_view k,  const T& value)
{
	if(!lua_istable(L, index)) {
		luaL_argerror(L, index, "table expected");
	}

	index = lua_absindex(L, index);
	lua_pushlstring(L, k.data(), k.size());
	lua_push(L, value);
	lua_settable(L, index);
}
