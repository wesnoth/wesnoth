/*
	Copyright (C) 2011 - 2021
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

#include "gui/dialogs/folder_create.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(folder_create)

folder_create::folder_create(std::string& folder_name)
	: bookmark_mode_(false)
{
	register_text("name", true, folder_name, true);
}

void folder_create::pre_show(window& window)
{
	if(bookmark_mode_) {
		find_widget<styled_widget>(&window, "title", false).set_label(_("New Bookmark"));
	}
}

} // namespace dialogs
