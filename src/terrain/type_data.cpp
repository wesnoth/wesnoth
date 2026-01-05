/*
	Copyright (C) 2014 - 2025
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "terrain/type_data.hpp"

#include "serialization/string_utils.hpp"
#include "game_config_view.hpp"

#include <map>

#include "log.hpp"
#define ERR_G LOG_STREAM(err, lg::general())
#define LOG_G LOG_STREAM(info, lg::general())
#define DBG_G LOG_STREAM(debug, lg::general())

terrain_type_data* terrain_type_data::get()
{
	assert(singleton_);
	return singleton_;
}

terrain_type_data::terrain_type_data(const game_config_view & game_config)
	: terrainList_()
	, tcodeToTerrain_()
	, initialized_(false)
	, game_config_(game_config)
{
	assert(!singleton_);
	singleton_ = this;
}

terrain_type_data::~terrain_type_data()
{
	assert(singleton_);
	singleton_ = nullptr;
}

void terrain_type_data::reset() const
{
	terrainList_.clear();
	tcodeToTerrain_.clear();
	initialized_ = false;
}

void terrain_type_data::lazy_initialization() const
{
	if(initialized_)
		return;

	for (const config &terrain_data : game_config_.child_range("terrain_type"))
	{
		terrain_type terrain(terrain_data);
		DBG_G << "create_terrain_maps: " << terrain.number() << " "
			<< terrain.id() << " " << terrain.name() << " : " << terrain.editor_group();

		std::pair<std::map<t_translation::terrain_code, terrain_type>::iterator, bool> res;
		res = tcodeToTerrain_.emplace(terrain.number(), terrain);
		if (!res.second) {
			terrain_type& curr = res.first->second;
			if(terrain == curr) {
				LOG_G << "Merging terrain " << terrain.number()
					<< ": " << terrain.id() << " (" << terrain.name() << ")";
				std::vector<std::string> eg1 = utils::split(curr.editor_group());
				std::vector<std::string> eg2 = utils::split(terrain.editor_group());
				std::set<std::string> egs;
				bool clean_merge = true;
				for (std::string& t : eg1) {
					clean_merge &= egs.insert(t).second;
				}
				for (std::string& t : eg2) {
					clean_merge &= egs.insert(t).second;
				}
				std::string joined = utils::join(egs);

				if(clean_merge) {
					LOG_G << "Editor groups merged to: " << joined;
				} else {
					LOG_G << "Merged terrain " << terrain.number()
					<< ": " << terrain.id() << " (" << terrain.name() << ") "
					<< "with duplicate editor groups [" << terrain.editor_group() << "] "
					<< "and [" << curr.editor_group() << "]";
				}
				curr.set_editor_group(joined);
			} else {
				ERR_G << "Duplicate terrain code definition found for " << terrain.number() << "\n"
					<< "Failed to add terrain " << terrain.id() << " (" << terrain.name() << ") "
					<< "[" << terrain.editor_group() << "]" << "\n"
					<< "which conflicts with  " << curr.id() << " (" << curr.name() << ") "
					<< "[" << curr.editor_group() << "]" << "\n";
			}
		} else {
			terrainList_.push_back(terrain.number());
		}
	}
	initialized_ = true;
}

const t_translation::ter_list & terrain_type_data::list() const
{
	lazy_initialization();
	return terrainList_;
}


const std::map<t_translation::terrain_code, terrain_type> & terrain_type_data::map() const
{
	lazy_initialization();
	return tcodeToTerrain_;
}

const terrain_type& terrain_type_data::get_terrain_info(const t_translation::terrain_code & terrain) const
{
	auto i = find_or_create(terrain);

	if(i != tcodeToTerrain_.end()) {
		return i->second;
	} else {
		static const terrain_type default_terrain;
		return default_terrain;
	}
}

t_string terrain_type_data::get_terrain_string(const t_translation::terrain_code& terrain) const
{
	t_string str =
		get_terrain_info(terrain).description();

	str += get_underlying_terrain_string(terrain);

	return str;
}

t_string terrain_type_data::get_terrain_editor_string(const t_translation::terrain_code& terrain) const
{
	t_string str =
		get_terrain_info(terrain).editor_name();
	const t_string& desc =
		get_terrain_info(terrain).description();

	if(str != desc) {
		str += "/";
		str += desc;
	}

	str += get_underlying_terrain_string(terrain);

	return str;
}

t_string terrain_type_data::get_underlying_terrain_string(const t_translation::terrain_code& terrain) const
{
	std::string str;

	const t_translation::ter_list& underlying = get_terrain_info(terrain).union_type();
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

terrain_type_data::tcodeToTerrain_t::const_iterator terrain_type_data::find_or_create(t_translation::terrain_code terrain) const
{
	lazy_initialization();
	auto i = tcodeToTerrain_.find(terrain);
	if (i != tcodeToTerrain_.end()) {
		return i;
	}
	else {
		DBG_G << "find_or_create: creating terrain " << terrain;
		auto base_iter    = tcodeToTerrain_.find(t_translation::terrain_code(terrain.base, t_translation::NO_LAYER));
		auto overlay_iter = tcodeToTerrain_.find(t_translation::terrain_code(t_translation::NO_LAYER, terrain.overlay));

		if(base_iter == tcodeToTerrain_.end() || overlay_iter == tcodeToTerrain_.end()) {
			// This line is easily reachable, after the player has played multiple
			// campaigns. The code for showing movetypes for discovered terrains in the
			// sidebar will query every terrain listed in
			// prefs::get().encountered_terrains(), even those that are campaign-specific.
			// ERR_G << "couldn't find base or overlay for " << terrain;
			return tcodeToTerrain_.end();
		}

		terrain_type new_terrain(base_iter->second, overlay_iter->second);
		terrainList_.push_back(new_terrain.number());
		return tcodeToTerrain_.emplace(new_terrain.number(), std::move(new_terrain)).first;
	}
}

bool terrain_type_data::is_known(const t_translation::terrain_code& terrain) const
{
	// These can't be combined in to a single line, because find_or_create
	// can change the value of tcodeToTerrain_.end().
	const auto t = find_or_create(terrain);
	return t != tcodeToTerrain_.end();
}

t_translation::terrain_code terrain_type_data::merge_terrains(const t_translation::terrain_code & old_t, const t_translation::terrain_code & new_t, const merge_mode mode, bool replace_if_failed) const {
	t_translation::terrain_code result = t_translation::NONE_TERRAIN;

	if(mode == OVERLAY) {
		const t_translation::terrain_code t = t_translation::terrain_code(old_t.base, new_t.overlay);
		if (is_known(t)) {
			result = t;
		}
	}
	else if(mode == BASE) {
		const t_translation::terrain_code t = t_translation::terrain_code(new_t.base, old_t.overlay);
		if (is_known(t)) {
			result = t;
		}
	}
	else if(mode == BOTH && new_t.base != t_translation::NO_LAYER) {
		// We need to merge here, too, because the dest terrain might be a combined one.
		if (is_known(new_t)) {
			result = new_t;
		}
	}

	// if merging of overlay and base failed, and replace_if_failed is set,
	// replace the terrain with the complete new terrain (if given)
	// or with (default base)^(new overlay)
	if(result == t_translation::NONE_TERRAIN && replace_if_failed && is_known(new_t)) {
		if(new_t.base != t_translation::NO_LAYER) {
			result = new_t;
		}
		else if (get_terrain_info(new_t).has_default_base()) {
			result = get_terrain_info(new_t).terrain_with_default_base();
		}
	}
	return result;
}
