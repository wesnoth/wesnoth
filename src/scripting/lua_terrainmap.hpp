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
#include "map/map.hpp"
#include "terrain/translation.hpp"
#include "terrain/type_data.hpp"

struct lua_State;
class lua_unit;
struct map_location;

// Unlike the original gamemap, this offers 'raw' access to the data.
// The original gamemap uses terrain type data.
class mapgen_gamemap : public gamemap_base {
public:
	explicit mapgen_gamemap(std::string_view data);
	mapgen_gamemap(int w, int h, terrain_code);
	void set_terrain(const map_location& loc, const terrain_code & terrain, const terrain_type_data::merge_mode mode = terrain_type_data::BOTH, bool replace_if_failed = false) override;

	template<typename F>
	void for_each_loc(const F& f) const
	{
		for (int x = 0; x < total_width(); ++x) {
			for (int y = 0; y < total_height(); ++y) {
				f({ x, y , wml_loc()});
			}
		}
	}
};

int intf_terrain_mask(lua_State *L);

bool luaW_isterrainmap(lua_State* L, int index);

gamemap_base* luaW_toterrainmap(lua_State *L, int index);

gamemap_base& luaW_checkterrainmap(lua_State *L, int index);

void lua_terrainmap_setmetatable(lua_State *L);

int intf_terrainmap_create(lua_State *L);
int intf_terrainmap_get(lua_State *L);

int intf_replace_if_failed(lua_State* L);

namespace lua_terrainmap {
	std::string register_metatables(lua_State *L, bool use_tf);
}
