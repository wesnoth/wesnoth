/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
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

#include "utils/name_generator.hpp"
#include <map>
#include <vector>

typedef std::map<std::u32string, std::u32string> markov_prefix_map;

class markov_generator : public name_generator {
	markov_prefix_map prefixes_;
	std::size_t chain_size_, max_len_;
public:
	markov_generator(const std::vector<std::string>& items, std::size_t chain_size, std::size_t max_len);
	std::string generate() const override;
};
