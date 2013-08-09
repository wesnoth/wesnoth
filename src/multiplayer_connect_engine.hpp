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

#include <boost/tuple/tuple.hpp>

namespace mp {

class side_engine;

struct connected_user
{
	connected_user(const std::string& name, mp::controller controller,
		network::connection connection) :
		name(name),
		controller(controller),
		connection(connection)
	{};
	std::string name;
	mp::controller controller;
	network::connection connection;
};

typedef boost::shared_ptr<side_engine> side_engine_ptr;
typedef std::vector<connected_user> connected_user_list;
typedef boost::tuple<bool, bool, bool, std::vector<int> > network_res_tuple;

class connect_engine
{
public:
	connect_engine(game_display& disp, const mp_game_settings& params,
		const bool local_players_only, const bool first_scenario);
	~connect_engine();

	config* current_config();

	void add_side_engine(side_engine_ptr engine);
	void assign_side_for_host();

	// Returns true if there are still sides available for this game.
	bool sides_available() const;

	// Import all sides into the level.
	void update_level();
	// Updates the level and sends a diff to the clients.
	void update_and_send_diff(bool update_time_of_day = false);

	bool can_start_game() const;
	void start_game();
	void start_game_commandline(const commandline_options& cmdline_opts);

	// Acts according to the given data and returns tuple
	// holding information on what has changed.
	// 0th - quit?
	// 1st - update UI?
	// 2nd - silent UI update?
	// 3rd - side UIs to update.
	network_res_tuple process_network_data(const config& data,
		const network::connection sock);
	// Returns -1 if UI should not be updated at all,
	// 0 if UI should be updated or (side index + 1)
	// if some side's UI should be updated as well.
	int process_network_error(network::error& error);
	void process_network_connection(const network::connection sock);

	connected_user_list::iterator find_player_by_id(const std::string& id);


	/* Setters & Getters */

	const config& level() const { return level_; }
	const game_state& state() const { return state_; }
	bool local_players_only() const { return local_players_only_; }
	connected_user_list& users() { return users_; }
	std::vector<std::string>& team_names() { return team_names_; }
	std::vector<std::string>& user_team_names() { return user_team_names_; }

protected:
	std::vector<side_engine_ptr>& side_engines() { return side_engines_; }

private:
	connect_engine(const connect_engine&);
	void operator=(const connect_engine&);

	friend side_engine;

	// Returns the side which is taken by a given player,
	// or -1 if none was found.
	int find_player_side_index_by_id(const std::string& id) const;

	config level_;
	game_state state_;

	const mp_game_settings& params_;

	const bool local_players_only_;
	const bool first_scenario_;

	std::vector<side_engine_ptr> side_engines_;
	std::vector<const config*> era_factions_;
	std::vector<std::string> team_names_;
	std::vector<std::string> user_team_names_;
	connected_user_list users_;
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
	bool available(const std::string& name = "") const;

	void set_player_from_users_list(const std::string& player_id);

	void swap_sides_on_drop_target(const int drop_target);

	void resolve_random();

	// Resets this side to its default state.
	void reset(mp::controller controller);

	// Imports data from the network into this side.
	void import_network_user(const config& data);

	void set_current_faction(const config* current_faction);
	void set_current_leader(const std::string& current_leader);
	void set_current_gender(const std::string& current_gender);

	int current_faction_index() const;

	// Game set up from command line helpers.
	void set_faction_commandline(const std::string& faction_name);
	void set_controller_commandline(const std::string& controller_name);
	void set_ai_algorithm_commandline(const std::string& algorithm_name);


	/* Setters & Getters */

	const std::vector<const config*>& choosable_factions()
		{ return choosable_factions_; }
	const std::vector<std::string>& choosable_leaders()
		{ return choosable_leaders_; }
	const std::vector<std::string>& choosable_genders()
		{ return choosable_genders_; }
	const config& cfg() const { return cfg_; }
	const std::string& current_leader() const { return current_leader_; }
	const std::string& current_gender() const { return current_gender_; }
	controller mp_controller() const { return mp_controller_; }
	void set_mp_controller(controller mp_controller)
		{ mp_controller_ = mp_controller; }
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
	void set_player_id(const std::string& player_id) { player_id_ = player_id; }
	const std::string& save_id() const { return save_id_; }
	const std::string& current_player() const { return current_player_; }
	const std::string& ai_algorithm() const { return ai_algorithm_; }
	void set_ai_algorithm(const std::string& ai_algorithm)
		{ ai_algorithm_ = ai_algorithm; }
	void set_ready_for_start(const bool ready_for_start)
		{ ready_for_start_ = ready_for_start; }
	bool allow_player() const { return allow_player_; }

private:
	side_engine(const side_engine& engine);
	void operator=(const connect_engine&);

	void update_choosable_leaders();
	void update_choosable_genders();

	config cfg_;
	connect_engine& parent_;

	controller mp_controller_;

	// All factions which could be played by a side (including Random).
	std::vector<const config*> available_factions_;

	std::vector<const config*> choosable_factions_;
	std::vector<std::string> choosable_leaders_;
	std::vector<std::string> choosable_genders_;

	const config* current_faction_;
	std::string current_leader_;
	std::string current_gender_;

	bool ready_for_start_;
	bool allow_player_;
	bool allow_changes_;

	// Configurable variables.
	int index_;
	int team_;
	int color_;
	int gold_;
	int income_;
	std::string id_;
	std::string player_id_;
	std::string save_id_;
	std::string current_player_;
	std::string ai_algorithm_;
};

} // end namespace mp

#endif
