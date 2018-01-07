/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/multiplayer/player_list_helper.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "preferences/credentials.hpp"

namespace gui2
{
player_list_helper::player_list_helper(window* window)
	: list_(find_widget<listbox>(window, "player_list", false))
{
	// add ourselves as the host
	std::map<std::string, string_map> data = {
		{ "player_type_icon", {{ "label", "misc/leader-crown.png~CROP(12, 1, 15, 15)"}}},
		{ "player_name",      {{ "label", preferences::login()}}}
	};
	list_.add_row(data);
	list_.select_row(0);
}

void player_list_helper::update_list(const config::const_child_itors& users)
{
	list_.clear();
	unsigned i = 0;

	for(const config& user : users) {
		std::map<std::string, string_map> data;
		string_map item;

		const std::string name = user["name"];
		const bool is_you = name == preferences::login();

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
