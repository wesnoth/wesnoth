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


namespace {
	//Create default value functions
	DEFAULT_TOKEN_BODY(zf_lawful_bonus, "lawful_bonus");
	DEFAULT_TOKEN_BODY(zf_name, "name");
	DEFAULT_TOKEN_BODY(zf_id, "id");
	DEFAULT_TOKEN_BODY(zf_image, "image");
	DEFAULT_TOKEN_BODY(zf_mask, "mask");
	DEFAULT_TOKEN_BODY(zf_red, "red");
	DEFAULT_TOKEN_BODY(zf_green, "green");
	DEFAULT_TOKEN_BODY(zf_blue, "blue");
	DEFAULT_TOKEN_BODY(zf_sound, "sound");
	DEFAULT_TOKEN_BODY(zf_NULL_TOD, "NULL_TOD");
	DEFAULT_TOKEN_BODY(zf_nulltod, "nulltod");

}


time_of_day::time_of_day(const config& cfg):
	lawful_bonus(cfg[zf_lawful_bonus()]),
	bonus_modified(0),
	image(cfg[zf_image()].token()), name(cfg[zf_name()].t_str()), id(cfg[zf_id()].token()),
	image_mask(cfg[zf_mask()].token()),
	red(cfg[zf_red()]), green(cfg[zf_green()]), blue(cfg[zf_blue()]),
	sounds(cfg[zf_sound()].token())
{
}

time_of_day::time_of_day()
: lawful_bonus(0)
, bonus_modified(0)
, image()
, name(zf_NULL_TOD())
, id(zf_nulltod())
, image_mask()
, red(0)
, green(0)
, blue(0)
, sounds()
{
}

void time_of_day::write(config& cfg) const
{

	static const n_token::t_token & z_lawful_bonus( generate_safe_static_const_t_interned(n_token::t_token("lawful_bonus")) );
	static const n_token::t_token & z_red( generate_safe_static_const_t_interned(n_token::t_token("red")) );
	static const n_token::t_token & z_green( generate_safe_static_const_t_interned(n_token::t_token("green")) );
	static const n_token::t_token & z_blue( generate_safe_static_const_t_interned(n_token::t_token("blue")) );
	static const n_token::t_token & z_image( generate_safe_static_const_t_interned(n_token::t_token("image")) );
	static const n_token::t_token & z_name( generate_safe_static_const_t_interned(n_token::t_token("name")) );
	static const n_token::t_token & z_id( generate_safe_static_const_t_interned(n_token::t_token("id")) );
	static const n_token::t_token & z_mask( generate_safe_static_const_t_interned(n_token::t_token("mask")) );

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
	static const n_token::t_token & z_time( generate_safe_static_const_t_interned(n_token::t_token("time")) );

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

