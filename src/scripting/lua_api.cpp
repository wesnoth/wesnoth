/*
   Copyright (C) 2009 - 2015 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "lua_jailbreak_exception.hpp"  // for tlua_jailbreak_exception

#include "chat_events.hpp"              // for chat_handler, etc
#include "config.hpp"
#include "display_chat_manager.hpp"
#include "game_display.hpp"
#include "log.hpp"
#include "map_location.hpp"             // for map_location
#include "resources.hpp"
#include "scripting/lua_common.hpp"
#include "tstring.hpp"
#include "unit.hpp"
#include "unit_map.hpp"
#include "variable.hpp"

#include <boost/foreach.hpp>
#include <boost/variant/static_visitor.hpp>
#include <map>                          // for map<>::key_type
#include <new>                          // for operator new
#include <ostream>                      // for operator<<, basic_ostream, etc
#include <utility>                      // for pair

#include <string>

#include "lua/lauxlib.h"
#include "lua/lua.h"                    // for lua_State, lua_settop, etc

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

void chat_message(std::string const &caption, std::string const &msg)
{
	if (!resources::screen) return;
	resources::screen->get_chat_manager().add_chat_message(time(NULL), caption, 0, msg,
		events::chat_handler::MESSAGE_PUBLIC, false);
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

lua_unit::~lua_unit()
{
}

unit* lua_unit::get()
{
	if (ptr) return ptr.get();
	if (c_ptr) return c_ptr;
	if (side) {
		return (*resources::teams)[side - 1].recall_list().find_if_matches_underlying_id(uid).get();
	}
	unit_map::unit_iterator ui = resources::units->find(uid);
	if (!ui.valid()) return NULL;
	return ui.get_shared_ptr().get(); //&*ui would not be legal, must get new shared_ptr by copy ctor because the unit_map itself is holding a boost shared pointer.
}
unit_ptr lua_unit::get_shared()
{
	if (ptr) return ptr;
	if (side) {
		return (*resources::teams)[side - 1].recall_list().find_if_matches_underlying_id(uid);
	}
	unit_map::unit_iterator ui = resources::units->find(uid);
	if (!ui.valid()) return unit_ptr();
	return ui.get_shared_ptr(); //&*ui would not be legal, must get new shared_ptr by copy ctor because the unit_map itself is holding a boost shared pointer.
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
			ptr.reset();
			uid = res.first->underlying_id();
		} else {
			ERR_LUA << "Could not move unit " << ptr->underlying_id() << " onto map location " << loc << '\n';
			return false;
		}
	} else if (side) { // recall list
		unit_ptr it = (*resources::teams)[side - 1].recall_list().extract_if_matches_underlying_id(uid);
		if (it) {
			side = 0;
			// uid may be changed by unit_map on insertion
			uid = resources::units->replace(loc, *it).first->underlying_id();
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
			ERR_LUA << "Could not find unit " << uid << " on the map" << std::endl;
			return false;
		}
	}
	return true;
}

unit* luaW_tounit(lua_State *L, int index, bool only_on_map)
{
	if (!luaW_hasmetatable(L, index, getunitKey)) return NULL;
	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, index));
	if (only_on_map && !lu->on_map()) return NULL;
	return lu->get();
}

unit& luaW_checkunit(lua_State *L, int index, bool only_on_map)
{
	unit* u = luaW_tounit(L, index, only_on_map);
	if (!u) luaL_typerror(L, index, "unit");
	return *u;
}

lua_unit* LuaW_pushlocalunit(lua_State *L, unit& u)
{
	lua_unit* res = new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(u);
	lua_pushlightuserdata(L, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return res;
}