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

namespace ai
{

struct ai_target_defines
{
	static constexpr const char* const village = "village";
	static constexpr const char* const leader = "leader";
	static constexpr const char* const xplicit = "explicit";
	static constexpr const char* const threat = "threat";
	static constexpr const char* const battle_aid = "battle aid";
	static constexpr const char* const mass = "mass";
	static constexpr const char* const support = "support";

	ENUM_AND_ARRAY(village, leader, xplicit, threat, battle_aid, mass, support)
};
using ai_target = string_enums::enum_base<ai_target_defines>;

}
