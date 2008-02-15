/* $Id$ */
/*
   Copyright (C) 2007
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file multiplayer_connect.hpp
//!

#ifndef MULTIPLAYER_CONNECT_H_INCLUDED
#define MULTIPLAYER_CONNECT_H_INCLUDED

#include "config.hpp"
#include "gamestatus.hpp"
#include "leader_list.hpp"
#include "multiplayer_ui.hpp"
#include "multiplayer_create.hpp"
#include "network.hpp"
#include "widgets/scrollpane.hpp"

#include <string>

namespace mp {

class connect : public mp::ui
{
public:
	struct connected_user {
		connected_user(const std::string& name, mp::controller controller,
				network::connection connection) :
			name(name), controller(controller), connection(connection)
		{};
		std::string name;
		mp::controller controller;
		network::connection connection;
	};

	typedef std::vector<connected_user> connected_user_list;

	class side {
	public:
		side(connect& parent, const config& cfg, int index);

		side(const side& a);

		void add_widgets_to_scrollpane(gui::scrollpane& pane, int pos);

		void process_event();

		//! Returns true if this side changed since last call to changed()
		bool changed();

		//! Gets a config object representing this side. 
		//! If include_leader is set to true, the config objects include
		//! the "type=" defining the leader type, else it does not.
		config get_config() const;

		//! Returns true if this side is waiting for a network player
		//! and players allowed
		bool available() const;

		//! Return true if players are allowed to take this side
		bool allow_player() const;

		//! Sets the controller of a side.
		void set_controller(mp::controller controller);
		mp::controller get_controller() const;

		//! Adds an user to the user list combo
		void update_user_list();

		//! Returns the username of this side
		const std::string& get_id() const;
		//! Sets the username of this side
		void set_id(const std::string& id);

		const std::string& get_save_id() const;

		//! Imports data from the network into this side, 
		//! and updates the UI accordingly.
		void import_network_user(const config& data);

		//! Resets this side to its default state, and updates the UI accordingly.
		void reset(mp::controller controller);

		//! Resolves the random leader / factions.
		void resolve_random();
		void hide_ai_algorithm_combo(bool invis);
	private:
		void init_ai_algorithm_combo();
		void update_ai_algorithm_combo() {hide_ai_algorithm_combo(parent_->hidden());}
		void update_controller_ui();
		void update_ui();

		//! The mp::connect widget owning this mp::connect::side. 
		//! Used in the constructor, must be first.
		connect* parent_;

		//! A non-const config. Be careful not to insert keys when only reading.
		//! (Use cfg_.get_attribute().)
		config cfg_;

		// Configurable variables
		int index_;
		std::string id_;
		std::string save_id_;
		mp::controller controller_;
		int faction_;
		int team_;
		int colour_;
		int gold_;
		int income_;
		std::string leader_;
		std::string gender_;
		std::string ai_algorithm_;
		//bool taken_;

		// Widgets for this side
		gui::label player_number_;
		gui::combo combo_controller_;
		gui::label orig_controller_;
		gui::combo combo_ai_algorithm_;
		gui::combo combo_faction_;
		gui::combo combo_leader_;
		gui::combo combo_gender_;
		gui::combo combo_team_;
		gui::combo combo_colour_;
		gui::slider slider_gold_;
		gui::slider slider_income_;
		gui::label label_gold_;
		gui::label label_income_;

		bool allow_player_;
		bool enabled_;
		bool changed_;
		leader_list_manager llm_;
	};

	friend class side;

	typedef std::vector<side> side_list;


	connect(game_display& disp, const config& game_config, const game_data& data,
			chat& c, config& gamelist, const create::parameters& params,
			mp::controller default_controller);

	virtual void process_event();

	/** Returns the game state, which contains all information 
	 * about the current scenario.
	 */
	const game_state& get_state();

	/** Updates the current game state, resolves random factions, 
	 * and sends a "start game" message to the network.
	 */
	void start_game();

protected:
	virtual void layout_children(const SDL_Rect& rect);

	virtual void process_network_data(const config& data, const network::connection sock);
	virtual void process_network_error(network::error& error);
	virtual bool accept_connections();
	virtual void process_network_connection(const network::connection sock);

	virtual void hide_children(bool hide=true);

private:
	// Those 2 functions are actually the steps of the (complex)
	// construction of this class.
	void load_game();
	void lists_init();

	// Convenience function
	config* current_config();

	//! Updates the level_ variable to reflect the sides in the sides_ vector
	void update_level();

	//! Updates the level, and send a diff to the clients.
	void update_and_send_diff(bool update_time_of_day = false);

	//! Returns true if there still are sides available for this game.
	bool sides_available();

	//! Updates the state of the player list, 
	//! the launch button and of the start game label, 
	//! to reflect the actual state.
	void update_playerlist_state(bool silent=true);

	//! Returns the index of a player, from its id, or -1 if the player was not found.
	connected_user_list::iterator find_player(const std::string& id);

	//! Returns the side which is taken by a given player, or -1 if none was found.
	int find_player_side(const std::string& id) const;

	//! Adds a player.
	void update_user_combos();

	//! Removes a player and kicks it from the game.
	void kick_player(const std::string& name);

	//! This is the main, and global, game data.
	const game_data& game_data_;

	config level_;

	//! This is the "game state" object which is created by this dialog.
	game_state state_;

	create::parameters params_;

	//! The list of available sides for the current era.
	config::child_list era_sides_;

	// Lists used for combos
	std::vector<std::string> player_types_;
	std::vector<std::string> player_factions_;
	std::vector<std::string> player_teams_;
	std::vector<std::string> player_colours_;
	std::vector<std::string> ai_algorithms_;

	// team_name list and "Team" prefix
	std::vector<std::string> team_names_;
	std::vector<std::string> user_team_names_;
	const std::string team_prefix_;

	side_list sides_;
	connected_user_list users_;

	gui::label waiting_label_;
	bool message_full_;

	controller default_controller_;

	// Widgets
	gui::scrollpane scroll_pane_;

	gui::label type_title_label_;
	gui::label faction_title_label_;
	gui::label team_title_label_;
	gui::label colour_title_label_;
	gui::label gold_title_label_;
	gui::label income_title_label_;

	gui::button launch_;
	gui::button cancel_;

}; // end class connect

} // end namespace mp

#endif

