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

struct team_shared_vision_defines
{
	static constexpr const char* const all = "all";
	static constexpr const char* const shroud = "shroud";
	static constexpr const char* const none = "none";

	ENUM_AND_ARRAY(all, shroud, none)
};
using team_shared_vision = string_enums::enum_base<team_shared_vision_defines>;
