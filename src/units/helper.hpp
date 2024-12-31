/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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

#include "units/map.hpp"
#include "units/types.hpp"

namespace unit_helper {

/**
 * Encapsulates the logic for deciding whether an iterator @a u points to
 * a unit that can advance.
 * @return true if the unit exists, has available advances, and can_advance().
 */
bool will_certainly_advance(const unit_map::iterator &u);

/**
 * Determines the total number of available advancements (of any kind) for
 * a given unit. This includes normal advances and modifiers.
 * @return the total number of possible advancements.
 */
int number_of_possible_advances(const unit &unit);

/**
 * @return the name of the color encoding the weight of the unit's
 * resistance value for presenting it to the player.
 */
std::string resistance_color(const int resistance);

/**
 * @return the tooltip text showing a unit's level and what it can advance to.
 */
std::string unit_level_tooltip(const unit &u);

/**
 * @return the tooltip text showing a unit type's level and what it can
 * advance to.
 */
std::string unit_level_tooltip(const unit_type &u);

/**
 * @return If active, do nothing. If inactive, surround with pango tag that
 * changes font color to grey.
 */
std::string maybe_inactive(const std::string& str, bool active);

/**
 * @return If unit recall cost is greater than team's recall cost,
 * return a help markup string that shows the unit recall cost
 * in red font color. If unit recall cost is less than team's cost,
 * color in green instead. If equal, default color.
 * In all cases, a gold icon using `<img>` tag is prepended to the formatted cost.
 * Icon is greyscaled when unit recall cost > team recall cost.
 * Eg. `<img src='themes/gold.png~GS()'/><span color='#ff0000'>40</span>`.
 */
std::string format_cost_string(int unit_recall_cost, const int team_recall_cost);

/**
 * @return Help markup string with gold icon followed by unit_cost.
 * Eg. `<img src='themes/gold.png'/>40`
 */
std::string format_cost_string(int unit_cost);

/**
 * @return A pango formatted string representation of level.
 * The applied formatting is different for each level.
 * L0: level number in normal text, grey color
 * L1: level number in normal text
 * L2: level number in bold text
 * L3: level number in bold, font color `#e2b776`
 * L3+: level number in bold, font color `#dd6600`.
 * If not recallable, return greyscaled versions.
 * So, L0, L1: normal text, L2+: bold text
 */
std::string format_level_string(const int level, bool recallable);

/**
 * @return A pango formatted string representation of "moves_left/moves_max",
 * colored based on condition below:
 * moves_left is zero: "moves_left/moves_max" colored in red
 * moves_left is less than/equal to moves_max: "moves_left/moves_max" colored in green
 * moves_left is greater than moves_max: "moves_left/moves_max" colored in yellow
 */
std::string format_movement_string(const int moves_left, const int moves_max);
}
