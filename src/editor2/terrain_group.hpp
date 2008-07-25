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

//! @file action.hpp
//! Editor action classes

#ifndef EDITOR2_TERRAIN_GROUP_HPP
#define EDITOR2_TERRAIN_GROUP_HPP

#include "action_base.hpp"
#include "editor_map.hpp"
#include "map_fragment.hpp"
#include "../map.hpp"
#include "../terrain.hpp"


namespace editor2 {

class terrain_group
{
public:
	terrain_group(const config& cfg);
	explicit terrain_group(const std::string id);
	void add_terrain(t_translation::t_terrain terrain);
	const std::string& get_id() const { return id_; }
	std::vector<t_translation::t_terrain> get_terrains();
	
private:
	std::string id_;
	t_string name_;
	std::string icon_string_;
	std::vector<t_translation::t_terrain> terrains_;
};

class terrain_palette
{
public:
	terrain_palette(const config& cfg, const gamemap& map);
	terrain_group& current_group();
	int groups_count();
	void next_group();
	void prev_group();
	void set_group_index(int index);
	void set_group(const std::string& id);
	
	static const std::string TERRAIN_GROUP_DEFAULT;
private:
	std::vector<terrain_group> groups_;
	int current_group_index_;
};


} //end namespace editor2

#endif
