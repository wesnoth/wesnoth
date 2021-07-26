/*
	Copyright (C) 2008 - 2021
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

#include <functional>

#include "gui/dialogs/addon/addon_auth.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/password_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(addon_auth)

addon_auth::addon_auth(config& cfg)
	: cfg_(cfg)
{

}

void addon_auth::pre_show(window& win)
{
	win.add_to_tab_order(find_widget<password_box>(&win, "password", false, true));
}

void addon_auth::post_show(window& win)
{
	if(get_retval() == gui2::retval::OK)
	{
		cfg_["passphrase"] = find_widget<password_box>(&win, "password", false).get_real_value();
	}
}

} // namespace dialogs
