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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2
{
class field_text;

namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the dialog to log in to the MP server.
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 * user_name         | text_box      |yes      |The login user name.
 * password          | text_box      |yes      |The password.
 * remember_password | toggle_button |no       |A toggle button to offer to remember the password in the preferences.
 * password_reminder | @ref button   |no       |Request a password reminder.
 * change_username   | @ref button   |no       |Use a different username.
 * login_label       | @ref button   |no       |Displays the information received from the server.
 */
class mp_login : public modal_dialog
{
public:
	mp_login(const std::string& host, const std::string& label, const bool focus_password);

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

	void load_password();
	void save_password();

	const std::string host_;
	field_text* username_;
	bool focus_password_;
};

} // namespace dialogs
} // namespace gui2
