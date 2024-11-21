/*
	Copyright (C) 2010 - 2024
	by Fabian Müller <fabianmueller5@gmx.de>
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

#include "gui/dialogs/editor/edit_scenario.hpp"

#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_edit_scenario)

editor_edit_scenario::editor_edit_scenario(
		std::string& id,
		std::string& name,
		std::string& description,
		int& turns,
		int& experience_modifier,
		bool& victory_when_enemies_defeated,
		bool& random_start_time)
	: modal_dialog(window_id())
{
	register_text("id", true, id, true);
	register_text("name", true, name, false);
	register_text("description", true, description, false);
	register_integer("turns", true, turns);
	register_integer("experience_modifier", true, experience_modifier);
	register_bool("victory_when_enemies_defeated",
				  true,
				  victory_when_enemies_defeated);
	register_bool("random_start_time", true, random_start_time);
}

void editor_edit_scenario::pre_show()
{
	add_to_tab_order(find_widget<text_box>("id", false, true));
	add_to_tab_order(find_widget<text_box>("name", false, true));
	add_to_tab_order(find_widget<text_box>("description", false, true));
}

} // namespace dialogs
