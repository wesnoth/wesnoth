/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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
 * Map-generator, with standalone testprogram.
 */

#include "global.hpp"
#include "map_generator.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "log.hpp"

static lg::log_domain log_mapgen("mapgen");
#define ERR_NG LOG_STREAM(err, log_mapgen)
#define LOG_NG LOG_STREAM(info, log_mapgen)

config map_generator::create_scenario(boost::optional<boost::uint32_t> randomseed)
{
	config res;
	res["map_data"] = create_map(randomseed);
	return res;
}
/**
	by default we don't allow user configs.
*/
bool map_generator::allow_user_config() const
{
	return false;
}

void map_generator::user_config(CVideo& /*v*/)
{
}

#ifdef TEST_MAPGEN

/** Standalone testprogram for the mapgenerator. */
int main(int argc, char** argv)
{
	int x = 50, y = 50, iterations = 50,
		hill_size = 50, lakes=3,
	    nvillages = 25, nplayers = 2;
	if(argc >= 2) {
		x = atoi(argv[1]);
	}

	if(argc >= 3) {
		y = atoi(argv[2]);
	}

	if(argc >= 4) {
		iterations = atoi(argv[3]);
	}

	if(argc >= 5) {
		hill_size = atoi(argv[4]);
	}

	if(argc >= 6) {
		lakes = atoi(argv[5]);
	}

	if(argc >= 7) {
		nvillages = atoi(argv[6]);
	}

	if(argc >= 8) {
		nplayers = atoi(argv[7]);
	}

	srand(time(NULL));
	std::cout << generate_map(x,y,iterations,hill_size,lakes,nvillages,nplayers) << "\n";
}

#endif
