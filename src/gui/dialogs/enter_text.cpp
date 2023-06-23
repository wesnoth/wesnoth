/*
	Copyright (C) 2023 - 2023
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

#include "gui/dialogs/enter_text.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/text_box.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(enter_text)

enter_text::enter_text(std::string& value)
    : modal_dialog(window_id())
    , value_(value)
{
	find_widget<text_box>(get_window(), "enter_text_box", false).set_value(value);
}

void enter_text::pre_show(window& win)
{
	text_box* enter_text_box = find_widget<text_box>(&win, "enter_text_box", false, true);
	win.keyboard_capture(enter_text_box);
}

void enter_text::post_show(window& win)
{
    value_ = find_widget<text_box>(&win, "enter_text_box", false).get_value();
}

}