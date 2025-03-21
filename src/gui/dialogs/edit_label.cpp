/*
	Copyright (C) 2010 - 2025
	by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/edit_label.hpp"


namespace gui2::dialogs
{

REGISTER_DIALOG(edit_label)

edit_label::edit_label(std::string& label, bool& team_only)
	: modal_dialog(window_id())
{
	register_text("label", true, label, true);
	register_bool("team_only_toggle", true, team_only);
}
} // namespace dialogs
