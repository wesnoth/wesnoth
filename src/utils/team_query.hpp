/*
	Copyright (C) 2021 - 2024
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

#include "team.hpp"

#include <vector>

namespace utils
{
/**
 * A wrapper for the sides or teams array, answering questions such as "are these units on
 * the same side".
 *
 * WML numbers sides starting from 1, C++ likes indicies to start from zero, and so dealing
 * directly with a vector of sides requires handling the off-by-one numbering. This wrapper
 * encapsulates that logic.
 */
class team_query
{
public:
	/**
	 * The lifetime of this object needs to be shorter than the teams vector, however the
	 * code it replaces will already be relying on the lifetime of the global teams vector.
	 */
	team_query(const std::vector<team>& teams)
		: teams_(teams)
	{
	}

	/**
	 * Returns true if the two units belong to opposing sides, false if they're
	 * on allied sides or the same side. If either unit has an invalid side
	 * number, it's an enemy of any unit except those with the identical
	 * invalid number.
	 */
	bool is_enemy(const unit& a, const unit& b) const;

private:
	const std::vector<team>& teams_;
};

} // namespace utils
