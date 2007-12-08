/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GAME_HPP_INCLUDED
#define GAME_HPP_INCLUDED

#include "../config.hpp"
#include "../network.hpp"

#include "player.hpp"

//#include <algorithm>
#include <map>
//#include <set>
#include <vector>

typedef std::map<network::connection,player> player_map;
typedef std::vector<network::connection> user_vector;
typedef std::vector<network::connection> side_vector;

class game
{
public:
	game(player_map& players, const network::connection host=0, const std::string name="");

	int id() const { return id_; }
	const std::string& name() const { return name_; }

	bool is_owner(const network::connection player) const { return (player == owner_); }
	bool is_member(const network::connection player) const
	{ return is_player(player) || is_observer(player); }
	bool allow_observers() const { return allow_observers_; }
	bool is_observer(const network::connection player) const;
	bool is_muted_observer(const network::connection player) const;
	bool all_observers_muted() const { return all_observers_muted_; }
	bool is_player(const network::connection player) const;
	bool player_is_banned(const network::connection player) const;

	size_t nplayers() const { return players_.size(); }
	size_t nobservers() const { return observers_.size(); }

	bool mute_all_observers() { return all_observers_muted_ = !all_observers_muted_; }
	//! Mute an observer by name.
	void mute_observer(const config& mute);
	//! Kick a member by name.
	network::connection kick_member(const config& kick);
	//! Ban and kick a user by name. He doesn't need to be in this game.
	network::connection ban_user(const config& ban);

	void add_player(const network::connection player, const bool observer = false);
	void remove_player(const network::connection player, const bool notify_creator=true);

	//! Adds players from one game to another. This is used to add players and
	//! observers from a game to the lobby (which is also implemented as a game),
	//! if that game ends. The second parameter controls, wether the players are
	//! added to the players_ or observers_ vector (default observers_).
	void add_players(const game& other_game, const bool observer = true);

	bool started() const { return started_; }

	//function which filters commands sent by a player to remove commands
	//that they don't have permission to execute.
	//Returns true iff there are still some commands left
	bool filter_commands(const network::connection player, config& cfg) const;

	void start_game();
	//! Make everyone leave the game and clean up.
	void end_game(const config& games_and_users_list);

	void update_side_data();
	bool take_side(const network::connection player, const config& cfg = config());
	//! Let's a player owning a side give it to another player or observer.
	void transfer_side_control(const network::connection sock, const config& cfg);

	//! Set the description to the number of slots.
	//! Returns true if the number of slots has changed.
	bool describe_slots();

	config construct_server_message(const std::string& message) const;
	//! Send data to all players in this game except 'exclude'.
	void send_data(const config& data, const network::connection exclude=0) const;
	void send_data_team(const config& data, const std::string& team,
		const network::connection exclude=0) const;
	void send_data_observers(const config& data, const network::connection exclude=0) const;

	void record_data(const config& data);
	void reset_history();

	//the full scenario data
	bool level_init() const { return level_.child("side") != NULL; }
	config& level() { return level_; }

	//functions to set/get the address of the game's summary description as
	//sent to players in the lobby
	void set_description(config* desc) { description_ = desc; }
	config* description() const { return description_; }

	//function which will process game commands and update the state of the
	//game accordingly. Will return true iff the game's description changes.
	bool process_commands(const config& cfg);

	const std::string& termination_reason() const {
		static const std::string aborted = "aborted";
		return termination_.empty() ? aborted : termination_;
	}

	void set_termination_reason(const std::string& reason) {
		if (termination_.empty()) { termination_ = reason; }
	}

	//! This is a tempory variable to have a flag to switch between 
	//! gzipped data and not.
	//! @todo remove after 1.3.12 is no longer allowed on the server.
	static bool send_gzipped;

private:
	//! Adds players and observers into one vector and returns that.
	const user_vector all_game_users() const;
	//! In case of a host transfer, notify the new host about its status.
	void notify_new_host();
	//! Convenience function for finding a user by name.
	player_map::const_iterator find_user(const std::string& name) const;

	bool observers_can_label() const { return false; }
	bool observers_can_chat() const { return true; }
	//! Function which returns true iff 'player' is on 'team'.
	bool player_on_team(const std::string& team, const network::connection player) const;

	//function which should be called every time a player ends their turn
	//(i.e. [end_turn] received). This will update the 'turn' attribute for
	//the game's description when appropriate. Will return true if there has
	//been a change.
	bool end_turn();

	//function to send a list of users to all clients. Only sends data before
	//the game has started.
	void send_user_list(const network::connection exclude=0) const;

	//! Helps debugging player and observer lists.
	std::string debug_player_info() const;

	player_map* player_info_;

	static int id_num;
	int id_;
	std::string name_;
	network::connection owner_;
	user_vector players_;
	user_vector observers_;
	user_vector muted_observers_;
	side_vector sides_;
	std::vector<bool> sides_taken_;
	std::vector<std::string> side_controllers_;
	bool started_;

	config level_;

	config history_;

	config* description_;

	int end_turn_;

	bool allow_observers_;
	bool all_observers_muted_;

	std::vector<std::string> bans_;

	std::string termination_;
};

struct game_id_matches {
	game_id_matches(int id) : id_(id) {}
	bool operator()(const game& g) const { return g.id() == id_; }

private:
	int id_;
};

#endif
