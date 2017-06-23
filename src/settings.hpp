/*
   Copyright (C) 2007 - 2017 by Mark de Wever <koraq@xs4all.nl>
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
 *  @file
 *  General settings and defaults for scenarios.
 */

#pragma once

#include <string>
#include "game_classification.hpp"

/**
 * Contains the general settings which have a default.
 * These values can be used to initialize the preferences,
 * but also to set map values if these aren't defined.
 */

namespace settings {

	/**
	 *  Gets the number of turns.
	 *  If no valid value supplied, it will return a default.
	 *  The value is also range checked.
	 *  When out of bounds, it will be set to the nearest bound.
	 *
	 *  @param value        string containing the number of turns
	 *
	 *  @returns            the number of turns
	 */
	int get_turns(const std::string& value);
	const int turns_min = 1;          /**< minimum number of turns */
	const int turns_max = 100;        /**< maximum number of turns */
	const int turns_default = 100;    /**< default number of turns */
	const int turns_step = 1;         /**< slider step size for turns */

	/**
	 *  Gets the village gold.
	 *  If no valid value supplied, it will return a default.
	 *  The default is 1 for singleplayer, and 2 for multiplayer.
	 *  The value is also range checked.
	 *  When out of bounds, it will be set to the nearest bound.
	 *
	 *  @param value        string containing the village gold
	 *
	 *  @returns            the village gold
	 */
	int get_village_gold(const std::string& value, const game_classification* classification = nullptr);

	/**
	 *  Gets the village unit level support.
	 *  If no valid value supplied, it will return a default.
	 *  The value is also range checked.
	 *  When out of bounds, it will be set to the nearest bound.
	 *
	 *  @param value        string containing the village support
	 *
	 *  @returns            the village support
	 */
	int get_village_support(const std::string& value);

	/**
	 *  Gets the xp modifier.
	 *  If no valid value supplied, it will return a default.
	 *  The value is also range checked.
	 *  When out of bounds, it will be set to the nearest bound.
	 *
	 *  @param value        string containing the xp modifier
	 *
	 *  @returns            the xp modifier
	 */
	int get_xp_modifier(const std::string& value);

} // namespace settings
