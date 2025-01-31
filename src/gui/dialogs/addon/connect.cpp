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

#include "gui/dialogs/addon/connect.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/text_box.hpp"
#include "help/help.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(addon_connect)

addon_connect::addon_connect(std::string& host_name, bool allow_remove)
	: modal_dialog(window_id())
	, allow_remove_(allow_remove)
{
	register_text("host_name", false, host_name, true);
}

void addon_connect::help_button_callback()
{
	help::show_help("installing_addons");
}

void addon_connect::pre_show()
{
	find_widget<button>("remove_addons")
			.set_active(allow_remove_);

	connect_signal_mouse_left_click(
			find_widget<button>("show_help"),
			std::bind(&addon_connect::help_button_callback, this));
}

void addon_connect::post_show()
{
	if(get_retval() == retval::OK) {
		text_box& host_widget
				= find_widget<text_box>("host_name");

		host_widget.save_to_history();
	}
}

} // namespace dialogs
