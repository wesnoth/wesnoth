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

/** @file */

#ifndef TIME_OF_DAY_HPP_INCLUDED
#define TIME_OF_DAY_HPP_INCLUDED

#include "tstring.hpp"
#include "utils/general.hpp"

#include <vector>

class config;

/** Small struct to store and manipulate ToD color adjusts. */
// This is a color delta, so do not replace with color_t!
struct tod_color {
	explicit tod_color(int red = 0, int green = 0, int blue = 0)
		: r(util::clamp(red, -255, 255))
		, g(util::clamp(green, -255, 255))
		, b(util::clamp(blue, -255, 255))
	{}
	bool operator==(const tod_color& o) const {
		return r == o.r && g == o.g && b == o.b;
	}
	bool is_zero() const {
		return r == 0 && g == 0 && b == 0;
	}
	bool operator!=(const tod_color& o) const {
		return !operator==(o);
	}
	tod_color operator+(const tod_color& o) const {
		return tod_color(r + o.r, g + o.g, b + o.b);
	}

	int r,g,b;
};

std::ostream &operator<<(std::ostream &s, const tod_color& tod);


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
	time_of_day();

	/** Construct a time of day from config */
	explicit time_of_day(const config& cfg);

	bool operator==(const time_of_day& o) const {
		return lawful_bonus == o.lawful_bonus
            && bonus_modified == o.bonus_modified
			&& image == o.image
			&& name == o.name
			&& id == o.id
			&& image_mask == o.image_mask
			//&& color == o.color
			&& sounds == o.sounds;
	}

	void write(config& cfg) const;

	/** The % bonus lawful units receive. Chaotics receive -lawful_bonus. */
	int lawful_bonus;
	int bonus_modified;

	/** The image to be displayed in the game status. */
	std::string image;
	t_string name;
	t_string description;
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
	tod_color color;

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
