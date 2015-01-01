/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
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
 * Contains code common to the application and game lua kernels which
 * cannot or should not go into the lua kernel base files.
 *
 * Currently contains implementation functions related to vconfig and
 * gettext, also some macros to assist in writing C lua callbacks.
 */

#include "scripting/lua_common.hpp"

#include "global.hpp"

#include "config.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "scripting/lua_api.hpp"
#include "scripting/lua_types.hpp"      // for gettextKey, tstringKey, etc
#include "tstring.hpp"                  // for t_string
#include "variable.hpp" // for vconfig

#include <boost/foreach.hpp>
#include <cstring>
#include <iterator>                     // for distance, advance
#include <new>                          // for operator new
#include <string>                       // for string, basic_string

static const char * gettextKey = "gettext";
static const char * vconfigKey = "vconfig";
const char * tstringKey = "translatable string";

namespace lua_common {

/**
 * Creates a t_string object (__call metamethod).
 * - Arg 1: userdata containing the domain.
 * - Arg 2: string to translate.
 * - Ret 1: string containing the translatable string.
 */
static int impl_gettext(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);
	char const *d = static_cast<char *>(lua_touserdata(L, 1));
	// Hidden metamethod, so d has to be a string. Use it to create a t_string.
	luaW_pushtstring(L, t_string(m, d));
	return 1;
}

/**
 * Creates an interface for gettext
 * - Arg 1: string containing the domain.
 * - Ret 1: a full userdata with __call pointing to lua_gettext.
 */
int intf_textdomain(lua_State *L)
{
	size_t l;
	char const *m = luaL_checklstring(L, 1, &l);

	void *p = lua_newuserdata(L, l + 1);
	memcpy(p, m, l + 1);

	luaL_setmetatable(L, gettextKey);
	return 1;
}

/**
 * Converts a Lua value at position @a src and appends it to @a dst.
 * @note This function is private to lua_tstring_concat. It expects two things.
 *       First, the t_string metatable is at the top of the stack on entry. (It
 *       is still there on exit.) Second, the caller hasn't any valuable object
 *       with dynamic lifetime, since they would be leaked on error.
 */
static void tstring_concat_aux(lua_State *L, t_string &dst, int src)
{
	switch (lua_type(L, src)) {
		case LUA_TNUMBER:
		case LUA_TSTRING:
			dst += lua_tostring(L, src);
			return;
		case LUA_TUSERDATA:
			// Compare its metatable with t_string's metatable.
			if (t_string * src_ptr = static_cast<t_string *> (luaL_testudata(L, src, tstringKey))) {
				dst += *src_ptr;
				return;
			}
			//intentional fall-through
		default:
			luaL_typerror(L, src, "string");
	}
}

/**
 * Appends a scalar to a t_string object (__concat metamethod).
 */
static int impl_tstring_concat(lua_State *L)
{
	// Create a new t_string.
	t_string *t = new(lua_newuserdata(L, sizeof(t_string))) t_string;
	luaL_setmetatable(L, tstringKey);

	// Append both arguments to t.
	tstring_concat_aux(L, *t, 1);
	tstring_concat_aux(L, *t, 2);

	return 1;
}

/**
 * Destroys a t_string object before it is collected (__gc metamethod).
 */
static int impl_tstring_collect(lua_State *L)
{
	t_string *t = static_cast<t_string *>(lua_touserdata(L, 1));
	t->t_string::~t_string();
	return 0;
}

/**
 * Converts a t_string object to a string (__tostring metamethod);
 * that is, performs a translation.
 */
static int impl_tstring_tostring(lua_State *L)
{
	t_string *t = static_cast<t_string *>(lua_touserdata(L, 1));
	lua_pushstring(L, t->c_str());
	return 1;
}

/**
 * Gets the parsed field of a vconfig object (_index metamethod).
 * Special fields __literal, __shallow_literal, __parsed, and
 * __shallow_parsed, return Lua tables.
 */
static int impl_vconfig_get(lua_State *L)
{
	vconfig *v = static_cast<vconfig *>(lua_touserdata(L, 1));

	if (lua_isnumber(L, 2))
	{
		vconfig::all_children_iterator i = v->ordered_begin();
		unsigned len = std::distance(i, v->ordered_end());
		unsigned pos = lua_tointeger(L, 2) - 1;
		if (pos >= len) return 0;
		std::advance(i, pos);

		lua_createtable(L, 2, 0);
		lua_pushstring(L, i.get_key().c_str());
		lua_rawseti(L, -2, 1);
		luaW_pushvconfig(L, vconfig(i.get_child()));
		lua_rawseti(L, -2, 2);
		return 1;
	}

	char const *m = luaL_checkstring(L, 2);
	if (strcmp(m, "__literal") == 0) {
		luaW_pushconfig(L, v->get_config());
		return 1;
	}
	if (strcmp(m, "__parsed") == 0) {
		luaW_pushconfig(L, v->get_parsed_config());
		return 1;
	}

	bool shallow_literal = strcmp(m, "__shallow_literal") == 0;
	if (shallow_literal || strcmp(m, "__shallow_parsed") == 0)
	{
		lua_newtable(L);
		BOOST_FOREACH(const config::attribute &a, v->get_config().attribute_range()) {
			if (shallow_literal)
				luaW_pushscalar(L, a.second);
			else
				luaW_pushscalar(L, v->expand(a.first));
			lua_setfield(L, -2, a.first.c_str());
		}
		vconfig::all_children_iterator i = v->ordered_begin(),
			i_end = v->ordered_end();
		if (shallow_literal) {
			i.disable_insertion();
			i_end.disable_insertion();
		}
		for (int j = 1; i != i_end; ++i, ++j)
		{
			lua_createtable(L, 2, 0);
			lua_pushstring(L, i.get_key().c_str());
			lua_rawseti(L, -2, 1);
			luaW_pushvconfig(L, i.get_child());
			lua_rawseti(L, -2, 2);
			lua_rawseti(L, -2, j);
		}
		return 1;
	}

	if (v->null() || !v->has_attribute(m)) return 0;
	luaW_pushscalar(L, (*v)[m]);
	return 1;
}

/**
 * Returns the number of a child of a vconfig object.
 */
static int impl_vconfig_size(lua_State *L)
{
	vconfig *v = static_cast<vconfig *>(lua_touserdata(L, 1));
	lua_pushinteger(L, v->null() ? 0 :
		std::distance(v->ordered_begin(), v->ordered_end()));
	return 1;
}

/**
 * Destroys a vconfig object before it is collected (__gc metamethod).
 */
static int impl_vconfig_collect(lua_State *L)
{
	vconfig *v = static_cast<vconfig *>(lua_touserdata(L, 1));
	v->vconfig::~vconfig();
	return 0;
}

/**
 * Creates a vconfig containing the WML table.
 * - Arg 1: WML table.
 * - Ret 1: vconfig userdata.
 */
int intf_tovconfig(lua_State *L)
{
	vconfig vcfg = luaW_checkvconfig(L, 1);
	luaW_pushvconfig(L, vcfg);
	return 1;
}

/**
 * Adds the gettext metatable
 */
std::string register_gettext_metatable(lua_State *L)
{
	luaL_newmetatable(L, gettextKey);

	static luaL_Reg const callbacks[] = {
		{ "__call", 	    &impl_gettext},
		{ NULL, NULL }
	};
	luaL_setfuncs(L, callbacks, 0);

	lua_pushstring(L, "message domain");
	lua_setfield(L, -2, "__metatable");

	return "Adding gettext metatable...\n";
}

/**
 * Adds the tstring metatable
 */
std::string register_tstring_metatable(lua_State *L)
{
	luaL_newmetatable(L, tstringKey);

	static luaL_Reg const callbacks[] = {
		{ "__concat", 	    &impl_tstring_concat},
		{ "__gc",           &impl_tstring_collect},
		{ "__tostring",	    &impl_tstring_tostring},
		{ NULL, NULL }
	};
	luaL_setfuncs(L, callbacks, 0);

	lua_pushstring(L, "translatable string");
	lua_setfield(L, -2, "__metatable");

	return "Adding tstring metatable...\n";
}

/**
 * Adds the vconfig metatable
 */
std::string register_vconfig_metatable(lua_State *L)
{
	luaL_newmetatable(L, vconfigKey);

	static luaL_Reg const callbacks[] = {
		{ "__gc",           &impl_vconfig_collect},
		{ "__index", 	    &impl_vconfig_get},
		{ "__len",	    &impl_vconfig_size},
		{ NULL, NULL }
	};
	luaL_setfuncs(L, callbacks, 0);

	lua_pushstring(L, "wml object");
	lua_setfield(L, -2, "__metatable");

	return "Adding vconfig metatable...\n";
}

} // end namespace lua_common

void luaW_pushvconfig(lua_State *L, vconfig const &cfg)
{
	new(lua_newuserdata(L, sizeof(vconfig))) vconfig(cfg);
	luaL_setmetatable(L, vconfigKey);
}

void luaW_pushtstring(lua_State *L, t_string const &v)
{
	new(lua_newuserdata(L, sizeof(t_string))) t_string(v);
	luaL_setmetatable(L, tstringKey);
}


namespace {
	struct luaW_pushscalar_visitor : boost::static_visitor<>
	{
		lua_State *L;
		luaW_pushscalar_visitor(lua_State *l): L(l) {}

		void operator()(boost::blank const &) const
		{ lua_pushnil(L); }
		void operator()(bool b) const
		{ lua_pushboolean(L, b); }
		void operator()(int i) const
		{ lua_pushinteger(L, i); }
		void operator()(unsigned long long ull) const
		{ lua_pushnumber(L, ull); }
		void operator()(double d) const
		{ lua_pushnumber(L, d); }
		void operator()(std::string const &s) const
		{ lua_pushstring(L, s.c_str()); }
		void operator()(t_string const &s) const
		{ luaW_pushtstring(L, s); }
	};
}//unnamed namespace for luaW_pushscalar_visitor

void luaW_pushscalar(lua_State *L, config::attribute_value const &v)
{
	v.apply_visitor(luaW_pushscalar_visitor(L));
}

bool luaW_hasmetatable(lua_State *L
		, int index
		, luatypekey key)
{
	if (!lua_getmetatable(L, index))
		return false;
	lua_pushlightuserdata(L, key);
	lua_rawget(L, LUA_REGISTRYINDEX);
	bool ok = lua_rawequal(L, -1, -2) == 1;
	lua_pop(L, 2);
	return ok;
}

bool luaW_totstring(lua_State *L, int index, t_string &str)
{
	switch (lua_type(L, index)) {
		case LUA_TBOOLEAN:
			str = lua_toboolean(L, index) ? "yes" : "no";
			break;
		case LUA_TNUMBER:
		case LUA_TSTRING:
			str = lua_tostring(L, index);
			break;
		case LUA_TUSERDATA:
		{
			if (t_string * tstr = static_cast<t_string *> (luaL_testudata(L, index, tstringKey))) {
				str = *tstr;
				break;
			} else {
				return false;
			}
		}
		default:
			return false;
	}
	return true;
}

t_string luaW_checktstring(lua_State *L, int index)
{
	t_string result;
	if (!luaW_totstring(L, index, result))
		luaL_typerror(L, index, "translatable string");
	return result;
}

void luaW_filltable(lua_State *L, config const &cfg)
{
	if (!lua_checkstack(L, LUA_MINSTACK))
		return;

	int k = 1;
	BOOST_FOREACH(const config::any_child &ch, cfg.all_children_range())
	{
		lua_createtable(L, 2, 0);
		lua_pushstring(L, ch.key.c_str());
		lua_rawseti(L, -2, 1);
		lua_newtable(L);
		luaW_filltable(L, ch.cfg);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, k++);
	}
	BOOST_FOREACH(const config::attribute &attr, cfg.attribute_range())
	{
		luaW_pushscalar(L, attr.second);
		lua_setfield(L, -2, attr.first.c_str());
	}
}

void luaW_pushconfig(lua_State *L, config const &cfg)
{
	lua_newtable(L);
	luaW_filltable(L, cfg);
}




#define return_misformed() \
  do { lua_settop(L, initial_top); return false; } while (0)

bool luaW_toconfig(lua_State *L, int index, config &cfg)
{
	if (!lua_checkstack(L, LUA_MINSTACK))
		return false;

	// Get the absolute index of the table.
	int initial_top = lua_gettop(L);
	if (-initial_top <= index && index <= -1)
		index = initial_top + index + 1;

	switch (lua_type(L, index))
	{
		case LUA_TTABLE:
			break;
		case LUA_TUSERDATA:
		{
			if (vconfig * ptr = static_cast<vconfig *> (luaL_testudata(L, index, vconfigKey))) {
				cfg = ptr->get_parsed_config();
				return true;
			} else {
				return false;
			}
		}
		case LUA_TNONE:
		case LUA_TNIL:
			return true;
		default:
			return false;
	}

	// First convert the children (integer indices).
	for (int i = 1, i_end = lua_rawlen(L, index); i <= i_end; ++i)
	{
		lua_rawgeti(L, index, i);
		if (!lua_istable(L, -1)) return_misformed();
		lua_rawgeti(L, -1, 1);
		char const *m = lua_tostring(L, -1);
		if (!m) return_misformed();
		lua_rawgeti(L, -2, 2);
		if (!luaW_toconfig(L, -1, cfg.add_child(m)))
			return_misformed();
		lua_pop(L, 3);
	}

	// Then convert the attributes (string indices).
	for (lua_pushnil(L); lua_next(L, index); lua_pop(L, 1))
	{
		if (lua_isnumber(L, -2)) continue;
		if (!lua_isstring(L, -2)) return_misformed();
		config::attribute_value &v = cfg[lua_tostring(L, -2)];
		switch (lua_type(L, -1)) {
			case LUA_TBOOLEAN:
				v = luaW_toboolean(L, -1);
				break;
			case LUA_TNUMBER:
				v = lua_tonumber(L, -1);
				break;
			case LUA_TSTRING:
				v = lua_tostring(L, -1);
				break;
			case LUA_TUSERDATA:
			{
				if (t_string * tptr = static_cast<t_string *>(luaL_testudata(L, -1, tstringKey))) {
					v = *tptr;
					break;
				} else {
					return_misformed();
				}
			}
			default:
				return_misformed();
		}
	}

	lua_settop(L, initial_top);
	return true;
}

#undef return_misformed


config luaW_checkconfig(lua_State *L, int index)
{
	config result;
	if (!luaW_toconfig(L, index, result))
		luaL_typerror(L, index, "WML table");
	return result;
}

bool luaW_tovconfig(lua_State *L, int index, vconfig &vcfg)
{
	switch (lua_type(L, index))
	{
		case LUA_TTABLE:
		{
			config cfg;
			bool ok = luaW_toconfig(L, index, cfg);
			if (!ok) return false;
			vcfg = vconfig(cfg, true);
			break;
		}
		case LUA_TUSERDATA:
			if (vconfig * ptr = static_cast<vconfig *> (luaL_testudata(L, index, vconfigKey))) {
				vcfg = *ptr;
			} else {
				return false;
			}
		case LUA_TNONE:
		case LUA_TNIL:
			break;
		default:
			return false;
	}
	return true;
}

vconfig luaW_checkvconfig(lua_State *L, int index, bool allow_missing)
{
	vconfig result = vconfig::unconstructed_vconfig();
	if (!luaW_tovconfig(L, index, result) || (!allow_missing && result.null()))
		luaL_typerror(L, index, "WML table");
	return result;
}


#ifdef __GNUC__
__attribute__((sentinel))
#endif
bool luaW_getglobal(lua_State *L, ...)
{
	lua_pushglobaltable(L);
	va_list ap;
	va_start(ap, L);
	while (const char *s = va_arg(ap, const char *))
	{
		if (!lua_istable(L, -1)) goto discard;
		lua_pushstring(L, s);
		lua_rawget(L, -2);
		lua_remove(L, -2);
	}

	if (lua_isnil(L, -1)) {
		discard:
		va_end(ap);
		lua_pop(L, 1);
		return false;
	}
	va_end(ap);
	return true;
}

bool luaW_toboolean(lua_State *L, int n)
{
	return lua_toboolean(L,n) != 0;
}
