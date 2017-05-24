/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "generators/lua_map_generator.hpp"

#include "config.hpp"
#include "game_errors.hpp"
#include "scripting/mapgen_lua_kernel.hpp"

#include <string>

lua_map_generator::lua_map_generator(const config & cfg)
	: id_(cfg["id"])
	, config_name_(cfg["config_name"])
	, user_config_(cfg["user_config"])
	, create_map_(cfg["create_map"])
	, create_scenario_(cfg["create_scenario"])
	, lk_()
	, generator_data_(cfg)
{
	lk_.load_core();
	const char* required[] {"id", "config_name", "create_map"};
	for (std::string req : required) {
		if (!cfg.has_attribute(req)) {
			if(req == "create_map" && cfg.has_attribute("create_scenario")) {
				// One of these is required, but not both
				continue;
			}
			std::string msg = "Error when constructing a lua map generator -- missing a required attribute '";
			msg += req + "'\n";
			msg += "Config was '" + cfg.debug() + "'";
			throw mapgen_exception(msg);
		}
	}
}

void lua_map_generator::user_config()
{
	try {
		lk_.user_config(user_config_.c_str(), generator_data_);
	} catch (game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator user_config.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		throw mapgen_exception(msg);
	}
}

std::string lua_map_generator::create_map(boost::optional<uint32_t> seed)
{
	if(create_map_.empty()) {
		return map_generator::create_map(seed);
	}

	try {
		return lk_.create_map(create_map_.c_str(), generator_data_, seed);
	} catch (game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator create_map.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		throw mapgen_exception(msg);
	}
}

config lua_map_generator::create_scenario(boost::optional<uint32_t> seed)
{
	if(create_scenario_.empty()) {
		return map_generator::create_scenario(seed);
	}

	try {
		return lk_.create_scenario(create_scenario_.c_str(), generator_data_, seed);
	} catch (game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator create_scenario.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		throw mapgen_exception(msg);
	}
}
