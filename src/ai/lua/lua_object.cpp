/*
   Copyright (C) 2011 - 2017 by Dmitry Kovalenko <nephro.wes@gmail.com>
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
 * Lua object(code) wrapper implementation
 */


#include "ai/lua/lua_object.hpp"
#include "ai/lua/engine_lua.hpp"
#include "ai/default/aspect_attacks.hpp"
#include "scripting/lua_common.hpp"
#include "resources.hpp"

#include "lua/lauxlib.h"

namespace ai {

	lua_object_base::lua_object_base()
	{
		// empty
	}

	// MSVC fails to compile without this line
	template class lua_object<aspect_attacks_lua_filter>;

	template <>
	std::shared_ptr<aspect_attacks_lua_filter> lua_object<aspect_attacks_lua_filter>::to_type(lua_State *L, int n)
	{
		std::shared_ptr<aspect_attacks_lua_filter> att(new aspect_attacks_lua_filter);
		att->lua = nullptr;
		att->ref_own_ = att->ref_enemy_ = -1;
		if(!lua_istable(L, n)) {
			return att;
		}
		lua_getfield(L, n, "own");
		if(lua_istable(L, -1)) {
			config cfg;
			vconfig vcfg(cfg, true);
			if(luaW_tovconfig(L, -1, vcfg)) {
				att->filter_own_.reset(new unit_filter(vcfg, resources::filter_con));
			}
		} else if(lua_isfunction(L, -1)) {
			att->lua = L;
			att->ref_own_ = luaL_ref(L, LUA_REGISTRYINDEX);
			assert(att->ref_own_ != -1);
		}
		lua_getfield(L, n, "enemy");
		if(lua_istable(L, -1)) {
			config cfg;
			vconfig vcfg(cfg, true);
			if(luaW_tovconfig(L, -1, vcfg)) {
				att->filter_enemy_.reset(new unit_filter(vcfg, resources::filter_con));
			}
		} else if(lua_isfunction(L, -1)) {
			att->lua = L;
			att->ref_enemy_ = luaL_ref(L, LUA_REGISTRYINDEX);
			assert(att->ref_enemy_ != -1);
		}
		lua_pop(L, 2);
		return att;
	}

} //end of namespace ai
