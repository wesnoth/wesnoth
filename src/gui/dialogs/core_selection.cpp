/*
   Copyright (C) 2009 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_core_selection
 *
 * == Core selection ==
 *
 * This shows the dialog which allows the user to choose which core to
 * play.
 *
 * @begin{table}{dialog_widgets}
 *
 * core_list & & listbox & m &
 *         A listbox that contains all available cores. $
 *
 * -icon & & image & o &
 *         The icon for the core. $
 *
 * -name & & styled_widget & o &
 *         The name of the core. $
 *
 * core_details & & multi_page & m &
 *         A multi page widget that shows more details for the selected
 *         core. $
 *
 * -image & & image & o &
 *         The image for the core. $
 *
 * -description & & styled_widget & o &
 *         The description of the core. $
 *
 * @end{table}
 */

REGISTER_DIALOG(core_selection)

void core_selection::core_selected(window& window)
{
	const int selected_row
			= find_widget<listbox>(&window, "core_list", false)
					  .get_selected_row();

	multi_page& pages
			= find_widget<multi_page>(&window, "core_details", false);

	pages.select_page(selected_row);
}

void core_selection::pre_show(window& window)
{
	/***** Setup core list. *****/
	listbox& list = find_widget<listbox>(&window, "core_list", false);

	connect_signal_notify_modified(list, std::bind(&core_selection::core_selected, this, std::ref(window)));

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

	core_selected(window);
}

void core_selection::post_show(window& window)
{
	choice_ = find_widget<listbox>(&window, "core_list", false)
					  .get_selected_row();
}

} // namespace dialogs
} // namespace gui2
