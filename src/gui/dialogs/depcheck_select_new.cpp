/*
	Copyright (C) 2012 - 2024
	by Boldizsár Lipka <lipkab@zoho.com>
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

#include "gui/dialogs/depcheck_select_new.hpp"

#include "gui/widgets/window.hpp"
#include "gui/widgets/listbox.hpp"
#include "gettext.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(depcheck_select_new)

depcheck_select_new::depcheck_select_new(
		ng::depcheck::component_type name,
		const std::vector<std::string>& items)
	: modal_dialog(window_id())
	, items_(items)
	, result_(-1)
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

void depcheck_select_new::pre_show()
{
	listbox& items = find_widget<listbox>("itemlist");

	for(const auto & item : items_)
	{
		widget_data data;
		data["option"]["label"] = item;

		items.add_row(data);
	}

	items.select_row(0);
}

void depcheck_select_new::post_show()
{
	if(get_retval() == retval::OK) {
		listbox& items = find_widget<listbox>("itemlist");
		result_ = items.get_selected_row();
	}
}
} // namespace dialogs
