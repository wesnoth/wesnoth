/*
	Copyright (C) 2023 - 2024
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

#include "gui/dialogs/prompt.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/text_box.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(prompt)

prompt::prompt(std::string& value)
	: modal_dialog(window_id())
	, value_(value)
{
	find_widget<text_box>(get_window(), "prompt_box", false).set_value(value);
}

void prompt::pre_show(window& win)
{
	text_box* prompt_box = find_widget<text_box>(&win, "prompt_box", false, true);
	win.keyboard_capture(prompt_box);
}

void prompt::post_show(window& win)
{
	value_ = find_widget<text_box>(&win, "prompt_box", false).get_value();
}

} // namespace gui2::dialogs
