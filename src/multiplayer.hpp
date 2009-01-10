/* $Id$ */
/*
   Copyright (C) 2005 - 2009 Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MULTIPLAYER_HPP_INCLUDED
#define MULTIPLAYER_HPP_INCLUDED

#include "multiplayer_ui.hpp"

class config;
class game_display;

namespace mp {

// max. length of a player name
const size_t max_login_size = 18;

/*
 * This is the main entry points of multiplayer mode.
 */

/** Starts a multiplayer game in server mode, or in single-user mode.
 *
 * @param disp        The global display
 * @param game_config The global, top-level WML configuration for the game
 * @param default_controller The default controller type
 * @param is_server   Whether to open a port to the outside, or not.
 */
void start_server(game_display& disp, const config& game_config,
		mp::controller default_controller, bool is_server);

/** Starts a multiplayer game in client mode.
 *
 * @param disp        The global display
 * @param game_config The global, top-level WML configuration for the game
 * @param host        The host to connect to.
 */
void start_client(game_display& disp, const config& game_config,
		const std::string host);


extern bool authenticated;
void admin_authentication(const std::string& sender, const std::string& message);

}
#endif
