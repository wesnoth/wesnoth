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

#include <string>

class commandline_options;
class config;
class saved_game;
class wesnothd_connection;

namespace ng { class connect_engine; }

namespace mp
{
// max. length of a player name
const std::size_t max_login_size = 20;

/*
 * This is the main entry points of multiplayer mode.
 */

/** Starts a multiplayer game in single-user mode. */
void start_local_game();

/**
 * Starts a multiplayer game in single-user mode.
 *
 * @param cmdline_opts        The commandline options
 */
void start_local_game_commandline(const commandline_options& cmdline_opts);

/**
 * Starts a multiplayer game in client mode.
 *
 * @param host                The host to connect to.
 */
void start_client(const std::string& host);

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

/** Gets whether the currently logged-in user is a moderator. */
bool logged_in_as_moderator();

/** Gets the forum profile link for the given user. */
std::string get_profile_link(int user_id);

}
