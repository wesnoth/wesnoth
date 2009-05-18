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

#ifndef SERVER_PLAYER_NETWORK_HPP_INCLUDED
#define SERVER_PLAYER_NETWORK_HPP_INCLUDED

#include "../network.hpp"
#include "player.hpp"
#include "simple_wml.hpp"

#include <boost/function.hpp>

namespace wesnothd {

typedef std::map<network::connection,player> player_map;
typedef std::vector<network::connection> connection_vector;

/** Convenience function for finding a user by name. */
player_map::const_iterator find_user(const player_map& all_players,
									 const simple_wml::string_span& name);


void send_to_one(simple_wml::document& data,
				 const network::connection sock,
				 std::string packet_type = "");

void send_to_many(simple_wml::document& data,
				  const connection_vector& vec,
				  const network::connection exclude = 0,
				  std::string packet_type = "");

void send_to_many(simple_wml::document& data,
				  const connection_vector& vec,
				  boost::function<bool (network::connection)> except_pred,
				  std::string packet_type);
} //end namespace wesnothd

#endif
