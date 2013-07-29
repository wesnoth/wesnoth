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
	operator std::string() const
	{
		return name;
	}
};

typedef boost::shared_ptr<side_engine> side_engine_ptr;
typedef std::vector<connected_user> connected_user_list;

class connect_engine
{
public:
	connect_engine(game_display& disp, controller mp_controller,
		const mp_game_settings& params);
	~connect_engine();

	config* current_config();

	void add_side_engine(side_engine_ptr engine);

	// Import all sides into the level.
	void update_level();
	// Updates the level and sends a diff to the clients.
	void update_and_send_diff(bool update_time_of_day = false);

	// Returns true if there are still sides available for this game.
	bool sides_available() const;

	bool can_start_game() const;
	void start_game();
	void start_game_commandline(const commandline_options& cmdline_opts);

	// Network methods.
	void process_network_connection(const network::connection sock);

	// Returns the index of a player, from its id,
	// or -1 if the player was not found.
	connected_user_list::iterator find_player(const std::string& id);

	// Returns the side which is taken by a given player,
	// or -1 if none was found.
	int find_player_side(const std::string& id) const;


	/* Setters & Getters */

	const config& level() const { return level_; }
	const game_state& state() const { return state_; }

	const std::vector<const config *>& era_factions() const
		{ return era_factions_; }

private:
	connect_engine(const connect_engine&);
	void operator=(const connect_engine&);

	friend side_engine;

	config level_;
	game_state state_;

	game_display& disp_;
	const mp_game_settings& params_;

	std::vector<side_engine_ptr> side_engines_;
	std::vector<const config *> era_factions_;

public:
	connected_user_list users_;
	controller mp_controller_;

	std::vector<std::string> team_names_;
	std::vector<std::string> user_team_names_;
};

class side_engine
{
public:
	side_engine(const config& cfg, connect_engine& parent_engine,
		const int index);
	~side_engine();

	// Sets a new config representing this side.
	config new_config() const;

	// Returns true, if the player has chosen his/her leader and this side
	// is ready for the game to start.
	bool ready_for_start() const;

	// Returns true if this side is waiting for a network player and
	// players are allowed.
	bool available(const std::string& name = "") const;

	void set_player_from_users_list(const std::string& player_id);

	void resolve_random();

	int selected_faction_index() const;

	void set_faction_commandline(const std::string& faction_name);
	void set_controller_commandline(const std::string& controller_name);
	void set_ai_algorithm_commandline(const std::string& algorithm_name);

	void assign_sides_on_drop_target(const int drop_target);


	/* Setters & Getters */

	const std::vector<const config*>& choosable_factions()
		{ return choosable_factions_; }
	const config& cfg() const { return cfg_; }
	const config* current_faction() const { return current_faction_; }
	void set_current_faction(const config* current_faction)
		{ current_faction_ = current_faction; }
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
	std::string leader() const;
	void set_leader(const std::string& leader) { leader_ = leader; }
	std::string gender() const;
	void set_gender(const std::string& gender) { gender_ = gender; }
	const std::string& ai_algorithm() const { return ai_algorithm_; }
	void set_ai_algorithm(const std::string& ai_algorithm)
		{ai_algorithm_ = ai_algorithm; }
	void set_ready_for_start(const bool ready_for_start)
		{ ready_for_start_ = ready_for_start; }
	bool allow_player() const { return allow_player_; }
	std::vector<std::string>& leaders() { return leaders_; }
	std::vector<std::string>& genders() { return genders_; }
	std::vector<std::string>& gender_ids() { return gender_ids_; }

private:
	side_engine(const side_engine& engine);
	void operator=(const connect_engine&);

	config cfg_;

	connect_engine& parent_;

	// All factions which could be played by a side (including Random).
	std::vector<const config*> available_factions_;
	// All factions which a side can choose.
	std::vector<const config*> choosable_factions_;

	const config* current_faction_;
	controller mp_controller_;

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
	std::string leader_;
	std::string gender_;
	std::string ai_algorithm_;

	bool ready_for_start_;
	bool allow_player_;
	bool allow_changes_;

	std::vector<std::string> leaders_;
	std::vector<std::string> genders_;
	std::vector<std::string> gender_ids_;
};

} // end namespace mp

#endif
