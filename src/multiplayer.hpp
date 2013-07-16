/*
   Copyright (C) 2005 - 2013 Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MULTIPLAYER_HPP_INCLUDED
#define MULTIPLAYER_HPP_INCLUDED

#include "mp_game_settings.hpp"
#include "multiplayer_ui.hpp"
#include "commandline_options.hpp"

class config;
class game_display;

namespace mp {

// max. length of a player name
const size_t max_login_size = 20;

/*
 * This is the main entry points of multiplayer mode.
 */

/** Starts a multiplayer game in single-user mode.
 *
 * @param disp        The global display
 * @param game_config The global, top-level WML configuration for the game
 * @param default_controller The default controller type
 */
void start_local_game(game_display& disp, const config& game_config,
		mp::controller default_controller);

/** Starts a multiplayer game in single-user mode.
 *
 * Same parameters as start_local_game plus:
 * cmdline_opts The commandline options
 */
void start_local_game_commandline(game_display& disp, const config& game_config,
		mp::controller default_controller, const commandline_options& cmdline_opts);

/** Starts a multiplayer game in client mode.
 *
 * @param disp        The global display
 * @param game_config The global, top-level WML configuration for the game
 * @param host        The host to connect to.
 */
void start_client(game_display& disp, const config& game_config,
		const std::string& host);

game_state goto_mp_connect(game_display& disp, const config& game_config,
	const mp_game_settings& params);

}
#endif
