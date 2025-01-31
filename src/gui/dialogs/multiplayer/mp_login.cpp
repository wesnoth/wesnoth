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

#include "gui/dialogs/multiplayer/mp_login.hpp"

#include "preferences/preferences.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/widgets/password_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(mp_login)

mp_login::mp_login(const std::string& host, const std::string& label, const bool focus_password)
	: modal_dialog(window_id())
	, host_(host)
	, focus_password_(focus_password)
{
	register_label("login_label", false, label);
	username_ = register_text("user_name", true,
		[]() {return prefs::get().login();},
		[](const std::string& v) {prefs::get().set_login(v);},
		!focus_password);

	register_bool("remember_password", false,
		[]() {return prefs::get().remember_password();},
		[](bool v) {prefs::get().set_remember_password(v);});
}

void mp_login::load_password()
{
	text_box& pwd = find_widget<text_box>("password");
	pwd.set_value(prefs::get().password(host_, username_->get_widget_value()));
}

void mp_login::save_password()
{
	password_box& pwd = find_widget<password_box>("password");
	prefs::get().set_password(host_, username_->get_widget_value(), pwd.get_real_value());
}

void mp_login::pre_show()
{
	text_box& login = find_widget<text_box>("user_name");
	login.connect_signal<event::RECEIVE_KEYBOARD_FOCUS>(std::bind(&mp_login::load_password, this));

	load_password();

	if(focus_password_) {
		keyboard_capture(find_widget<text_box>("password", false, true));
	}

	add_to_tab_order(&login);
	add_to_tab_order(find_widget<text_box>("password", false, true));
}

void mp_login::post_show() {
	if(get_retval() == retval::OK) {
		save_password();
	}
}

} // namespace dialogs
