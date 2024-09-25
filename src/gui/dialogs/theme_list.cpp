/*
	Copyright (C) 2014 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/dialogs/theme_list.hpp"

#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "theme.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(theme_list)

theme_list::theme_list(const std::vector<theme_info>& themes, int selection)
	: modal_dialog(window_id())
	, index_(selection)
	, themes_(themes)
{
}

void theme_list::pre_show()
{
	listbox& list = find_widget<listbox>("themes");
	keyboard_capture(&list);

	for(const auto & t : themes_)
	{
		widget_data data;
		widget_item column;

		std::string theme_name = t.name;
		if(theme_name.empty()) {
			theme_name = t.id;
		}

		column["label"] = theme_name;
		data.emplace("name", column);
		column["label"] = t.description;
		data.emplace("description", column);

		list.add_row(data);
	}

	if(index_ != -1 && static_cast<unsigned>(index_) < list.get_item_count()) {
		list.select_row(index_);
	}

	index_ = -1;
}

void theme_list::post_show()
{
	if(get_retval() != retval::OK) {
		return;
	}

	listbox& list = find_widget<listbox>("themes");
	index_ = list.get_selected_row();
}
} // namespace dialogs
