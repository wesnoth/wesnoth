/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef MAP_GEN_HPP_INCLUDED
#define MAP_GEN_HPP_INCLUDED

class config;
class display;

#include "exceptions.hpp"
#include "map_location.hpp"

#include <map>

struct mapgen_exception : public game::error
{
	mapgen_exception(const std::string& msg)
	: game::error(msg)
	{}
};

class map_generator
{
public:
	virtual ~map_generator() {}

	/**
	 * Returns true if the map generator has an interactive screen,
	 * which allows the user to modify how the generator behaves.
	 */
	virtual bool allow_user_config() const;

	/**
	 * Display the interactive screen, which allows the user
	 * to modify how the generator behaves.
	 * (This function will not be called if allow_user_config() returns false).
	 */
	virtual void user_config(display& disp);

	/**
	 * Returns a string identifying the generator by name.
	 * The name should not contain spaces.
	 */
	virtual std::string name() const = 0;

	/**
	 * Return a friendly name for the generator
	 * used to differentiate between different configs of the same generator
	 */
	virtual std::string config_name() const = 0;

	/**
	 * Creates a new map and returns it.
	 * args may contain arguments to the map generator.
	 */
	virtual std::string create_map() = 0;

	virtual config create_scenario();
};

/** Generate the map. */
std::string default_generate_map(size_t width, size_t height, size_t island_size, size_t island_off_center,
                                 size_t iterations, size_t hill_size,
								 size_t max_lakes, size_t nvillages, size_t castle_size, size_t nplayers,
								 bool roads_between_castles, std::map<map_location,std::string>* labels,
						         const config& cfg);

#endif
