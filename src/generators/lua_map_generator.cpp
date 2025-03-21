/*
	Copyright (C) 2014 - 2025
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "log.hpp"

#include <array>
#include <string>

static lg::log_domain log_mapgen("mapgen");
#define ERR_NG LOG_STREAM(err, log_mapgen)
#define LOG_NG LOG_STREAM(info, log_mapgen)

lua_map_generator::lua_map_generator(const config & cfg, const config* vars)
	: id_(cfg["id"])
	, config_name_(cfg["config_name"])
	, user_config_(cfg["user_config"])
	, create_map_(cfg["create_map"])
	, create_scenario_(cfg["create_scenario"])
	, lk_(vars)
	, generator_data_(cfg)
{
	lk_.load_core();
	using namespace std::string_literals;
	const std::array required {"id"s, "config_name"s, "create_map"s};
	for(const auto& req : required) {
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
	} catch(const game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator user_config.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		throw mapgen_exception(msg);
	}
}

std::string lua_map_generator::create_map(utils::optional<uint32_t> seed)
{
	if(create_map_.empty()) {
		return map_generator::create_map(seed);
	}

	try {
		return lk_.create_map(create_map_.c_str(), generator_data_, seed);
	} catch (const game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator create_map.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		throw mapgen_exception(msg);
	}
}

config lua_map_generator::create_scenario(utils::optional<uint32_t> seed)
{
	if(create_scenario_.empty()) {
		return map_generator::create_scenario(seed);
	}

	try {
		return lk_.create_scenario(create_scenario_.c_str(), generator_data_, seed);
	} catch (const game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator create_scenario.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		ERR_NG << msg;
		throw mapgen_exception(msg);
	}
}
