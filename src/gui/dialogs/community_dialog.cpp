/*
	Copyright (C) 2003 - 2024
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

#include "gui/dialogs/community_dialog.hpp"

#include "desktop/open.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(community_dialog)

community_dialog::community_dialog()
	: modal_dialog(window_id())
{
}

void community_dialog::pre_show(window& window)
{
	connect_signal_mouse_left_click(find_widget<button>(&window, "forums", false), std::bind(&desktop::open_object, "https://forums.wesnoth.org/"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "discord", false), std::bind(&desktop::open_object, "https://discord.gg/battleforwesnoth"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "irc", false), std::bind(&desktop::open_object, "https://web.libera.chat/#wesnoth"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "steam", false), std::bind(&desktop::open_object, "https://steamcommunity.com/app/599390/discussions/"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "reddit", false), std::bind(&desktop::open_object, "https://www.reddit.com/r/wesnoth/"));
	connect_signal_mouse_left_click(find_widget<button>(&window, "donate", false), std::bind(&desktop::open_object, "https://www.spi-inc.org/projects/wesnoth/"));
}

void community_dialog::post_show(window&)
{

}

} // namespace gui2::dialogs
