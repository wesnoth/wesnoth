/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/mp_login.hpp"

#include "game_preferences.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/password_box.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

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

tmp_login::tmp_login(const t_string& label,	const bool focus_password)
	: label_(label)
	, focus_password_(focus_password)
{
}

void tmp_login::pre_show(CVideo& /*video*/, twindow& window)
{
	ttext_box* username =
			find_widget<ttext_box>(&window, "user_name", false, true);
	username->set_value(preferences::login());

	tpassword_box* password =
			find_widget<tpassword_box>(&window, "password", false, true);
	password->set_value(preferences::password());

	window.keyboard_capture(focus_password_ ? password : username);

	if(tbutton* button = find_widget<tbutton>(
			&window, "password_reminder", false, false)) {

		button->set_retval(1);
	}

	if(tbutton* button = find_widget<tbutton>(
			&window, "change_username", false, false)) {

		button->set_retval(2);
	}

	// Needs to be a scroll_label since the text can get long and a normal
	// label can't wrap (at the moment).
	tcontrol* label =
		dynamic_cast<tscroll_label*>(window.find("login_label", false));
	if(label) label->set_label(label_);

	if(ttoggle_button* button = find_widget<ttoggle_button>(
			&window, "remember_password", false, false)) {

		button->set_value(preferences::remember_password());
	}
}

void tmp_login::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		if(ttoggle_button* button = find_widget<ttoggle_button>(
				&window, "remember_password", false, false)) {

			preferences::set_remember_password(button->get_value());
		}

		preferences::set_login(find_widget<ttext_box>(
				&window, "user_name", false).get_value());

		preferences::set_password(find_widget<tpassword_box>(
				&window, "password", false).get_real_value());
	}
}

} // namespace gui2

