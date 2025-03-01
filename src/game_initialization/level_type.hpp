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

struct level_type_defines
{
	static constexpr const char* const scenario = "scenario";
	static constexpr const char* const user_map = "user_map";
	static constexpr const char* const user_scenario = "user_scenario";
	static constexpr const char* const random_map = "random_map";
	static constexpr const char* const campaign = "campaign";
	static constexpr const char* const sp_campaign = "sp_campaign";

	ENUM_AND_ARRAY(scenario, user_map, user_scenario, random_map, campaign, sp_campaign)
};
using level_type = string_enums::enum_base<level_type_defines>;
