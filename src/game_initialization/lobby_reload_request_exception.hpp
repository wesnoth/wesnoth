/*
   Copyright (C) 2015 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef LOBBY_RELOAD_REQUEST_EXCEPTION_HPP_INCLUDED
#define LOBBY_RELOAD_REQUEST_EXCEPTION_HPP_INCLUDED

#include <exception>
#include <string>

namespace mp {

struct lobby_reload_request_exception : game::error {
	lobby_reload_request_exception() : game::error("Lobby requested to reload the game config and launch again") {}
};

}

#endif
