/*
	Copyright (C) 2003 - 2022
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

#include "config.hpp"
#include "units/ptr.hpp"

#include <string>
#include <string_view>
#include <vector>

using ability_vector = std::vector<ability_ptr>;

class unit_ability_t
{
public:
	unit_ability_t(std::string tag, config cfg, bool is_attack_ability = false);

	const std::string& tag() const { return tag_; };
	const config& cfg() const { return cfg_; };
	void write(config& abilities_cfg);


	static void parse_vector(const config& abilities_cfg, ability_vector& res, bool is_attack_ability = false);
	static config vector_to_cfg(const ability_vector& abilities);
	static ability_vector cfg_to_vector(const config& abilities_cfg, bool is_attack_ability = false);

private:
	std::string tag_;
	config cfg_;
};
