/* $Id$ */
/*
   Copyright (C) 2005 Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MULTIPLAYER_HPP_INCLUDED
#define MULTIPLAYER_HPP_INCLUDED

#include "multiplayer_ui.hpp"

class config;
class display;
struct game_data;

namespace mp {

/*
 * This is the main entry points of multiplayer mode.
 */

/** Starts a multiplayer game in server mode, or in single-user mode.
 *
 * @param disp        The global display
 * @param game_config The global, top-level WML configuration for the game
 * @param data        The global game data (unit types, etc)
 * @param default_controller The default controller type
 * @param is_server   Whether to open a port to the outside, or not.
 */
void start_server(display& disp, const config& game_config, game_data& data,
		mp::controller default_controller, bool is_server);

/** Starts a multiplayer game in client mode.
 *
 * @param disp        The global display
 * @param game_config The global, top-level WML configuration for the game
 * @param data        The global game data (unit types, etc)
 * @param host        The host to connect to.
 */
void start_client(display& disp, const config& game_config, game_data& data,
		const std::string host);

struct error {
	error(const std::string& msg) : message(msg) {}
	std::string message;
};

}
#endif
