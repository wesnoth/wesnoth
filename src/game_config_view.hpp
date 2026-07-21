/*
	Copyright (C) 2013 - 2025
	by Andrius Silinskas <silinskas.andrius@gmail.com>
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

#include <functional>
#include <vector>

using config_array_view = std::vector<std::reference_wrapper<const config>>;

/**
 * A class grating read only view to a vector of config objects, viewed as one config
 * with all children appended, used by the game_config class to read data from addons
 * config, and from the main config.
 **/
class game_config_view
{

public:
	game_config_view()
	{}

	static game_config_view wrap(const config& cfg)
	{
		return game_config_view(cfg);
	}

	config_array_view child_range(std::string_view key) const;

	optional_const_config find_child(std::string_view key, const std::string &name, const std::string &value) const;
	const config& find_mandatory_child(std::string_view key, const std::string &name, const std::string &value) const;

	// const config& child(std::string_view key) const;
	const config& mandatory_child(std::string_view key) const;
	optional_const_config optional_child(std::string_view key) const;

	const config& child_or_empty(std::string_view key) const;

	game_config_view merged_children_view(std::string_view key) const;


	config_array_view& data()
	{
		return cfgs_;
	}

private:

	explicit game_config_view(const config& cfg)
		: cfgs_()
	{
		cfgs_.push_back(cfg);
	}
	config_array_view cfgs_;
};
