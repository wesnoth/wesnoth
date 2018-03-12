/*
   Copyright (C) 2014 - 2018 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/theme_list.hpp"

#include "gui/auxiliary/find_widget.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "theme.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_theme_list
 *
 * == Theme list ==
 *
 * Dialog for selecting a GUI theme.
 *
 * @begin{table}{dialog_widgets}
 *
 * themes & & listbox & m &
 *         Listbox displaying user choices. $
 *
 * -name & & styled_widget & m &
 *         Widget which shows a theme item name. $
 *
 * -description & & styled_widget & m &
 *         Widget which shows a theme item description. $
 *
 * @end{table}
 */

REGISTER_DIALOG(theme_list)

theme_list::theme_list(const std::vector<theme_info>& themes, int selection)
	: index_(selection), themes_(themes)
{
}

void theme_list::pre_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "themes", false);
	window.keyboard_capture(&list);

	for(const auto & t : themes_)
	{
		std::map<std::string, string_map> data;
		string_map column;

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

void theme_list::post_show(window& window)
{
	if(get_retval() != retval::OK) {
		return;
	}

	listbox& list = find_widget<listbox>(&window, "themes", false);
	index_ = list.get_selected_row();
}
} // namespace dialogs
} // namespace gui2
