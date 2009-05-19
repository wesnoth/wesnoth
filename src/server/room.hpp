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

#ifndef SERVER_ROOM_HPP_INCLUDED
#define SERVER_ROOM_HPP_INCLUDED

#include "../network.hpp"
#include "player.hpp"
#include "simple_wml.hpp"

namespace wesnothd {

typedef std::vector<network::connection> connection_vector;

class game;

class room {
public:
	room();

	/**
	 * Return the number of players in this room
	 */
	size_t size() const {
		return members_.size();
	}

	/**
	 * Return the members of this room
	 */
	const std::vector<network::connection>& members() const {
		return members_;
	}

	bool is_member(network::connection player) const {
		return std::find(members_.begin(), members_.end(), player) != members_.end();
	}

	bool add_player(network::connection player);

	void add_players(const game& game);

	void remove_player(network::connection player);

	void process_message(simple_wml::document& data, const player_map::iterator user);


	void send_data(simple_wml::document& data, const network::connection exclude=0, std::string packet_type = "") const;

	void send_server_message_to_all(const char* message, network::connection exclude=0) const;
	void send_server_message(const char* message, network::connection sock, simple_wml::document* docptr = NULL) const;

private:
	connection_vector members_;
};

} //end namespace wesnothd

#endif
