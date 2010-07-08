/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file time_of_day.hpp */

#ifndef TIME_OF_DAY_HPP_INCLUDED
#define TIME_OF_DAY_HPP_INCLUDED

#include "config.hpp"
#include "tstring.hpp"
#include <string>
#include <vector>

/**
 * Object which defines a time of day
 * with associated bonuses, image, sounds etc.
 */
struct time_of_day
{
	/**
	 * A default-constructed time of day object shouldn't really be used
	 * so this only loads some null values. Ideally, there should be
	 * getters for properties that would emit a warning when such an object
	 * is actually used, but it does not seem necessary at the moment.
	 */
	explicit time_of_day();

	/** Construct a time of day from config */
	explicit time_of_day(const config& cfg);

	void write(config& cfg) const;

	/** The % bonus lawful units receive. Chaotics receive -lawful_bonus. */
	int lawful_bonus;
	int bonus_modified;

	/** The image to be displayed in the game status. */
	std::string image;
	t_string name;
	std::string id;

	/**
	 * The image that is to be laid over all images
	 * while this time of day lasts.
	 */
	std::string image_mask;

	/**
	 * The color modifications that should be made
	 * to the game board to reflect the time of day.
	 */
	int red, green, blue;

	/**
	 * List of "ambient" sounds associated with this time_of_day,
	 * Played at the beginning of turn.
	 */
	std::string sounds;

	/**
	 * Parse config and add time of day entries into passed vector
	 */
	static void parse_times(const config& cfg, std::vector<time_of_day>& normal_times);
};

#endif
