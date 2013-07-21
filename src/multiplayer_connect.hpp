/*
   Copyright (C) 2007 - 2013
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
#include "leader_list.hpp"
#include "multiplayer_connect_engine.hpp"
#include "multiplayer_ui.hpp"
#include "widgets/combo_drag.hpp"
#include "widgets/scrollpane.hpp"
#include "widgets/slider.hpp"

namespace ai {
	struct description;
}

namespace mp {

class connect : public mp::ui
{
public:

	class side {
	public:
		side(connect& parent, side_engine_ptr engine);

		side(const side& a);

		void add_widgets_to_scrollpane(gui::scrollpane& pane, int pos);

		void process_event();

		/** Returns true if this side changed since last call to changed(). */
		bool changed();

		/** Adds an user to the user list combo. */
		void update_user_list(const std::vector<std::string>& name_list);

		/**
		 * Imports data from the network into this side, and updates the UI
		 * accordingly.
		 */
		void import_network_user(const config& data);

		/** Resets this side to its default state, and updates the UI accordingly. */
		void reset(mp::controller controller);

		void hide_ai_algorithm_combo(bool invis);

		side_engine_ptr engine() { return engine_; }
		const side_engine_ptr engine() const { return engine_; }

	private:
		void init_ai_algorithm_combo();
		void update_ai_algorithm_combo() {hide_ai_algorithm_combo(parent_->hidden());}

		/** Fill or refresh the faction combo using the proper team color. */
		void update_faction_combo();
		void update_controller_ui();
		void update_ui();

		/**
		 * The mp::connect widget owning this mp::connect::side.
		 *
		 * Used in the constructor, must be first.
		 */
		connect* parent_;

		side_engine_ptr engine_;

		// Flags for controlling the dialog widgets of the game lobby
		bool gold_lock_;
		bool income_lock_;
		bool team_lock_;
		bool color_lock_;

		// Widgets for this side
		gui::label player_number_;
		gui::combo_drag_ptr combo_controller_;
		gui::label orig_controller_;
		gui::combo combo_ai_algorithm_;
		gui::combo combo_faction_;
		gui::combo combo_leader_;
		gui::combo combo_gender_;
		gui::combo combo_team_;
		gui::combo combo_color_;
		gui::slider slider_gold_;
		gui::slider slider_income_;
		gui::label label_gold_;
		gui::label label_income_;

		bool changed_;
	};

	typedef std::vector<side> side_list;

	/**
	 * Pointer to the display
	 */
	connect(game_display& disp, const config& game_config, chat& c,
			config& gamelist, const mp_game_settings& params,
			mp::controller default_controller, bool local_players_only = false);

	virtual void process_event();

	void take_reserved_side(connect::side& side, const config& data);

	/**
	 * Returns the game state, which contains all information about the current
	 * scenario.
	 */
	const game_state& get_state() const { return engine_.state(); }

	/**
	 * Updates the current game state, resolves random factions, and sends a
	 * "start game" message to the network.
	 */
	void start_game() { engine_.start_game(); }
	void start_game_commandline(const commandline_options& cmdline_opts)
		{ engine_.start_game_commandline(cmdline_opts); }

protected:
	virtual void layout_children(const SDL_Rect& rect);

	virtual void process_network_data(const config& data, const network::connection sock);
	virtual void process_network_error(network::error& error);
	virtual bool accept_connections();
	virtual void process_network_connection(const network::connection sock);

	virtual void hide_children(bool hide=true);

private:
	void lists_init();

	/**
	 * Updates the state of the player list, the launch button and of the start
	 * game label, to reflect the actual state.
	 */
	void update_playerlist_state(bool silent=true);

	/** Adds a player. */
	void update_user_combos();

	bool local_only_;

	const mp_game_settings params_;

	// Lists used for combos
	std::vector<std::string> player_types_;
	std::vector<std::string> player_teams_;
	std::vector<std::string> player_colors_;
	std::vector<ai::description*> ai_algorithms_;

	const std::string team_prefix_;

	side_list sides_;

	gui::label waiting_label_;

	// Widgets
	gui::scrollpane scroll_pane_;

	gui::label type_title_label_;
	gui::label faction_title_label_;
	gui::label team_title_label_;
	gui::label color_title_label_;
	gui::label gold_title_label_;
	gui::label income_title_label_;

	gui::button launch_;
	gui::button cancel_;
	gui::button add_local_player_;

	gui::drop_group_manager_ptr combo_control_group_;

	connect_engine engine_;
}; // end class connect

} // end namespace mp

#endif

