/*
	Copyright (C) 2020 - 2024
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

#include "gui/dialogs/server_info_dialog.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(server_info)

server_info::server_info(const std::string& info, const std::string& announcements)
	: modal_dialog(window_id())
	, server_information_(info)
	, announcements_(announcements)
{
}

void server_info::pre_show(window& window)
{
   find_widget<label>(&window, "server_information", false).set_label(server_information_);
   find_widget<label>(&window, "announcements", false).set_label(announcements_);

   stacked_widget& pager = find_widget<stacked_widget>(&window, "tabs_container", false);
	pager.select_layer(0);

	listbox& tab_bar = find_widget<listbox>(&window, "tab_bar", false);

	VALIDATE(tab_bar.get_item_count() == pager.get_layer_count(), "Tab bar and container size mismatch");

	connect_signal_notify_modified(tab_bar, std::bind(&server_info::tab_switch_callback, this));
}

void server_info::tab_switch_callback()
{
	stacked_widget& pager = find_widget<stacked_widget>(get_window(), "tabs_container", false);
	listbox& tab_bar = find_widget<listbox>(get_window(), "tab_bar", false);

	pager.select_layer(std::max<int>(0, tab_bar.get_selected_row()));
}

}
