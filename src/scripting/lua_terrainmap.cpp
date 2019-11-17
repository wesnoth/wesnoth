/*
   Copyright (C) 2018 the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/lua_terrainmap.hpp"
#include "scripting/lua_terrainfilter.hpp"

#include "formatter.hpp"
#include "global.hpp"
#include "log.hpp"
#include "map/location.hpp"
#include "map/map.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/push_check.hpp"
#include "scripting/game_lua_kernel.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char terrainmapKey[] = "terrainmap";
static const char maplocationKey[] = "special_locations";

using std::string_view;

////////  SPECIAL LOCATION  ////////

bool luaW_isslocs(lua_State* L, int index)
{
	return luaL_testudata(L, index, maplocationKey) != nullptr;
}


mapgen_gamemap* luaW_toslocs(lua_State *L, int index)
{
	if(!lua_istable(L, index)) {
		return nullptr;
	}

	lua_rawgeti(L, index, 1);
	mapgen_gamemap* m = luaW_toterrainmap(L, -1);
	lua_pop(L, 1);
	return m;
}

mapgen_gamemap& luaW_check_slocs(lua_State *L, int index)
{
	if(mapgen_gamemap* m = luaW_toslocs(L, index)) {
		return *m;
	}
	luaW_type_error(L, index, "terrainmap");
	throw "luaW_type_error didn't thow.";
}

void lua_slocs_setmetatable(lua_State *L)
{
	luaL_setmetatable(L, maplocationKey);
}
/**
  * @a index the index of the map object.
  */
void luaW_pushslocs(lua_State *L, int index)
{
	lua_pushvalue(L, index);
	//stack: map
	lua_createtable(L, 1, 0);
	//stack: map, slocs
	lua_pushvalue(L, -2);
	//stack: map, slocs, map
	lua_rawseti(L, -2, 1);
	//stack: map, slocs
	luaL_setmetatable(L, maplocationKey);
	//stack: map, slocs
	lua_remove(L, -2);
	//stack: slocs
}

int impl_slocs_get(lua_State* L)
{
	//todo: calling map.special_locations[1] will return the underlying map
	//      object instead of the first starting position, because the lua
	//      special locations is actually a table with the map object at
	//      index 1. The probably easiest way to fix this inconsistency is
	//      to just disallow all integer indices here.
	mapgen_gamemap& m = luaW_check_slocs(L, 1);
	string_view id = luaL_checkstring(L, 2);
	auto res = m.special_location(std::string(id));
	if(res.wml_x() >= 0) {
		luaW_pushlocation(L, res);
	}
	else {
		//functions with variable return numbers have been causing problem in the past
		lua_pushnil(L);
	}
	return 1;
}

int impl_slocs_set(lua_State* L)
{
	mapgen_gamemap& m = luaW_check_slocs(L, 1);
	string_view id = luaL_checkstring(L, 2);
	map_location loc = luaW_checklocation(L, 3);

	m.set_special_location(std::string(id), loc);
	return 0;
}

////////  MAP  ////////

mapgen_gamemap::mapgen_gamemap(std::string_view s)
	: tiles_()
	, starting_positions_()
{
	if(s.empty()) {
		return;
	}
	//throws t_translation::error
	//todo: make read_game_map take a string_view
	tiles_ = t_translation::read_game_map(s, starting_positions_, t_translation::coordinate{ 1, 1 });
}

mapgen_gamemap::mapgen_gamemap(int w, int h, terrain_code t)
	: tiles_(w, h, t)
	, starting_positions_()
{

}

std::string mapgen_gamemap::to_string() const
{
	return t_translation::write_game_map(tiles_, starting_positions_, { 1, 1 }) + "\n";
}

void mapgen_gamemap::set_terrain(const map_location& loc, const terrain_code & terrain, const terrain_type_data::merge_mode mode)
{
	terrain_code& t = (*this)[loc];
	terrain_code old = t;
	t = terrain;
	simplemerge(old, t, mode);

}

void mapgen_gamemap::simplemerge(terrain_code old_t, terrain_code& new_t, const terrain_type_data::merge_mode mode)
{
	if(mode == terrain_type_data::OVERLAY) {
		new_t = t_translation::terrain_code(old_t.base, new_t.overlay);
	}
	if(mode == terrain_type_data::BASE) {
		new_t = t_translation::terrain_code(new_t.base, old_t.overlay);
	}
}

void mapgen_gamemap::set_special_location(const std::string& id, const map_location& loc)
{
	bool valid = loc.valid();
	auto it_left = starting_positions_.left.find(id);
	if (it_left != starting_positions_.left.end()) {
		if (valid) {
			starting_positions_.left.replace_data(it_left, loc);
		}
		else {
			starting_positions_.left.erase(it_left);
		}
	}
	else {
		starting_positions_.left.insert(it_left, std::pair(id, loc));
	}
}

map_location mapgen_gamemap::special_location(const std::string& id) const
{
	auto it = starting_positions_.left.find(id);
	if (it != starting_positions_.left.end()) {
		auto& coordinate = it->second;
		return map_location(coordinate.x, coordinate.y);
	}
	else {
		return map_location();
	}
}

bool luaW_isterrainmap(lua_State* L, int index)
{
	return luaL_testudata(L, index, terrainmapKey) != nullptr;
}


mapgen_gamemap* luaW_toterrainmap(lua_State *L, int index)
{
	if(luaW_isterrainmap(L, index)) {
		return static_cast<mapgen_gamemap*>(lua_touserdata(L, index));
	}
	return nullptr;
}

mapgen_gamemap& luaW_checkterrainmap(lua_State *L, int index)
{
	if(luaW_isterrainmap(L, index)) {
		return *static_cast<mapgen_gamemap*>(lua_touserdata(L, index));
	}
	luaW_type_error(L, index, "terrainmap");
	throw "luaW_type_error didn't throw";
}

void lua_terrainmap_setmetatable(lua_State *L)
{
	luaL_setmetatable(L, terrainmapKey);
}

template<typename... T>
mapgen_gamemap* luaW_pushmap(lua_State *L, T&&... params)
{
	mapgen_gamemap* res = new(L) mapgen_gamemap(std::forward<T>(params)...);
	lua_terrainmap_setmetatable(L);
	return res;
}

/**
 * Create a map.
 * - Arg 1: string describing the map data.
 * - or:
 * - Arg 1: int, width
 * - Arg 2: int, height
 * - Arg 3: string, terrain
*/
int intf_terrainmap_create(lua_State *L)
{
	if(lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
		int w = lua_tonumber(L, 1);
		int h = lua_tonumber(L, 2);
		auto terrain = t_translation::read_terrain_code(luaL_checkstring(L, 3));
		luaW_pushmap(L, w, h, terrain);
		return 1;
	}
	else {
		string_view data_str = luaL_checkstring(L, 1);
		luaW_pushmap(L, data_str);
		return 1;
	}
}

/**
 * Destroys a map object before it is collected (__gc metamethod).
 */
static int impl_terrainmap_collect(lua_State *L)
{
	mapgen_gamemap *u = static_cast<mapgen_gamemap*>(lua_touserdata(L, 1));
	u->mapgen_gamemap::~mapgen_gamemap();
	return 0;
}

/**
 * Gets some data on a map (__index metamethod).
 * - Arg 1: full userdata containing the map.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_terrainmap_get(lua_State *L)
{
	mapgen_gamemap& tm = luaW_checkterrainmap(L, 1);
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("width", tm.total_width());
	return_int_attrib("height", tm.total_height());
	return_string_attrib("data", tm.to_string());

	if(strcmp(m, "special_locations") == 0) {
		luaW_pushslocs(L, 1);
		return 1;
	}
	if(luaW_getmetafield(L, 1, m)) {
		return 1;
	}
	return 0;
}

/**
 * Sets some data on a map (__newindex metamethod).
 * - Arg 1: full userdata containing the map.
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_terrainmap_set(lua_State *L)
{
	mapgen_gamemap& tm = luaW_checkterrainmap(L, 1);
	UNUSED(tm);
	char const *m = luaL_checkstring(L, 2);
	std::string err_msg = "unknown modifiable property of map: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}


/**
 * Sets a terrain code.
 * - Arg 1: map location.
 * - Arg 2: terrain code string.
 * - Arg 3: layer: (overlay|base|both, default=both)
*/
static int intf_set_terrain(lua_State *L)
{
	mapgen_gamemap& tm = luaW_checkterrainmap(L, 1);
	map_location loc = luaW_checklocation(L, 2);
	string_view t_str = luaL_checkstring(L, 3);

	auto terrain = t_translation::read_terrain_code(t_str);
	auto mode = terrain_type_data::BOTH;

	if(!lua_isnoneornil(L, 4)) {
		string_view mode_str = luaL_checkstring(L, 4);
		if(mode_str == "base") {
			mode = terrain_type_data::BASE;
		}
		else if(mode_str == "overlay") {
			mode = terrain_type_data::OVERLAY;
		}
	}

	tm.set_terrain(loc, terrain, mode);
	return 0;
}

/**
 * Gets a terrain code.
 * - Arg 1: map location.
 * - Ret 1: string.
 */
static int intf_get_terrain(lua_State *L)
{
	mapgen_gamemap& tm = luaW_checkterrainmap(L, 1);
	map_location loc = luaW_checklocation(L, 2);

	auto t = tm[loc];
	lua_pushstring(L, t_translation::write_terrain_code(t).c_str());
	return 1;
}

static std::vector<gamemap::overlay_rule> read_rules_vector(lua_State *L, int index)
{
	std::vector<gamemap::overlay_rule> rules;
	for (int i = 1, i_end = lua_rawlen(L, index); i <= i_end; ++i)
	{
		lua_rawgeti(L, index, i);
		if(!lua_istable(L, -1)) {
			luaL_argerror(L, index, "rules must be a table of tables");
		}
		rules.push_back(gamemap::overlay_rule());
		auto& rule = rules.back();
		if(luaW_tableget(L, -1, "old")) {
			rule.old_ = t_translation::read_list(luaW_tostring(L, -1));
			lua_pop(L, 1);
		}
		if(luaW_tableget(L, -1, "new")) {
			rule.new_ = t_translation::read_list(luaW_tostring(L, -1));
			lua_pop(L, 1);
		}

		if(luaW_tableget(L, -1, "mode")) {
			auto str = luaW_tostring(L, -1);
			rule.mode_ = str == "base" ? terrain_type_data::BASE : (str == "overlay" ? terrain_type_data::OVERLAY : terrain_type_data::BOTH);
			lua_pop(L, 1);
		}

		if(luaW_tableget(L, -1, "terrain")) {
			const t_translation::ter_list terrain = t_translation::read_list(luaW_tostring(L, -1));
			if(!terrain.empty()) {
				rule.terrain_ = terrain[0];
			}
			lua_pop(L, 1);
		}

		if(luaW_tableget(L, -1, "use_old")) {
			rule.use_old_ = luaW_toboolean(L, -1);
			lua_pop(L, 1);
		}

		if(luaW_tableget(L, -1, "replace_if_failed")) {
			rule.replace_if_failed_ = luaW_toboolean(L, -1);
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}
	return rules;
}
/**
 * Replaces part of the map.
 * - Arg 1: map location.
 * - Arg 2: map data string.
 * - Arg 3: table for optional named arguments
 *   - is_odd: boolean, if Arg2 has the odd map format (as if it was cut from a odd map location)
 *   - ignore_special_locations: boolean
 *   - rules: table of tables
*/
int mapgen_gamemap::intf_mg_terrain_mask(lua_State *L)
{
	mapgen_gamemap& tm1 = luaW_checkterrainmap(L, 1);
	map_location loc = luaW_checklocation(L, 2);
	mapgen_gamemap& tm2 = luaW_checkterrainmap(L, 3);

	bool is_odd = false;
	bool ignore_special_locations = false;
	std::vector<gamemap::overlay_rule> rules;

	if(lua_istable(L, 4)) {
		is_odd = luaW_table_get_def<bool>(L, 4, "is_odd", false);
		ignore_special_locations = luaW_table_get_def<bool>(L, 4, "ignore_special_locations", false);

		if(luaW_tableget(L, 4, "rules")) {
			if(!lua_istable(L, -1)) {
				return luaL_argerror(L, 4, "rules must be a table");
			}
			rules = read_rules_vector(L, -1);
			lua_pop(L, 1);
		}
	}

	gamemap::overlay_impl(
		tm1.tiles_,
		tm1.starting_positions_,
		tm2.tiles_,
		tm2.starting_positions_,
		[&](const map_location& loc, const t_translation::terrain_code& t, terrain_type_data::merge_mode mode, bool) { tm1.set_terrain(loc, t, mode); },
		loc,
		rules,
		is_odd,
		ignore_special_locations
	);

	return 0;
}

namespace lua_terrainmap {
	std::string register_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		cmd_out << "Adding terrainmap metatable...\n";

		luaL_newmetatable(L, terrainmapKey);
		lua_pushcfunction(L, impl_terrainmap_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, impl_terrainmap_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_terrainmap_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "terrainmap");
		lua_setfield(L, -2, "__metatable");
		// terrainmap methods
		lua_pushcfunction(L, intf_set_terrain);
		lua_setfield(L, -2, "set_terrain");
		lua_pushcfunction(L, intf_get_terrain);
		lua_setfield(L, -2, "get_terrain");
		lua_pushcfunction(L, intf_mg_get_locations);
		lua_setfield(L, -2, "get_locations");
		lua_pushcfunction(L, intf_mg_get_tiles_radius);
		lua_setfield(L, -2, "get_tiles_radius");
		lua_pushcfunction(L, &mapgen_gamemap::intf_mg_terrain_mask);
		lua_setfield(L, -2, "terrain_mask");

		cmd_out << "Adding terrainmap2 metatable...\n";

		luaL_newmetatable(L, maplocationKey);
		lua_pushcfunction(L, impl_slocs_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_slocs_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "special_locations");
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}
