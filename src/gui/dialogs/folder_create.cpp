/*
   Copyright (C) 2011 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/widgets/settings.hpp"

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_folder_create
 *
 * == Folder Create ==
 *
 * Dialog for providing the name of a new folder to create.
 * Used by the file dialog.
 *
 * @begin{table}{dialog_widgets}
 *
 * name & & text_box & m &
 *         Input field for the new folder name. $
 *
 * @end{table}
 */

REGISTER_DIALOG(folder_create)

tfolder_create::tfolder_create(std::string& folder_name)
{
	register_text("name", true, folder_name, true);
}
}
