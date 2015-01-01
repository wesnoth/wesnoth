/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
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

#ifndef GAME_EVENTS_CONDITIONAL_WML_H_INCLUDED
#define GAME_EVENTS_CONDITIONAL_WML_H_INCLUDED

class  config;
class vconfig;


namespace game_events
{
	bool conditional_passed(const vconfig& cond, bool backwards_compat=true);
	bool matches_special_filter(const config &cfg, const vconfig& filter);
}

#endif // GAME_EVENTS_CONDITIONAL_WML_H_INCLUDED

