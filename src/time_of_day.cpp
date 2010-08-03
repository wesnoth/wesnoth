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

/** @file */

#include "global.hpp"

#include "foreach.hpp"
#include "time_of_day.hpp"

time_of_day::time_of_day(const config& cfg):
	lawful_bonus(cfg["lawful_bonus"]), bonus_modified(0),
	image(cfg["image"]), name(cfg["name"].t_str()), id(cfg["id"]),
	image_mask(cfg["mask"]),
	red(cfg["red"]), green(cfg["green"]), blue(cfg["blue"]),
	sounds(cfg["sound"])
{
}

time_of_day::time_of_day()
: lawful_bonus(0)
, bonus_modified(0)
, image()
, name("NULL_TOD")
, id("nulltod")
, image_mask()
, red(0)
, green(0)
, blue(0)
, sounds()
{
}

void time_of_day::write(config& cfg) const
{
	cfg["lawful_bonus"] = lawful_bonus;
	cfg["red"] = red;
	cfg["green"] = green;
	cfg["blue"] = blue;
	cfg["image"] = image;
	cfg["name"] = name;
	cfg["id"] = id;
	cfg["mask"] = image_mask;
}

void time_of_day::parse_times(const config& cfg, std::vector<time_of_day>& normal_times)
{
	foreach (const config &t, cfg.child_range("time")) {
		normal_times.push_back(time_of_day(t));
	}

	if(normal_times.empty())
	{
		// Make sure we have at least default time
		config dummy_cfg;
		normal_times.push_back(time_of_day(dummy_cfg));
	}
}

