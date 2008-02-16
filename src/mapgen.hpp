/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file mapgen.hpp
//!

#ifndef MAP_GEN_HPP_INCLUDED
#define MAP_GEN_HPP_INCLUDED

class config;
class display;

#include "map.hpp"

#include <map>
#include <string>
#include <vector>

class map_generator
{
public:
	virtual ~map_generator() {}

	//! Returns true iff the map generator has an interactive screen,
	//! which allows the user to modify how the generator behaves.
	virtual bool allow_user_config() const = 0;

	//! Display the interactive screen, which allows the user
	//! to modify how the generator behaves.
	//! (This function will not be called if allow_user_config() returns false).
	virtual void user_config(display& disp) = 0;

	//! Returns a string identifying the generator by name.
	//! The name should not contain spaces.
	virtual std::string name() const = 0;

	//! Creates a new map and returns it.
	//! args may contain arguments to the map generator.
	virtual std::string create_map(const std::vector<std::string>& args) = 0;

	virtual config create_scenario(const std::vector<std::string>& args);
};

std::string default_generate_map(size_t width, size_t height, size_t island_size, size_t island_off_center,
                                 size_t iterations, size_t hill_size,
								 size_t max_lakes, size_t nvillages, size_t castle_size, size_t nplayers,
								 bool roads_between_castles, std::map<gamemap::location,std::string>* labels,
						         const config& cfg);

#endif
