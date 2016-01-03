/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef DEFAULT_MAP_GENERATOR_HPP_INCLUDED
#define DEFAULT_MAP_GENERATOR_HPP_INCLUDED

#include "config.hpp"
#include "generators/map_generator.hpp"

class default_map_generator : public map_generator
{
public:
	default_map_generator(const config &game_config);

	bool allow_user_config() const;
	void user_config(display& disp);

	std::string name() const;

	std::string config_name() const;

	std::string create_map(boost::optional<boost::uint32_t> randomseed);
	config create_scenario(boost::optional<boost::uint32_t> randomseed);

private:

	std::string generate_map(std::map<map_location,std::string>* labels, boost::optional<boost::uint32_t> randomseed);

	size_t default_width_, default_height_, width_, height_, island_size_, iterations_, hill_size_, max_lakes_, nvillages_, castle_size_, nplayers_;
	bool link_castles_, show_labels_;
	config cfg_;
};

#endif
