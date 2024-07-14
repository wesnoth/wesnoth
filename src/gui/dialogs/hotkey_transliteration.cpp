/*
   Copyright (C) 2022 by Steve Cotton (octalot)
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/hotkey_transliteration.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "hotkey/hotkey_item.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(hotkey_transliteration)

hotkey_transliteration::hotkey_transliteration()
 : modal_dialog(window_id())
{
}

void hotkey_transliteration::pre_show(window& window)
{
	auto& list = find_widget<listbox>(&window, "list_transliterations", false);
	list.clear();

	for(const auto& t : hotkey::get_transliterations()) {
		std::map<std::string, utils::string_map> row_data;
		utils::string_map item;

		item["label"] = t.name;
		row_data.emplace("name", item);

		std::stringstream example;
		for(const auto& x : t.text) {
			example << x.first << x.second << " ";
		}
		item["label"] = example.str();
		row_data.emplace("example", item);

		list.add_row(row_data);
	}
}

void hotkey_transliteration::post_show(window& /*window*/)
{
}

} // namespace dialogs
