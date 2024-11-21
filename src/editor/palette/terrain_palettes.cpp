/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "editor/palette/terrain_palettes.hpp"

#include "gettext.hpp"
#include "serialization/string_utils.hpp"
#include "game_config_view.hpp"

namespace {
	static t_translation::terrain_code fg_terrain;
	static t_translation::terrain_code bg_terrain;
}

namespace editor {

const t_translation::terrain_code& get_selected_bg_terrain() {
	return bg_terrain;
}

const t_translation::terrain_code& get_selected_fg_terrain() {
	return fg_terrain;
}

const t_translation::terrain_code& terrain_palette::selected_fg_item() const { return fg_terrain; }
const t_translation::terrain_code& terrain_palette::selected_bg_item() const { return bg_terrain; }


static bool is_valid_terrain(const t_translation::terrain_code & c) {
	return !(c == t_translation::VOID_TERRAIN || c == t_translation::FOGGED);
}

void terrain_palette::select_bg_item(const std::string& item_id) {
	bg_terrain = item_map_[item_id];
	editor_palette<t_translation::terrain_code>::select_bg_item(item_id);
}

void terrain_palette::select_fg_item(const std::string& item_id) {
	fg_terrain = item_map_[item_id];
	editor_palette<t_translation::terrain_code>::select_fg_item(item_id);
}

void terrain_palette::select_bg_item(const t_translation::terrain_code& terrain) {
	bg_terrain = terrain;
	editor_palette<t_translation::terrain_code>::select_bg_item(get_id(terrain));
}

void terrain_palette::select_fg_item(const t_translation::terrain_code& terrain) {
	fg_terrain = terrain;
	editor_palette<t_translation::terrain_code>::select_fg_item(get_id(terrain));
}


void terrain_palette::setup(const game_config_view& cfg)
{
	// Get the available terrains temporary in items
	t_translation::ter_list items = gui_.get_map().get_terrain_list();

	//move "invalid" items to the end
	std::stable_partition(items.begin(), items.end(), is_valid_terrain);

	// Get the available groups and add them to the structure
	std::set<std::string> group_names;
	for(const config &group : cfg.child_range("terrain_group")) {
		if(group_names.count(group["id"]) == 0) {
			config group_cfg;
			group_cfg["id"] = group["id"];
			group_cfg["name"] = group["name"];

			group_cfg["icon"] = group["icon"].str();
			group_cfg["core"] = group["core"];
			groups_.emplace_back(group_cfg);

			group_names.insert(groups_.back().id);
		}
	}
	for(const config &group : cfg.child_range("editor_group")) {
		if(group_names.count(group["id"]) == 0) {
			config group_cfg;
			group_cfg["id"] = group["id"];
			group_cfg["name"] = group["name"];

			group_cfg["icon"] = "icons/terrain/terrain_" + group["icon"].str();
			group_cfg["core"] = group["core"];
			groups_.emplace_back(group_cfg);

			group_names.insert(groups_.back().id);
		}
	}

	std::map<std::string, item_group*> id_to_group;
	for (item_group& group : groups_) {
		id_to_group.emplace(group.id, &group);
	}

	// add the groups for all terrains to the map
	for (const t_translation::terrain_code& t : items) {

		const terrain_type& t_info = gui_.get_map().get_terrain_info(t);
		DBG_ED << "Palette: processing terrain " << t_info.name()
			<< "(editor name: '" << t_info.editor_name() << "') "
			<< "(" << t_info.number() << ")"
			<< ": " << t_info.editor_group();

		// don't display terrains that were automatically created from base+overlay
		if (t_info.is_combined()) continue;
		// nor display terrains that have hide_in_editor=true
		if (t_info.hide_in_editor()) continue;

		// add the terrain to the requested groups
		const std::vector<std::string>& keys = utils::split(t_info.editor_group());
		bool core = false;

		item_map_[get_id(t)] = t;

		for (const std::string& k : keys) {
			group_map_[k].push_back(get_id(t));
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
		ERR_ED << "No items found.";
	}
}

void terrain_palette::setup_item(
	const t_translation::terrain_code& terrain,
	texture& base_image,
	texture& overlay_image,
	std::stringstream& tooltip_text)
{
	const auto& info = gui_.get_map().get_terrain_info(terrain);

	//Draw default base for overlay terrains
	if(info.has_default_base()) {
		const std::string base_filename = info.editor_image();
		base_image = image::get_texture(base_filename);

		if(!base_image) {
			tooltip_text << "BASE IMAGE NOT FOUND\n";
			ERR_ED << "image for terrain : '" << base_filename << "' not found";
			base_image = image::get_texture(game_config::images::missing);
			if(!base_image) {
				ERR_ED << "Placeholder image not found";
				return;
			}
		}
	}

	const std::string filename = info.editor_image();
	overlay_image = image::get_texture(filename);
	if(!overlay_image) {
		tooltip_text << "IMAGE NOT FOUND\n";
		ERR_ED << "image for terrain: '" << filename << "' not found";
		overlay_image = image::get_texture(game_config::images::missing);
		if(!overlay_image) {
			ERR_ED << "Placeholder image not found";
			return;
		}
	}

	tooltip_text << gui_.get_map().get_terrain_editor_string(terrain);
	if(gui_.debug_flag_set(display::DEBUG_TERRAIN_CODES)) {
		tooltip_text << " " + font::unicode_em_dash + " " << terrain;
	}
}

terrain_palette::terrain_palette(editor_display &gui, editor_toolkit &toolkit)
//TODO avoid magic numbers
	:	editor_palette<t_translation::terrain_code>(gui, 36, 4, toolkit)
{
}

const std::string& terrain_palette::get_id(const t_translation::terrain_code& terrain)
{
	const terrain_type& t_info = gui_.get_map().get_terrain_info(terrain);
	return t_info.id();
}

std::string terrain_palette::get_help_string() const
{
	std::ostringstream msg;
	msg << _("Left-click: ") << gui_.get_map().get_terrain_editor_string(selected_fg_item())	<< " | "
		<< _("Right-click: ") << gui_.get_map().get_terrain_editor_string(selected_bg_item()) << "\n";
	if(selected_fg_item().base == t_translation::NO_LAYER) {
		// TRANSLATORS: Similar behavior applies to shift + right-click. This message specifies left-click
		// because the logic of whether to show the "overlay only" or "base only" version depends on the
		// terrain currently selected for the left button.
		msg << _("Shift + left-click: paint overlay layer only") << " | ";
	} else {
		msg << _("Shift + left-click: paint base layer only") << " | ";
	}
#ifdef __APPLE__
	msg << _("Cmd + click: copy terrain") << std::endl;
#else
	msg << _("Ctrl + click: copy terrain") << std::endl;
#endif
	return msg.str();
}


}
