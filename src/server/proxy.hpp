/*
   Copyright (C) 2007 - 2015
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PROXY_HPP_INCLUDED
#define PROXY_HPP_INCLUDED

#include "network.hpp"
#include "simple_wml.hpp"

#include <string>

namespace proxy
{

void create_proxy(network::connection sock, const std::string& host, int port);
bool is_proxy(network::connection sock);
void disconnect(network::connection sock);

void received_data(network::connection sock, simple_wml::document& data);

}

#endif
