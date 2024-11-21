/*
	Copyright (C) 2011 - 2024
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

#include "gui/dialogs/folder_create.hpp"

#include "gettext.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(folder_create)

folder_create::folder_create(std::string& folder_name)
	: modal_dialog(window_id())
	, bookmark_mode_(false)
{
	register_text("name", true, folder_name, true);
}

void folder_create::pre_show()
{
	if(bookmark_mode_) {
		find_widget<styled_widget>("title").set_label(_("New Bookmark"));
	}
}

} // namespace dialogs
