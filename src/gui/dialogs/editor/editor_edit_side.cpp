/*
   Copyright (C) 2010 - 2013 by Fabian Müller <fabianmueller5@gmx.de>
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

#include "gui/dialogs/editor/editor_edit_side.hpp"

#include "gui/widgets/settings.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_edit_scenario
 *
 * == Edit scenario ==
 *
 * Dialog for editing gamemap scenarios.
 *
 * @begin{table}{dialog_widgets}
 *
 * title & & label & m &
 *         Dialog title label. $
 *
 * label & & text_box & m &
 *         Input field for the map label. $
 *
 * team_only_toggle & & toggle_button & m &
 *         Checkbox for whether to make the label visible to the player's team
 *         only or not. $
 *
 * @end{table}
 */

REGISTER_DIALOG(editor_edit_side)

teditor_edit_side::teditor_edit_side(std::string& id, std::string& name, int& gold, int& income,
		bool& fog, bool& share_view, bool& shroud, bool& share_maps)
{
	register_text("id", true, id, true);
	register_text("name", true, name, true);

	register_integer("gold", true, gold);
	register_integer("income", true, income);

	register_bool("fog", true, fog);
	register_bool("share_view", true, share_view);

	register_bool("shroud", true, shroud);
	register_bool("share_maps", true, share_maps);
}

}

