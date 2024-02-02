/*
	Copyright (C) 2008 - 2024
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

#include "enum_base.hpp"

struct random_faction_mode_defines
{
	static constexpr const char* const independent = "Independent";
	static constexpr const char* const no_mirror = "No Mirror";
	static constexpr const char* const no_ally_mirror = "No Ally Mirror";

	ENUM_AND_ARRAY(independent, no_mirror, no_ally_mirror)
};
using random_faction_mode = string_enums::enum_base<random_faction_mode_defines>;
