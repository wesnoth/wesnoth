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

#include "gui/dialogs/editor/edit_side.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/widgets/toggle_button.hpp"


namespace gui2::dialogs
{

REGISTER_DIALOG(editor_edit_side)

editor_edit_side::editor_edit_side(editor::editor_team_info& info)
	: modal_dialog(window_id())
	, controller_(info.controller)
	, share_vision_(info.share_vision)
{
	register_label("side_number", true, std::to_string(info.side), true);

	register_text("team_name", true, info.id, true);
	register_text("user_team_name", true, info.name, false);
	register_text("recruit_list", true, info.recruit_list, false);

	register_integer("gold", true, info.gold);
	register_integer("income", true, info.income);

	register_integer("village_income", true, info.village_income);
	register_integer("village_support", true, info.village_support);

	register_bool("fog", true, info.fog);
	register_bool("shroud", true, info.shroud);

	register_bool("no_leader", true, info.no_leader);
	register_bool("hidden", true, info.hidden);
}

void editor_edit_side::pre_show()
{
	controller_group.add_member(find_widget<toggle_button>("controller_human", false, true), side_controller::type::human);
	controller_group.add_member(find_widget<toggle_button>("controller_ai", false, true),    side_controller::type::ai);
	controller_group.add_member(find_widget<toggle_button>("controller_null", false, true),  side_controller::type::none);

	controller_group.set_member_states(controller_);

	vision_group.add_member(find_widget<toggle_button>("vision_all", false, true),    team_shared_vision::type::all);
	vision_group.add_member(find_widget<toggle_button>("vision_shroud", false, true), team_shared_vision::type::shroud);
	vision_group.add_member(find_widget<toggle_button>("vision_null", false, true),   team_shared_vision::type::none);

	vision_group.set_member_states(share_vision_);

	add_to_tab_order(find_widget<text_box>("team_name", false, true));
	add_to_tab_order(find_widget<text_box>("user_team_name", false, true));
	add_to_tab_order(find_widget<text_box>("recruit_list", false, true));
}

void editor_edit_side::post_show()
{
	controller_ = controller_group.get_active_member_value();
	share_vision_ = vision_group.get_active_member_value();
}

} // namespace dialogs
