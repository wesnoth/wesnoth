/* $Id$ */
/*
   Copyright (C) 2012 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERVER_PLAYER_CONNECTION_HPP_INCLUDED
#define SERVER_PLAYER_CONNECTION_HPP_INCLUDED

#include "simple_wml.hpp"
#include "player.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

typedef boost::bimaps::bimap<
		boost::bimaps::unordered_set_of<socket_ptr>, 
		boost::bimaps::unordered_set_of<std::string>,
		boost::bimaps::with_info<wesnothd::player> > PlayerMap;

void send_to_player(socket_ptr socket, simple_wml::document& doc);

template<typename Container> void send_to_players(simple_wml::document& data, const Container& players, socket_ptr exclude = socket_ptr())
{
	typename Container::const_iterator iter = players.begin(), iter_end = players.end();
	for(;iter != iter_end; ++iter)
		if(*iter != exclude)
			send_to_player(*iter, data);
}

void send_server_message(socket_ptr socket, const std::string& message);

std::string client_address(socket_ptr socket);

#endif
