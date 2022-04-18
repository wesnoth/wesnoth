/*
	Copyright (C) 2009 - 2022
	by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/core_selection.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(core_selection)

void core_selection::core_selected() const
{
	const int selected_row
			= find_widget<listbox>(get_window(), "core_list", false)
					  .get_selected_row();

	multi_page& pages
			= find_widget<multi_page>(get_window(), "core_details", false);

	pages.select_page(selected_row);
}

void core_selection::pre_show(window& window)
{
	/***** Setup core list. *****/
	listbox& list = find_widget<listbox>(&window, "core_list", false);

	connect_signal_notify_modified(list, std::bind(&core_selection::core_selected, this));

	window.keyboard_capture(&list);

	/***** Setup core details. *****/
	multi_page& pages
			= find_widget<multi_page>(&window, "core_details", false);

	for(const auto & core : cores_)
	{
		/*** Add list item ***/
		string_map list_item;
		std::map<std::string, string_map> list_item_item;

		list_item["label"] = core["image"];
		list_item_item.emplace("image", list_item);

		list_item["label"] = core["name"];
		list_item_item.emplace("name", list_item);

		grid* grid = &list.add_row(list_item_item);
		assert(grid);

		/*** Add detail item ***/
		string_map detail_item;
		std::map<std::string, string_map> detail_page;

		detail_item["label"] = core["description"];
		detail_item["use_markup"] = "true";
		detail_page.emplace("description", detail_item);

		pages.add_page(detail_page);
	}
	list.select_row(choice_, true);

	core_selected();
}

void core_selection::post_show(window& window)
{
	choice_ = find_widget<listbox>(&window, "core_list", false)
					  .get_selected_row();
}

} // namespace dialogs
