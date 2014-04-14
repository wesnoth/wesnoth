/*
   Copyright (C) 2013 - 2014 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MULTIPLAYER_CONNECT_ENGINE_HPP_INCLUDED
#define MULTIPLAYER_CONNECT_ENGINE_HPP_INCLUDED

#include "commandline_options.hpp"
#include "config.hpp"
#include "flg_manager.hpp"
#include "gamestatus.hpp"
#include "multiplayer_ui.hpp"

#include <boost/scoped_ptr.hpp>

namespace mp {

enum controller {
	CNTR_NETWORK = 0,
	CNTR_LOCAL,
	CNTR_COMPUTER,
	CNTR_EMPTY,
	CNTR_RESERVED,
	CNTR_LAST
};

class connect_engine;
class side_engine;

typedef boost::scoped_ptr<connect_engine> connect_engine_ptr;
typedef boost::shared_ptr<side_engine> side_engine_ptr;
typedef std::pair<mp::controller, std::string> controller_option;

class connect_engine
{
public:
	connect_engine(game_display& disp, game_state& state,
		const mp_game_settings& params, const bool local_players_only,
		const bool first_scenario);
	~connect_engine();

	enum LOAD_USERS { NO_LOAD, RESERVE_USERS, FORCE_IMPORT_USERS };

	config* current_config();

	void import_user(const std::string& name, const bool observer,
		int side_taken = -1);
	void import_user(const config& data, const bool observer,
		int side_taken = -1);

	// Returns true if there are still sides available for this game.
	bool sides_available() const;

	// Import all sides into the level.
	void update_level();
	// Updates the level and sends a diff to the clients.
	void update_and_send_diff(bool update_time_of_day = false);

	bool can_start_game() const;
	void start_game(LOAD_USERS load_users = NO_LOAD);
	void start_game_commandline(const commandline_options& cmdline_opts);

	// Return pair first element specifies whether to leave the game
	// and second element whether to silently update UI.
	std::pair<bool, bool> process_network_data(const config& data,
		const network::connection sock);
	void process_network_error(network::error& error);
	void process_network_connection(const network::connection sock);

	// Returns the side which is taken by a given user,
	// or -1 if none was found.
	int find_user_side_index_by_id(const std::string& id) const;


	/* Setters & Getters */

	const config& level() const { return level_; }
	const std::set<std::string>& connected_users() const
		{ return connected_users_; }
	const std::vector<std::string>& user_team_names()
		{ return user_team_names_; }
	std::vector<side_engine_ptr>& side_engines() { return side_engines_; }
	const mp_game_settings& params() const { return params_; }
	bool first_scenario() const { return first_scenario_; }
	bool force_lock_settings() const { return force_lock_settings_; }

private:
	connect_engine(const connect_engine&);
	void operator=(const connect_engine&);

	void send_level_data(const network::connection sock) const;

	void save_reserved_sides_information();
	void load_previous_sides_users(LOAD_USERS load_users);

	void update_side_controller_options();

	friend class side_engine;

	config level_;
	game_state& state_;

	const mp_game_settings& params_;

	const mp::controller default_controller_;
	const bool local_players_only_;
	const bool first_scenario_;

	bool force_lock_settings_;

	std::vector<side_engine_ptr> side_engines_;
	std::vector<const config*> era_factions_;
	std::vector<std::string> team_names_;
	std::vector<std::string> user_team_names_;
	std::vector<std::string> player_teams_;
	std::set<std::string> connected_users_;
};

class side_engine
{
public:
	side_engine(const config& cfg, connect_engine& parent_engine,
		const int index);
	~side_engine();

	// Creates a config representing this side.
	config new_config() const;

	// Returns true, if the player has chosen his/her leader and this side
	// is ready for the game to start.
	bool ready_for_start() const;
	// Returns true if this side is waiting for a network player and
	// players are allowed.
	bool available_for_user(const std::string& name = "") const;

	bool swap_sides_on_drop_target(const unsigned drop_target);

	void resolve_random();

	// Resets this side to its default state.
	void reset();

	// Place user into this side.
	void place_user(const std::string& name);
	void place_user(const config& data, bool contains_selection = false);

	void update_controller_options();
	void update_current_controller_index();
	bool controller_changed(const int selection);
	void set_controller(mp::controller controller);

	// Game set up from command line helpers.
	void set_faction_commandline(const std::string& faction_name);
	void set_controller_commandline(const std::string& controller_name);

	/* Setters & Getters */

	std::string save_id() const
		{ return (!cfg_["save_id"].empty()) ? cfg_["save_id"] : cfg_["id"]; }
	const std::vector<controller_option>& controller_options()
		{ return controller_options_; }
	const config& cfg() const { return cfg_; }
	mp::controller controller() const { return controller_; }
	unsigned current_controller_index() const
		{ return current_controller_index_; }
	int index() const { return index_; }
	void set_index(int index) { index_ = index; }
	int team() const { return team_; }
	void set_team(int team) { team_ = team; }
	std::vector<std::string> get_children_to_swap();
	std::multimap<std::string, config> get_side_children();
	void set_side_children(std::multimap<std::string, config> children); 
	int color() const { return color_; }
	void set_color(int color) { color_ = color; }
	int gold() const { return gold_; }
	void set_gold(int gold) { gold_ = gold; }
	int income() const { return income_; }
	void set_income(int income) { income_ = income; }
	const std::string& player_id() const { return player_id_; }
	const std::string& current_player() const { return current_player_; }
	void set_current_player(const std::string& current_player)
		{ current_player_ = current_player; }
	const std::string& ai_algorithm() const { return ai_algorithm_; }
	void set_ai_algorithm(const std::string& ai_algorithm)
		{ ai_algorithm_ = ai_algorithm; }
	bool allow_player() const { return allow_player_; }
	bool allow_changes() const { return allow_changes_; }
	bool waiting_to_choose_faction() const { return waiting_to_choose_faction_; }
	void set_waiting_to_choose_status(bool status) { waiting_to_choose_faction_ = status;}
	bool chose_random() const { return chose_random_;}
	const std::vector<std::string>& player_teams() const
		{ return parent_.player_teams_; }
	flg_manager& flg() { return flg_; }

private:
	side_engine(const side_engine& engine);
	void operator=(const side_engine&);

	void add_controller_option(mp::controller controller,
		const std::string& name, const std::string& controller_value);

	config cfg_;
	connect_engine& parent_;

	mp::controller controller_;
	unsigned current_controller_index_;
	std::vector<controller_option> controller_options_;

	const bool allow_player_;
	const bool allow_changes_;
	const bool controller_lock_;

	int index_;
	int team_;
	int color_;
	int gold_;
	int income_;
	std::string current_player_;
	std::string player_id_;
	std::string ai_algorithm_;

	bool waiting_to_choose_faction_;
	bool chose_random_;
	flg_manager flg_;
};

} // end namespace mp

#endif
