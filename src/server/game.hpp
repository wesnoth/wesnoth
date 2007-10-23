/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
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
#include <set>
#include <vector>

typedef std::vector<network::connection> user_vector;
typedef std::map<network::connection,player> player_map;

class game
{
public:
	game(const player_map& info);

	void set_owner(const network::connection player);
	bool is_owner(const network::connection player) const;
	bool is_member(const network::connection player) const;
	bool is_needed(const network::connection player) const;
	bool is_observer(const network::connection player) const;
	bool is_muted_observer(const network::connection player) const;
	bool is_player(const network::connection player) const;
	bool all_observers_muted() const;

	bool observers_can_label() const;
	bool observers_can_chat() const;

	//function which filters commands sent by a player to remove commands
	//that they don't have permission to execute.
	//Returns true iff there are still some commands left
	bool filter_commands(network::connection player, config& cfg);

	void start_game();

	bool take_side(network::connection player, const config& cfg);

	void update_side_data();

	const std::string& transfer_side_control(const config& cfg);

	//function to set the description to the number of slots
	//returns true if the number of slots has changed
	bool describe_slots();

	bool player_is_banned(network::connection player) const;
	void ban_player(network::connection player);
	const player* mute_observer(network::connection player);
	void mute_all_observers(bool mute);

	void add_player(network::connection player, bool observer = false);
	void remove_player(network::connection player, bool notify_creator=true);

	int id() const;

	config construct_server_message(const std::string& message);
	void send_data(const config& data, network::connection exclude=0);
	void send_data_team(const config& data, const std::string& team, network::connection exclude=0);
	void send_data_observers(const config& data);

	void record_data(const config& data);
	void reset_history();

	//the full scenario data
	bool level_init() const;
	config& level();
	bool empty() const;
	void disconnect();

	//functions to set/get the address of the game's summary description as
	//sent to players in the lobby
	void set_description(config* desc);
	config* description();

	//adds players from one game to another. This is used to add players and
	//observers from a game to the lobby (which is also implemented as a game),
	//if that game ends. The second parameter controls, wether the players are
	//added to the players_ or observers_ vector (default observers_).
	void add_players(const game& other_game, bool observer = true);

	//function which will process game commands and update the state of the
	//game accordingly. Will return true iff the game's description changes.
	bool process_commands(const config& cfg);

	bool started() const;

	size_t nplayers() const { return players_.size(); }

	size_t nobservers() const { return observers_.size(); }

	const std::string& termination_reason() const {
		static const std::string aborted = "aborted";
		return termination_.empty() ? aborted : termination_;
	}

	void set_termination_reason(const std::string& reason) {
		if(termination_.empty()) { termination_ = reason; }
	}

	//adds players and observers into one vector and returns that
	const user_vector all_game_users() const;

	const player* find_player(network::connection sock) const;

	const player* transfer_game_control();

private:
	//returns an iterator on the users vector if sock is found
	user_vector::iterator find_connection(network::connection sock, user_vector& users);

	//helps debugging player and observer lists
	std::string debug_player_info() const;

	//function which returns true iff 'player' is on 'team'.
	bool player_on_team(const std::string& team, network::connection player) const;

	//function which should be called every time a player ends their turn
	//(i.e. [end_turn] received). This will update the 'turn' attribute for
	//the game's description when appropriate. Will return true if there has
	//been a change.
	bool end_turn();

	//function to send a list of users to all clients. Only sends data before
	//the game has started.
	void send_user_list();

	const player_map* player_info_;

	static int id_num;
	int id_;
	network::connection owner_;
	user_vector players_;
	user_vector observers_;
	user_vector muted_observers_;
	std::multimap<network::connection,size_t> sides_;
	std::vector<bool> sides_taken_;
	std::vector<std::string> side_controllers_;
	bool started_;

	config level_;

	config history_;

	config* description_;

	int end_turn_;

	bool allow_observers_;
	bool all_observers_muted_;

	struct ban {
		ban(const std::string& name, const std::string& address)
		      : username(name), ipaddress(address)
		{}

		std::string username;
		std::string ipaddress;
	};

	std::vector<ban> bans_;

	std::string termination_;
};

struct game_id_matches
{
	game_id_matches(int id) : id_(id) {}
	bool operator()(const game& g) const { return g.id() == id_; }

private:
	int id_;
};

#endif
