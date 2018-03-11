/*
   Copyright (C) 2012 - 2018 by Fabian Mueller <fabianmueller5@gmx.de>
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
 * Manage the item-palette in the editor.
 */

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/palette/item_palette.hpp"
#include "gettext.hpp"

#include <string>

namespace editor {

std::string item_palette::get_help_string()
{
	return selected_fg_item().name;
}

void item_palette::setup(const config& cfg)
{
	for(const config& group : cfg.child_range("item_group")) {
		groups_.emplace_back(group);

		for(const config& item : group.child_range("item")) {
			item_map_.emplace(item["id"], overlay(item));
			group_map_[group["id"]].push_back(item["id"]);
			if(!group["core"].to_bool(false))
				non_core_items_.insert(item["id"]);
		}
		nmax_items_ = std::max<int>(nmax_items_, group_map_[group["id"]].size());
	}

	select_fg_item("anvil");
	select_bg_item("altar");

	// Set the default group
	set_group("items");

	if(active_group().empty()) {
		ERR_ED << "No items found." << std::endl;
	}
}

void item_palette::draw_item(const overlay& item, surface& image, std::stringstream& tooltip_text)
{
	std::stringstream filename;
	filename << item.image;
	if(item.image.empty()) {
		filename << item.halo;
	}

	image = image::get_image(filename.str());
	if(image == nullptr) {
		tooltip_text << "IMAGE NOT FOUND\n";
		ERR_ED << "image for item type: '" << filename.str() << "' not found" << std::endl;
		image = image::get_image(game_config::images::missing);
		if(image == nullptr) {
			ERR_ED << "Placeholder image not found" << std::endl;
			return;
		}
	}

	if(image->w != item_size_ || image->h != item_size_) {
		image.assign(scale_surface(image, item_size_, item_size_));
	}

	tooltip_text << item.name;
}

item_palette::item_palette(editor_display &gui, const config& cfg,
                           editor_toolkit &toolkit)
//TODO avoid magic numbers
	:	editor_palette<overlay>(gui, cfg, 36, 4, toolkit)
{
}

const std::string& item_palette::get_id(const overlay& item)
{
	return item.id;
}

}
