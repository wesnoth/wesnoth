/*
   Copyright (C) 2009 - 2015 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERVER_ROOM_HPP_INCLUDED
#define SERVER_ROOM_HPP_INCLUDED

#include "../network.hpp"
#include "player.hpp"
#include "simple_wml.hpp"

namespace wesnothd {

typedef std::vector<network::connection> connection_vector;
typedef std::map<network::connection,player> player_map;
class game;

/**
 * A room is a group of players that can communicate via messages.
 */
class room {
public:
	/**
	 * Construct a room with just a name and default settings
	 */
	room(const std::string& name);

	/**
	 * Construct a room from WML
	 */
	room(const config& cfg);

	/**
	 * Write room info to a config
	 */
	void write(config& cfg) const;

	/**
	 * The name of this room
	 */
	const std::string& name() const;

	/**
	 * Whether this room should be 'persistent', i.e. not deleted when there
	 * are no players within and stored on disk if needed.
	 */
	bool persistent() const;

	/**
	 * Set the persistent flag for this room
	 */
	void set_persistent(bool v);

	/**
	 * Whether the room is logged (and might end up in e.g. the lobby bot
	 */
	bool logged() const;

	/**
	* Set the room's logged flag
	*/
	void set_logged(bool v);

	/**
	 * This room's topic/motd, sent to all joining players
	 */
	const std::string& topic() const;

	/**
	 * Set the topic for this room
	 */
	void set_topic(const std::string& v);

	/**
	 * Return the number of players in this room
	 */
	size_t size() const {
		return members_.size();
	}

	/**
	 * Return true iif the room is empty
	 */
	bool empty() const {
		return members_.empty();
	}

	/**
	 * Return the members of this room
	 */
	const std::vector<network::connection>& members() const {
		return members_;
	}

	/**
	 * Membership checker.
	 * @return true iif player is a member of this room
	 */
	bool is_member(network::connection player) const {
		return std::find(members_.begin(), members_.end(), player) != members_.end();
	}

	/**
	 * Joining the room
	 * @return true if the player was successfully added
	 */
	bool add_player(network::connection player);

	/**
	 * Leaving the room
	 */
	//void remove_player(network::connection player);

	/**
	 * Chat message processing
	 */
	void process_message(simple_wml::document& data, const player_map::iterator user);

	/**
	 * Convenience function for sending a wml document to all (or all except
	 * one) members.
	 * @see send_to_many
	 * @param data        the document to send
	 * @param exclude     if nonzero, do not send to this player
	 * @param packet_type the packet type, if empty the root node name is used
	 */
	void send_data(simple_wml::document& data, const network::connection exclude=0, std::string packet_type = "") const;

	/**
	 * Send a text message to all members
	 * @param message the message text
	 * @param exclude if nonzero, do not send to this player
	 */
	void send_server_message_to_all(const char* message, network::connection exclude=0) const;
	void send_server_message_to_all(const std::string& message, network::connection exclude=0) const
	{
		send_server_message_to_all(message.c_str(), exclude);
	}

	/**
	 * Prepare a text message and/or send it to a player. If a nonzero sock
	 * is passed, the message is sent to this player. If a non-null pointer
	 * to a simple_wml::document is passed, the message is stored in that
	 * document.
	 * @param message the message text
	 * @param sock    the socket to send the message to, if nonzero
	 * @param docptr  the wml document to store the message in, if nonnull
	 */
	void send_server_message(const char* message, network::connection sock,
		simple_wml::document* docptr = NULL) const;

	void send_server_message(const std::string& message, network::connection sock,
		simple_wml::document* docptr = NULL) const
	{
		send_server_message(message.c_str(), sock, docptr);
	}


private:
	std::string name_;
	connection_vector members_;
	bool persistent_;
	std::string topic_;
	bool logged_;
};

} //end namespace wesnothd

#endif
