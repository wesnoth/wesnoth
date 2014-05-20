/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * Terrain-palette in the editor.
 */

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "terrain_palettes.hpp"

#include "../../gettext.hpp"

#include <boost/foreach.hpp>

namespace {
	static t_translation::t_terrain fg_terrain;
	static t_translation::t_terrain bg_terrain;
}

namespace editor {

const t_translation::t_terrain& get_selected_bg_terrain() {
	return bg_terrain;
}

const t_translation::t_terrain& get_selected_fg_terrain() {
	return fg_terrain;
}

const t_translation::t_terrain& terrain_palette::selected_fg_item() const { return fg_terrain; }
const t_translation::t_terrain& terrain_palette::selected_bg_item() const { return bg_terrain; }


static bool is_valid_terrain(const t_translation::t_terrain & c) {
	return !(c == t_translation::VOID_TERRAIN || c == t_translation::FOGGED);
}

void terrain_palette::select_bg_item(const std::string& item_id) {
	bg_terrain = item_map_[item_id];
	editor_palette<t_translation::t_terrain>::select_bg_item(item_id);
}

void terrain_palette::select_fg_item(const std::string& item_id) {
	fg_terrain = item_map_[item_id];
	editor_palette<t_translation::t_terrain>::select_fg_item(item_id);
}

void terrain_palette::select_bg_item(const t_translation::t_terrain& terrain) {
	bg_terrain = terrain;
	editor_palette<t_translation::t_terrain>::select_bg_item(get_id(terrain));
}

void terrain_palette::select_fg_item(const t_translation::t_terrain& terrain) {
	fg_terrain = terrain;
	editor_palette<t_translation::t_terrain>::select_fg_item(get_id(terrain));
}


void terrain_palette::setup(const config& cfg)
{
	// Get the available terrains temporary in items
	t_translation::t_list items = map().get_terrain_list();

	//move "invalid" items to the end
	std::stable_partition(items.begin(), items.end(), is_valid_terrain);

	// Get the available groups and add them to the structure
	std::set<std::string> group_names;
	BOOST_FOREACH(const config &group, cfg.child_range("editor_group"))
	{
		if (group_names.find(group["id"]) == group_names.end()) {

			config cfg;
			cfg["id"] = group["id"];
			cfg["name"] = group["name"];

			cfg["icon"] = "icons/terrain/terrain_" + group["icon"].str();
			cfg["core"] = group["core"];
			groups_.push_back(item_group(cfg));

			group_names.insert(groups_.back().id);
		}
	}

	std::map<std::string, item_group*> id_to_group;
	BOOST_FOREACH(item_group& group, groups_) {
		id_to_group.insert(std::make_pair(group.id, &group));
	}

	// add the groups for all terrains to the map
	BOOST_FOREACH(const t_translation::t_terrain& t, items) {

		const terrain_type& t_info = map().get_terrain_info(t);
		DBG_ED << "Palette: processing terrain " << t_info.name()
			<< "(editor name: '" << t_info.editor_name() << "') "
			<< "(" << t_info.number() << ")"
			<< ": " << t_info.editor_group() << "\n";

		// don't display terrains that were automatically created from base+overlay
		if (t_info.is_combined()) continue;
		// nor display terrains that have hide_in_editor=true
		if (t_info.hide_in_editor()) continue;

		// add the terrain to the requested groups
		const std::vector<std::string>& keys = utils::split(t_info.editor_group());
		bool core = false;

		item_map_[get_id(t)] = t;

		BOOST_FOREACH(const std::string& k, keys) {
			group_map_[k].push_back(get_id(t));
			nmax_items_ = std::max(nmax_items_, group_map_[k].size());
			std::map<std::string, item_group*>::iterator i = id_to_group.find(k);
			if (i != id_to_group.end()) {
				if (i->second->core) {
					core = true;
				}
			}
		}

		// A terrain is considered core iff it appears in at least
		// one core terrain group
		if (core) {
		// Add the terrain to the default group
			group_map_["all"].push_back(get_id(t));
		nmax_items_ = std::max(nmax_items_, group_map_["all"].size());
		} else {
			non_core_items_.insert(get_id(t));
		}

	}

	// Set the default terrain
	select_fg_item("regular_mountains");
	select_bg_item("grassland");

	// Set the default group
	set_group("all");

	if(active_group().empty()) {
		ERR_ED << "No items found." << std::endl;
	}
}

void terrain_palette::draw_item(const t_translation::t_terrain& terrain,
		surface& image, std::stringstream& tooltip_text) {

	const t_translation::t_terrain base_terrain =
			map().get_terrain_info(terrain).default_base();

	//Draw default base for overlay terrains
	if(base_terrain != t_translation::NONE_TERRAIN) {
		const std::string base_filename = map().get_terrain_info(base_terrain).editor_image();
		surface base_image(image::get_image(base_filename));

		if(base_image == NULL) {
			tooltip_text << "BASE IMAGE NOT FOUND\n";
			ERR_ED << "image for terrain : '" << base_filename << "' not found" << std::endl;
			base_image = image::get_image(game_config::images::missing);
			if (base_image == NULL) {
				ERR_ED << "Placeholder image not found" << std::endl;
				return;
			}
		}

		if(base_image->w != item_size_ || base_image->h != item_size_) {
			base_image.assign(scale_surface(base_image,
					item_size_, item_size_));
		}
	}

	const std::string filename = map().get_terrain_info(terrain).editor_image();
	image = image::get_image(filename);
	if(image == NULL) {
		tooltip_text << "IMAGE NOT FOUND\n";
		ERR_ED << "image for terrain: '" << filename << "' not found" << std::endl;
		image = image::get_image(game_config::images::missing);
		if (image == NULL) {
			ERR_ED << "Placeholder image not found" << std::endl;
			return;
		}
	}

	if(image->w != item_size_ || image->h != item_size_) {
		image.assign(scale_surface(image,
				item_size_, item_size_));
	}

	tooltip_text << map().get_terrain_editor_string(terrain);
	if (gui_.get_draw_terrain_codes()) {
		tooltip_text << " " + utils::unicode_em_dash + " " << terrain;
	}
}

terrain_palette::terrain_palette(editor_display &gui, const config& cfg,
								 mouse_action** active_mouse_action)
//TODO avoid magic numbers
	:	editor_palette<t_translation::t_terrain>(gui, cfg, 36, 4, active_mouse_action)
{
}

const std::string& terrain_palette::get_id(const t_translation::t_terrain& terrain)
{
	const terrain_type& t_info = map().get_terrain_info(terrain);
	return t_info.id();
}

std::string terrain_palette::get_help_string()
{
	std::ostringstream msg;
	msg << _("FG: ") << map().get_terrain_editor_string(selected_fg_item())	<< " | "
		<< _("BG: ") << map().get_terrain_editor_string(selected_bg_item());
	return msg.str();
}


}
