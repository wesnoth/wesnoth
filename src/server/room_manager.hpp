/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "room.hpp"

#include <boost/utility.hpp>

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
	room_manager(player_map& all_players);
	~room_manager();

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
	 * Delete a room. Players in the room are kicked from it.
	 */
	void delete_room(const std::string& name);

	/**
	 * @return true iif the player is in the lobby
	 */
	bool in_lobby(network::connection player) const;

	/**
	 * Player-enters-lobby action. Will auto(re)join the default room(s)
	 */
	void enter_lobby(network::connection player);

	/**
	 * All players from a game re-enter the lobby
	 */
	void enter_lobby(const game& game);

	/**
	 * Player exits lobby.
	 * TODO: should it remove information about the rooms the player is in?
	 */
	void exit_lobby(network::connection player);

	/**
	 * Remove info abut given player from all rooms
	 */
	void remove_player(network::connection player);

	/**
	 * Check if the room exists and if the player is a member, log failures.
	 * @return non-NULL iff the room exists and the player is a member
	 */
	room* require_member(const std::string& room_name,
		const player_map::iterator user, const char* log_string = "use");

	void process_message(simple_wml::document& data, const player_map::iterator user);

	void process_room_join(simple_wml::document& data, const player_map::iterator user);

	void process_room_part(simple_wml::document& data, const player_map::iterator user);

	const room& lobby() const { return *lobby_; }
private:
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



	player_map& all_players_;
	room* lobby_;
	typedef std::map<std::string, room*> t_rooms_by_name_;
	t_rooms_by_name_ rooms_by_name_;
	typedef std::map<network::connection, std::set<room*> > t_rooms_by_player_;
	t_rooms_by_player_ rooms_by_player_;

	static const char* const lobby_name_;
};

} //namespace wesnothd


#endif
