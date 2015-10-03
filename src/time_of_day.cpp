/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file time_of_day.cpp */

#include "global.hpp"

#include "foreach.hpp"
#include "time_of_day.hpp"

#include <cstdio>

time_of_day::time_of_day(const config& cfg)
                 : lawful_bonus(atoi(cfg["lawful_bonus"].c_str())),
                   bonus_modified(0),
                   image(cfg["image"]), name(cfg["name"]), id(cfg["id"]),
			       image_mask(cfg["mask"]),
                   red(atoi(cfg["red"].c_str())),
                   green(atoi(cfg["green"].c_str())),
                   blue(atoi(cfg["blue"].c_str())),
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
	char buf[50];
	snprintf(buf,sizeof(buf),"%d",lawful_bonus);
	cfg["lawful_bonus"] = buf;

	snprintf(buf,sizeof(buf),"%d",red);
	cfg["red"] = buf;

	snprintf(buf,sizeof(buf),"%d",green);
	cfg["green"] = buf;

	snprintf(buf,sizeof(buf),"%d",blue);
	cfg["blue"] = buf;

	cfg["image"] = image;
	cfg["name"] = name;
	cfg["id"] = id;
	cfg["mask"] = image_mask;
}

void time_of_day::parse_times(const config& cfg, std::vector<time_of_day>& normal_times)
{
	BOOST_FOREACH (const config &t, cfg.child_range("time")) {
		normal_times.push_back(time_of_day(t));
	}

	if(normal_times.empty())
	{
		// Make sure we have at least default time
		config dummy_cfg;
		normal_times.push_back(time_of_day(dummy_cfg));
	}
}

