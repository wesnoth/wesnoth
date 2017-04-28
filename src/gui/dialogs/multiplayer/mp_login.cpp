/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/multiplayer/mp_login.hpp"

#include "game_preferences.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/password_box.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_login
 *
 * == Multiplayer connect ==
 *
 * This shows the dialog to log in to the MP server
 *
 * @begin{table}{dialog_widgets}
 *
 * user_name & & text_box & m &
 *         The login user name. $
 *
 * password & & text_box & m &
 *         The password. $
 *
 * remember_password & & toggle_button & o &
 *         A toggle button to offer to remember the password in the
 *         preferences. $
 *
 * password_reminder & & button & o &
 *         Request a password reminder. $
 *
 * change_username & & button & o &
 *         Use a different username. $
 *
 * login_label & & button & o &
 *         Displays the information received from the server. $
 *
 * @end{table}
 */

REGISTER_DIALOG(mp_login)

mp_login::mp_login(const std::string& label, const bool focus_password)
{
	register_label("login_label", false, label);
	register_text("user_name", true,
		&preferences::login,
		&preferences::set_login,
		!focus_password);

	register_text("password",  true,
		&preferences::password,
		nullptr /* The password box returns '*' as value. */,
		focus_password);

	register_bool("remember_password", false,
		&preferences::remember_password,
		&preferences::set_remember_password);
}

void mp_login::pre_show(window& win)
{
	if(button* btn = find_widget<button>(&win, "password_reminder", false, false)) {

		btn->set_retval(1);
	}

	if(button* btn = find_widget<button>(&win, "change_username", false, false)) {

		btn->set_retval(2);
	}
}

void mp_login::post_show(window& win)
{
	if(get_retval() == window::OK) {
		preferences::set_password(find_widget<password_box>(&win, "password", false).get_real_value());
	}
}

} // namespace dialogs
} // namespace gui2
