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

#include "utils/team_query.hpp"

#include "units/unit.hpp"

using namespace utils;

bool team_query::is_enemy(const unit& a, const unit& b) const
{
	// If sides aren't valid teams, then they are enemies.
	if(std::size_t(a.side() - 1) >= teams_.size() || std::size_t(b.side() - 1) >= teams_.size()) {
		return true;
	}

	// Defender and opposite are enemies.
	if(teams_[a.side() - 1].is_enemy(b.side())) {
		return true;
	}

	return false;
}
