/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef LUA_COMMON_HPP_INCLUDED
#define LUA_COMMON_HPP_INCLUDED

struct lua_State;

namespace lua_common {
	int impl_gettext(lua_State *L);
	int intf_textdomain(lua_State *L);
	int impl_tstring_concat(lua_State *L);
	int impl_tstring_collect(lua_State *L);
	int impl_tstring_tostring(lua_State *L);
	int impl_vconfig_get(lua_State *L);
	int impl_vconfig_size(lua_State *L);
	int impl_vconfig_collect(lua_State *L);
}

#define return_tstring_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		luaW_pushtstring(L, accessor); \
		return 1; \
	}

#define return_cstring_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushstring(L, accessor); \
		return 1; \
	}

#define return_string_attrib(name, accessor) \
	return_cstring_attrib(name, accessor.c_str())

#define return_int_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushinteger(L, accessor); \
		return 1; \
	}

#define return_float_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushnumber(L, accessor); \
		return 1; \
	}

#define return_bool_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushboolean(L, accessor); \
		return 1; \
	}

#define return_cfg_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		config cfg; \
		accessor; \
		luaW_pushconfig(L, cfg); \
		return 1; \
	}

#define return_cfgref_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		luaW_pushconfig(L, accessor); \
		return 1; \
	}

#define return_vector_string_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		const std::vector<std::string>& vector = accessor; \
		lua_createtable(L, vector.size(), 0); \
		int i = 1; \
		BOOST_FOREACH(const std::string& s, vector) { \
			lua_pushstring(L, s.c_str()); \
			lua_rawseti(L, -2, i); \
			++i; \
		} \
		return 1; \
	}

#define modify_tstring_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		t_string value = luaW_checktstring(L, 3); \
		accessor; \
		return 0; \
	}

#define modify_string_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		const char *value = luaL_checkstring(L, 3); \
		accessor; \
		return 0; \
	}

#define modify_int_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		int value = luaL_checkinteger(L, 3); \
		accessor; \
		return 0; \
	}

#define modify_int_attrib_check_range(name, accessor, allowed_min, allowed_max) \
	if (strcmp(m, name) == 0) { \
		int value = luaL_checkinteger(L, 3); \
		if (value < allowed_min || allowed_max < value) return luaL_argerror(L, 3, "out of bounds"); \
		accessor; \
		return 0; \
	}

#define modify_bool_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		bool value = luaW_toboolean(L, 3); \
		accessor; \
		return 0; \
	}

#define modify_vector_string_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		std::vector<std::string> vector; \
		char const* message = "table with unnamed indices holding strings expected"; \
		if (!lua_istable(L, 3)) return luaL_argerror(L, 3, message); \
		unsigned length = lua_rawlen(L, 3); \
		for (unsigned i = 1; i <= length; ++i) { \
			lua_rawgeti(L, 3, i); \
			char const* string = lua_tostring(L, 4); \
			if(!string) return luaL_argerror(L, 2 + i, message); \
			vector.push_back(string); \
			lua_pop(L, 1); \
		} \
		accessor; \
		return 0; \
	}

#endif
