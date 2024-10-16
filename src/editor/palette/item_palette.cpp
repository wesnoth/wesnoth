/*
	Copyright (C) 2012 - 2024
	by Fabian Mueller <fabianmueller5@gmx.de>
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
 * Manage the item-palette in the editor.
 */

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/palette/item_palette.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "game_config_view.hpp"

#include <string>

namespace editor {

std::string item_palette::get_help_string() const
{
	return VGETTEXT("Left-click: Place item $item | Right-click to remove", {{ "item", selected_fg_item().name }});
}

void item_palette::setup(const game_config_view& cfg)
{
	for(const config& group : cfg.child_range("item_group")) {
		groups_.emplace_back(group);

		for(const config& item : group.child_range("item")) {
			item_map_.emplace(item["id"], overlay(item));
			group_map_[group["id"]].push_back(item["id"]);
			if(!group["core"].to_bool(false))
				non_core_items_.insert(item["id"]);
		}
	}

	select_fg_item("anvil");
	select_bg_item("altar");

	// Set the default group
	set_group("items");

	if(active_group().empty()) {
		ERR_ED << "No items found.";
	}
}

void item_palette::setup_item(
	const overlay& item,
	texture& base_image,
	texture& /*overlay_image*/,
	std::stringstream& tooltip_text)
{
	std::stringstream filename;
	filename << item.image;
	if(item.image.empty()) {
		filename << item.halo;
	}

	base_image = image::get_texture(filename.str());
	if(!base_image) {
		tooltip_text << "IMAGE NOT FOUND\n";
		ERR_ED << "image for item type: '" << filename.str() << "' not found";
		base_image = image::get_texture(game_config::images::missing);
		if(!base_image) {
			ERR_ED << "Placeholder image not found";
			return;
		}
	}

	tooltip_text << item.name;
}

item_palette::item_palette(editor_display &gui, editor_toolkit &toolkit)
//TODO avoid magic numbers
	:	editor_palette<overlay>(gui, 36, 4, toolkit)
{
}

const std::string& item_palette::get_id(const overlay& item)
{
	return item.id;
}

}
