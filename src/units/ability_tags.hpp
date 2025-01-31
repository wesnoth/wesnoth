/*
	Copyright (C) 2024 - 2024
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

#include <set>

struct ability_list_defines
{
	// abilities
	static constexpr const char* const heals = "heals";
	static constexpr const char* const regenerate = "regenerate";
	static constexpr const char* const resistance = "resistance";
	static constexpr const char* const leadership = "leadership";
	static constexpr const char* const skirmisher = "skirmisher";
	static constexpr const char* const illuminates = "illuminates";
	static constexpr const char* const teleport = "teleport";
	static constexpr const char* const hides = "hides";
	static constexpr const char* const dummy = "dummy";
	// weapon specials
	static constexpr const char* const attacks = "attacks";
	static constexpr const char* const berserk = "berserk";
	static constexpr const char* const chance_to_hit = "chance_to_hit";
	static constexpr const char* const damage = "damage";
	static constexpr const char* const damage_type = "damage_type";
	static constexpr const char* const disable = "disable";
	static constexpr const char* const drains = "drains";
	static constexpr const char* const firststrike = "firststrike";
	static constexpr const char* const heal_on_hit = "heal_on_hit";
	static constexpr const char* const petrifies = "petrifies";
	static constexpr const char* const plague = "plague";
	static constexpr const char* const poison = "poison";
	static constexpr const char* const slow = "slow";
	static constexpr const char* const swarm = "swarm";

	ENUM_AND_ARRAY(heals, regenerate, resistance, leadership, skirmisher, illuminates, teleport, hides, dummy, attacks, berserk, chance_to_hit, damage, damage_type, disable, drains, firststrike, heal_on_hit, petrifies, plague, poison, slow, swarm)

	static const std::set<std::string> weapon_number_tags()
	{
		static std::set<std::string> tags{attacks, damage, chance_to_hit, berserk, swarm, drains, heal_on_hit};
		return tags;
	}

	static const std::set<std::string> no_weapon_number_tags()
	{
		static std::set<std::string> tags{disable, plague, slow, petrifies, firststrike, poison, damage_type};
		return tags;
	}

	static const std::set<std::string> ability_value_tags()
	{
		static std::set<std::string> tags{resistance, leadership, heals, regenerate, illuminates};
		return tags;
	}

	static const std::set<std::string> ability_no_value_tags()
	{
		static std::set<std::string> tags{teleport, hides, skirmisher};
		return tags;
	}
};
using abilities_list = string_enums::enum_base<ability_list_defines>;
