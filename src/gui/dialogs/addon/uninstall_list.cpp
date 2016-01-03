/*
   Copyright (C) 2011 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/addon/uninstall_list.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/grid.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "utils/foreach.tpp"

#include <algorithm>

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_uninstall_list
 *
 * == Add-on uninstall list ==
 *
 * Dialog with a checkbox list for choosing installed add-ons to remove.
 *
 * @begin{table}{dialog_widgets}
 *
 * addons_list & & listbox & m &
 *     A listbox containing add-on selection entries. $
 *
 * -checkbox & & toggle_button & m &
 *     A toggle button allowing the user to mark/unmark the add-on. $
 *
 * -name & & control & o &
 *     The name of the add-on. $
 *
 * @end{table}
 */

REGISTER_DIALOG(addon_uninstall_list)

void taddon_uninstall_list::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "addons_list", false);
	window.keyboard_capture(&list);

	this->selections_.clear();

	FOREACH(const AUTO & entry, titles_map_)
	{
		const std::string& id = entry.first;
		const std::string& title = entry.second;

		this->ids_.push_back(id);
		this->selections_[id] = false;

		std::map<std::string, string_map> data;
		string_map column;

		column["label"] = title;
		data.insert(std::make_pair("name", column));
		list.add_row(data);
	}
}

void taddon_uninstall_list::post_show(twindow& window)
{
	const tlistbox& list = find_widget<tlistbox>(&window, "addons_list", false);
	const unsigned rows = list.get_item_count();

	assert(rows == this->ids_.size() && rows == this->titles_map_.size());

	if(!rows || get_retval() != twindow::OK) {
		return;
	}

	for(unsigned k = 0; k < rows; ++k) {
		tgrid const* g = list.get_row_grid(k);
		const ttoggle_button& checkbox
				= find_widget<const ttoggle_button>(g, "checkbox", false);
		this->selections_[this->ids_[k]] = checkbox.get_value_bool();
	}
}

std::vector<std::string> taddon_uninstall_list::selected_addons() const
{
	std::vector<std::string> retv;

	FOREACH(const AUTO & entry, selections_)
	{
		if(entry.second) {
			retv.push_back(entry.first);
		}
	}

	return retv;
}


} // namespace gui2
