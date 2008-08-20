/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

// UNUSED CODE -- TODO FOR THE NEW TERRAIN PALETTE

#include "terrain_group.hpp"
#include "editor_common.hpp"
#include "../foreach.hpp"

namespace editor2 {

terrain_group::terrain_group(const config& cfg);
: id_(cfg["id"]), name_(cfg["name"]), icon_string_(cfg["icon"])
{
}

terrain_group::terrain_group(const std::string& id)
: id_(id), name_(id), icon_string_()
{
}

void terrain_group::add_terrain(t_translation::t_terrain terrain)
{
	terrains_.push_back(terrain);
}

const std::string terrain_palette::TERRAIN_GROUP_DEFAULT = "all";

static bool is_invalid_terrain(t_translation::t_terrain c) {
	return (c == t_translation::VOID_TERRAIN || c == t_translation::FOGGED);
}

terrain_palette::terrain_palette(const gamemap &map, const config& cfg)
: map_(map), current_group_index_(0)
{
	// Get the available terrains
	std::vector<t_translation::t_terrain> terrains = map_.get_terrain_list();
	terrains.erase(std::remove_if(terrains.begin(), terrains.end(), is_invalid_terrain), terrains.end());
	
	// Get the available groups and add them to the structure
    const config::child_list& group_configs = cfg.get_children("editor_group");
	std::set<std::string> group_ids;
	foreach (config* c, group_configs) {
		if (group_ids.find((*c)["id"]) == group_ids.end()) {
			groups_.push_back(terrain_group(*c, gui));
		} else {
			ERR_ED << "Duplicate terrain group: " << (*c)["id"] << "\n";
		}
	}
	if (groups_.find(TERRAIN_GROUP_DEFAULT) == groups_.end()) {
		ERR_ED << "No default (" << TERRAIN_GROUP_DEFAULT << ") terrain group defined!\n";
		groups_.push_back(terrain_group(TERRAIN_GROUP_DEFAULT)));
	}
	std::map<std::string, terrain_group*> groups_map;
	foreach (terrain_group& tg, groups_) {
		groups_map.insert(std::make_pair(tg.get_id(), &tg));
	}
	
	// add the groups for all terrains to the map
	foreach (t_translation::t_terrain t, terrains) {
        const terrain_type& t_info = map_.get_terrain_info(t);
        // don't display terrains that were automatically created from base+overlay
		if (t_info.is_combined()) continue;
		
		std::vector<std::string>& keys = utils::split(t_info.editor_group());		
		foreach (std::string& key, keys) {
			std::map<std::string, terrain_group*>::iterator it = groups_map.find(key);
			if (it != groups_map.end()) {
				groups_map[key]->add_terrain(t);
			} else {
				WRN_ED << "Undefined terrain group used in terrain: " << key << "\n";
			}
		}
		// Add the terrain to the default group
		groups_[TERRAIN_GROUP_DEFAULT].add_terrain(t);
	}
}

terrain_palette::current_group()
{
	return groups_[current_group_index_];
}

int terrain_palette::groups_count()
{
	return groups_.size();
}

void terrain_palette::next_group();
{
	current_group_index_++;
	current_group_index_ %= groups_.size();
}

void terrain_palette::prev_group()
{
	current_group_index_ += groups_size();
	current_group_index_--;
	current_group_index_ %= groups_.size();
}

void terrain_palette::set_group_index(int index)
{
	assert(index >= 0);
	assert(index < groups_.size());
	current_group_index_ = index;
}

void set_group(const std::string& id)
{
	int idx = 0;
	while (idx < groups_.size() && groups_[idx].get_id() != id) idx++;
	if (idx < groups_.size()) {
		current_group_index_ = index;
	} else {
		ERR_ED << "Invalid terrain group id (" << id << ") in set_group\n";
	}
}

} //end namespace editor2
