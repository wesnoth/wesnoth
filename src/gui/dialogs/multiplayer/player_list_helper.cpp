/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/dialogs/multiplayer/player_list_helper.hpp"

#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "preferences/preferences.hpp"

namespace gui2
{
player_list_helper::player_list_helper(window* window)
	: list_(window->find_widget<listbox>("player_list"))
{
	// add ourselves as the host
	widget_data data = {
		{ "player_type_icon", {{ "label", "misc/leader-crown.png~CROP(12, 1, 15, 15)"}}},
		{ "player_name",      {{ "label", prefs::get().login()}}}
	};
	list_.add_row(data);
	list_.select_row(0);
}

void player_list_helper::update_list(const config::const_child_itors& users)
{
	list_.clear();
	unsigned i = 0;

	for(const config& user : users) {
		widget_data data;
		widget_item item;

		const std::string name = user["name"];
		const bool is_you = name == prefs::get().login();

		std::string icon;
		if(user["host"].to_bool()) {
			icon = "misc/leader-crown.png~CROP(12, 1, 15, 15)";
		} else if(user["observer"].to_bool()) {
			icon = "misc/eye.png";
		} else if(is_you) {
			icon = "lobby/status-lobby-s.png";
		} else {
			icon = "lobby/status-lobby-n.png";
		}

		item["label"] = icon;
		data.emplace("player_type_icon", item);

		item["label"] = name;
		data.emplace("player_name", item);

		list_.add_row(data);

		if(is_you) {
			list_.select_row(i);
		}

		++i;
	}
}

} // namespace gui2
