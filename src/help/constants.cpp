/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "help/constants.hpp"

namespace help
{
const unsigned max_section_recursion_level = 15;
const unsigned max_history = 100;

const std::string default_topic = "..introduction";
const std::string unknown_unit_topic = ".unknown_unit";

const std::string ability_prefix = "ability_";
const std::string era_prefix = "era_";
const std::string faction_prefix = "faction_";
const std::string race_prefix = "race_";
const std::string terrain_prefix = "terrain_";
const std::string tod_prefix = "time_of_day_";
const std::string trait_prefix = "traits_";
const std::string unit_prefix = "unit_";
const std::string variation_prefix = "variation_";
const std::string weapon_special_prefix = "weaponspecial_";

} // namespace help
