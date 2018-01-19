/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "serialization/unicode_types.hpp"
#include "utils/name_generator.hpp"
#include <map>

typedef std::map<ucs4::string, ucs4::string> markov_prefix_map;

class markov_generator : public name_generator {
	markov_prefix_map prefixes_;
	size_t chain_size_, max_len_;
public:
	markov_generator(const std::vector<std::string>& items, size_t chain_size, size_t max_len);
	std::string generate() const override;
};
