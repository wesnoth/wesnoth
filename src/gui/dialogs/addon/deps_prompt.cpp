/* $Id$ */
/*
   Copyright (C) 2012 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/addon/deps_prompt.hpp"

#include "gui/dialogs/addon/description.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/grid.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	#include "gui/widgets/list.hpp"
#else
	#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

namespace gui2 {

REGISTER_DIALOG(addon_deps_prompt)

void taddon_deps_prompt::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "addons_list", false);
	window.keyboard_capture(&list);

	std::map<std::string, string_map> data;

	for(std::vector<addon_info>::const_iterator i = deps_.begin(); i != deps_.end(); ++i) {

		data["icon"]["label"] = i->display_icon();
		data["title"]["label"] = i->display_title();

		list.add_row(data);

		tgrid* const g = list.get_row_grid(list.get_item_count() - 1);
		tbutton* const desc_button = dynamic_cast<tbutton*>(g->find("description", false));

		if(desc_button) {
			connect_signal_mouse_left_click(*desc_button, boost::bind(
				&taddon_deps_prompt::description_button_callback,
				this, boost::ref(window), boost::ref(*i)));
		}
	}
}

void taddon_deps_prompt::description_button_callback(twindow& window, const addon_info& addon)
{
	gui2::taddon_description::display(addon, window.video());
}

} // end namespace gui2
