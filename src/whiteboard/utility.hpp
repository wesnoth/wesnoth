/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file
 */

#ifndef WB_UTILITY_HPP_
#define WB_UTILITY_HPP_

#include "typedefs.hpp"

class unit;

namespace wb {

/// Utility struct used to automatize memory management for the whiteboard's fake/ghosted units
struct fake_unit_deleter {
	void operator() (unit*& ptr);
};

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
 */
unit const* find_backup_leader(unit const& leader);

///Applies the future unit map and @return a pointer to the unit at hex,
///NULL if none is visible to the specified viewer side
unit* future_visible_unit(map_location hex, int viewer_side = wb::viewer_side());

///Applies the future unit map and @return a pointer to the unit at hex,
///NULL if none is visible to the specified viewer side
/// @param on_side Only search for units of this side.
unit* future_visible_unit(int on_side, map_location hex, int viewer_side = wb::viewer_side());

} //end namespace wb

#endif /* WB_UTILITY_HPP_ */
