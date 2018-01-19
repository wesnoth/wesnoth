/*
   Copyright (C) 2012 - 2018 by Boldizsár Lipka <lipkab@zoho.com>
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

#include "gui/dialogs/depcheck_select_new.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gettext.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_depcheck_select_new
 *
 * == SP/MP Dependency Check: Select New ==
 *
 * Offers a list of compatible items if a currently selected one is
 * incompatible. Currently used for switching era or map.
 *
 * @begin{table}{dialog_widgets}
 *
 * message & & label & m &
 * 		displays the details of the required changes $
 *
 * itemlist & & listbox & m &
 * 		displays the available items to choose from $
 *
 * cancel & & button & m &
 * 		refuse to apply any changes $
 *
 * ok & & button & m &
 * 		select the chosen item $
 *
 * @end{table}
 *
 */

REGISTER_DIALOG(depcheck_select_new)

depcheck_select_new::depcheck_select_new(
		ng::depcheck::component_type name,
		const std::vector<std::string>& items)
	: items_(items), result_(-1)
{

	std::string message;

	switch(name) {
		case ng::depcheck::SCENARIO:
			message = _("The currently chosen scenario "
						"is not compatible with your setup."
						"\nPlease select a compatible one.");
			break;
		case ng::depcheck::ERA:
			message = _("The currently chosen era "
						"is not compatible with your setup."
						"\nPlease select a compatible one.");
			break;
		case ng::depcheck::MODIFICATION:
			// currently this can't happen, but be prepared for anything...
			message = _("The currently chosen modification "
						"is not compatible with your setup."
						"\nPlease select a compatible one.");
	}

	register_label("message", false, message);
}

void depcheck_select_new::pre_show(window& window)
{
	listbox& items = find_widget<listbox>(&window, "itemlist", false);

	for(const auto & item : items_)
	{
		std::map<std::string, string_map> data;
		data["option"]["label"] = item;

		items.add_row(data);
	}

	items.select_row(0);
}

void depcheck_select_new::post_show(window& window)
{
	if(get_retval() == window::OK) {
		listbox& items = find_widget<listbox>(&window, "itemlist", false);
		result_ = items.get_selected_row();
	}
}
} // namespace dialogs
} // namespace gui2
