/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

struct generator_data {
	generator_data(const config& cfg);

	int width;
	int height;
	int default_width;
	int default_height;
	int nplayers;
	int nvillages;
	int iterations;
	int hill_size;
	int castle_size;
	int island_size;
	int island_off_center;
	int max_lakes;
	bool link_castles;
	bool show_labels;
};

class default_map_generator : public map_generator
{
public:
	default_map_generator(const config &game_config);

	bool allow_user_config() const override;
	void user_config() override;

	std::string name() const override;

	std::string config_name() const override;

	std::string create_map(boost::optional<uint32_t> randomseed) override;
	config create_scenario(boost::optional<uint32_t> randomseed) override;

private:
	std::string generate_map(std::map<map_location,std::string>* labels, boost::optional<uint32_t> randomseed);

	config cfg_;

	generator_data data_;
};

#endif
