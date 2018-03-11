/*
   Copyright (C) 2007 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  General settings and defaults for scenarios.
 */

#include "lexical_cast.hpp"
#include "settings.hpp"

#include "serialization/string_utils.hpp"
#include "utils/general.hpp"

namespace settings {

int get_turns(const std::string& value)
{
	// Special case, -1 is also allowed, which means unlimited turns
	int val = lexical_cast_default<int>(value);

	if(val == -1) {
		return turns_max;
	}

	return utils::clamp<int>(lexical_cast_default<int>(value, turns_default), turns_min, turns_max);
}

int get_village_gold(const std::string& value, const game_classification* classification)
{
	return utils::clamp<int>(lexical_cast_default<int>(value, ((classification && !classification->is_normal_mp_game()) ? 1 : 2)), 1, 5);
}

int get_village_support(const std::string& value)
{
	return utils::clamp<int>(lexical_cast_default<int>(value, 1), 0, 4);
}

int get_xp_modifier(const std::string& value)
{
	return utils::clamp<int>(lexical_cast_default<int>(value, 70), 30, 200);
}

} // end namespace settings
