/*
   Copyright (C) 2009 - 2014 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "lua_api.hpp"
#include "lua_types.hpp"

#include "lua/lualib.h"
#include "lua/lauxlib.h"

#include "variable.hpp"
#include "tstring.hpp"
#include "resources.hpp"
#include "game_display.hpp"
#include "log.hpp"
#include "config.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/variant/static_visitor.hpp>

#include <string>

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

void chat_message(std::string const &caption, std::string const &msg)
{
	resources::screen->add_chat_message(time(NULL), caption, 0, msg,
		events::chat_handler::MESSAGE_PUBLIC, false);
}

void luaW_pushvconfig(lua_State *L, vconfig const &cfg)
{
	new(lua_newuserdata(L, sizeof(vconfig))) vconfig(cfg);
	lua_pushlightuserdata(L
			, vconfigKey);

	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
}

void luaW_pushtstring(lua_State *L, t_string const &v)
{
	new(lua_newuserdata(L, sizeof(t_string))) t_string(v);
	lua_pushlightuserdata(L
			, tstringKey);

	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
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
			if (!luaW_hasmetatable(L, index, tstringKey)) return false;
			str = *static_cast<t_string *>(lua_touserdata(L, index));
			break;
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

bool luaW_toconfig(lua_State *L, int index, config &cfg, int tstring_meta)
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
			if (!luaW_hasmetatable(L, index, vconfigKey))
				return false;
			cfg = static_cast<vconfig *>(lua_touserdata(L, index))->get_parsed_config();
			return true;
		}
		case LUA_TNONE:
		case LUA_TNIL:
			return true;
		default:
			return false;
	}

	// Get t_string's metatable, so that it can be used later to detect t_string object.
	if (!tstring_meta) {
		lua_pushlightuserdata(L
				, tstringKey);

		lua_rawget(L, LUA_REGISTRYINDEX);
		tstring_meta = initial_top + 1;
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
		if (!luaW_toconfig(L, -1, cfg.add_child(m), tstring_meta))
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
				if (!lua_getmetatable(L, -1)) return_misformed();
				bool tstr = lua_rawequal(L, -1, tstring_meta) != 0;
				lua_pop(L, 1);
				if (!tstr) return_misformed();
				v = *static_cast<t_string *>(lua_touserdata(L, -1));
				break;
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
			if (!luaW_hasmetatable(L, index, vconfigKey))
				return false;
			vcfg = *static_cast<vconfig *>(lua_touserdata(L, index));
			break;
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

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4706)
#endif
bool luaW_pcall(lua_State *L
		, int nArgs, int nRets, bool allow_wml_error)
{
	// Load the error handler before the function and its arguments.
	lua_pushlightuserdata(L
			, executeKey);

	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_insert(L, -2 - nArgs);

	int error_handler_index = lua_gettop(L) - nArgs - 1;

	// Call the function.
	int res = lua_pcall(L, nArgs, nRets, -2 - nArgs);
	tlua_jailbreak_exception::rethrow();

	if (res)
	{
		/*
		 * When an exception is thrown which doesn't derive from
		 * std::exception m will be NULL pointer.
		 */
		char const *m = lua_tostring(L, -1);
		if(m) {
			if (allow_wml_error && strncmp(m, "~wml:", 5) == 0) {
				m += 5;
				char const *e = strstr(m, "stack traceback");
				lg::wml_error << std::string(m, e ? e - m : strlen(m));
			} else if (allow_wml_error && strncmp(m, "~lua:", 5) == 0) {
				m += 5;
				char const *e = NULL, *em = m;
				while (em[0] && ((em = strstr(em + 1, "stack traceback"))))
#ifdef _MSC_VER
#pragma warning (pop)
#endif
					e = em;
				chat_message("Lua error", std::string(m, e ? e - m : strlen(m)));
			} else {
				ERR_LUA << m << '\n';
				chat_message("Lua error", m);
			}
		} else {
			chat_message("Lua caught unknown exception", "");
		}
		lua_pop(L, 2);
		return false;
	}

	// Remove the error handler.
	lua_remove(L, error_handler_index);

	return true;
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


lua_unit::~lua_unit()
{
	delete ptr;
}

unit *lua_unit::get()
{
	if (ptr) return ptr;
	if (side) {
		BOOST_FOREACH(unit &u, (*resources::teams)[side - 1].recall_list()) {
			if (u.underlying_id() == uid) return &u;
		}
		return NULL;
	}
	unit_map::unit_iterator ui = resources::units->find(uid);
	if (!ui.valid()) return NULL;
	return &*ui;
}

// Having this function here not only simplifies other code, it allows us to move
// pointers around from one structure to another.
// This makes bare pointer->map in particular about 2 orders of magnitude faster,
// as benchmarked from Lua code.
bool lua_unit::put_map(const map_location &loc)
{
	if (ptr) {
		ptr->set_location(loc);
		resources::units->erase(loc);
		std::pair<unit_map::unit_iterator, bool> res = resources::units->insert(ptr);
		if (res.second) {
			ptr = NULL;
			uid = res.first->underlying_id();
		} else {
			ERR_LUA << "Could not move unit " << ptr->underlying_id() << " onto map location " << loc << '\n';
			return false;
		}
	} else if (side) { // recall list
		std::vector<unit> &recall_list = (*resources::teams)[side - 1].recall_list();
		std::vector<unit>::iterator it = recall_list.begin();
		for(; it != recall_list.end(); ++it) {
			if (it->underlying_id() == uid) {
				break;
			}
		}
		if (it != recall_list.end()) {
			side = 0;
			// uid may be changed by unit_map on insertion
			uid = resources::units->replace(loc, *it).first->underlying_id();
			recall_list.erase(it);
		} else {
			ERR_LUA << "Could not find unit " << uid << " on recall list of side " << side << '\n';
			return false;
		}
	} else { // on map
		unit_map::unit_iterator ui = resources::units->find(uid);
		if (ui != resources::units->end()) {
			map_location from = ui->get_location();
			if (from != loc) { // This check is redundant in current usage
				resources::units->erase(loc);
				resources::units->move(from, loc);
			}
			// No need to change our contents
		} else {
			ERR_LUA << "Could not find unit " << uid << " on the map\n";
			return false;
		}
	}
	return true;
}

unit *luaW_tounit(lua_State *L, int index, bool only_on_map)
{
	if (!luaW_hasmetatable(L, index, getunitKey)) return NULL;
	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, index));
	if (only_on_map && !lu->on_map()) return NULL;
	return lu->get();
}

unit *luaW_checkunit(lua_State *L, int index, bool only_on_map)
{
	unit *u = luaW_tounit(L, index, only_on_map);
	if (!u) luaL_typerror(L, index, "unit");
	return u;
}

bool luaW_toboolean(lua_State *L, int n)
{
	return lua_toboolean(L,n) != 0;
}
