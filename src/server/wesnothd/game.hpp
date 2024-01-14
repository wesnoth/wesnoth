/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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
#include "server/wesnothd/player_connection.hpp"
#include "server/common/simple_wml.hpp"
#include "side_controller.hpp"

#include "utils/optional_fwd.hpp"

#include <vector>

// class player;

namespace wesnothd
{
typedef std::vector<player_iterator> user_vector;
typedef std::vector<utils::optional<player_iterator>> side_vector;
class server;

class game
{
public:
	game(wesnothd::server& server, player_connections& player_connections,
			player_iterator host,
			const std::string& name = "",
			bool save_replays = false,
			const std::string& replay_save_path = "");

	~game();

	/**
	 * This ID is reused between scenarios of MP campaigns.
	 * This ID resets when wesnothd is restarted.
	 * This is generally used when needing to find a particular running game.
	 * @return an ID that uniquely identifies the game within the currently running wesnothd instance.
	 */
	int id() const
	{
		return id_;
	}

	/**
	 * This ID is not reused between scenarios of MP campaigns.
	 * This ID resets when wesnothd is restarted.
	 * This is generally used during database queries.
	 *
	 * @return an ID that uniquely identifies the game within the currently running wesnothd instance.
	 */
	int db_id() const
	{
		return db_id_;
	}

	/**
	 * Increments the ID used when running database queries.
	 */
	void next_db_id()
	{
		db_id_ = db_id_num++;
	}

	/**
	 * @return The game's name.
	 */
	const std::string& name() const
	{
		return name_;
	}

	/**
	 * @param player The player being checked.
	 * @return Whether the provided player is the game's owner(host).
	 */
	bool is_owner(player_iterator player) const
	{
		return (player == owner_);
	}

	/**
	 * @param player The player being checked.
	 * @return Whether the provided player has joined the game.
	 */
	bool is_member(player_iterator player) const
	{
		return is_player(player) || is_observer(player);
	}

	/**
	 * @return Whether observers are allowed to join.
	 */
	bool allow_observers() const;

	/**
	 * @param player The player being checked.
	 * @return Whether the provided player is an observer of this game.
	 */
	bool is_observer(player_iterator player) const;

	/**
	 * @param player The player being checked.
	 * @return Whether the provided player is playing this game (aka owns one or more sides).
	 */
	bool is_player(player_iterator player) const;

	/**
	 * @param player The player being checked (by iterator).
	 * @param name The player being checked (by username).
	 * @return Whether the connection's ip address or username is banned from this game.
	 */
	bool player_is_banned(player_iterator player, const std::string& name) const;

	/**
	 * When the host sends the new scenario of a mp campaign
	 *
	 * @param sender The player sending the scenario data.
	 */
	void new_scenario(player_iterator sender);

	/**
	 * @return Whether this game contains scenario data and thus has been initialized.
	 */
	bool level_init() const
	{
		return level_.child("snapshot") || level_.child("scenario");
	}

	/**
	 * The non-const version.
	 *
	 * @param data The data describing the level for a game.
	 * @return The [scenario] child node if it exists, else the [snapshot] child if it exists, else @a data.
	 */
	static simple_wml::node* starting_pos(simple_wml::node& data)
	{
		if(simple_wml::node* scenario = data.child("scenario")) {
			return scenario;
		} else if(simple_wml::node* snapshot = data.child("snapshot")) {
			return snapshot;
		}

		return &data;
	}

	/**
	 * The const version.
	 *
	 * @param data The data describing the level for a game.
	 * @return The [scenario] child node if it exists, else the [snapshot] child if it exists, else @a data.
	 */
	static const simple_wml::node* starting_pos(const simple_wml::node& data)
	{
		if(const simple_wml::node* scenario = data.child("scenario")) {
			return scenario;
		} else if(const simple_wml::node* snapshot = data.child("snapshot")) {
			return snapshot;
		}

		return &data;
	}

	/**
	 * @return The nodes containing the sides in this game.
	 */
	const simple_wml::node::child_list& get_sides_list() const
	{
		return starting_pos(level_.root())->children("side");
	}

	/**
	 * @return Whether this game has started yet.
	 */
	bool started() const
	{
		return started_;
	}

	/**
	 * @return The number of players. One player can have multiple sides.
	 */
	std::size_t nplayers() const
	{
		return players_.size();
	}

	/**
	 * @return The number of observers in this game.
	 */
	std::size_t nobservers() const
	{
		return observers_.size();
	}

	/**
	 * @return This game's current turn.
	 */
	std::size_t current_turn() const
	{
		return current_turn_;
	}

	/**
	 * @return The name of the replay for this game.
	 */
	std::string get_replay_filename();

	/** Toggles whether all observers are muted or not. */
	void mute_all_observers();

	/**
	 * Mute an observer or give a message of all currently muted observers if no name is given.
	 *
	 * @param mute The observer to mute. Empty if sending a message to muted observers.
	 * @param muter The player doing the muting.
	 */
	void mute_observer(const simple_wml::node& mute, player_iterator muter);

	/**
	 * Unmute an observer or unmute all currently muted observers if no name is given.
	 *
	 * @param unmute The observer to unmute. Empty if unmuting all observers.
	 * @param unmuter The player doing the unmuting.
	 */
	void unmute_observer(const simple_wml::node& unmute, player_iterator unmuter);

	/**
	 * Kick a user from this game by name.
	 *
	 * @param kick The user to kick.
	 * @param kicker The player doing the kicking.
	 * @return The iterator to the removed member if successful, empty optional otherwise.
	 */
	utils::optional<player_iterator> kick_member(const simple_wml::node& kick, player_iterator kicker);

	/**
	 * Ban a user by name.
	 *
	 * The user does not need to be in this game but logged in.
	 *
	 * @param ban The user to ban.
	 * @param banner The player doing the banning.
	 * @return The iterator to the banned player if he was in this game, empty optional otherwise.
	 */
	utils::optional<player_iterator> ban_user(const simple_wml::node& ban, player_iterator banner);

	/**
	 * Unban a user by name.
	 *
	 * The user does not need to be in this game but logged in.
	 *
	 * @param unban The user to unban.
	 * @param unbanner The player doing the unbanning.
	 */
	void unban_user(const simple_wml::node& unban, player_iterator unbanner);

	/**
	 * Add a user to the game.
	 *
	 * @todo differentiate between "observers not allowed" and "player already in the game" errors.
	 * maybe return a string with an error message.
	 *
	 * @param player The player to add.
	 * @param observer Whether to add the player as an observer.
	 * @return True if the user successfully joined the game, false otherwise.
	 */
	bool add_player(player_iterator player, bool observer = false);

	/**
	 * Removes a user from the game.
	 *
	 * @param player The player to remove.
	 * @param disconnect If the player disconnected from the server entirely.
	 * @param destruct If the game is ending as well.
	 * @return True if the player's removal ends the game. That is, if there are no more players or the host left on a not yet started game.
	 */
	bool remove_player(player_iterator player, const bool disconnect = false, const bool destruct = false);

	/**
	 * @return A vector containing all players and observers currently in this game.
	 */
	const user_vector all_game_users() const;

	/**
	 * Starts the game (if a new game) or starts the next scenario of an MP campaign.
	 * @param starter The game's host.
	 */
	void start_game(player_iterator starter);

	/**
	 * This is performed just before starting and before the [start_game] signal.
	 * Sends [scenario_diff]s specific to each client so that they locally control their human sides.
	 */
	void perform_controller_tweaks();

	/**
	 * A user asks for the next scenario to advance to.
	 *
	 * @param user The user asking for the next scenario.
	 */
	void load_next_scenario(player_iterator user);

	/** Resets the side configuration according to the scenario data. */
	void update_side_data();

	/**
	 * Lets a player owning a side give it to another player or observer.
	 *
	 * @param player The player owning the side.
	 * @param cfg The node containing the transfer information.
	 */
	void transfer_side_control(player_iterator player, const simple_wml::node& cfg);

	/**
	 * Sends an ingame message to all other players.
	 *
	 * @param data The message to send.
	 * @param user The user sending the message.
	 */
	void process_message(simple_wml::document& data, player_iterator user);

	/**
	 * Handles [end_turn], repackages [commands] with private [speak]s in them
	 * and sends the data.
	 * Also filters commands from all but the current player.
	 * Currently removes all commands but [speak] for observers and all but
	 * [speak], [label], and [rename] for players.
	 *
	 * @param data The turn commands.
	 * @param user The user who sent a command to be processed during the turn. This may not be the player whose turn it currently is.
	 * @returns True if the turn ended.
	 */
	bool process_turn(simple_wml::document& data, player_iterator user);

	/**
	 * Handles incoming [whiteboard] data.
	 *
	 * @param data The whiteboard data.
	 * @param user The user sending the whiteboard data.
	 */
	void process_whiteboard(simple_wml::document& data, player_iterator user);

	/**
	 * Handles incoming [change_turns_wml] data.
	 *
	 * @param data The [change_turns_wml] data.
	 * @param user The player changing turns.
	 */
	void process_change_turns_wml(simple_wml::document& data, player_iterator user);

	/**
	 * Set the description to the number of available slots.
	 */
	void describe_slots();

	/**
	 * Sends a message to all players in this game that aren't excluded.
	 *
	 * @param message The message to send.
	 * @param exclude The players to not send the message to.
	 */
	void send_server_message_to_all(const char* message, utils::optional<player_iterator> exclude = {});
	/**
	 * @ref send_server_message_to_all
	 */
	void send_server_message_to_all(const std::string& message, utils::optional<player_iterator> exclude = {})
	{
		send_server_message_to_all(message.c_str(), exclude);
	}

	/**
	 * Send a server message to the specified player.
	 *
	 * @param message The message to send.
	 * @param player The player to send the message to. If empty then the message is not sent.
	 * @param doc The document to create the message in. If nullptr then a new document is created.
	 */
	void send_server_message(
			const char* message, utils::optional<player_iterator> player = {}, simple_wml::document* doc = nullptr) const;
	/**
	 * @ref send_server_message
	 */
	void send_server_message(
			const std::string& message, utils::optional<player_iterator> player = {}, simple_wml::document* doc = nullptr) const
	{
		send_server_message(message.c_str(), player, doc);
	}

	/**
	 * Send data to all players in this game except 'exclude'.
	 * Also record this data for the replay.
	 *
	 * @param message The message to send.
	 * @param exclude The players to not send the message to.
	 */
	void send_and_record_server_message(const char* message, utils::optional<player_iterator> exclude = {});
	/**
	 * @ref send_and_record_server_message
	 */
	void send_and_record_server_message(const std::string& message, utils::optional<player_iterator> exclude = {})
	{
		send_and_record_server_message(message.c_str(), exclude);
	}

	/**
	 * Send data to all players except those excluded.
	 * For example, to send a message to all players except the player who typed the original message.
	 *
	 * @param data The data to send.
	 * @param players The players to send the data to.
	 * @param exclude The player from @a players to not send the data to.
	 */
	template<typename Container>
	void send_to_players(simple_wml::document& data, const Container& players, utils::optional<player_iterator> exclude = {});

	/**
	 * Send data to all players and observers except those excluded.
	 *
	 * @param data The data to send.
	 * @param exclude The players/observers to not send the data to.
	 */
	void send_data(simple_wml::document& data, utils::optional<player_iterator> exclude = {});

	/**
	 * Clears the history of recorded WML documents.
	 */
	void clear_history();

	/**
	 * Clears the history of recorded chat WML documents.
	 */
	void clear_chat_history();

	/**
	 * Records a WML document in the game's history.
	 *
	 * @param data The WML document to record.
	 */
	void record_data(std::unique_ptr<simple_wml::document> data);

	/**
	 * Move the level information and recorded history into a replay file and save it.
	 */
	void save_replay();

	/**
	 * @return The full scenario data.
	 */
	simple_wml::document& level()
	{
		return level_;
	}

	/**
	 * Set the game's description.
	 * Also set the game as requiring a password if a password is set.
	 *
	 * @param desc The node containing the game's description.
	 */
	void set_description(simple_wml::node* desc);

	/**
	 * @return The node containing the game's current description.
	 */
	simple_wml::node* description() const
	{
		return description_;
	}

	/**
	 * @return The node containing the game's current description. and remembers that it was changed.
	 */
	simple_wml::node* description_for_writing()
	{
		description_updated_ = true;
		return description_;
	}

	/**
	 * @return The node containing the game's current description if it was changed.
	 */
	simple_wml::node* changed_description()
	{
		if(description_updated_) {
			description_updated_ = false;
			return description_;
		}
		return nullptr;
	}

	/**
	 * Sets the password required to access the game.
	 *
	 * @param passwd The password to set.
	 */
	void set_password(const std::string& passwd)
	{
		password_ = passwd;
	}

	/**
	 * Set a list of usernames that should all be banned from joining the game.
	 *
	 * @param name_bans The list of usernames.
	 */
	void set_name_bans(const std::vector<std::string> name_bans)
	{
	  name_bans_ = name_bans;
	}

	/**
	 * @param passwd The password to join with.
	 * @return True if the game's password is empty or if the provided password matches, false otherwise.
	 */
	bool password_matches(const std::string& passwd) const
	{
		return password_.empty() || passwd == password_;
	}

	/**
	 * @return Whether the game has a password set.
	 */
	bool has_password() const
	{
		return !password_.empty();
	}

	/**
	 * Provides the reason the game was ended.
	 *
	 * @return Either that the game was aborted (after starting), not started, or has some other reason set.
	 */
	const std::string& termination_reason() const
	{
		static const std::string aborted = "aborted";
		static const std::string not_started = "not started";

		return started_ ? (termination_.empty() ? aborted : termination_) : not_started;
	}

	/**
	 * Sets the termination reason for this game.
	 *
	 * @param reason The termination reason.
	 */
	void set_termination_reason(const std::string& reason);

	/**
	 * Handle a choice requested by a client, such as changing a side's controller, if initiated by WML/lua.
	 *
	 * @param data The data needed to process the choice.
	 * @param user The player making the request.
	 */
	void handle_choice(const simple_wml::node& data, player_iterator user);

	/**
	 * Send a randomly generated number to the requestor.
	 */
	void handle_random_choice();

	/**
	 * Handle a request to change a side's controller.
	 * Note that this does not change who owns a side.
	 *
	 * @param data Contains the information about which side to change the controller of.
	 */
	void handle_controller_choice(const simple_wml::node& data);

	/**
	 * Adds a new, empty side owned by no one.
	 */
	void handle_add_side_wml();

	/**
	 * Reset the internal counter for choice requests made by clients to the server.
	 */
	void reset_last_synced_context_id()
	{
		last_choice_request_id_ = -1;
	}

	/**
	 * Function which returns true if 'player' controls any of the sides specified in 'sides'.
	 *
	 * @param sides The list of sides in this game.
	 * @param player The player being checked for whether they own any sides.
	 */
	bool controls_side(const std::vector<int>& sides, player_iterator player) const;

	/**
	 * @return Whether the loaded WML has the attribute indicating that this is a reloaded savegame rather than a brand new game.
	 */
	bool is_reload() const;

	void emergency_cleanup()
	{
		players_.clear();
		observers_.clear();
	}

private:
	// forbidden operations
	game(const game&) = delete;
	game& operator=(const game&) = delete;

	/**
	 * @return 0 if there are no sides, or the current side index otherwise.
	 */
	std::size_t current_side() const
	{
		// At the start of the game it can happen that current_side_index_ is 0,
		// but the first side is empty. It's better to do this than to skip empty
		// sides in start_game() in case the controller changes during start events.
		return get_next_nonempty(current_side_index_);
	}

	/**
	 * @return The player who owns the side at index @a index.
	 * nullopt if wither index is invalid or the side is not owned.
	 */
	utils::optional<player_iterator> get_side_player(size_t index) const
	{
		return index >= sides_.size() ? utils::optional<player_iterator>() : sides_[index];
	}

	/**
	 * @return The player who owns the current side.
	 */
	utils::optional<player_iterator> current_player() const
	{
		// sides_ should never be empty but just to be sure.
		return get_side_player(current_side());
	}

	/**
	 * @param player The player being checked.
	 * @return Whether the player being checked is the current player taking their turn.
	 */
	bool is_current_player(player_iterator player) const
	{
		return (current_player() == player);
	}

	/**
	 * @param player The observer being checked.
	 * @return True if the observer is muted or if all observers are muted, false otherwise.
	 */
	bool is_muted_observer(player_iterator player) const;

	/**
	 * @return True if all observers have been muted via that command (not if each individual observer happens to have been manually muted).
	 */
	bool all_observers_muted() const
	{
		return all_observers_muted_;
	}

	/**
	 * Sends a message either stating that all observers are muted or listing the observers that are muted.
	 *
	 * @param user The player to send the message to.
	 */
	void send_muted_observers(player_iterator user) const;

	/**
	 * Tell the host who owns a side.
	 *
	 * @param cfg The document to send to the host.
	 * @param side The side information to send.
	 * @return True if the document was sent, false otherwise.
	 */
	bool send_taken_side(simple_wml::document& cfg, const simple_wml::node* side) const;

	/**
	 * Figures out which side to take and tells that side to the game owner.
	 *
	 * The owner then should send a [scenario_diff] that implements the side
	 * change and a subsequent update_side_data() call makes it actually
	 * happen.
	 * First we look for a side where save_id= or current_player= matches the
	 * new user's name then we search for the first controller=human or reserved side.
	 *
	 * @param user The player taking a side.
	 * @return True if the side was taken, false otherwise.
	 */
	bool take_side(player_iterator user);

	/**
	 * Send [change_controller] message to tell all clients the new controller's name or controller type (human or ai).
	 *
	 * @param side_index The index of the side whose controller is changing.
	 * @param player The player who is taking control of the side.
	 * @param player_name The name of the player who is taking control of the side.
	 * @param player_left We use the "player_left" field as follows. Normally change_controller sends one message to the owner, and one message to everyone else.
	 *                    In case that a player drops, the owner is gone and should not get a message, instead the host gets a [side_drop] message.
	 */
	void change_controller(const std::size_t side_index,
			player_iterator player,
			const std::string& player_name,
			const bool player_left = true);

	/**
	 * Tell everyone else but the source player that the controller type changed.
	 *
	 * @param side_index The index of the side whose controller type is changing.
	 * @param player The player who owns the side whose controller type is changing.
	 * @param player_name The name of the player who owns the side whose controller type is changing.
	 * @return The document that was sent to all other players.
	 */
	std::unique_ptr<simple_wml::document> change_controller_type(const std::size_t side_index,
			player_iterator player,
			const std::string& player_name);

	/**
	 * Tells a player to leave the game.
	 *
	 * @param user The player leaving the game.
	 */
	void send_leave_game(player_iterator user) const;

	/**
	 * Sends a document to the provided list of sides.
	 *
	 * @param data The data to be sent to the provided sides.
	 * @param sides A comma sperated list of side numbers to which the document should be sent.
	 * @param exclude Players to not send the data to.
	 */
	void send_data_sides(simple_wml::document& data,
			const simple_wml::string_span& sides,
			utils::optional<player_iterator> exclude = {});

	/**
	 * Send a document per observer in the game.
	 * If @a player is blank, send these documents to everyone, else send them to just the observer who joined.
	 *
	 * @param player The observer who joined.
	 */
	void send_observerjoins(utils::optional<player_iterator> player = {});
	void send_observerquit(player_iterator observer);
	void send_history(player_iterator sock) const;
	void send_chat_history(player_iterator sock) const;

	/** In case of a host transfer, notify the new host about its status. */
	void notify_new_host();

	/**
	 * Shortcut to a convenience function for finding a user by name.
	 *
	 * @param name The name of the user to find.
	 * @return The player if found, else empty.
	 */
	utils::optional<player_iterator> find_user(const simple_wml::string_span& name);

	bool is_legal_command(const simple_wml::node& command, player_iterator user);

	/**
	 * Checks whether a user has the same IP as any other members of this game.
	 * @return  A comma separated string of members with matching IPs.
	 */
	std::string has_same_ip(player_iterator user) const;

	/**
	 * Function which should be called every time a player ends their turn
	 * (i.e. [end_turn] received).
	 *
	 * @param new_side The side number whose turn to move it has become.
	 */
	void end_turn(int new_side);
	/**
	 * Function which should be called every time a player starts their turn
	 * (i.e. [init_side] received). This will update the 'turn' attribute for
	 * the game's description when appropriate.
	 */
	void init_turn();

	/**
	 * Set or update the current and max turn values in the game's description.
	 */
	void update_turn_data();

	/**
	 * Function to send a list of users to all clients.
	 * Only sends data if the game is initialized but not yet started.
	 *
	 * @param exclude The players to not send the list of users to.
	 */
	void send_user_list(utils::optional<player_iterator> exclude = {});

	/**
	 * @param pl The player.
	 * @return The player's username.
	 */
	std::string username(player_iterator pl) const;

	/**
	 * @param users The users to create a comma separated list from.
	 * @return A comma separated list of user names.
	 */
	std::string list_users(const user_vector& users) const;

	/** calculates the initial value for sides_, side_controllerds_, nsides_*/
	void reset_sides();

	/**
	 * Helps debugging player and observer lists.
	 *
	 * @return A string listing the game IDs, players, and observers.
	 */
	std::string debug_player_info() const;

	/**
	 * Helps debugging controller tweaks.
	 *
	 * @return A string listing the game IDs and side information.
	 */
	std::string debug_sides_info() const;

	/// @return the side index for which we accept [init_side]
	int get_next_side_index() const;
	/**
	 * finds the first side starting at @a side_index that is non empty.
	 */
	int get_next_nonempty(int side_index) const;

	/** The wesnothd server instance this game exists on. */
	wesnothd::server& server;
	player_connections& player_connections_;

	/**
	 * Incremented to retrieve a unique ID for game instances within wesnothd.
	 */
	static int id_num;
	/** This game's ID within wesnothd */
	int id_;

	/**
	 * Incremented to retrieve a unique ID per wesnothd instance for game instances within the database.
	 */
	static int db_id_num;
	/**
	 * Used for unique identification of games played in the database.
	 * Necessary since for MP campaigns multiple scenarios can be played within the same game instance
	 * and we need a unique ID per scenario played, not per game instance.
	 */
	int db_id_;

	/** The name of the game. */
	std::string name_;
	/** The password needed to join the game. */
	std::string password_;

	/** The game host or later owner (if the host left). */
	player_iterator owner_;

	/** A vector of players (members owning a side). */
	user_vector players_;

	/** A vector of observers (members not owning a side). */
	user_vector observers_;
	/** A vector of muted observers. */
	user_vector muted_observers_;

	/** A vector of side owners. */
	side_vector sides_;

	/** A vector containiner the controller type for each side. */
	std::vector<side_controller::type> side_controllers_;

	/** Number of sides in the current scenario. */
	int nsides_;
	/** Whether the game has been started or not. */
	bool started_;

	/**
		The current scenario data.

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
	/** Replay chat history data. */
	mutable std::vector<std::unique_ptr<simple_wml::document>> chat_history_;

	/** Pointer to the game's description in the games_and_users_list_. */
	simple_wml::node* description_;

	/** Set to true whenever description_ was changed that an update needs to be sent to clients. */
	bool description_updated_;

	/** The game's current turn. */
	int current_turn_;
	/** The index of the current side. The side number is current_side_index_+1. */
	int current_side_index_;
	/**
	 * after [end_turn] was received, this contains the side for who we accept [init_side].
	 * -1 if we currently don't accept [init_side] because the current player didn't end his turn yet.
	 **/
	int next_side_index_;
	/** The maximum number of turns before the game ends. */
	int num_turns_;
	/** Whether all observers should be treated as muted. */
	bool all_observers_muted_;

	/** List of banned IPs */
	std::vector<std::string> bans_;
	/** List of banned usernames */
	std::vector<std::string> name_bans_;

	/**
	 * in multiplayer campaigns it can happen that some players are still in the previous scenario
	 * keep track of those players because processing certain
	 * input from those side wil lead to error (oos)
	 */
	std::set<const player_record*> players_not_advanced_;

	/** The reason the game ended. */
	std::string termination_;

	/** Whether to save a replay of this game. */
	bool save_replays_;
	/** Where to save the replay of this game. */
	std::string replay_save_path_;

	/** A wrapper for mersenne twister rng which generates randomness for this game */
	randomness::mt_rng rng_;
	/**
	 * The ID of the last request received from a client.
	 * New requests should never have a lower value than this.
	 */
	int last_choice_request_id_;
};

} // namespace wesnothd
