/*
   Copyright (C) 2007 - 2014
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef MULTIPLAYER_CONNECT_H_INCLUDED
#define MULTIPLAYER_CONNECT_H_INCLUDED

#include "commandline_options.hpp"
#include "gamestatus.hpp"
#include "multiplayer_connect_engine.hpp"
#include "multiplayer_ui.hpp"
#include "widgets/combo_drag.hpp"
#include "widgets/scrollpane.hpp"
#include "widgets/slider.hpp"

namespace ai {
	struct description;
}

namespace mp {

// Helper function to retrieve controller names.
std::vector<std::string> controller_options_names(
	const std::vector<controller_option>& controller_options);

class connect : public mp::ui
{
public:

	class side {
	public:
		side(connect& parent, side_engine_ptr engine);
		side(const side& a);
		~side();

		void process_event();

		// Returns true if this side changed since last call to this method.
		bool changed();

		void update_ui();

		void add_widgets_to_scrollpane(gui::scrollpane& pane, int pos);

		side_engine_ptr engine() { return engine_; }
		const side_engine_ptr engine() const { return engine_; }

	private:
		// Update UI methods and their helper(s).
		void update_faction_combo();
		void update_controller_ui();

		// The mp::connect widget owning this mp::connect::side.
		connect* parent_;
		side_engine_ptr engine_;

		// Flags for controlling which configuration widgets should be locked.
		bool gold_lock_;
		bool income_lock_;
		bool team_lock_;
		bool color_lock_;

		bool changed_;

		gui::label label_player_number_;
		gui::label label_original_controller_;
		gui::label label_gold_;
		gui::label label_income_;
		gui::combo_drag_ptr combo_controller_;
		gui::combo combo_ai_algorithm_;
		gui::combo combo_faction_;
		gui::label label_leader_name_;
		gui::combo combo_leader_;
		gui::combo combo_gender_;
		gui::combo combo_team_;
		gui::combo combo_color_;
		gui::slider slider_gold_;
		gui::slider slider_income_;
	};

	typedef std::vector<side> side_list;

	connect(game_display& disp, const std::string& game_name,
		const config& game_config, chat& c, config& gamelist,
		connect_engine& engine);
	~connect();

	// Updates the current game state, resolves random factions, and sends a
	// "start game" message to the network.
	void start_game() { engine_.start_game(); }
	void start_game_commandline(const commandline_options& cmdline_opts)
		{ engine_.start_game_commandline(cmdline_opts); }

protected:
	virtual void process_event();

	virtual void layout_children(const SDL_Rect& rect);
	virtual void hide_children(bool hide = true);

	virtual void process_network_data(const config& data,
		const network::connection sock);
	virtual void process_network_error(network::error& error);
	virtual void process_network_connection(const network::connection sock);
	virtual bool accept_connections();

private:
	connect(const connect&);
	void operator=(const connect&);

	// Updates the state of the player list, the launch button and of the start
	// game label, to reflect the actual state.
	void update_playerlist_state(bool silent = true);

	const mp_game_settings& params() { return engine_.params(); }
	bool force_lock_settings() const { return engine_.force_lock_settings(); }

	// Lists used for combos.
	std::vector<std::string> player_colors_;
	std::vector<ai::description*> ai_algorithms_;

	side_list sides_;
	connect_engine& engine_;

	gui::label waiting_label_;
	gui::label type_title_label_;
	gui::label faction_name_title_label_;
	gui::label leader_gender_title_label_;
	gui::label team_color_title_label_;
	gui::label gold_title_label_;
	gui::label income_title_label_;
	gui::scrollpane scroll_pane_;
	gui::button launch_;
	gui::button cancel_;
	gui::drop_group_manager_ptr combo_control_group_;
};

} // end namespace mp

#endif

