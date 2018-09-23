/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include <string>

namespace help
{
extern const unsigned max_section_recursion_level;
extern const unsigned max_history;

//
// Constant topics IDs
//

/** Topic to open by default when opening the help browser. */
extern const std::string default_topic;

/** Topic to show when a unit hasn't been encountered in-game yet */
extern const std::string unknown_unit_topic;

//
// Standard topic ID prefixes
//

extern const std::string ability_prefix;
extern const std::string era_prefix;
extern const std::string faction_prefix;
extern const std::string race_prefix;
extern const std::string terrain_prefix;
extern const std::string tod_prefix;
extern const std::string trait_prefix;
extern const std::string unit_prefix;
extern const std::string variation_prefix;
extern const std::string weapon_special_prefix;

} // namespace help
