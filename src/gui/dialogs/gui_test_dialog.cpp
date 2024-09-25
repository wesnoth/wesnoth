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

#include "gui/dialogs/gui_test_dialog.hpp"

#include "gui/widgets/text_box.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(gui_test_dialog)

gui_test_dialog::gui_test_dialog()
	: modal_dialog(window_id())
{
}

void gui_test_dialog::pre_show()
{
}

void gui_test_dialog::post_show()
{
}

} // namespace gui2::dialogs
