/*
	Copyright (C) 2011 - 2024
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

#include "gui/dialogs/modeless_dialog.hpp"

#include "gui/core/gui_definition.hpp" // get_window_builder
#include "video.hpp"

namespace gui2::dialogs
{

modeless_dialog::modeless_dialog(const std::string& window_id)
	: window(get_window_builder(window_id))
{
	widget::set_id(window_id);
}

modeless_dialog::~modeless_dialog()
{
}

void modeless_dialog::show(const bool allow_interaction, const unsigned /*auto_close_time*/)
{
	if(video::headless()) {
		return;
	}

	if(allow_interaction) {
		window::show_non_modal();
	} else {
		window::show_tooltip(/*auto_close_time*/);
	}
}

} // namespace dialogs
