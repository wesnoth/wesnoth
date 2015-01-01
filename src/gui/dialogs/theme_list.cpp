/*
   Copyright (C) 2014 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/auxiliary/find_widget.tpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "theme.hpp"
#include "utils/foreach.tpp"

namespace gui2
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
 * -name & & control & m &
 *         Widget which shows a theme item name. $
 *
 * -description & & control & m &
 *         Widget which shows a theme item description. $
 *
 * @end{table}
 */

REGISTER_DIALOG(theme_list)

ttheme_list::ttheme_list(const std::vector<theme_info>& themes, int selection)
	: index_(selection), themes_(themes)
{
}

void ttheme_list::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "themes", false);
	window.keyboard_capture(&list);

	FOREACH(const AUTO & t, themes_)
	{
		std::map<std::string, string_map> data;
		string_map column;

		std::string theme_name = t.name;
		if(theme_name.empty()) {
			theme_name = t.id;
		}

		column["label"] = theme_name;
		data.insert(std::make_pair("name", column));
		column["label"] = t.description;
		data.insert(std::make_pair("description", column));

		list.add_row(data);
	}

	if(index_ != -1 && static_cast<unsigned>(index_) < list.get_item_count()) {
		list.select_row(index_);
	}

	index_ = -1;
}

void ttheme_list::post_show(twindow& window)
{
	if(get_retval() != twindow::OK) {
		return;
	}

	tlistbox& list = find_widget<tlistbox>(&window, "themes", false);
	index_ = list.get_selected_row();
}
}
