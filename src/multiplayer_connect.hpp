/* $Id$ */
/*
   Copyright (C) 
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MULTIPLAYER_CONNECT_H_INCLUDED
#define MULTIPLAYER_CONNECT_H_INCLUDED

#include "widgets/scrollpane.hpp"
#include "multiplayer_ui.hpp"
#include "multiplayer_create.hpp"
#include "config.hpp"
#include "network.hpp"
#include "leader_list.hpp"

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

		// Returns true if this side changed since last call to changed()
		bool changed();

		// Gets a config object representing this side. If
		// include_leader is set to true, the config objects include
		// the "type=" defining the leader type, else it does not.
		config get_config() const;

		// Returns true if this side is waiting for a network player.
		bool available() const;

		// Sets the controller of a side.
		void set_controller(mp::controller controller);
		mp::controller get_controller() const;

		// Adds an user to the user list combo
		void update_user_list();

		// Returns the username of this side
		const std::string& get_id() const;
		// Sets the username of this side
		void set_id(const std::string& id);

		const std::string& get_save_id() const;

		// Imports data from the network into this side, and updates
		// the UI accordingly.
		void import_network_user(const config& data);

		// Resets this side to its default state, and updates the UI
		// accordingly.
		void reset(mp::controller controller);

		// Resolves the random leader / factions.
		void resolve_random();

	private:
		void update_controller_ui();
		void update_ui();

		// The mp::connect widget owning this mp::connect::side. Used
		// in the constructor, must be first.
		connect* parent_; 

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
		std::string leader_;
		//bool taken_;

		// Widgets for this side
		gui::label player_number_;
		gui::combo combo_controller_;
		gui::label orig_controller_;
		gui::combo combo_faction_;
		gui::combo combo_leader_;
		gui::combo combo_team_;
		gui::combo combo_colour_;
		gui::slider slider_gold_;
		gui::label label_gold_;

		leader_list_manager llm_;

		bool enabled_;
		bool changed_;
	};

	friend class side;

	typedef std::vector<side> side_list;


	connect(display& disp, const config& game_config, const game_data& data, 
			chat& c, config& gamelist, const create::parameters& params, 
			mp::controller default_controller);

	virtual void process_event();

	// Returns the level data useful to play the game
	const config& get_level();
	const game_state& get_state();

	// Updates the current level, resolves random factions, and sends a
	// "start game" message to the network, with the current level
	void start_game();

protected:
	virtual void layout_children(const SDL_Rect& rect);

	virtual void process_network_data(const config& data, const network::connection sock);
	virtual void process_network_error(network::error& error);
	virtual bool accept_connections();
	virtual void process_network_connection(const network::connection sock);

	virtual void hide_children(bool hide=true);

	virtual void gamelist_updated();
private:
	// Those 2 functions are actually the steps of the (complex)
	// construction of this class.
	void load_game();
	void lists_init();

	// Updates the level_ variable to reflect the sides in the sides_ vector
	void update_level();
	
	// Updates the level, and send a diff to the clients
	void update_and_send_diff();

	// Returns true if there still are sides available for this game
	bool sides_available();
	
	// Updates the state of the player list, the launch button and of the
	// start game label, to reflect the actual state.
	void update_playerlist_state();

	// Returns the index of a player, from its id, or -1 if the player was not found
	connected_user_list::iterator find_player(const std::string& id);

	// Returns the side which is taken by a given player, or -1 if none was found.
	int find_player_side(const std::string& id) const;

	// Adds a player
	void update_user_combos();

	// Removes a player and kicks it from the game
	void kick_player(const std::string& name);

	// This is the main, and global, game data.
	const game_data& game_data_;

	// This is the configuration object which represents the level which
	// will be generated by configuring this multiplayer game.
	config level_;

	create::parameters params_;

	// This is the "game state" object, which is constructed along with the
	// "level" object
	game_state state_;

	// The era used for factions
	std::string era_;

	// The list of available sides for the current era
	config::child_list era_sides_;

	// Lists used for combos
	std::vector<std::string> player_types_;
	std::vector<std::string> player_factions_;
	std::vector<std::string> player_teams_;
	std::vector<std::string> player_colours_;

	// team_name list and "Team" prefix
	std::vector<std::string> team_names_;
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

	gui::button ai_;
	gui::button launch_;
	gui::button cancel_;

};

}

#endif

