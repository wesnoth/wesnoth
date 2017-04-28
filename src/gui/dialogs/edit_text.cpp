/*
   Copyright (C) 2013 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/edit_text.hpp"

#include "gui/widgets/settings.hpp"

namespace gui2
{
namespace dialogs
{

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

REGISTER_DIALOG(edit_text)

//TODO: add  a way to disallow certain chracters (like spaces or ")
edit_text::edit_text(const std::string& title,
					   const std::string& label,
					   std::string& text)
{
	register_label("title", true, title, true);
	register_label("label", true, label, true);
	register_text("text", true, text, true);
}
} // namespace dialogs
} // namespace gui2
