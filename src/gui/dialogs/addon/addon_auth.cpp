/*
	Copyright (C) 2008 - 2024
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
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/password_box.hpp"
#include "gui/widgets/window.hpp"
#include "preferences/credentials.hpp"
#include "serialization/string_utils.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(addon_auth)

addon_auth::addon_auth(config& cfg)
	: modal_dialog(window_id())
	, cfg_(cfg)
{
	register_bool("remember_password", false,
		&preferences::remember_password,
		&preferences::set_remember_password);
}

void addon_auth::pre_show(window& win)
{
	text_box* pwd = find_widget<text_box>(&win, "password", false, true);
	win.add_to_tab_order(pwd);
	pwd->set_value(cfg_["passphrase"].str(""));

	std::vector<config> content_list;
	content_list.emplace_back("label", cfg_["author"].str(""));

	for(const auto& author : utils::split(cfg_["secondary_authors"].str(""), ',')) {
		content_list.emplace_back("label", author);
	}
	find_widget<menu_button>(&win, "choose_uploader", false).set_values(content_list);
}

void addon_auth::post_show(window& win)
{
	if(get_retval() == gui2::retval::OK)
	{
		cfg_["passphrase"] = find_widget<password_box>(&win, "password", false).get_real_value();
		cfg_["uploader"] = find_widget<menu_button>(&win, "choose_uploader", false).get_value_string();
	}
}

} // namespace dialogs
