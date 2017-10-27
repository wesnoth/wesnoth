/*
   Copyright (C) 2012 - 2017 by Fabian Mueller <fabianmueller5@gmx.de>
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
 * Manage the unit-palette in the editor.
 */

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/palette/unit_palette.hpp"

#include "gettext.hpp"

#include "team.hpp"
#include "units/types.hpp"

namespace editor {

std::string unit_palette::get_help_string() {
	return selected_fg_item().type_name();
}

void unit_palette::setup(const config& /*cfg*/)
{
	for(const unit_type_data::unit_type_map::value_type &i : unit_types.types()) {
		if(i.second.do_not_list())
			continue;
		item_map_.emplace(i.second.id(), i.second);
		group_map_[i.second.race_id()].push_back(i.second.id());
		nmax_items_ = std::max<int>(nmax_items_, group_map_[i.second.race_id()].size());
		// Add the unit to the default group
		group_map_["all"].push_back(i.second.id());
		nmax_items_ = std::max<int>(nmax_items_, group_map_["all"].size());
	}

	for(const race_map::value_type &i : unit_types.races()) {
		if(group_map_[i.second.id()].empty())
			continue;
		config cfg;
		cfg["id"] = i.second.id();
		cfg["name"] = i.second.plural_name();
		if(i.second.editor_icon().empty()) {
			cfg["icon"] = "icons/unit-groups/race_" + i.second.id();
		} else {
			cfg["icon"] = i.second.editor_icon();
		}
		cfg["core"] = true;
		groups_.emplace_back(cfg);
	}

	//TODO
	//move "invalid" items to the end
	//std::stable_partition(items.begin(), items.end(), is_valid_terrain);

	select_fg_item(item_map_.begin()->second.id());
	select_bg_item(item_map_.begin()->second.id());

	// Set the default group
	set_group(groups_[0].id);

	if(active_group().empty()) {
		ERR_ED << "No items found." << std::endl;
	}
}

void unit_palette::draw_item(const unit_type& u, surface& image, std::stringstream& tooltip_text) {

	std::stringstream filename;
	filename << u.image() << "~RC(" << u.flag_rgb() << '>'
	    	 << team::get_side_color_id(gui_.viewing_side()) << ')';

	image = image::get_image(filename.str());
	if(image == nullptr) {
		tooltip_text << "IMAGE NOT FOUND\n";
		ERR_ED << "image for unit type: '" << filename.str() << "' not found" << std::endl;
		image = image::get_image(game_config::images::missing);
		if(image == nullptr) {
			ERR_ED << "Placeholder image not found" << std::endl;
			return;
		}
	}

	if(image->w != item_size_ || image->h != item_size_) {
		image.assign(scale_surface(image, item_size_, item_size_));
	}

	tooltip_text << u.type_name();
}

unit_palette::unit_palette(editor_display &gui, const config& cfg,
                           editor_toolkit &toolkit)
//TODO avoid magic numbers
	:	editor_palette<unit_type>(gui, cfg, 36, 4, toolkit),
	 	selected_bg_items_()
{
}

const std::string& unit_palette::get_id(const unit_type& u)
{
	return u.id();
}

bool unit_palette::is_selected_bg_item(const std::string& id)
{
	return selected_bg_items_.count(id) != 0;
}

void unit_palette::select_bg_item(const std::string& item_id) {

	if(selected_bg_items_.count(item_id) != 0) {
		selected_bg_items_.erase(item_id);
	} else {
		selected_bg_items_.insert(item_id);
	}

	set_dirty();
}

}

