/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "terrain/type_data.hpp"

#include "terrain/terrain.hpp"

#include <map>

terrain_type_data::terrain_type_data(const config & game_config)
	: terrainList_()
	, tcodeToTerrain_()
	, initialized_(false)
	, game_config_(game_config)
{
}

const t_translation::ter_list & terrain_type_data::list() const
{
	if (!initialized_) {
		create_terrain_maps(game_config_.child_range("terrain_type"), terrainList_, tcodeToTerrain_);
		initialized_ = true;
	}

	return terrainList_;
}


const std::map<t_translation::terrain_code, terrain_type> & terrain_type_data::map() const
{
	if (!initialized_) {
		create_terrain_maps(game_config_.child_range("terrain_type"), terrainList_, tcodeToTerrain_);
		initialized_ = true;
	}

	return tcodeToTerrain_;
}


const terrain_type& terrain_type_data::get_terrain_info(const t_translation::terrain_code & terrain) const
{
	auto i = find_or_merge(terrain);

	if(i != tcodeToTerrain_.end()) {
		return i->second;
	} else {
		static const terrain_type default_terrain;
		return default_terrain;
	}
}

const t_translation::ter_list& terrain_type_data::underlying_mvt_terrain(const t_translation::terrain_code & terrain) const
{
	auto i = find_or_merge(terrain);

	if(i == tcodeToTerrain_.end()) {
		// TODO: At least in some cases(for exampel when this is called form lua) it
		// seems to make more sense to throw an excaption here, samge goes for get_terrain_info
		// and underlying_def_terrain
		static t_translation::ter_list result(1);
		result[0] = terrain;
		return result;
	} else {
		return i->second.mvt_type();
	}
}

const t_translation::ter_list& terrain_type_data::underlying_def_terrain(const t_translation::terrain_code & terrain) const
{
	auto i = find_or_merge(terrain);

	if(i == tcodeToTerrain_.end()) {
		static t_translation::ter_list result(1);
		result[0] = terrain;
		return result;
	} else {
		return i->second.def_type();
	}
}

const t_translation::ter_list& terrain_type_data::underlying_union_terrain(const t_translation::terrain_code & terrain) const
{
	const std::map<t_translation::terrain_code,terrain_type>::const_iterator i =
		tcodeToTerrain_.find(terrain);

	if(i == tcodeToTerrain_.end()) {
		static t_translation::ter_list result(1);
		result[0] = terrain;
		return result;
	} else {
		return i->second.union_type();
	}
}



std::string terrain_type_data::get_terrain_string(const t_translation::terrain_code& terrain) const
{
	std::string str =
		get_terrain_info(terrain).description();

	str += get_underlying_terrain_string(terrain);

	return str;
}

std::string terrain_type_data::get_terrain_editor_string(const t_translation::terrain_code& terrain) const
{
	std::string str =
		get_terrain_info(terrain).editor_name();
	const std::string desc =
		get_terrain_info(terrain).description();

	if(str != desc) {
		str += "/";
		str += desc;
	}

	str += get_underlying_terrain_string(terrain);

	return str;
}

std::string terrain_type_data::get_underlying_terrain_string(const t_translation::terrain_code& terrain) const
{
	std::string str;

	const t_translation::ter_list& underlying = underlying_union_terrain(terrain);
	assert(!underlying.empty());

	if(underlying.size() > 1 || underlying[0] != terrain) {
		str += " (";
        t_translation::ter_list::const_iterator i = underlying.begin();
        str += get_terrain_info(*i).name();
        while (++i != underlying.end()) {
			str += ", " + get_terrain_info(*i).name();
        }
		str += ")";
	}

	return str;
}

tcodeToTerrain_t::const_iterator terrain_type_data::find_or_merge(t_translation::terrain_code terrain) const
{
	auto i = tcodeToTerrain_.find(terrain);
	if (i != tcodeToTerrain_.end()) {
		return i;
	}
	else {
		auto base_iter    = tcodeToTerrain_.find(t_translation::terrain_code(terrain.base, t_translation::NO_LAYER));
		auto overlay_iter = tcodeToTerrain_.find(t_translation::terrain_code(t_translation::NO_LAYER, terrain.overlay));

		if(base_iter == tcodeToTerrain_.end() || overlay_iter == tcodeToTerrain_.end()) {
			return tcodeToTerrain_.end();
		}

		terrain_type new_terrain(base_iter->second, overlay_iter->second);
		terrainList_.push_back(new_terrain.number());
		return tcodeToTerrain_.emplace(new_terrain.number(), std::move(new_terrain)).first;
	}
}

bool terrain_type_data::try_merge_terrains(const t_translation::terrain_code & terrain)
{
	return find_or_merge(terrain) != tcodeToTerrain_.end();
}

t_translation::terrain_code terrain_type_data::merge_terrains(const t_translation::terrain_code & old_t, const t_translation::terrain_code & new_t, const merge_mode mode, bool replace_if_failed) {
	t_translation::terrain_code result = t_translation::NONE_TERRAIN;

	if(mode == OVERLAY) {
		const t_translation::terrain_code t = t_translation::terrain_code(old_t.base, new_t.overlay);
		if (try_merge_terrains(t)) {
			result = t;
		}
	}
	else if(mode == BASE) {
		const t_translation::terrain_code t = t_translation::terrain_code(new_t.base, old_t.overlay);
		if (try_merge_terrains(t)) {
			result = t;
		}
	}
	else if(mode == BOTH && new_t.base != t_translation::NO_LAYER) {
		// We need to merge here, too, because the dest terrain might be a combined one.
		if (try_merge_terrains(new_t)) {
			result = new_t;
		}
	}

	// if merging of overlay and base failed, and replace_if_failed is set,
	// replace the terrain with the complete new terrain (if given)
	// or with (default base)^(new overlay)
	if(result == t_translation::NONE_TERRAIN && replace_if_failed && tcodeToTerrain_.count(new_t) > 0) {
		if(new_t.base != t_translation::NO_LAYER) {
			// Same as above
			if (try_merge_terrains(new_t)) {
				result = new_t;
			}
		}
		else if (get_terrain_info(new_t).default_base() != t_translation::NONE_TERRAIN) {
			result = get_terrain_info(new_t).terrain_with_default_base();
		}
	}
	return result;
}
