/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
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
#include "gamestatus.hpp"
#include "multiplayer_ui.hpp"

namespace mp {

enum controller {
	CNTR_NETWORK = 0,
	CNTR_LOCAL,
	CNTR_COMPUTER,
	CNTR_EMPTY,
	CNTR_RESERVED,
	CNTR_LAST
};

class side_engine;

typedef boost::shared_ptr<side_engine> side_engine_ptr;
typedef std::pair<mp::controller, std::string> controller_option;

class connect_engine
{
public:
	connect_engine(game_display& disp, const mp_game_settings& params,
		const bool local_players_only, const bool first_scenario);
	~connect_engine();

	config* current_config();

	void add_side_engine(side_engine_ptr engine);
	// Should be called after all calls to 'add_side_engine()'
	// have been made, so that everything could be initialized.
	void init_after_side_engines_assigned();

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
	void start_game();
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
	const game_state& state() const { return state_; }
	const std::set<std::string>& connected_users() const
		{ return connected_users_; }
	const std::vector<std::string>& user_team_names()
		{ return user_team_names_; }

protected:
	std::vector<side_engine_ptr>& side_engines() { return side_engines_; }

private:
	connect_engine(const connect_engine&);
	void operator=(const connect_engine&);

	void update_side_controller_options();

	friend side_engine;

	config level_;
	game_state state_;

	const mp_game_settings& params_;

	const mp::controller default_controller_;
	const bool first_scenario_;

	std::vector<side_engine_ptr> side_engines_;
	std::vector<const config*> era_factions_;
	std::vector<std::string> team_names_;
	std::vector<std::string> user_team_names_;
	std::vector<std::string> player_teams_;
	std::set<std::string> connected_users_;
	std::vector<controller_option> default_controller_options_;
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

	void swap_sides_on_drop_target(const int drop_target);

	void resolve_random();

	// Resets this side to its default state.
	void reset();

	// Place user into this side.
	void place_user(const std::string& name);
	void place_user(const config& data);

	void update_controller_options();
	void update_current_controller_index();
	bool controller_changed(const int selection);
	void set_controller(mp::controller controller);

	void set_current_faction(const config* current_faction);
	void set_current_leader(const std::string& current_leader);
	void set_current_gender(const std::string& current_gender);

	int current_faction_index() const;

	std::string save_id() const { return cfg_["save_id"]; }

	// Game set up from command line helpers.
	void set_faction_commandline(const std::string& faction_name);
	void set_controller_commandline(const std::string& controller_name);

	/* Setters & Getters */

	const std::vector<const config*>& choosable_factions()
		{ return choosable_factions_; }
	const std::vector<std::string>& choosable_leaders()
		{ return choosable_leaders_; }
	const std::vector<std::string>& choosable_genders()
		{ return choosable_genders_; }
	const std::vector<controller_option>& controller_options()
		{ return controller_options_; }
	const config& cfg() const { return cfg_; }
	const std::string& current_leader() const { return current_leader_; }
	const std::string& current_gender() const { return current_gender_; }
	mp::controller controller() const { return controller_; }
	unsigned current_controller_index() const
		{ return current_controller_index_; }
	int index() const { return index_; }
	void set_index(int index) { index_ = index; }
	int team() const { return team_; }
	void set_team(int team) { team_ = team; }
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
	const std::vector<std::string>& player_teams() const
		{ return parent_.player_teams_; }

private:
	side_engine(const side_engine& engine);
	void operator=(const connect_engine&);

	void update_choosable_leaders();
	void update_choosable_genders();

	config cfg_;
	connect_engine& parent_;

	mp::controller controller_;
	unsigned current_controller_index_;

	// All factions which could be played by a side (including Random).
	std::vector<const config*> available_factions_;

	std::vector<const config*> choosable_factions_;
	std::vector<std::string> choosable_leaders_;
	std::vector<std::string> choosable_genders_;

	std::vector<controller_option> controller_options_;

	const config* current_faction_;
	std::string current_leader_;
	std::string current_gender_;

	const bool allow_player_;
	const bool allow_changes_;
	const std::string leader_id_;

	int index_;
	int team_;
	int color_;
	int gold_;
	int income_;
	std::string current_player_;
	std::string player_id_;
	std::string ai_algorithm_;
};

} // end namespace mp

#endif
