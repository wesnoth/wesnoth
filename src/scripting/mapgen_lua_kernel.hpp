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

#pragma once

#include "scripting/lua_kernel_base.hpp"
#include "utils/optional_fwd.hpp"

#include <cstdint>
#include <random>

class config;

#include <string>

class mapgen_lua_kernel : public lua_kernel_base {
public:
	mapgen_lua_kernel(const config* vars);

	virtual std::string my_name() { return "Mapgen Lua Kernel"; }

	void user_config(const char * prog, const config & generator); // throws game::lua_error
	std::string create_map(const char * prog, const config & generator, utils::optional<uint32_t> seed); // throws game::lua_error
	config create_scenario(const char * prog, const config & generator, utils::optional<uint32_t> seed); // throws game::lua_error

	virtual uint32_t get_random_seed();
	std::mt19937& get_default_rng();
private:
	void run_generator(const char * prog, const config & generator);
	int intf_get_variable(lua_State *L);
	int intf_get_all_vars(lua_State *L);
	utils::optional<uint32_t> random_seed_;
	utils::optional<std::mt19937> default_rng_;
	const config* vars_;
};
