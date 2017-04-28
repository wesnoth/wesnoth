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

#include "gui/dialogs/editor/edit_side.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/settings.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_edit_side
 *
 * == Edit side ==
 *
 * Dialog for editing gamemap sides.
 *
 * @begin{table}{dialog_widgets}
 *
 * title & & label & m &
 *         Dialog title label. $
 *
 * id & & text_box & m &
 *         Input field for the id. $
 *
 * team_only_toggle & & toggle_button & m &
 *         Checkbox for whether to make the label visible to the player's team
 *         only or not. $
 *
 * @end{table}
 */

REGISTER_DIALOG(editor_edit_side)

editor_edit_side::editor_edit_side(editor::editor_team_info& info)
	: controller_(info.controller)
	, share_vision_(info.share_vision)
{
	register_label("side_number", true, std::to_string(info.side), true);

	register_text("team_name", true, info.id, true);
	register_text("user_team_name", true, info.name, true);

	register_integer("gold", true, info.gold);
	register_integer("income", true, info.income);

	register_integer("village_income", true, info.village_income);
	register_integer("village_support", true, info.village_support);

	register_bool("fog", true, info.fog);
	register_bool("shroud", true, info.shroud);

	register_bool("no_leader", true, info.no_leader);
	register_bool("hidden", true, info.hidden);
}

void editor_edit_side::pre_show(window& window)
{
	controller_group.add_member(&find_widget<toggle_button>(&window, "controller_human", false), team::CONTROLLER::HUMAN);
	controller_group.add_member(&find_widget<toggle_button>(&window, "controller_ai", false),    team::CONTROLLER::AI);
	controller_group.add_member(&find_widget<toggle_button>(&window, "controller_null", false),  team::CONTROLLER::EMPTY);

	controller_group.set_member_states(controller_);

	vision_group.add_member(&find_widget<toggle_button>(&window, "vision_all", false),    team::SHARE_VISION::ALL);
	vision_group.add_member(&find_widget<toggle_button>(&window, "vision_shroud", false), team::SHARE_VISION::SHROUD);
	vision_group.add_member(&find_widget<toggle_button>(&window, "vision_null", false),   team::SHARE_VISION::NONE);

	vision_group.set_member_states(share_vision_);
}

void editor_edit_side::post_show(window&)
{
	controller_ = controller_group.get_active_member_value();
	share_vision_ = vision_group.get_active_member_value();
}

} // namespace dialogs
} // namespace gui2
