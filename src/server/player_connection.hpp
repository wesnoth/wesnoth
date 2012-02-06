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

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

void send_to_player(socket_ptr socket, simple_wml::document& doc);

void send_server_message(socket_ptr socket, const std::string& message);

#endif
