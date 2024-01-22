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

struct campaign_type_defines
{
	static constexpr const char* const scenario = "scenario";
	static constexpr const char* const multiplayer = "multiplayer";
	static constexpr const char* const test = "test";
	static constexpr const char* const tutorial = "tutorial";

	ENUM_AND_ARRAY(scenario, multiplayer, test, tutorial)
};
using campaign_type = string_enums::enum_base<campaign_type_defines>;
