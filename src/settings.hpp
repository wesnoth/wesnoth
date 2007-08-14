/* $Id: boilerplate-header.cpp 18943 2007-07-21 07:28:04Z mordante $ */
/*
   Copyright (C) 2007 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SETTINGS_HPP_INCLUDED
#define SETTINGS_HPP_INCLUDED

/**
 * Contains the general settings which have a default. These values
 * can be used to initialize the preferences but also to set map 
 * values if these aren't defined
 */

namespace settings {

	/**
	 *  Gets the number of turns, if no valid value supplied it will return
	 *  a default. The value is also range checked, when out of bounds it 
	 *  will be set to the nearest bound.
	 *
	 *  @param value	string containing the number of turns
	 *
	 *  @returns		the number of turns
	 */
	int get_turns(const std::string& value);

	/**
	 *  Gets the village gold, if no valid value supplied it will return
	 *  a default. The value is also range checked, when out of bounds it 
	 *  will be set to the nearest bound.
	 *
	 *  @param value	string containing the village gold
	 *
	 *  @returns		the village gold
	 */
	int get_village_gold(const std::string& value);

	/**
	 *  Gets the xp modifier, if no valid value supplied it will return
	 *  a default. The value is also range checked, when out of bounds it 
	 *  will be set to the nearest bound.
	 *
	 *  @param value	string containing the xp modifier
	 *
	 *  @returns		the xp modifier
	 */
	int get_xp_modifier(const std::string& value);

	/**
	 * Returns whether the fog of war should be used.
	 * If no valid value is supplied it will return a default
	 *
	 * @param value		string containing a boolean value
	 *
	 * @returns			whether or not the fog of war should be used
	 */
	bool use_fog(const std::string& value);

	/**
	 * Returns whether the random start time feature should be used.
	 * If no valid value is supplied it will return a default
	 *
	 * @param value		string containing a boolean value
	 *
	 * @returns			whether or not the random start time should be used
	 */
	bool use_random_start_time(const std::string& value);

	/**
	 * Returns whether the shroud should be used.
	 * If no valid value is supplied it will return a default
	 *
	 * @param value		string containing a boolean value
	 *
	 * @returns			whether or not the shroud should be used
	 */
	bool use_shroud(const std::string& value);

} // namespace settings

#endif
