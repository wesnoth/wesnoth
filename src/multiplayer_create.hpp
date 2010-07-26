/* $Id$ */
/*
   Copyright (C) 2007 - 2010
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef MULTIPLAYER_CREATE_HPP_INCLUDED
#define MULTIPLAYER_CREATE_HPP_INCLUDED

#include "mp_game_settings.hpp"
#include "multiplayer_ui.hpp"
#include "widgets/slider.hpp"
#include "widgets/label.hpp"
#include "widgets/combo.hpp"
#include "mapgen.hpp"
#include "tooltips.hpp"

namespace mp {

class create : public mp::ui
{
public:
	create(game_display& dist, const config& game_config, chat& c, config& gamelist);
	~create();

	mp_game_settings& get_parameters();
	int num_turns() const { return num_turns_; }

protected:
	virtual void layout_children(const SDL_Rect& rect);
	virtual void process_event();
	virtual void hide_children(bool hide=true);

private:
	void update_minimap();

	tooltips::manager tooltip_manager_;
	int map_selection_;
	int mp_countdown_init_time_;
	int mp_countdown_reservoir_time_;


	std::vector<std::string> user_maps_;
	std::vector<std::string> map_options_;

	/**
	 * Due to maps not available the index of the selected map and mp scenarios
	 * is not 1:1 so we use a lookup table.
	 */
	std::vector<size_t> map_index_;

	gui::menu maps_menu_;
	gui::slider turns_slider_;
	gui::label turns_label_;
	gui::button countdown_game_;
	gui::slider countdown_init_time_slider_;
	gui::label countdown_init_time_label_;
	gui::slider countdown_reservoir_time_slider_;
	gui::label countdown_reservoir_time_label_;
	gui::label countdown_turn_bonus_label_;
	gui::slider countdown_turn_bonus_slider_;
	gui::label countdown_action_bonus_label_;
	gui::slider countdown_action_bonus_slider_;
	gui::slider village_gold_slider_;
	gui::label village_gold_label_;
	gui::slider xp_modifier_slider_;
	gui::label xp_modifier_label_;

	gui::label name_entry_label_;
	gui::label num_players_label_;
	gui::label map_size_label_;
	gui::label era_label_;
	gui::label map_label_;

	gui::button use_map_settings_;
	gui::button random_start_time_;
	gui::button fog_game_;
	gui::button shroud_game_;
	gui::button observers_game_;
	gui::button cancel_game_;
	gui::button launch_game_;
	gui::button regenerate_map_;
	gui::button generator_settings_;
	gui::button password_button_;

	gui::combo era_combo_;
	gui::combo vision_combo_;
	gui::textbox name_entry_;

	util::scoped_ptr<surface_restorer> minimap_restorer_;
	SDL_Rect minimap_rect_;

	util::scoped_ptr<map_generator> generator_;

	int num_turns_;
	mp_game_settings parameters_;
};

} // end namespace mp

#endif

