/*
   Copyright (C) 2013 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/rename_unit.hpp"

#include "gui/widgets/settings.hpp"

namespace gui2 {
	
/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_rename_unit
 *
 * == Rename unit ==
 *
 * Dialog for renaming units in-game.
 *
 * @begin{table}{dialog_widgets}
 *
 * name & & text_box & m &
 *         Input field for the unit name. $
 *
 * @end{table}
 */

REGISTER_DIALOG(rename_unit)

trename_unit::trename_unit(std::string& name)
{
	register_text("name", true, name, true);
}

}
