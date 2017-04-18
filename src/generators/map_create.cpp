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

#include "generators/map_create.hpp"

#include "generators/cave_map_generator.hpp"
#include "generators/default_map_generator.hpp"
#include "generators/lua_map_generator.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

#include <cassert>
#include <sstream>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

map_generator* create_map_generator(const std::string& name, const config &cfg)
{
	if(name == "default" || name == "") {
		return new default_map_generator(cfg);
	} else if(name == "cave") {
		ERR_CF << "map/scenario_generation=cave is deprecatd and will be removed soon, use map/scenario_generation=lua with lua/cave_map_generator.lua instead.\n";
		return new cave_map_generator(cfg);
	} else if(name == "lua") {
		return new lua_map_generator(cfg);
	} else {
		return nullptr;
	}
}

//function to generate a random map, from a string which describes
//the generator to use
std::string random_generate_map(const std::string& name, const config &cfg)
{
	std::unique_ptr<map_generator> generator(create_map_generator(name,cfg));
	if(generator == nullptr) {
		std::stringstream ss;
		ss << "could not find map generator '" << name << "'";
		throw mapgen_exception(ss.str());
	}

	return generator.get()->create_map();
}

config random_generate_scenario(const std::string& name, const config &cfg)
{
	std::unique_ptr<map_generator> generator(create_map_generator(name,cfg));
	if(generator == nullptr) {
		std::stringstream ss;
		ss << "could not find map generator '" << name << "'";
		throw mapgen_exception(ss.str());
	}

	return generator->create_scenario();
}
