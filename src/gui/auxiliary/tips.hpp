/*
	Copyright (C) 2010 - 2025
	by Mark de Wever <koraq@xs4all.nl>
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

#include "tstring.hpp"

#include <vector>

class config;

namespace gui2
{
struct game_tip;

namespace tip_of_the_day
{
/**
 * Loads the tips from a config.
 *
 * @param cfg                     A config with the tips.
 *
 * @returns                       The loaded tips.
 */
std::vector<game_tip> load(const config& cfg);

/**
 * Shuffles the tips.
 *
 * This routine shuffles the tips and filters out the unwanted ones.
 *
 * @param tips                    The tips.
 *
 * @returns                       The filtered tips in random order.
 */
std::vector<game_tip> shuffle(const std::vector<game_tip>& tips);

} // namespace tip_of_the_day

struct game_tip
{
	game_tip(const config& cfg);

	/** The text of the tip. */
	t_string text;

	/** The source of the tip. */
	t_string source;

	/**
	 * List of units to filter the tip upon.
	 *
	 * If the list is empty the tip is shown.
	 * Else the unit must have encountered at least one of the units in the list.
	 */
	std::vector<std::string> unit_filter;
};

} // namespace gui2
