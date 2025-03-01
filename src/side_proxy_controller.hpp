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

struct side_proxy_controller_defines
{
	static constexpr const char* const idle = "idle";
	static constexpr const char* const human = "human";
	static constexpr const char* const ai = "ai";

	ENUM_AND_ARRAY(idle, human, ai)
};
using side_proxy_controller = string_enums::enum_base<side_proxy_controller_defines>;
