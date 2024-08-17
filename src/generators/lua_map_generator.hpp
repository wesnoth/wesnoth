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

#pragma once

#include "config.hpp"
#include "map_generator.hpp"

#include "scripting/mapgen_lua_kernel.hpp"

#include <string>

class lua_map_generator : public map_generator {
public:
	lua_map_generator(const std::string& file_name, const config & cfg, const config* vars);

	bool allow_user_config() override;

	std::string name() const override { return "lua"; }

	std::string config_name() const override { return config_name_; }

	virtual void user_config() override;
	virtual std::string create_map(utils::optional<uint32_t> randomseed) override;
	virtual config create_scenario(utils::optional<uint32_t> randomseed) override;

private:
	std::string config_name_;
	std::string file_name_;

	mapgen_lua_kernel lk_;

	config generator_data_;
};
