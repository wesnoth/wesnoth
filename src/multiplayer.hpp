/* $Id$ */
/*
   Copyright (C)
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MULTIPLAYER_HPP_INCLUDED
#define MULTIPLAYER_HPP_INCLUDED

#include "display.hpp"
#include "config.hpp"
#include "unit_types.hpp"

namespace mp {
	
enum controller { CNTR_NETWORK = 0, CNTR_LOCAL, CNTR_COMPUTER, CNTR_EMPTY, CNTR_LAST };

/**
 * This is the main entry points of multiplayer mode.
 */
void start_server(display& disp, const config& game_config, game_data& data,
		mp::controller default_controller, bool is_server);

void start_client(display& disp, const config& game_config, game_data& data, 
		const std::string host);

}

#endif
