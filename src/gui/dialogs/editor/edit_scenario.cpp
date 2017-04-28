/*
   Copyright (C) 2010 - 2017 by Fabian Müller <fabianmueller5@gmx.de>
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

#include "gui/dialogs/editor/edit_scenario.hpp"

#include "gui/widgets/settings.hpp"

namespace gui2
{
namespace dialogs
{

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

REGISTER_DIALOG(editor_edit_scenario)

editor_edit_scenario::editor_edit_scenario(
		std::string& id,
		std::string& name,
		std::string& description,
		int& turns,
		int& experience_modifier,
		bool& victory_when_enemies_defeated,
		bool& random_start_time)
{
	register_text("id", true, id, true);
	register_text("name", true, name, true);
	register_text("description", true, description, true);
	register_integer("turns", true, turns);
	register_integer("experience_modifier", true, experience_modifier);
	register_bool("victory_when_enemies_defeated",
				  true,
				  victory_when_enemies_defeated);
	register_bool("random_start_time", true, random_start_time);
}
} // namespace dialogs
} // namespace gui2
