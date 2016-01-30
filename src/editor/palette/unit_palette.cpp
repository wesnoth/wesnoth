/*
   Copyright (C) 2012 - 2016 by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "unit_palette.hpp"

#include "../../gettext.hpp"

#include "../../unit_types.hpp"

#include <boost/foreach.hpp>

namespace editor {

std::string unit_palette::get_help_string() {
	return selected_fg_item().type_name();
}

void unit_palette::setup(const config& /*cfg*/)
{
	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		if (i.second.do_not_list())
			continue;
		item_map_.insert(std::pair<std::string, unit_type>(i.second.id(), i.second));
		group_map_[i.second.race_id()].push_back(i.second.id());
		nmax_items_ = std::max(nmax_items_, group_map_[i.second.race_id()].size());
		//TODO
		bool core = true;
		if (core) {
			// Add the unit to the default group
			group_map_["all"].push_back(i.second.id());
			nmax_items_ = std::max(nmax_items_, group_map_["all"].size());
		} else {
			non_core_items_.insert(i.second.id());
		}
	}

	BOOST_FOREACH(const race_map::value_type &i, unit_types.races())
	{
		if (group_map_[i.second.id()].empty())
			continue;
		config cfg;
		cfg["id"] = i.second.id();
		cfg["name"] = i.second.plural_name();
		cfg["icon"] = "icons/unit-groups/race_" + i.second.id();
		cfg["core"] = "yes";
		groups_.push_back(item_group(cfg));
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
	    	 << team::get_side_color_index(gui_.viewing_side()) << ')';

	image = image::get_image(filename.str());
	if(image == NULL) {
		tooltip_text << "IMAGE NOT FOUND\n";
		ERR_ED << "image for unit type: '" << filename.str() << "' not found" << std::endl;
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

	tooltip_text << u.type_name();
}

unit_palette::unit_palette(editor_display &gui, const config& cfg,
								 mouse_action** active_mouse_action)
//TODO avoid magic numbers
	:	editor_palette<unit_type>(gui, cfg, 36, 4, active_mouse_action),
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

	if (selected_bg_items_.count(item_id) != 0)
		selected_bg_items_.erase(item_id);
	else selected_bg_items_.insert(item_id);

	set_dirty();
}

}

