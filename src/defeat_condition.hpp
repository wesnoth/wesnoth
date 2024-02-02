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

struct defeat_condition_defines
{
	static constexpr const char* const no_leader_left = "no_leader_left";
	static constexpr const char* const no_units_left = "no_units_left";
	static constexpr const char* const never = "never";
	static constexpr const char* const always = "always";

	ENUM_AND_ARRAY(no_leader_left, no_units_left, never, always)
};
using defeat_condition = string_enums::enum_base<defeat_condition_defines>;
