/* $Id$ */
/*
 Copyright (C) 2010 - 2011 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file
 */

#ifndef WB_UTILITY_HPP_
#define WB_UTILITY_HPP_

#include <vector>

#include "typedefs.hpp"

class unit;

namespace wb {

/// @return The current viewing team's index
size_t viewer_team();

/// @return The current viewing side's number (i.e. team index + 1)
int viewer_side();

/// @return The side_actions instance belonging to the current viewing team
side_actions_ptr viewer_actions();

/// @return The side_actions instance belonging to the current playing team
side_actions_ptr current_side_actions();

/**
 * For a given leader on a keep, find another leader on another keep in the same castle.
 * @retval NULL if no such leader has been found
 */
unit const* find_backup_leader(unit const& leader);

/**
 * @return a leader from the specified team who can recruit on the specified hex
 * @retval NULL if no such leader has been found
 */
unit* find_recruiter(size_t team_index, map_location const&);

/// Applies the future unit map and @return a pointer to the unit at hex
/// @retval NULL if none is visible to the specified viewer side
unit* future_visible_unit(map_location hex, int viewer_side = wb::viewer_side());

/// Applies the future unit map and @return a pointer to the unit at hex
/// @retval NULL if none is visible to the specified viewer side
/// @param on_side Only search for units of this side.
unit* future_visible_unit(int on_side, map_location hex, int viewer_side = wb::viewer_side());

/// Computes the MP cost for u to travel path
int path_cost(std::vector<map_location> const& path, unit const& u);

struct temporary_unit_hider {
	temporary_unit_hider(unit& u);
	~temporary_unit_hider();
	unit* const unit_;
};

/// finalizer struct to help with exception safety
/// sets variable to value on destruction
template <typename T>
struct variable_finalizer
{
	variable_finalizer(T & variable, T value):
		variable_(variable),
		value_(value)
	{}
	~variable_finalizer()
	{
		variable_ = value_;
	}
	T & variable_;
	T value_;
};

void ghost_owner_unit(unit* unit);
void unghost_owner_unit(unit* unit);

/** Return whether the whiteboard has actions. */
bool has_actions();

} //end namespace wb

#endif /* WB_UTILITY_HPP_ */
