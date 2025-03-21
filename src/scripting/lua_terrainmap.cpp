/*
	Copyright (C) 2018 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_terrainmap.hpp"

#include "log.hpp"
#include "map/location.hpp"
#include "map/map.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/push_check.hpp"
#include "resources.hpp"
#include "game_board.hpp"
#include "play_controller.hpp"


static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char terrainmapKey[] = "terrain map";
static const char maplocationKey[] = "special locations";
static const char mapReplaceIfFailedKey[] = "replace_if_failed terrain code";

namespace replace_if_failed_idx {
	enum {CODE = 1, MODE = 2};
}

using std::string_view;

////////  SPECIAL LOCATION  ////////

static int impl_slocs_get(lua_State* L)
{
	gamemap_base& m = luaW_checkterrainmap(L, 1);
	string_view id = luaL_checkstring(L, 2);
	auto res = m.special_location(std::string(id));
	if(res.valid()) {
		luaW_pushlocation(L, res);
	} else {
		//functions with variable return numbers have been causing problem in the past
		lua_pushnil(L);
	}
	return 1;
}

static int impl_slocs_set(lua_State* L)
{
	gamemap_base& m = luaW_checkterrainmap(L, 1);
	string_view id = luaL_checkstring(L, 2);
	map_location loc = luaW_checklocation(L, 3);

	m.set_special_location(std::string(id), loc);
	return 0;
}

static int impl_slocs_next(lua_State *L)
{
	gamemap_base& m = luaW_checkterrainmap(L, lua_upvalueindex(1));
	const t_translation::starting_positions::left_map& left = m.special_locations().left;

	t_translation::starting_positions::left_const_iterator it;
	if (lua_isnoneornil(L, 2)) {
		it = left.begin();
	}
	else {
		it = left.find(luaL_checkstring(L, 2));
		if (it == left.end()) {
			return 0;
		}
		++it;
	}
	if (it == left.end()) {
		return 0;
	}
	lua_pushstring(L, it->first.c_str());
	luaW_pushlocation(L, it->second);
	return 2;
}

static int impl_slocs_iter(lua_State *L)
{
	lua_settop(L, 1);
	lua_pushvalue(L, 1);
	lua_pushcclosure(L, &impl_slocs_next, 1);
	lua_pushvalue(L, 1);
	lua_pushnil(L);
	return 3;
}

////////  MAP  ////////

mapgen_gamemap::mapgen_gamemap(std::string_view s)
{
	if(s.empty()) {
		return;
	}
	//throws t_translation::error
	//todo: make read_game_map take a string_view
	tiles() = t_translation::read_game_map(s, special_locations(), t_translation::coordinate{ 1, 1 });
}

mapgen_gamemap::mapgen_gamemap(int w, int h, terrain_code t)
	: gamemap_base(w, h, t)
{

}

// This can produce invalid combinations in rare case
// where an overlay doesn't have an independent terrain definition,
// or if you set an overlay with no base and merge mode other than OVERLAY.
static void simplemerge(t_translation::terrain_code old_t, t_translation::terrain_code& new_t, const terrain_type_data::merge_mode mode)
{
	switch(mode) {
		case terrain_type_data::OVERLAY:
			new_t = t_translation::terrain_code(old_t.base, new_t.overlay);
			break;
		case terrain_type_data::BASE:
			new_t = t_translation::terrain_code(new_t.base, old_t.overlay);
			break;
		case terrain_type_data::BOTH:
			new_t = t_translation::terrain_code(new_t.base, new_t.overlay);
			break;
	}
}

void mapgen_gamemap::set_terrain(const map_location& loc, const terrain_code & terrain, const terrain_type_data::merge_mode mode, bool)
{
	terrain_code old = get_terrain(loc);
	terrain_code t = terrain;
	simplemerge(old, t, mode);
	tiles().get(loc.x + border_size(), loc.y + border_size()) = t;
}

struct lua_map_ref {
	lua_map_ref(int w, int h, const t_translation::terrain_code& ter)
		: owned_map(new mapgen_gamemap(w, h, ter))
		, map_ptr(owned_map.get())
	{}
	lua_map_ref(string_view data)
		: owned_map(new mapgen_gamemap(data))
		, map_ptr(owned_map.get())
	{}
	lua_map_ref(gamemap_base& ref)
		: map_ptr(&ref)
	{}
	gamemap_base& get_map() {
		return *map_ptr;
	}
private:
	std::unique_ptr<gamemap_base> owned_map;
	gamemap_base* map_ptr; // either owned or unowned
};

bool luaW_isterrainmap(lua_State* L, int index)
{
	return luaL_testudata(L, index, terrainmapKey) != nullptr || luaL_testudata(L, index, maplocationKey) != nullptr;
}


gamemap_base* luaW_toterrainmap(lua_State *L, int index)
{
	if(luaW_isterrainmap(L, index)) {
		return &static_cast<lua_map_ref*>(lua_touserdata(L, index))->get_map();
	}
	return nullptr;
}

gamemap_base& luaW_checkterrainmap(lua_State *L, int index)
{
	if(luaW_isterrainmap(L, index)) {
		return static_cast<lua_map_ref*>(lua_touserdata(L, index))->get_map();
	}
	luaW_type_error(L, index, "terrainmap");
	throw "luaW_type_error didn't throw";
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
		int w = lua_tointeger(L, 1);
		int h = lua_tointeger(L, 2);
		auto terrain = t_translation::read_terrain_code(luaL_checkstring(L, 3));
		new(L) lua_map_ref(w, h, terrain);
	} else {
		string_view data_str = luaL_checkstring(L, 1);
		new(L) lua_map_ref(data_str);
	}
	luaL_setmetatable(L, terrainmapKey);
	return 1;
}

int intf_terrainmap_get(lua_State* L)
{
	new(L) lua_map_ref(const_cast<gamemap&>(resources::gameboard->map()));
	luaL_setmetatable(L, terrainmapKey);
	return 1;
}

/**
 * Destroys a map object before it is collected (__gc metamethod).
 */
static int impl_terrainmap_collect(lua_State *L)
{
	lua_map_ref* m = static_cast<lua_map_ref*>(lua_touserdata(L, 1));
	m->lua_map_ref::~lua_map_ref();
	return 0;
}

static void luaW_push_terrain(lua_State* L, gamemap_base& map, map_location loc)
{
	auto t = map.get_terrain(loc);
	lua_pushstring(L, t_translation::write_terrain_code(t).c_str());
}

static void impl_merge_terrain(lua_State* L, gamemap_base& map, map_location loc)
{
	auto mode = terrain_type_data::BOTH;
	bool replace_if_failed = false;
	string_view t_str;
	if(luaL_testudata(L, 3, mapReplaceIfFailedKey)) {
		replace_if_failed = true;
		lua_getiuservalue(L, 3, replace_if_failed_idx::CODE);
		t_str = luaL_checkstring(L, -1);
		lua_getiuservalue(L, 3, replace_if_failed_idx::MODE);
		mode = terrain_type_data::merge_mode(luaL_checkinteger(L, -1));
	} else {
		t_str = luaL_checkstring(L, 3);
		if(t_str.front() == '^') {
			mode = terrain_type_data::OVERLAY;
		} else if(t_str.back() == '^') {
			mode = terrain_type_data::BASE;
		}
	}

	auto ter = t_translation::read_terrain_code(t_str);

	if(auto gm = dynamic_cast<gamemap*>(&map)) {
		if(resources::gameboard) {
			bool result = resources::gameboard->change_terrain(loc, ter, mode, replace_if_failed);

			for(team& t : resources::gameboard->teams()) {
				t.fix_villages(*gm);
			}

			if(resources::controller) {
				resources::controller->get_display().needs_rebuild(result);
			}
		}
	} else map.set_terrain(loc, ter, mode, replace_if_failed);
}

/**
 * Gets some data on a map (__index metamethod).
 * - Arg 1: full userdata containing the map.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_terrainmap_get(lua_State *L)
{
	gamemap_base& tm = luaW_checkterrainmap(L, 1);
	map_location loc;
	if(luaW_tolocation(L, 2, loc)) {
		luaW_push_terrain(L, tm, loc);
		return 1;
	}

	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("width", tm.total_width());
	return_int_attrib("height", tm.total_height());
	return_int_attrib("playable_width", tm.w());
	return_int_attrib("playable_height", tm.h());
	return_int_attrib("border_size", tm.border_size());
	return_string_attrib("data", tm.to_string());

	if(strcmp(m, "special_locations") == 0) {
		new(L) lua_map_ref(tm);
		luaL_setmetatable(L, maplocationKey);
		return 1;
	}
	if(luaW_getglobal(L, "wesnoth", "map", m)) {
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
	gamemap_base& tm = luaW_checkterrainmap(L, 1);
	map_location loc;
	// The extra check that value (arg 3) isn't a number is because without it,
	// map[4] = 5 would be interpreted as map[{4, 5}] = nil, due to the way
	// luaW_tolocation modifies the stack if it finds a pair of numbers on it.
	if(lua_type(L, 3) != LUA_TNUMBER && luaW_tolocation(L, 2, loc)) {
		impl_merge_terrain(L, tm, loc);
		return 0;
	}
	char const *m = luaL_checkstring(L, 2);
	std::string err_msg = "unknown modifiable property of map: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}

/*
The map iterator, when called with false as its parameter
is roughly equivalent to the following Lua function:

function map_iter()
	local map, x, y = wesnoth.current.map, 0, 1
	return function()
		if x == map.playable_width then
			if y == map.playable_height then
				return nil
			else
				x, y = 1, y + 1
			end
		else
			x = x + 1
		end
		return x, y, map[{x, y}]
	end
end

*/

template<bool with_border>
static int impl_terrainmap_iter(lua_State* L)
{
	// Retrieve the upvalues stored with the function
	gamemap_base& tm = luaW_checkterrainmap(L, lua_upvalueindex(1));
	map_location prev_loc = luaW_checklocation(L, lua_upvalueindex(2));
	int w = with_border ? tm.total_width() - 1 : tm.w();
	int h = with_border ? tm.total_height() - 1 : tm.h();
	int x, y;

	// Given the previous location, determine the next one to be returned
	if(prev_loc.wml_x() == w) {
		if(prev_loc.wml_y() == h) {
			lua_pushnil(L);
			return 1;
		} else {
			x = with_border ? 0 : 1;
			y = prev_loc.wml_y() + 1;
		}
	} else {
		x = prev_loc.wml_x() + 1;
		y = prev_loc.wml_y();
	}

	// Assign the upvalue representing the previous location
	map_location next_loc(x, y, wml_loc{});
	luaW_pushlocation(L, next_loc);
	lua_replace(L, lua_upvalueindex(2));

	// Return the new location and its terrain code
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	luaW_push_terrain(L, tm, next_loc);

	return 3;
}

int intf_terrainmap_iter(lua_State* L)
{
	luaW_checkterrainmap(L, 1);
	bool with_border = lua_isboolean(L, 2) ? luaW_toboolean(L, 2) : false;
	lua_settop(L, 1);
	luaW_pushlocation(L, map_location(with_border ? -1 : 0, 1, wml_loc{}));

	if(with_border) {
		lua_pushcclosure(L, impl_terrainmap_iter<true>, 2);
	} else {
		lua_pushcclosure(L, impl_terrainmap_iter<false>, 2);
	}
	return 1;
}

int intf_on_board(lua_State* L)
{
	gamemap_base& tm = luaW_checkterrainmap(L, 1);
	map_location loc = luaW_checklocation(L, 2);
	bool with_border = luaL_opt(L, luaW_toboolean, 3, false);

	lua_pushboolean(L, with_border ? tm.on_board_with_border(loc) : tm.on_board(loc));
	return 1;
}

int intf_on_border(lua_State* L)
{
	gamemap_base& tm = luaW_checkterrainmap(L, 1);
	map_location loc = luaW_checklocation(L, 2);

	lua_pushboolean(L, tm.on_board_with_border(loc) && !tm.on_board(loc));
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

		if(luaW_tableget(L, -1, "layer")) {
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
int intf_terrain_mask(lua_State *L)
{
	gamemap_base& map = luaW_checkterrainmap(L, 1);
	map_location loc = luaW_checklocation(L, 2);

	bool is_odd = false;
	bool ignore_special_locations = false;
	std::vector<gamemap::overlay_rule> rules;

	if(lua_istable(L, 4)) {
		is_odd = luaW_table_get_def(L, 4, "is_odd", false);
		ignore_special_locations = luaW_table_get_def(L, 4, "ignore_special_locations", false);

		if(luaW_tableget(L, 4, "rules")) {
			if(!lua_istable(L, -1)) {
				return luaL_argerror(L, 4, "rules must be a table");
			}
			rules = read_rules_vector(L, -1);
			lua_pop(L, 1);
		}
	}

	if(lua_isstring(L, 3)) {
		const std::string t_str = luaL_checkstring(L, 3);
		std::unique_ptr<gamemap_base> mask;
		if(dynamic_cast<gamemap*>(&map)) {
			auto mask_ptr = new gamemap("");
			mask_ptr->read(t_str, false);
			mask.reset(mask_ptr);
		} else {
			mask.reset(new mapgen_gamemap(t_str));
		}
		map.overlay(*mask, loc, rules, is_odd, ignore_special_locations);
	} else {
		gamemap_base& mask = luaW_checkterrainmap(L, 3);
		map.overlay(mask, loc, rules, is_odd, ignore_special_locations);
	}

	if(resources::gameboard) {
		if(auto gmap = dynamic_cast<gamemap*>(&map)) {
			for(team& t : resources::gameboard->teams()) {
				t.fix_villages(*gmap);
			}
		}
	}

	if(resources::controller) {
		resources::controller->get_display().needs_rebuild(true);
	}

	return 0;
}

int intf_replace_if_failed(lua_State* L)
{
	auto mode = terrain_type_data::BOTH;
	if(!lua_isnoneornil(L, 2)) {
		string_view mode_str = luaL_checkstring(L, 2);
		if(mode_str == "base") {
			mode = terrain_type_data::BASE;
		} else if(mode_str == "overlay") {
			mode = terrain_type_data::OVERLAY;
		} else if(mode_str != "both") {
			return luaL_argerror(L, 2, "must be one of 'base', 'overlay', or 'both'");
		}
	}

	lua_newuserdatauv(L, 0, 2);
	lua_pushinteger(L, int(mode));
	lua_setiuservalue(L, -2, replace_if_failed_idx::MODE);
	lua_pushvalue(L, 1);
	lua_setiuservalue(L, -2, replace_if_failed_idx::CODE);
	luaL_setmetatable(L, mapReplaceIfFailedKey);
	return 1;
}

static int impl_replace_if_failed_tostring(lua_State* L)
{
	static const char* mode_strs[] = {"base", "overlay", "both"};
	lua_getiuservalue(L, 1, replace_if_failed_idx::CODE);
	string_view t_str = luaL_checkstring(L, -1);
	lua_getiuservalue(L, 1, replace_if_failed_idx::MODE);
	int mode = luaL_checkinteger(L, -1);
	lua_pushfstring(L, "replace_if_failed('%s', '%s')", t_str.data(), mode_strs[mode]);
	return 1;
}

namespace lua_terrainmap {
	std::string register_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		cmd_out << "Adding terrain map metatable...\n";

		luaL_newmetatable(L, terrainmapKey);
		lua_pushcfunction(L, impl_terrainmap_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, impl_terrainmap_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_terrainmap_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, terrainmapKey);
		lua_setfield(L, -2, "__metatable");

		luaL_newmetatable(L, mapReplaceIfFailedKey);
		lua_pushcfunction(L, impl_replace_if_failed_tostring);
		lua_setfield(L, -2, "__tostring");
		lua_pushstring(L, mapReplaceIfFailedKey);
		lua_setfield(L, -2, "__metatable");

		cmd_out << "Adding special locations metatable...\n";

		luaL_newmetatable(L, maplocationKey);
		lua_pushcfunction(L, impl_slocs_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_slocs_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_slocs_iter);
		lua_setfield(L, -2, "__pairs");
		lua_pushstring(L, maplocationKey);
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}
