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

struct level_result_defines
{
	static constexpr const char* const victory = "victory";
	static constexpr const char* const defeat = "defeat";
	static constexpr const char* const quit = "quit";
	static constexpr const char* const observer_end = "observer_end";
	static constexpr const char* const pass = "pass";
	static constexpr const char* const fail = "fail";

	ENUM_AND_ARRAY(victory, defeat, quit, observer_end, pass, fail)
};
using level_result = string_enums::enum_base<level_result_defines>;
