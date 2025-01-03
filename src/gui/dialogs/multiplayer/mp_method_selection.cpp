/*
	Copyright (C) 2008 - 2024
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

#include "gui/dialogs/multiplayer/mp_method_selection.hpp"

#include "desktop/open.hpp"
#include "game_initialization/multiplayer.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/text_box.hpp"
#include "preferences/preferences.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(mp_method_selection)

/** Link to the wesnoth forum account registration page */
static const std::string forum_registration_url = "https://forums.wesnoth.org/ucp.php?mode=register";

void mp_method_selection::pre_show()
{
	text_box* user_widget = find_widget<text_box>("user_name", false, true);
	user_widget->set_value(prefs::get().login());
	user_widget->set_maximum_length(mp::max_login_size);

	listbox* list = find_widget<listbox>("method_list", false, true);
	list->select_row(prefs::get().mp_connect_type());

	add_to_tab_order(list);
	add_to_tab_order(user_widget);

	connect_signal_mouse_left_click(find_widget<button>("register"),
		std::bind(&desktop::open_object, forum_registration_url));
}

void mp_method_selection::post_show()
{
	prefs::get().set_mp_connect_type(find_widget<listbox>("method_list").get_selected_row());

	if(get_retval() == retval::OK) {
		text_box& user_widget = find_widget<text_box>("user_name");
		user_widget.save_to_history();
		prefs::get().set_login(user_widget.get_value());
	}
}

mp_method_selection::choice mp_method_selection::get_choice() const
{
	return static_cast<choice>(prefs::get().mp_connect_type());
}

} // namespace dialogs
