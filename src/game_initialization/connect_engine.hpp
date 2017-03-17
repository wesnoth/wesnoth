/*
   Copyright (C) 2013 - 2016 by Andrius Silinskas <silinskas.andrius@gmail.com>
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
#include "saved_game.hpp"
#include <set>

namespace rand_rng { class mt_rng; }
struct mp_campaign_info;

namespace ng {

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

typedef const std::unique_ptr<connect_engine> connect_engine_ptr;
typedef std::shared_ptr<side_engine> side_engine_ptr;
typedef std::pair<ng::controller, std::string> controller_option;

class connect_engine
{
public:
	/// @param players the player which are already connected to the current game.
	///                This is always empty unless we advance form a previous scenario.
	connect_engine(saved_game& state,
		const bool first_scenario, mp_campaign_info* campaign_info);

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
	void start_game();
	void start_game_commandline(const commandline_options& cmdline_opts);

	void leave_game();

	// Return pair first element specifies whether to leave the game
	// and second element whether to silently update UI.
	std::pair<bool, bool> process_network_data(const config& data);

	// Returns the side which is taken by a given user,
	// or -1 if none was found.
	int find_user_side_index_by_id(const std::string& id) const;


	/* Setters & Getters */

	const config& level() const { return level_; }
	config& scenario()
	{
		if(config& scenario = level_.child("scenario"))
			return scenario;
		else if(config& snapshot = level_.child("snapshot"))
			return snapshot;
		else
			throw "No scenariodata found";
	}
	const std::set<std::string>& connected_users() const;
	const std::vector<std::string>& user_team_names()
		{ return user_team_names_; }
	std::vector<side_engine_ptr>& side_engines() { return side_engines_; }
	const mp_game_settings& params() const { return params_; }
	bool first_scenario() const { return first_scenario_; }
	bool force_lock_settings() const { return force_lock_settings_; }

	bool receive_from_server(config& dst) const;

	const mp_campaign_info* campaign_info() const
	{
		return campaign_info_;
	}

private:
	connect_engine(const connect_engine&) = delete;
	void operator=(const connect_engine&) = delete;

	void send_level_data() const;

	void save_reserved_sides_information();
	void load_previous_sides_users();

	void update_side_controller_options();

	friend class side_engine;

	config level_;
	saved_game& state_;

	const mp_game_settings& params_;

	const ng::controller default_controller_;
	mp_campaign_info* campaign_info_;
	const bool first_scenario_;

	bool force_lock_settings_;

	std::vector<side_engine_ptr> side_engines_;
	std::vector<const config*> era_factions_;
	std::vector<std::string> team_names_;
	std::vector<std::string> user_team_names_;
	std::vector<std::string> player_teams_;

	std::set<std::string>& connected_users_rw();
	void send_to_server(const config& cfg) const;
};

class side_engine
{
public:
	side_engine(const config& cfg, connect_engine& parent_engine,
		const int index);

	// An untranslated user_description which is used by other clients
	// An empty string means the other clients should generate the description on their own
	// Used by new_config().
	std::string user_description() const;

	// Creates a config representing this side.
	config new_config() const;

	// Returns true, if the player has chosen his/her leader and this side
	// is ready for the game to start.
	bool ready_for_start() const;
	// Returns true if this side is waiting for a network player and
	// players are allowed.
	bool available_for_user(const std::string& name = "") const;

	void resolve_random( rand_rng::mt_rng & rng, const std::vector<std::string> & avoid_faction_ids = std::vector<std::string>());

	// Resets this side to its default state.
	void reset();

	// Place user into this side.
	void place_user(const std::string& name);
	void place_user(const config& data, bool contains_selection = false);

	void update_controller_options();
	void update_current_controller_index();
	bool controller_changed(const int selection);
	void set_controller(ng::controller controller);

	// Game set up from command line helpers.
	void set_faction_commandline(const std::string& faction_name);
	void set_controller_commandline(const std::string& controller_name);

	/* Setters & Getters */

	std::string save_id() const
		{ return (!cfg_["save_id"].empty()) ? cfg_["save_id"] : cfg_["id"]; }
	// The id of the side of the previous scenario that should control this side.
	std::string previous_save_id() const
		{ return (!cfg_["previous_save_id"].empty()) ? cfg_["previous_save_id"] : save_id(); }
	const std::vector<controller_option>& controller_options()
		{ return controller_options_; }
	const config& cfg() const { return cfg_; }
	ng::controller controller() const { return controller_; }
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
	void set_color(int color) { color_ = color; color_id_ = color_options_[color]; }
	int gold() const { return gold_; }
	void set_gold(int gold) { gold_ = gold; }
	int income() const { return income_; }
	void set_income(int income) { income_ = income; }
	const std::string& player_id() const { return player_id_; }
	const std::string& reserved_for() const { return reserved_for_; }
	void set_reserved_for(const std::string& reserved_for)
		{ reserved_for_ = reserved_for; }
	const std::string& ai_algorithm() const { return ai_algorithm_; }
	void set_ai_algorithm(const std::string& ai_algorithm)
		{ ai_algorithm_ = ai_algorithm; }
	bool allow_player() const { return allow_player_; }
	bool allow_changes() const { return allow_changes_; }
	bool waiting_to_choose_faction() const { return waiting_to_choose_faction_; }
	void set_waiting_to_choose_status(bool status) { waiting_to_choose_faction_ = status;}
	bool allow_shuffle() const { return !disallow_shuffle_;}
	const std::vector<std::string>& player_teams() const
		{ return parent_.player_teams_; }
	flg_manager& flg() { return flg_; }

	const std::string color_id() const { return color_id_; }
	const std::vector<std::string>& color_options() const { return color_options_; }

	const std::string team_name() const
	{
		return parent_.team_names_[team_];
	}

	const std::string user_team_name() const
	{
		return t_string::from_serialized(parent_.user_team_names_[team_]);
	}

private:
	side_engine(const side_engine& engine) = delete;
	void operator=(const side_engine&) = delete;

	void add_controller_option(ng::controller controller,
		const std::string& name, const std::string& controller_value);

	config cfg_;
	connect_engine& parent_;

	ng::controller controller_;
	unsigned current_controller_index_;
	std::vector<controller_option> controller_options_;

	const bool allow_player_;
	const bool controller_lock_;

	int index_;
	int team_;
	int color_;
	int gold_;
	int income_;
	// set during create_engines constructor never set after that.
	// the name of the player who is preferred for this side,
	// if controller_ == reserved only this player can take this side.
	// can also be a number of a side if this side shoudl be controlled
	// by the player who controlls  that side
	std::string reserved_for_;
	std::string player_id_;
	std::string ai_algorithm_;

	bool chose_random_;
	bool disallow_shuffle_;
	flg_manager flg_;
	const bool allow_changes_;
	bool waiting_to_choose_faction_;
	std::string custom_color_;
	std::string color_id_;

	std::vector<std::string> color_options_;
};

} // end namespace ng

#endif
