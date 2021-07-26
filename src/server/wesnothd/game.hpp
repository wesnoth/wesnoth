/*
	Copyright (C) 2003 - 2021
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "mt_rng.hpp"
#include "server/wesnothd/player.hpp"
#include "server/wesnothd/player_connection.hpp"
#include "server/common/simple_wml.hpp"
#include "utils/make_enum.hpp"

#include <map>
#include <optional>
#include <vector>

// class player;

namespace wesnothd
{
typedef std::vector<player_iterator> user_vector;
typedef std::vector<std::optional<player_iterator>> side_vector;
class server;

class game
{
public:
	MAKE_ENUM(CONTROLLER,
		(HUMAN, "human")
		(AI, "ai")
		(EMPTY, "null")
	);

	game(wesnothd::server& server, player_connections& player_connections,
			player_iterator host,
			const std::string& name = "",
			bool save_replays = false,
			const std::string& replay_save_path = "");

	~game();

	int id() const
	{
		return id_;
	}

	int db_id() const
	{
		return db_id_;
	}

	void next_db_id()
	{
		db_id_ = db_id_num++;
	}

	const std::string& name() const
	{
		return name_;
	}

	bool is_owner(player_iterator player) const
	{
		return (player == owner_);
	}

	bool is_member(player_iterator player) const
	{
		return is_player(player) || is_observer(player);
	}

	bool allow_observers() const;
	bool is_observer(player_iterator player) const;
	bool is_player(player_iterator player) const;

	/** Checks whether the connection's ip address or username is banned. */
	bool player_is_banned(player_iterator player, const std::string& name) const;

	/** when the host sends the new scenario of a mp campaign */
	void new_scenario(player_iterator player);

	bool level_init() const
	{
		return level_.child("snapshot") || level_.child("scenario");
	}

	static simple_wml::node* starting_pos(simple_wml::node& data)
	{
		if(simple_wml::node* scenario = data.child("scenario")) {
			return scenario;
		} else if(simple_wml::node* snapshot = data.child("snapshot")) {
			return snapshot;
		}

		return &data;
	}

	static const simple_wml::node* starting_pos(const simple_wml::node& data)
	{
		if(const simple_wml::node* scenario = data.child("scenario")) {
			return scenario;
		} else if(const simple_wml::node* snapshot = data.child("snapshot")) {
			return snapshot;
		}

		return &data;
	}

	const simple_wml::node::child_list& get_sides_list() const
	{
		return starting_pos(level_.root())->children("side");
	}

	bool started() const
	{
		return started_;
	}

	std::size_t nplayers() const
	{
		return players_.size();
	}

	std::size_t nobservers() const
	{
		return observers_.size();
	}

	std::size_t current_turn() const
	{
		return current_turn_;
	}

	void set_current_turn(int turn)
	{
		current_turn_ = turn;
	}

	std::string get_replay_filename();

	void mute_all_observers();

	/**
	 * Mute an observer or give a message of all currently muted observers if no
	 * name is given.
	 */
	void mute_observer(const simple_wml::node& mute, player_iterator muter);

	void unmute_observer(const simple_wml::node& unmute, player_iterator unmuter);

	/**
	 * Kick a member by name.
	 *
	 * @return                    The iterator to the removed member if
	 *                            successful, empty optional otherwise.
	 */
	std::optional<player_iterator> kick_member(const simple_wml::node& kick, player_iterator kicker);

	/**
	 * Ban and kick a user by name.
	 *
	 * The user does not need to be in this game but logged in.
	 *
	 * @return                    The iterator to the banned player if he
	 *                            was in this game, empty optional otherwise.
	 */
	std::optional<player_iterator> ban_user(const simple_wml::node& ban, player_iterator banner);

	void unban_user(const simple_wml::node& unban, player_iterator unbanner);

	/**
	 * Add a user to the game.
	 *
	 * @return                    True iff the user successfully joined the game.
	 */
	bool add_player(player_iterator player, bool observer = false);

	/**
	 * Removes a user from the game.
	 *
	 * @return                    True iff the game ends. That is, if there are
	 *                            no more players or the host left on a not yet
	 *                            started game.
	 */
	bool remove_player(player_iterator player, const bool disconnect = false, const bool destruct = false);

	/** Adds players and observers into one vector and returns that. */
	const user_vector all_game_users() const;

	void start_game(player_iterator starter);

	// this is performed just before starting and before [start_game] signal
	// send scenario_diff's specific to each client so that they locally
	// control their human sides
	void perform_controller_tweaks();

	void update_game();

	/** A user (player only?) asks for the next scenario to advance to. */
	void load_next_scenario(player_iterator user); // const

	// iceiceice: I unmarked this const because I want to send and record server messages when I fail to tweak sides
	// properly

	/** Resets the side configuration according to the scenario data. */
	void update_side_data();

	/** Let's a player owning a side give it to another player or observer. */
	void transfer_side_control(player_iterator player, const simple_wml::node& cfg);

	void process_message(simple_wml::document& data, player_iterator);

	/**
	 * Handles [end_turn], repackages [commands] with private [speak]s in them
	 * and sends the data.
	 * Also filters commands from all but the current player.
	 * Currently removes all commands but [speak] for observers and all but
	 * [speak], [label] and [rename] for players.
	 *
	 * @returns                   True if the turn ended.
	 */
	bool process_turn(simple_wml::document& data, player_iterator user);

	/** Handles incoming [whiteboard] data. */
	void process_whiteboard(simple_wml::document& data, player_iterator user);
	/** Handles incoming [change_turns_wml] data. */
	void process_change_turns_wml(simple_wml::document& data, player_iterator user);

	/**
	 * Set the description to the number of available slots.
	 *
	 * @returns                   True iff the number of slots has changed.
	 */
	bool describe_slots();

	void send_server_message_to_all(const char* message, std::optional<player_iterator> exclude = {});
	void send_server_message_to_all(const std::string& message, std::optional<player_iterator> exclude = {})
	{
		send_server_message_to_all(message.c_str(), exclude);
	}

	void send_server_message(
			const char* message, std::optional<player_iterator> player = {}, simple_wml::document* doc = nullptr) const;
	void send_server_message(
			const std::string& message, std::optional<player_iterator> player = {}, simple_wml::document* doc = nullptr) const
	{
		send_server_message(message.c_str(), player, doc);
	}

	/** Send data to all players in this game except 'exclude'. */
	void send_and_record_server_message(const char* message, std::optional<player_iterator> exclude = {});
	void send_and_record_server_message(const std::string& message, std::optional<player_iterator> exclude = {})
	{
		send_and_record_server_message(message.c_str(), exclude);
	}

	template<typename Container>
	void send_to_players(simple_wml::document& data, const Container& players, std::optional<player_iterator> exclude = {});
	void send_data(simple_wml::document& data, std::optional<player_iterator> exclude = {}, std::string packet_type = "");

	void clear_history();
	void record_data(std::unique_ptr<simple_wml::document> data);
	void save_replay();

	/** The full scenario data. */
	simple_wml::document& level()
	{
		return level_;
	}

	/**
	 * Functions to set/get the address of the game's summary description as
	 * sent to players in the lobby.
	 */
	void set_description(simple_wml::node* desc);

	simple_wml::node* description() const
	{
		return description_;
	}

	void set_password(const std::string& passwd)
	{
		password_ = passwd;
	}

	void set_name_bans(const std::vector<std::string> name_bans)
	{
	  name_bans_ = name_bans;
	}

	bool password_matches(const std::string& passwd) const
	{
		return password_.empty() || passwd == password_;
	}

	bool has_password() const
	{
		return !password_.empty();
	}

	const std::string& termination_reason() const
	{
		static const std::string aborted = "aborted";
		static const std::string not_started = "not started";

		return started_ ? (termination_.empty() ? aborted : termination_) : not_started;
	}

	void set_termination_reason(const std::string& reason);

	void handle_choice(const simple_wml::node& data, player_iterator user);

	void handle_random_choice(const simple_wml::node& data);

	void handle_controller_choice(const simple_wml::node& data);

	void handle_add_side_wml(const simple_wml::node& req);

	void reset_last_synced_context_id()
	{
		last_choice_request_id_ = -1;
	}

	/**
	 * Function which returns true iff 'player' controls any of the sides spcified in 'sides'.
	 */
	bool controls_side(const std::vector<int>& sides, player_iterator player) const;

	bool is_reload() const;

private:
	// forbidden operations
	game(const game&) = delete;
	game& operator=(const game&) = delete;

	std::size_t current_side() const
	{
		return nsides_ ? (current_side_index_ % nsides_) : 0;
	}

	std::optional<player_iterator> current_player() const
	{
		return sides_[current_side()];
	}

	bool is_current_player(player_iterator player) const
	{
		return (current_player() == player);
	}

	bool is_muted_observer(player_iterator player) const;
	bool all_observers_muted() const
	{
		return all_observers_muted_;
	}

	void send_muted_observers(player_iterator user) const;

	bool send_taken_side(simple_wml::document& cfg, const simple_wml::node* side) const;

	/**
	 * Figures out which side to take and tells that side to the game owner.
	 *
	 * The owner then should send a [scenario_diff] that implements the side
	 * change and a subsequent update_side_data() call makes it actually
	 * happen.
	 * First we look for a side where save_id= or current_player= matches the
	 * new user's name then we search for the first controller="network" side.
	 */
	bool take_side(player_iterator user);

	/**
	 * Send [change_controller] message to tell all clients the new controller's name
	 * or controller type (human or ai).
	 */
	void change_controller(const std::size_t side_num,
			player_iterator sock,
			const std::string& player_name,
			const bool player_left = true);
	std::unique_ptr<simple_wml::document> change_controller_type(const std::size_t side_num,
			player_iterator player,
			const std::string& player_name);
	void transfer_ai_sides(player_iterator player);
	void send_leave_game(player_iterator user) const;

	/**
	 * @param data the data to be sent to the sides.
	 * @param sides a comma sperated list of side numbers to which the package should be sent.
	 * @param exclude sides to not send the data to.
	 */
	void send_data_sides(simple_wml::document& data,
			const simple_wml::string_span& sides,
			std::optional<player_iterator> exclude = {});

	void send_data_observers(
			simple_wml::document& data, std::optional<player_iterator> exclude = {}, std::string packet_type = "") const;

	/**
	 * Send [observer] tags of all the observers in the game to the user or
	 * everyone if none given.
	 */
	void send_observerjoins(std::optional<player_iterator> player = {});
	void send_observerquit(player_iterator observer);
	void send_history(player_iterator sock) const;

	/** In case of a host transfer, notify the new host about its status. */
	void notify_new_host();

	/** Shortcut to a convenience function for finding a user by name. */
	std::optional<player_iterator> find_user(const simple_wml::string_span& name);

	bool observers_can_label() const
	{
		return false;
	}

	bool observers_can_chat() const
	{
		return true;
	}

	bool is_legal_command(const simple_wml::node& command, player_iterator user);

	/**
	 * Checks whether a user has the same IP as any other members of this game.
	 * @return  A comma separated string of members with matching IPs.
	 */
	std::string has_same_ip(player_iterator user) const;

	/**
	 * Function which should be called every time a player ends their turn
	 * (i.e. [end_turn] received). This will update the 'turn' attribute for
	 * the game's description when appropriate. Will return true iff there has
	 * been a change.
	 */
	bool end_turn(int new_side);

	void update_turn_data();
	/**
	 * Function to send a list of users to all clients.
	 *
	 * Only sends data if the game is initialized but not yet started.
	 */
	void send_user_list(std::optional<player_iterator> exclude = {});

	/** Returns the name of the user or "(unfound)". */
	std::string username(player_iterator pl) const;

	/** Returns a comma separated list of user names. */
	std::string list_users(user_vector users) const;

	/** calculates the initial value for sides_, side_controllerds_, nsides_*/
	void reset_sides();

	/** Helps debugging player and observer lists. */
	std::string debug_player_info() const;

	/** Helps debugging controller tweaks. */
	std::string debug_sides_info() const;

	wesnothd::server& server;
	player_connections& player_connections_;

	// used for unique identification of game instances within wesnothd
	static int id_num;
	int id_;

	// used for unique identification of games played in the database
	// necessary since for MP campaigns multiple scenarios can be played within the same game instance
	// and we need a unique ID per scenario played, not per game instance
	static int db_id_num;
	int db_id_;

	/** The name of the game. */
	std::string name_;
	std::string password_;

	/** The game host or later owner (if the host left). */
	player_iterator owner_;

	/** A vector of players (members owning a side). */
	user_vector players_;

	/** A vector of observers (members not owning a side). */
	user_vector observers_;
	user_vector muted_observers_;

	/** A vector of side owners. */
	side_vector sides_;

	std::vector<CONTROLLER> side_controllers_;

	/** Number of sides in the current scenario. */
	int nsides_;
	bool started_;

	/**
		The current scenario data.´
		WRONG! This contains the initial state or the state from which
		the game was loaded from.
		Using this to make assumptions about the current gamestate is
		extremely dangerous and should especially not be done for anything
		that can be nodified by wml (especially by [modify_side]),
		like team_name, controller ... in [side].
		FIXME: move every code here that uses this object to query those
		information to the clients. But note that there are some checks
		(like controller == null) that are definitely needed by the server and
		in this case we should try to modify the client to inform the server if
		a change of those properties occur. Ofc we shouldn't update level_
		then, but rather store that information in a separate object
		(like in side_controllers_).
	*/
	simple_wml::document level_;

	/** Replay data. */
	mutable std::vector<std::unique_ptr<simple_wml::document>> history_;

	/** Pointer to the game's description in the games_and_users_list_. */
	simple_wml::node* description_;

	int current_turn_;
	int current_side_index_;
	int num_turns_;
	bool all_observers_muted_;

	// IP ban list and name ban list
	std::vector<std::string> bans_;
	std::vector<std::string> name_bans_;
	/**
	 * in multiplayer campaigns it can happen that some players are still in the previous scenario
	 * keep track of those players because processing certain
	 * input from those side wil lead to error (oos)
	 */
	std::set<const player_record*> players_not_advanced_;

	std::string termination_;

	bool save_replays_;
	std::string replay_save_path_;

	/** A wrapper for mersenne twister rng which generates randomness for this game */
	randomness::mt_rng rng_;
	int last_choice_request_id_;
};

} // namespace wesnothd
