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

struct strike_result_defines
{
	static constexpr const char* const hit = "hit";
	static constexpr const char* const miss = "miss";
	static constexpr const char* const kill = "kill";
	static constexpr const char* const invalid = "invalid";

	ENUM_AND_ARRAY(hit, miss, kill, invalid)
};
using strike_result = string_enums::enum_base<strike_result_defines>;
