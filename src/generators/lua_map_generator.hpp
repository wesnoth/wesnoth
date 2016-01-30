/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef LUA_MAP_GENERATOR_HPP_INCLUDED
#define LUA_MAP_GENERATOR_HPP_INCLUDED

#include "config.hpp"
#include "map_generator.hpp"

#include "scripting/mapgen_lua_kernel.hpp"

#include <string>

struct lua_State;
class CVideo;

// TODO: Add support for user configurability (via defining a gui2 dialog in lua)
// What's missing is that you need access to the 'wesnoth' object to call show dialog
// at the moment.

class lua_map_generator : public map_generator {
public:
	lua_map_generator(const config & cfg);

	bool allow_user_config() const { return false; }

	std::string name() const { return "lua"; }

	std::string id() const { return id_; }

	std::string config_name() const { return config_name_; }

	virtual void user_config(CVideo & v);
	virtual std::string create_map(boost::optional<boost::uint32_t> randomseed);
	virtual config create_scenario(boost::optional<boost::uint32_t> randomseed);

private:
	std::string id_, config_name_;

	std::string user_config_;
	std::string create_map_;
	std::string create_scenario_;

	mapgen_lua_kernel lk_;

	config generator_data_;
};

#endif
