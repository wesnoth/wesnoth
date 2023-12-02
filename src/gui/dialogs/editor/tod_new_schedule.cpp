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

/*  dialog that takes new schedule ID and name */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/editor/tod_new_schedule.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/text_box.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(tod_new_schedule);

tod_new_schedule::tod_new_schedule(std::string& schedule_id, std::string& schedule_name)
	: modal_dialog(window_id())
	, schedule_id_(schedule_id)
	, schedule_name_(schedule_name)
{
}

void tod_new_schedule::post_show(window& win)
{
	schedule_id_ = find_widget<text_box>(&win, "id_box", false).get_value();
	schedule_name_ = find_widget<text_box>(&win, "name_box", false).get_value();
}

}
