/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * @file
 * Define conditionals for the game's events mechanism,
 * a.k.a. conditional actions WML.
 */

#pragma once

class  config;
class vconfig;


namespace game_events
{
	bool conditional_passed(const vconfig& cond);
	bool matches_special_filter(const config &cfg, const vconfig& filter);

	namespace builtin_conditions {
		bool have_unit(const vconfig& cfg);
		bool have_location(const vconfig& cfg);
		bool variable_matches(const vconfig& cfg);
	}
}
