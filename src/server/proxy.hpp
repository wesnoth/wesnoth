/* $Id$ */
/*
   Copyright (C) 2007
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PROXY_HPP_INCLUDED
#define PROXY_HPP_INCLUDED

#include "config.hpp"
#include "network.hpp"

#include <string>

namespace proxy
{

void create_proxy(network::connection sock, const std::string& host, int port);
bool is_proxy(network::connection sock);
void disconnect(network::connection sock);

//! @todo remove gzip param after 1.3.12 is no longer allowed on the server.
void received_data(network::connection sock, const config& data, const bool send_gzipped);

}

#endif
