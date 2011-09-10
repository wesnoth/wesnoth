/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
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

#include "global.hpp"

#include "config.hpp"
#include "foreach.hpp"
#include "time_of_day.hpp"

namespace{
	//Static tokens are replacements for string literals in code
	//They allow for fast comparison, copying and hashing operations.

	static const config::t_token z_lawful_bonus("lawful_bonus", false);
	static const config::t_token z_name("name", false);
	static const config::t_token z_id("id", false);
	static const config::t_token z_time("time", false);
	static const config::t_token z_image("image", false);
	static const config::t_token z_mask("mask", false);
	static const config::t_token z_red("red", false);
	static const config::t_token z_green("green", false);
	static const config::t_token z_blue("blue", false);
	static const config::t_token z_sound("sound", false);
	static const config::t_token z_NULL_TOD("NULL_TOD", false);
	static const config::t_token z_nulltod("nulltod", false);

}


time_of_day::time_of_day(const config& cfg):
	lawful_bonus(cfg[z_lawful_bonus]),
	bonus_modified(0),
	image(cfg[z_image].token()), name(cfg[z_name].t_str()), id(cfg[z_id].token()),
	image_mask(cfg[z_mask].token()),
	red(cfg[z_red]), green(cfg[z_green]), blue(cfg[z_blue]),
	sounds(cfg[z_sound].token())
{
}

time_of_day::time_of_day()
: lawful_bonus(0)
, bonus_modified(0)
, image()
, name(z_NULL_TOD)
, id(z_nulltod)
, image_mask()
, red(0)
, green(0)
, blue(0)
, sounds()
{
}

void time_of_day::write(config& cfg) const
{
	cfg[z_lawful_bonus] = lawful_bonus;
	cfg[z_red] = red;
	cfg[z_green] = green;
	cfg[z_blue] = blue;
	cfg[z_image] = image;
	cfg[z_name] = name;
	cfg[z_id] = id;
	cfg[z_mask] = image_mask;
}

void time_of_day::parse_times(const config& cfg, std::vector<time_of_day>& normal_times)
{
	foreach (const config &t, cfg.child_range(z_time)) {
		normal_times.push_back(time_of_day(t));
	}

	if(normal_times.empty())
	{
		// Make sure we have at least default time
		config dummy_cfg;
		normal_times.push_back(time_of_day(dummy_cfg));
	}
}

