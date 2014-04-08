/*
   Copyright (C) 2010 - 2014 by Fabian Müller <fabianmueller5@gmx.de>
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

#include "gui/dialogs/field.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

namespace gui2
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

teditor_edit_side::teditor_edit_side(int side,
									 std::string& id,
									 std::string& name,
									 int& gold,
									 int& income,
									 int& village_income,
									 int& village_support,
									 bool& fog,
									 bool& share_view,
									 bool& shroud,
									 bool& share_maps,
									 team::CONTROLLER& controller,
									 int controller_num,
									 bool& no_leader,
									 bool& hidden)
	: controller_(controller)
{
	std::stringstream side_stream;
	side_stream << side;
	register_label("side_number", true, side_stream.str(), true);

	register_text("team_name", true, id, true);
	register_text("user_team_name", true, name, true);

	register_integer("gold", true, gold);
	register_integer("income", true, income);

	register_integer("village_income", true, village_income);
	register_integer("village_support", true, village_support);

	register_integer("controller_number_player", true, controller_num);

	register_bool("fog", true, fog);
	register_bool("share_view", true, share_view);

	register_bool("shroud", true, shroud);
	register_bool("share_maps", true, share_maps);

	register_bool("no_leader", true, no_leader);
	register_bool("hidden", true, hidden);
}

void teditor_edit_side::pre_show(CVideo& /*video*/, twindow& window)
{
	register_controller_toggle(window, "human", team::HUMAN);
	register_controller_toggle(window, "ai", team::AI);
	register_controller_toggle(window, "null", team::EMPTY);
	register_controller_toggle(window, "number", team::CONTROLLER(-1));
}

void teditor_edit_side::register_controller_toggle(twindow& window,
												   const std::string& toggle_id,
												   team::CONTROLLER value)
{
	ttoggle_button* b = &find_widget<ttoggle_button>(
								 &window, "controller_" + toggle_id, false);

	b->set_value(value == controller_);
	connect_signal_mouse_left_click(
			*b,
			boost::bind(
					&teditor_edit_side::toggle_controller_callback, this, b));

	controller_tgroup_.push_back(std::make_pair(b, value));
}

void teditor_edit_side::toggle_controller_callback(ttoggle_button* active)
{
	FOREACH(const AUTO & e, controller_tgroup_)
	{
		ttoggle_button* const b = e.first;
		if(b == NULL) {
			continue;
		} else if(b == active && !b->get_value()) {
			b->set_value(true);
		} else if(b == active) {
			controller_ = e.second;
		} else if(b != active && b->get_value()) {
			b->set_value(false);
		}
	}
}
}
