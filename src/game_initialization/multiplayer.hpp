/*
   Copyright (C) 2005 - 2018 Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "commandline_options.hpp"
#include "connect_engine.hpp"
#include "mp_game_settings.hpp"
#include "scripting/plugins/context.hpp"

class config;
class wesnothd_connection;
namespace mp {

// max. length of a player name
const std::size_t max_login_size = 20;

/*
 * This is the main entry points of multiplayer mode.
 */

/** Starts a multiplayer game in single-user mode.
 *
 * @param game_config The global, top-level WML configuration for the game
 */
void start_local_game(const config& game_config,
	saved_game& state);

/** Starts a multiplayer game in single-user mode.
 *
 * Same parameters as start_local_game plus:
 * cmdline_opts The commandline options
 */
void start_local_game_commandline(const config& game_config,
	saved_game& state, const commandline_options& cmdline_opts);

/** Starts a multiplayer game in client mode.
 *
 * @param game_config The global, top-level WML configuration for the game
 * @param host        The host to connect to.
 */
void start_client(const config& game_config,
	saved_game& state, const std::string& host);

/**
 * Opens mp::connect screen and sets game state according to the
 * changes made.
 */
bool goto_mp_connect(ng::connect_engine& engine, wesnothd_connection* connection);

/**
 * Opens mp::wait screen and sets game state according to the
 * changes made.
 */
bool goto_mp_wait(saved_game& state, wesnothd_connection* connection, bool observe);

}
