/* $Id$ */
/*
   Copyright (C) 2011 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "foreach.hpp"
#include "gui/widgets/grid.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	#include "gui/widgets/list.hpp"
#else
	#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include <algorithm>

namespace {
	std::string make_addon_name(const std::string& id)
	{
		std::string r(id);
		std::replace(r.begin(), r.end(), '_', ' ');
		return r;
	}

}

namespace gui2 {

REGISTER_DIALOG(addon_uninstall_list)

void taddon_uninstall_list::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "addons_list", false);
	window.keyboard_capture(&list);

	this->names_.clear();
	this->selections_.clear();

	foreach(const std::string& id, this->ids_) {
		this->names_.push_back(make_addon_name(id));
		this->selections_[id] = false;

		std::map<std::string, string_map> data;
		string_map column;

		column["label"] = this->names_.back();
		data.insert(std::make_pair("name", column));
		list.add_row(data);
	}
}

void taddon_uninstall_list::post_show(twindow& window)
{
	const tlistbox& list = find_widget<tlistbox>(&window, "addons_list", false);
	const unsigned rows = list.get_item_count();

	assert(rows == this->ids_.size() && rows == this->names_.size());

	if(!rows || get_retval() != twindow::OK) {
		return;
	}

	for(unsigned k = 0; k < rows; ++k) {
		tgrid const* g = list.get_row_grid(k);
		const ttoggle_button& checkbox = find_widget<const ttoggle_button>(g, "checkbox", false);
		this->selections_[this->ids_[k]] = checkbox.get_value();
	}

}

std::vector<std::string> taddon_uninstall_list::selected_addons() const
{
	std::vector<std::string> retv;

	typedef std::map<std::string, bool> selections_map_type;
	foreach(const selections_map_type::value_type& entry, this->selections_) {
		if(entry.second) {
			retv.push_back(entry.first);
		}
	}

	return retv;
}


} // namespace gui2
