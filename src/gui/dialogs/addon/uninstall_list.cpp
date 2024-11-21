/*
	Copyright (C) 2011 - 2024
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

#include "gui/dialogs/addon/uninstall_list.hpp"

#include "gui/widgets/grid.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"


namespace gui2::dialogs
{

REGISTER_DIALOG(addon_uninstall_list)

void addon_uninstall_list::pre_show()
{
	listbox& list = find_widget<listbox>("addons_list");
	keyboard_capture(&list);
	selections_.clear();

	for(const auto & entry : titles_map_)
	{
		const std::string& id = entry.first;
		const std::string& title = entry.second;

		ids_.push_back(id);
		selections_[id] = false;

		widget_data data;
		widget_item column;

		column["label"] = title;
		data.emplace("name", column);

		list.add_row(data);
	}
}

void addon_uninstall_list::post_show()
{
	const listbox& list = find_widget<listbox>("addons_list");
	const unsigned rows = list.get_item_count();

	assert(rows == ids_.size() && rows == titles_map_.size());

	if(!rows || get_retval() != retval::OK) {
		return;
	}

	for(unsigned k = 0; k < rows; ++k) {
		const grid* g = list.get_row_grid(k);
		const toggle_button& checkbox
				= g->find_widget<const toggle_button>("checkbox");
		selections_[ids_[k]] = checkbox.get_value_bool();
	}
}

std::vector<std::string> addon_uninstall_list::selected_addons() const
{
	std::vector<std::string> retv;

	for(const auto & entry : selections_)
	{
		if(entry.second) {
			retv.push_back(entry.first);
		}
	}

	return retv;
}

} // namespace dialogs
