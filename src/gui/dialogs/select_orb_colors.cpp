/*
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

#include "select_orb_colors.hpp"

#include "gui/auxiliary/event/dispatcher.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/iterator/walker.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "preferences.hpp"
#include "game_config.hpp"

#include <boost/bind.hpp>

namespace gui2 {
REGISTER_DIALOG(select_orb_colors);

tselect_orb_colors::tselect_orb_colors()
	: show_unmoved_(preferences::show_unmoved_orb())
	, show_partial_(preferences::show_partial_orb())
	, show_moved_(preferences::show_moved_orb())
	, show_ally_(preferences::show_allied_orb())
	, show_enemy_(preferences::show_enemy_orb())
{
}

void tselect_orb_colors::pre_show(twindow& window)
{
	setup_orb_group("unmoved", show_unmoved_, preferences::unmoved_color(), window);
	setup_orb_group("partial", show_partial_, preferences::partial_color(), window);
	setup_orb_group("moved", show_moved_, preferences::moved_color(), window);
	setup_orb_group("ally", show_ally_, preferences::allied_color(), window);
	setup_orb_group("enemy", show_enemy_, preferences::enemy_color(), window);
	
	tbutton& reset = find_widget<tbutton>(&window, "orb_defaults", false);
	connect_signal_mouse_left_click(reset, boost::bind(
		&tselect_orb_colors::handle_reset_click,
		this, boost::ref(window)
	));
}

void tselect_orb_colors::post_show(twindow&)
{
	if(get_retval() == twindow::OK) {
		preferences::set_show_unmoved_orb(show_unmoved_);
		preferences::set_show_partial_orb(show_partial_);
		preferences::set_show_moved_orb(show_moved_);
		preferences::set_show_allied_orb(show_ally_);
		preferences::set_show_enemy_orb(show_enemy_);

		preferences::set_unmoved_color(groups_["unmoved"].get_active_member_value());
		preferences::set_partial_color(groups_["partial"].get_active_member_value());
		preferences::set_moved_color(groups_["moved"].get_active_member_value());
		preferences::set_allied_color(groups_["ally"].get_active_member_value());
		preferences::set_enemy_color(groups_["enemy"].get_active_member_value());
	}
}

void tselect_orb_colors::setup_orb_group(const std::string& base_id, bool& shown, const std::string& initial, twindow& window, bool connect)
{
	std::string prefix = "orb_" + base_id + "_";
	ttoggle_button& toggle = find_widget<ttoggle_button>(&window, prefix + "show", false);
	toggle.set_value_bool(shown);
	if(connect) {
		connect_signal_mouse_left_click(toggle, boost::bind(
			&tselect_orb_colors::handle_toggle_click,
			this,
			boost::ref(shown)
		));
	}

	tgrid& selection = find_widget<tgrid>(&window, prefix + "selection", false);
	tgroup<std::string>& group = groups_[base_id];

	using iterator::twalker_;
	twalker_* iter = selection.create_walker();
	while(!iter->at_end(twalker_::child)) {
		twidget* next = iter->get(twalker_::child);
		if(ttoggle_button* button = dynamic_cast<ttoggle_button*>(next)) {
			const std::string& id = button->id();
			group.add_member(button, id.substr(prefix.size()));
		}
		iter->next(twalker_::child);
	}
	group.set_member_states(initial);
}

void tselect_orb_colors::handle_toggle_click(bool& storage)
{
	storage = !storage;
}

void tselect_orb_colors::handle_reset_click(twindow& window)
{
	show_unmoved_ = game_config::show_unmoved_orb;
	show_partial_ = game_config::show_partial_orb;
	show_moved_ = game_config::show_moved_orb;
	show_ally_ = game_config::show_ally_orb;
	show_enemy_ = game_config::show_enemy_orb;
	
	setup_orb_group("unmoved", show_unmoved_, game_config::colors::unmoved_orb_color, window, false);
	setup_orb_group("partial", show_partial_, game_config::colors::partial_orb_color, window, false);
	setup_orb_group("moved", show_moved_, game_config::colors::moved_orb_color, window, false);
	setup_orb_group("ally", show_ally_, game_config::colors::ally_orb_color, window, false);
	setup_orb_group("enemy", show_enemy_, game_config::colors::enemy_orb_color, window, false);
}

}
