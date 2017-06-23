/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Yann Dirson <ydirson@altern.org>
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

#include "exceptions.hpp"
#include "lua_jailbreak_exception.hpp"

namespace game {

struct mp_server_error : public error {
	mp_server_error(const std::string& msg) : error("MP server error: " + msg) {}
};

/**
 * Error used when game loading fails.
 */
struct load_game_failed : public error {
	load_game_failed() {}
	load_game_failed(const std::string& msg) : error("load_game_failed: " + msg) {}
};

/**
 * Error used when game saving fails.
 */
struct save_game_failed : public error {
	save_game_failed() {}
	save_game_failed(const std::string& msg) : error("save_game_failed: " + msg) {}
};

/**
 * Error used for any general game error, e.g. data files are corrupt.
 */
struct game_error : public error {
	game_error(const std::string& msg) : error("game_error: " + msg) {}
};

/**
 * Error used to report an error in a lua script or in the lua interpreter.
 */
struct lua_error : public error {
	lua_error(const std::string& msg) : error("lua_error: " + msg) {}
	lua_error(const std::string& msg, const std::string& context) : error(context + ":\n  " + msg) {}
};

}
