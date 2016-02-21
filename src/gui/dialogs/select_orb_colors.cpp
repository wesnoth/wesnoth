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
#include "gui/auxiliary/find_widget.tpp"
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
	, unmoved_(preferences::unmoved_color())
	, partial_(preferences::partial_color())
	, moved_(preferences::moved_color())
	, ally_(preferences::allied_color())
	, enemy_(preferences::enemy_color())
{
}

void tselect_orb_colors::pre_show(CVideo&, twindow& window)
{
	setup_orb_group("unmoved", show_unmoved_, unmoved_, window);
	setup_orb_group("partial", show_partial_, partial_, window);
	setup_orb_group("moved", show_moved_, moved_, window);
	setup_orb_group("ally", show_ally_, ally_, window);
	setup_orb_group("enemy", show_enemy_, enemy_, window);
	
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

		preferences::set_unmoved_color(unmoved_);
		preferences::set_partial_color(partial_);
		preferences::set_moved_color(moved_);
		preferences::set_allied_color(ally_);
		preferences::set_enemy_color(enemy_);
	}
}

void tselect_orb_colors::setup_orb_group(const std::string& base_id, bool& shown, std::string& color, twindow& window, bool connect)
{
	ttoggle_button& toggle = find_widget<ttoggle_button>(&window, "orb_" + base_id + "_show", false);
	toggle.set_value_bool(shown);
	if(connect) {
		connect_signal_mouse_left_click(toggle, boost::bind(
			&tselect_orb_colors::handle_toggle_click,
			this,
			boost::ref(shown)
		));
	}

	tgrid& selection = find_widget<tgrid>(&window, "orb_" + base_id + "_selection", false);
	std::vector<ttoggle_button*>& group = groups_[base_id];

	using iterator::twalker_;
	twalker_* iter = selection.create_walker();
	while(!iter->at_end(twalker_::child)) {
		twidget* next = iter->get(twalker_::child);
		if(ttoggle_button* button = dynamic_cast<ttoggle_button*>(next)) {
			group.push_back(button);
			if(button->id().rfind("_" + color) != std::string::npos) {
				button->set_value_bool(true);
			} else {
				button->set_value_bool(false);
			}
			if(connect) {
				connect_signal_mouse_left_click(*button, boost::bind(
					&tselect_orb_colors::handle_orb_click,
					this,
					button,
					boost::ref(group),
					boost::ref(color)
				));
			}
		}
		iter->next(twalker_::child);
	}
}

void tselect_orb_colors::handle_orb_click(ttoggle_button* clicked, const std::vector<ttoggle_button*>& group, std::string& storage)
{
	int split = clicked->id().find_last_of('_');
	storage = clicked->id().substr(split + 1);
	
	FOREACH(const AUTO& button, group) {
		button->set_value_bool(false);
	}
	
	clicked->set_value_bool(true);
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
	
	unmoved_ = game_config::colors::unmoved_orb_color;
	partial_ = game_config::colors::partial_orb_color;
	moved_ = game_config::colors::moved_orb_color;
	ally_ = game_config::colors::ally_orb_color;
	enemy_ = game_config::colors::enemy_orb_color;
	
	setup_orb_group("unmoved", show_unmoved_, unmoved_, window, false);
	setup_orb_group("partial", show_partial_, partial_, window, false);
	setup_orb_group("moved", show_moved_, moved_, window, false);
	setup_orb_group("ally", show_ally_, ally_, window, false);
	setup_orb_group("enemy", show_enemy_, enemy_, window, false);
}

}