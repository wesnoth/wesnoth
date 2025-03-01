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

namespace gui2
{

struct grow_direction_defines
{
	static constexpr const char* const horizontal = "horizontal";
	static constexpr const char* const vertical = "vertical";

	ENUM_AND_ARRAY(horizontal, vertical)
};
using grow_direction = string_enums::enum_base<grow_direction_defines>;

}
