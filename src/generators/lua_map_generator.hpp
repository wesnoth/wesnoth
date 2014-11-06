/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef LUA_MAP_GENERATOR_DEFINED
#define LUA_MAP_GENERATOR_DEFINED

#include "config.hpp"
#include "map_generator.hpp"

#include <string>

struct lua_State;

// TODO: Add support for user configurability (via defining a gui2 dialog in lua)
// What's missing is that you need access to the 'wesnoth' object to call show dialog
// at the moment.

class lua_map_generator : public map_generator {
public:
	lua_map_generator(const config & cfg);

	~lua_map_generator();

	bool allow_user_config() const { return false; }

	std::string name() const { return "lua"; }

	std::string id() const { return id_; }

	std::string config_name() const { return config_name_; }

	virtual std::string create_map();
	virtual config create_scenario();

private:
	std::string id_, config_name_;

	std::string create_map_;
	std::string create_scenario_;

	lua_State * mState_;

	config generator_data_;
};

#endif
