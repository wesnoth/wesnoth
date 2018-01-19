/*
   Copyright (C) 2011 - 2018 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <boost/system/error_code.hpp>
#include "exceptions.hpp"
#include "lua_jailbreak_exception.hpp"
///An error occured during when trying to coommunicate with the wesnothd server.
struct wesnothd_error : public game::error
{
	wesnothd_error(const std::string& error) : game::error(error) {}
};

/**
 * Error used when the client is rejected by the MP server.
 * Technically, this is not an error but the only way to handle the condition is as if it were an error.
 */
struct wesnothd_rejected_client_error : public game::error
{
    wesnothd_rejected_client_error (const std::string& msg) : game::error (msg) {}
};

///We received invalid data from wesnothd during a game
///This means we cannot continue with the game but we can stay connected to wesnothd and start a new game
///TODO: find a short name
struct ingame_wesnothd_error : public wesnothd_error ,public lua_jailbreak_exception
{
	ingame_wesnothd_error(const std::string& error) : wesnothd_error(error) {}
	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(ingame_wesnothd_error)
};


///an error occured inside the underlying network comminication code (boost asio)
///TODO: find a short name
struct wesnothd_connection_error : public wesnothd_error ,public lua_jailbreak_exception
{
	wesnothd_connection_error(const boost::system::error_code& error) : wesnothd_error(error.message()) {}
	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(wesnothd_connection_error)
};

