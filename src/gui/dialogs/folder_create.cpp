/*
   Copyright (C) 2011 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_folder_create
 *
 * == Folder Create ==
 *
 * Dialog for providing the name of a new folder or bookmark to create.
 * Used by the file dialog.
 *
 * @begin{table}{dialog_widgets}
 *
 * title & & styled_widget & m &
 *         Label with the dialog caption. Changed in bookmark mode. $
 * name & & text_box & m &
 *         Input field for the new folder/bookmark name. $
 *
 * @end{table}
 */

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
} // namespace gui2
