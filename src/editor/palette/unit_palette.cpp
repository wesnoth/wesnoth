/* $Id$ */
/*
   Copyright (C) 2012 by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "../../foreach.hpp"
#include "../../gettext.hpp"

#include "../../unit_types.hpp"

namespace editor {

//TODO
/*
void unit_palette::update_report()
{
//	std::ostringstream msg;
//	msg << _("FG: ") << map().get_terrain_editor_string(selected_fg_item())
//		<< '\n' << _("BG: ") << map().get_terrain_editor_string(selected_fg_item());
//	selected_terrain = msg.str();
}
*/

void unit_palette::setup(const config& /*cfg*/)
{
	foreach (const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		item_map_.insert(std::pair<std::string, unit_type>(i.second.id(), i.second));
		group_map_[i.second.race()].push_back(i.second.id());
		nmax_items_ = std::max(nmax_items_, group_map_[i.second.race()].size());
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

	foreach (const race_map::value_type &i, unit_types.races())
	{
		config cfg;
		cfg["id"] = i.second.id();
		cfg["name"] = i.second.plural_name();
		//TODO
		std::string& tmp = group_map_[i.second.id()][0];
		cfg["icon"] = item_map_.find(tmp)->second.image();
		cfg["core"] = "yes";
		groups_.push_back(item_group(cfg));
	}


	//TODO
	//move "invalid" items to the end
	//std::stable_partition(items.begin(), items.end(), is_valid_terrain);

	// Set the default group

	select_fg_item("Elvish Fighter");
	select_bg_item("Elvish Archer");

	set_group("human");

	if(active_group().empty()) {
		ERR_ED << "No items found.\n";
	}

	//TODO
//	update_report();
}

void unit_palette::draw_item(SDL_Rect& dstrect, const unit_type& u, std::stringstream& tooltip_text) {

	surface screen = gui_.video().getSurface();

	const std::string filename = u.image();
	surface image(image::get_image(filename));
	if(image == NULL) {
		tooltip_text << "IMAGE NOT FOUND\n";
		ERR_ED << "image for terrain: '" << filename << "' not found\n";
		image = image::get_image(game_config::images::missing);
		if (image == NULL) {
			ERR_ED << "Placeholder image not found\n";
			return;
		}
	}

	if(image->w != item_size_ || image->h != item_size_) {
		image.assign(scale_surface(image,
				item_size_, item_size_));
	}

	sdl_blit(image, NULL, screen, &dstrect);

	tooltip_text << u.type_name();
}

unit_palette::unit_palette(editor_display &gui, const size_specs &sizes,
								 const config& cfg,
								 mouse_action** active_mouse_action)
//TODO avoid magic numbers
	:	editor_palette<unit_type>(gui, sizes, cfg, 72, 2, active_mouse_action)
{
}

const std::string& unit_palette::get_id(const unit_type& u)
{
	return u.id();
}


}

