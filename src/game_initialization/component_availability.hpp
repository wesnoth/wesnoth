/*
	Copyright (C) 2008 - 2025
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

struct component_availability_defines
{
	static constexpr const char* const sp = "sp";
	static constexpr const char* const mp = "mp";
	static constexpr const char* const hybrid = "hybrid";

	ENUM_AND_ARRAY(sp, mp, hybrid)
};
using component_availability = string_enums::enum_base<component_availability_defines>;
