/*
   Copyright (C) 2013 - 2016 Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MULTIPLAYER_CONFIGURE_HPP_INCLUDED
#define MULTIPLAYER_CONFIGURE_HPP_INCLUDED

#include "depcheck.hpp"
#include "mp_game_settings.hpp"
#include "multiplayer_ui.hpp"
#include "widgets/slider.hpp"
#include "widgets/scrollpane.hpp"
#include "widgets/combo.hpp"
#include "tooltips.hpp"
#include "mp_options.hpp"
#include "configure_engine.hpp"
#include <boost/scoped_ptr.hpp>

class saved_game;
namespace mp {

class configure : public mp::ui
{
public:
	///gives the user the option to adjust the passed saved_game
	///Call get_parameters to finalize;
	configure(CVideo& v, const config& game_config, chat& c, config& gamelist, saved_game& game, bool local_players_only);
	~configure();

	void get_parameters();
protected:
	virtual void layout_children(const SDL_Rect& rect);
	virtual void process_event();
	virtual void hide_children(bool hide=true);

private:
	//Settings that can be changed unledd wml forbids it
	struct nolock_settings
	{
		nolock_settings(CVideo& video);

		gui::slider turns_slider_;
		gui::label turns_label_;
		gui::slider village_gold_slider_;
		gui::label village_gold_label_;
		gui::slider village_support_slider_;
		gui::label village_support_label_;
		gui::slider xp_modifier_slider_;
		gui::label xp_modifier_label_;
		gui::label generic_label_;
		gui::button use_map_settings_;
		gui::button random_start_time_;
		gui::button fog_game_;
		gui::button shroud_game_;
	};
	bool local_players_only_;

	tooltips::manager tooltip_manager_;
	int mp_countdown_init_time_;
	int mp_countdown_reservoir_time_;

	gui::button countdown_game_;
	gui::slider countdown_init_time_slider_;
	gui::label countdown_init_time_label_;
	gui::slider countdown_reservoir_time_slider_;
	gui::label countdown_reservoir_time_label_;
	gui::label countdown_turn_bonus_label_;
	gui::slider countdown_turn_bonus_slider_;
	gui::label countdown_action_bonus_label_;
	gui::slider countdown_action_bonus_slider_;

	gui::label name_entry_label_;
	gui::button observers_game_;
	gui::button oos_debug_;
	gui::button shuffle_sides_;
	gui::label random_faction_mode_label_;
	gui::combo random_faction_mode_;

	gui::button cancel_game_;
	gui::button launch_game_;
	gui::button password_button_;

	gui::textbox name_entry_;

	gui::label entry_points_label_;
	gui::combo entry_points_combo_;

	gui::scrollpane options_pane_left_;
	gui::scrollpane options_pane_right_;

	std::vector<config const*> entry_points_;
	bool show_entry_points_;

	bool force_use_map_settings_check_;
	saved_game& state_;
	mp_game_settings& parameters_;
	ng::configure_engine engine_;
	options::manager options_manager_;

	boost::scoped_ptr<nolock_settings> nolock_settings_;
	struct process_event_data {
		bool launch, quit;

		process_event_data()
			: launch(false), quit(false)
		{}
		process_event_data(bool l, bool q)
			: launch(l), quit(q)
		{}
	};

	void process_event_impl(const process_event_data &);
	bool plugin_event_helper(const process_event_data &);
};

} // end namespace mp

#endif
