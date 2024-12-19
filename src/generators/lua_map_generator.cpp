/*
	Copyright (C) 2014 - 2024
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
#include "filesystem.hpp"
#include "game_errors.hpp"
#include "scripting/mapgen_lua_kernel.hpp"
#include "scripting/lua_common.hpp"
#include "log.hpp"

#include <array>
#include <string>

static lg::log_domain log_mapgen("mapgen");
#define ERR_NG LOG_STREAM(err, log_mapgen)
#define LOG_NG LOG_STREAM(info, log_mapgen)

lua_map_generator::lua_map_generator(const std::string& file_name, const config & cfg, const config* vars)
	: config_name_(cfg["config_name"])
	, file_name_()
	, lk_(vars)
	, generator_data_(cfg)
{
	file_name_ = filesystem::get_wml_binary_file_path("map_generators", file_name + ".lua").value_or("");

	lk_.load_core();


	using namespace std::string_literals;
	const std::array required {"config_name"s};
	for(const auto& req : required) {
		if (!cfg.has_attribute(req)) {
			std::string msg = "Error when constructing a lua map generator -- missing a required attribute '";
			msg += req + "'\n";
			msg += "Config was '" + cfg.debug() + "'";
			throw mapgen_exception(msg);
		}
	}
}

bool lua_map_generator::allow_user_config()
{
	return lk_.has_user_config(file_name_);
}

void lua_map_generator::user_config()
{
	try {
		lk_.user_config(file_name_, generator_data_);
	} catch(const game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator user_config.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		throw mapgen_exception(msg);
	}
}

std::string lua_map_generator::create_map(utils::optional<uint32_t> seed)
{

	try {
		return lk_.create_map(file_name_, generator_data_, seed);
	} catch (const game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator create_map.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		throw mapgen_exception(msg);
	}
	return map_generator::create_map(seed);
}

config lua_map_generator::create_scenario(utils::optional<uint32_t> seed)
{
	try {
		return lk_.create_scenario(file_name_.c_str(), generator_data_, seed);
	} catch (const game::lua_error & e) {
		std::string msg = "Error when running lua_map_generator create_scenario.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += e.what();
		ERR_NG << msg;
		throw mapgen_exception(msg);
	}
	return map_generator::create_scenario(seed);
}
