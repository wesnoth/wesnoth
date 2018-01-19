/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *
 * This class is an abstract base class which represents a display context
 * (game map, units, and teams) together with a TOD manager. This, plus
 * the game data (WML variables) and a lua kernel (currently a singleton)
 * is sufficient to evaluate filters.
 *
 **/

#pragma once

#include <vector>

class display_context;
class tod_manager;
class game_data;
class game_lua_kernel;

class filter_context {
public:
	// accessors

	virtual const display_context & get_disp_context() const = 0;
	virtual const tod_manager & get_tod_man() const = 0;
	virtual const game_data * get_game_data() const = 0;
	virtual game_lua_kernel * get_lua_kernel() const = 0;

	// Dtor

	virtual ~filter_context() {}
};
