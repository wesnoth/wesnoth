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

#pragma once

#include <cstddef>
#include <string>
#include "scripting/lua_common.hpp"
#include "map/location.hpp"
#include "terrain/translation.hpp"
#include "terrain/type_data.hpp"

struct lua_State;
class lua_unit;
struct map_location;

// this clas is similar to the orginal gamemap class but they have no 'is' rlation:
//  mapgen_gamemap, unlike gamemap offers 'raw' access to the data
//  gamemap, unlike mapgen_gamemap uses terrain type data.
class mapgen_gamemap
{
public:
	using terrain_code = t_translation::terrain_code;
	using terrain_map = t_translation::ter_map;
	using starting_positions = t_translation::starting_positions;
	explicit mapgen_gamemap(std::string_view data);
	mapgen_gamemap(int w, int h, terrain_code);

	std::string to_string() const;

	/** Effective map width. */
	int w() const { return total_width() - 2; }

	/** Effective map height. */
	int h() const { return total_height() - 2; }

	/** Real width of the map, including borders. */
	int total_width()  const { return tiles_.w; }

	/** Real height of the map, including borders */
	int total_height() const { return tiles_.h; }

	bool on_map(const map_location& loc) const
	{
		return loc.wml_x() >= 0 && loc.wml_x() < total_width() &&  loc.wml_y() >= 0 && loc.wml_y() < total_height();
	}

	bool on_map_noborder(const map_location& loc) const
	{
		return loc.wml_x() > 0 && loc.wml_x() < total_width() - 1 &&  loc.wml_y() > 0 && loc.wml_y() < total_height() - 1;
	}

	terrain_code& operator[](const map_location& loc)
	{
		return tiles_.get(loc.wml_x(), loc.wml_y());
	}

	const terrain_code& operator[](const map_location& loc) const
	{
		return tiles_.get(loc.wml_x(), loc.wml_y());
	}

	void set_terrain(const map_location& loc, const terrain_code & terrain, const terrain_type_data::merge_mode mode);
	static void simplemerge(terrain_code old, terrain_code& t, const terrain_type_data::merge_mode mode);

	starting_positions& special_locations() { return starting_positions_; }
	const starting_positions& special_locations() const { return starting_positions_; }

	void set_special_location(const std::string& id, const map_location& loc);
	map_location special_location(const std::string& id) const;

	template<typename F>
	void for_each_loc(const F& f) const
	{
		for (int x = 0; x < total_width(); ++x) {
			for (int y = 0; y < total_height(); ++y) {
				f({ x, y , wml_loc()});
			}
		}
	}
	static int intf_mg_terrain_mask(lua_State *L);
private:
	t_translation::ter_map tiles_;
	starting_positions starting_positions_;
};

bool luaW_isslocs(lua_State* L, int index);

mapgen_gamemap* luaW_toslocs(lua_State *L, int index);

mapgen_gamemap& luaW_check_slocs(lua_State *L, int index);

void lua_slocs_setmetatable(lua_State *L);

void luaW_pushslocs(lua_State *L, int index);

int impl_slocs_get(lua_State* L);

int impl_slocs_set(lua_State* L);

bool luaW_isterrainmap(lua_State* L, int index);

mapgen_gamemap* luaW_toterrainmap(lua_State *L, int index);

mapgen_gamemap& luaW_checkterrainmap(lua_State *L, int index);

void lua_terrainmap_setmetatable(lua_State *L);

mapgen_gamemap* luaW_pushmap(lua_State *L, mapgen_gamemap&& u);

int intf_terrainmap_create(lua_State *L);

namespace lua_terrainmap {
	std::string register_metatables(lua_State *L);
}
