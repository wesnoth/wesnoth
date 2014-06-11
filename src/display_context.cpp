/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "display_context.hpp"

#include "map.hpp"
#include "map_location.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

const unit * display_context::get_visible_unit(const map_location & loc, const team &current_team, bool see_all) const
{
	if (!map().on_board(loc)) return NULL;
	const unit_map::const_iterator u = units().find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, map(), see_all)) {
		return NULL;
	}
	return &*u;
}
