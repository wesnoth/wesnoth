/*
   Copyright (C) 2009 - 2014 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "room.hpp"

#include <boost/utility.hpp>

class config;

#ifndef SERVER_ROOM_MANAGER_HPP_INCLUDED
#define SERVER_ROOM_MANAGER_HPP_INCLUDED

namespace wesnothd {

/**
 * The room manager manages the lobby and other rooms in the server, and related
 * client message processing.
 * The lobby represents players that are on the server, but not in any game.
 */
class room_manager : private boost::noncopyable
{
public:
	/**
	 * Room manager constructor
	 */
	room_manager(player_map& all_players);

	/**
	 * Room manager destructor
	 */
	~room_manager();

	enum PRIVILEGE_POLICY {
		PP_EVERYONE,
		PP_REGISTERED,
		PP_ADMINS,
		PP_NOBODY,
		PP_COUNT
	};

	static PRIVILEGE_POLICY pp_from_string(const std::string& str);

	/**
	 * Load settings from the main config file
	 */
	void load_config(const config& cfg);

	/**
	 * Reads stored rooms from a file on disk, or returns immediately
	 * if load_config was not called before or the storage filename is empty
	 */
	void read_rooms();

	/**
	 * Writes rooms to the storage file or returns immediately if load_config
	 * was not called beforethe storage filename is empty
	 */
	void write_rooms();

	/**
	 * Dirty flag for rooms -- true if there were changes that should be written
	 * to disk
	 */
	bool dirty() const { return dirty_; }

	/**
	 * Get a room by name, or NULL if it does not exist
	 */
	room* get_room(const std::string& name);

	/**
	 * @param  name the room name to check
	 * @return true iif the room existst
	 */
	bool room_exists(const std::string& name) const;

	/**
	 * Create room named "name" if it does not exist already.
	 */
	room* create_room(const std::string& name);

	/**
	 * Get a room by name or create that room if it does not exist and
	 * creating rooms is allowed.
	 * @return a valid pointer to a room or NULL if the room did not exist and
	 *         could not be created.
	 */
	room* get_create_room(const std::string& name, network::connection player);

	/**
	 * @return true iif the player is in the lobby
	 */
	bool in_lobby(network::connection player) const;

	/**
	 * Player-enters-lobby action. Will autorejoin "stored" rooms (the ones
	 * the player was before enetering a game, for instance)
	 */
	void enter_lobby(network::connection player);

	/**
	 * All players from a game re-enter the lobby
	 */
	void enter_lobby(const game& game);

	/**
	 * Player exits lobby.
	 */
	void exit_lobby(network::connection player);

	/**
	 * Remove info abut given player from all rooms
	 */
	void remove_player(network::connection player);

	/**
	 * Check if the room exists, log failures.
	 * @return non-NULL iff the room exists and the player is a member
	 */
	room* require_room(const std::string& room_name,
		const player_map::iterator user, const char* log_string = "use");

	/**
	 * Check if the room exists and if the player is a member, log failures.
	 * @return non-NULL iff the room exists and the player is a member
	 */
	room* require_member(const std::string& room_name,
		const player_map::iterator user, const char* log_string = "use");

	/**
	 * Process a message (chat message) sent to a room. Check conditions
	 * and resend to other players in the room.
	 */
	void process_message(simple_wml::document& data, const player_map::iterator user);

	/**
	 * Process a player's request to join a room
	 */
	void process_room_join(simple_wml::document& data, const player_map::iterator user);

	/**
	 * Process a player's request to leave a room
	 */
	void process_room_part(simple_wml::document& data, const player_map::iterator user);

	/**
	 * Process a general room query
	 */
	void process_room_query(simple_wml::document& data, const player_map::iterator user);

	/**
	 * Lobby convenience accesor
	 */
	const room& lobby() const { return *lobby_; }

private:
	void do_room_join(network::connection player, const std::string& room_name);

	/**
	 * Adds a player to a room, maintaining internal consistency
	 * Will send appropriate error messages to the player.
	 * @return true iif the operation was successful, false otherwise
	 */
	bool player_enters_room(network::connection player, room* room);

	/**
	 * Removes a player from a room, maintaining internal consistency
	 */
	void player_exits_room(network::connection player, room* room);

	/**
	 * Stores the room names (other than lobby) of the given player for future
	 * use (rejoin)
	 */
	void store_player_rooms(network::connection player);

	/**
	 * Unstores (rejoins) player's rooms that were previously stored.
	 * No action if not stored earlier or no rooms.
	 */
	void unstore_player_rooms(const player_map::iterator user);

	/**
	 * Helper function that calls the player_map::iterator version
	 * of unstore_player_rooms
	 */
	void unstore_player_rooms(network::connection player);

	/**
	 * Fill a wml node (message) with members of a room
	 */
	void fill_member_list(const room* room, simple_wml::node& root);

	/**
	 * Fill a wml node (message) with a room list
	 */
	void fill_room_list(simple_wml::node& root);

	/** Reference to the all players map */
	player_map& all_players_;

	/** The lobby-room, treated separetely */
	room* lobby_;

	/** Rooms by name */
	typedef std::map<std::string, room*> t_rooms_by_name_;
	t_rooms_by_name_ rooms_by_name_;

	/** Rooms by player */
	typedef std::map<network::connection, std::set<room*> > t_rooms_by_player_;
	t_rooms_by_player_ rooms_by_player_;

	/** Room names stored for players that have entered a game */
	typedef std::map<network::connection, std::set<std::string> > t_player_stored_rooms_;
	t_player_stored_rooms_ player_stored_rooms_;

	/**
	 * Persistent room storage filename.  If empty, rooms are not stored on disk.
	 */
	std::string filename_;

	/**
	 * Flag controlling whether to compress the stored rooms or not
	 */
	bool compress_stored_rooms_;

	/**
	 * Policy regarding who can create new rooms
	 */
	PRIVILEGE_POLICY new_room_policy_;

	/**
	 * 'Dirty' flag with regards to room info that will be stored on disk
	 */
	bool dirty_;

	/**
	 * The main (lobby) room name
	 */
	static const char* const lobby_name_;
};

} //namespace wesnothd


#endif
