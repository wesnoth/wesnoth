/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERVER_PLAYER_GROUP_HPP_INCLUDED
#define SERVER_PLAYER_GROUP_HPP_INCLUDED

#include "../network.hpp"
#include "player.hpp"
#include "simple_wml.hpp"

namespace wesnothd {

typedef std::map<network::connection,player> player_map;
typedef std::vector<network::connection> connection_vector;


/**
 * A player_group represents a group of players, with convenience functions for
 * sending data to all of them
 */
class player_group
{
public:
	player_group();

	explicit player_group(const connection_vector& v);

	/**
	 * Check if the player is a member of this group
	 * @return true iif the playes is a member
	 */
	bool is_member(const network::connection player) const;

	/**
	 * Add a user to the group.
	 * @return True iff the user successfully joined.
	 */
	bool add_member(const network::connection player);

	const connection_vector members() const { return members_; }

	/** Send data to all members except 'exclude' */
	void send_and_record_server_message(const char* message, const network::connection exclude = 0);
	void send_data(simple_wml::document& data, const network::connection exclude=0, std::string packet_type = "") const;
	void send_to_one(simple_wml::document& data, const network::connection sock, std::string packet_type = "") const;

protected:
	connection_vector members_;
};



class room
{
public:
	room(const std::string& name);

private:
	std::string name_;
	connection_vector members_;
};


} //end namespace wesnothd

#endif
