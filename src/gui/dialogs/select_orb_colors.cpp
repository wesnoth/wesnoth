/*
	Copyright (C) 2017 - 2024
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

#include "gui/dialogs/select_orb_colors.hpp"

#include "gui/auxiliary/iterator/iterator.hpp"
#include "gui/core/event/dispatcher.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "game_config.hpp"
#include "preferences/preferences.hpp"
#include <functional>

namespace gui2::dialogs
{
namespace
{
std::string get_orb_widget_prefix(const std::string& base_id)
{
	return "orb_" + base_id + "_";
}

} // namespace

REGISTER_DIALOG(select_orb_colors)

select_orb_colors::select_orb_colors()
	: modal_dialog(window_id())
	, show_unmoved_(prefs::get().show_unmoved_orb())
	, show_partial_(prefs::get().show_partial_orb())
	, show_disengaged_(prefs::get().show_disengaged_orb())
	, show_moved_(prefs::get().show_moved_orb())
	, show_ally_(prefs::get().show_ally_orb())
	, two_color_ally_(prefs::get().show_status_on_ally_orb())
	, show_enemy_(prefs::get().show_enemy_orb())
{
}

void select_orb_colors::pre_show()
{
	setup_orb_group("unmoved", show_unmoved_, prefs::get().unmoved_color());
	setup_orb_group_two_color("partial", show_partial_, show_disengaged_, prefs::get().partial_color());
	setup_orb_group("moved", show_moved_, prefs::get().moved_color());
	setup_orb_group_two_color("ally", show_ally_, two_color_ally_, prefs::get().allied_color());
	setup_orb_group("enemy", show_enemy_, prefs::get().enemy_color());

	connect_signal_mouse_left_click(
		find_widget<button>("orb_defaults"), std::bind(&select_orb_colors::reset_orb_callback, this));
}

void select_orb_colors::post_show()
{
	if(get_retval() != retval::OK) {
		return;
	}

	prefs::get().set_show_unmoved_orb(show_unmoved_);
	prefs::get().set_show_partial_orb(show_partial_);
	prefs::get().set_show_disengaged_orb(show_disengaged_);
	prefs::get().set_show_moved_orb(show_moved_);
	prefs::get().set_show_ally_orb(show_ally_);
	prefs::get().set_show_status_on_ally_orb(two_color_ally_);
	prefs::get().set_show_enemy_orb(show_enemy_);

	prefs::get().set_unmoved_color(groups_["unmoved"].get_active_member_value());
	prefs::get().set_partial_color(groups_["partial"].get_active_member_value());
	prefs::get().set_moved_color(groups_["moved"].get_active_member_value());
	prefs::get().set_allied_color(groups_["ally"].get_active_member_value());
	prefs::get().set_enemy_color(groups_["enemy"].get_active_member_value());
}

void select_orb_colors::setup_orb_toggle(const std::string& base_id, bool& shown)
{
	const std::string prefix = get_orb_widget_prefix(base_id);
	toggle_button& toggle = find_widget<toggle_button>(prefix + "show");
	toggle.set_value_bool(shown);

	connect_signal_mouse_left_click(toggle, std::bind(&select_orb_colors::toggle_orb_callback, this, std::ref(shown)));
}

void select_orb_colors::setup_orb_group(const std::string& base_id, bool& shown, const std::string& initial)
{
	setup_orb_toggle(base_id, shown);

	//
	// Set up the toggle group.
	//
	group<std::string>& group = groups_[base_id];

	// Grid containing each color option toggle.
	const std::string prefix = get_orb_widget_prefix(base_id);
	grid& selection = find_widget<grid>(prefix + "selection");

	for(iteration::bottom_up_iterator<true, false, true> iter(selection); !iter.at_end(); ++iter) {
		if(toggle_button* button = dynamic_cast<toggle_button*>(iter.get())) {
			const std::string& id = button->id();
			group.add_member(button, id.substr(prefix.size()));
		}
	}

	group.set_member_states(initial);
}

void select_orb_colors::setup_orb_group_two_color(const std::string& base_id, bool& shown, bool& two_color, const std::string& initial)
{
	setup_orb_group(base_id, shown, initial);

	const std::string prefix = get_orb_widget_prefix(base_id);
	toggle_button& toggle = find_widget<toggle_button>(prefix + "two_color");
	toggle.set_value_bool(two_color);

	connect_signal_mouse_left_click(toggle, std::bind(&select_orb_colors::toggle_orb_callback, this, std::ref(two_color)));
}

void select_orb_colors::reset_orb_toggle(const std::string& base_id, bool shown)
{
	const std::string prefix = get_orb_widget_prefix(base_id);

	toggle_button& toggle = find_widget<toggle_button>(prefix + "show");
	toggle.set_value_bool(shown);
}

void select_orb_colors::reset_orb_group(const std::string& base_id, bool shown, const std::string& initial)
{
	reset_orb_toggle(base_id, shown);
	groups_[base_id].set_member_states(initial);
}

void select_orb_colors::reset_orb_group_two_color(const std::string& base_id, bool shown, bool two_color, const std::string& initial)
{
	reset_orb_group(base_id, shown, initial);

	const std::string prefix = get_orb_widget_prefix(base_id);

	toggle_button& toggle = find_widget<toggle_button>(prefix + "two_color");
	toggle.set_value_bool(two_color);
}

void select_orb_colors::toggle_orb_callback(bool& storage)
{
	// The code for the two-color groups uses this for both the main setting and the two_color setting - if
	// you add any extra logic here, check that it's still also applicable to the two_color setting.
	storage = !storage;
}

void select_orb_colors::reset_orb_callback()
{
	show_unmoved_ = game_config::show_unmoved_orb;
	show_partial_ = game_config::show_partial_orb;
	show_disengaged_ = game_config::show_disengaged_orb;
	show_moved_ = game_config::show_moved_orb;
	show_ally_ = game_config::show_ally_orb;
	two_color_ally_ = game_config::show_status_on_ally_orb;
	show_enemy_ = game_config::show_enemy_orb;

	reset_orb_group("unmoved", show_unmoved_, game_config::colors::unmoved_orb_color);
	reset_orb_group_two_color("partial", show_partial_, show_disengaged_, game_config::colors::partial_orb_color);
	reset_orb_group("moved", show_moved_, game_config::colors::moved_orb_color);
	reset_orb_group_two_color("ally", show_ally_, two_color_ally_, game_config::colors::ally_orb_color);
	reset_orb_group("enemy", show_enemy_, game_config::colors::enemy_orb_color);
}

} // namespace dialogs
