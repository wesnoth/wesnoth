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
 * @file validate_visitor.cpp
 */

#include "validate_visitor.hpp"
#include "arrow.hpp"
#include "move.hpp"

#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"

namespace wb
{

validate_visitor::validate_visitor(unit_map& unit_map)
	: mapbuilder_visitor(unit_map)
{
}

validate_visitor::~validate_visitor()
{
}

//FIXME: move this some place it can be accessible from the whole whiteboard
static team& get_current_team()
{
	int current_side = resources::controller->current_side();
	team& current_team = (*resources::teams)[current_side - 1];
	return current_team;
}

void validate_visitor::visit_move(boost::shared_ptr<move> move)
{
	bool valid = true;

	if (!(move->orig_hex_.valid() && move->dest_hex_.valid()))
		valid = false;

	if (valid && resources::units->find(move->orig_hex_) == resources::units->end())
		valid = false;

	pathfind::plain_route route;
	if (valid)
	{
		pathfind::shortest_path_calculator path_calc(move->unit_, get_current_team(), *resources::units,
				*resources::teams, *resources::game_map);
		route = pathfind::a_star_search(move->orig_hex_,
				move->dest_hex_, 10000, &path_calc, resources::game_map->w(), resources::game_map->h());
		if (route.move_cost >= path_calc.getNoPathValue())
		{
			valid = false;
		}
	}

	if (valid)
	{
		if (!std::equal(route.steps.begin(), route.steps.end(), move->arrow_->get_path().begin()))
		{
			//new valid path differs from the previous one, replace
			move->arrow_->set_path(route.steps);

			//TODO: Since this might lengthen the path, we probably need a special conflict state
			// to warn the player that the initial path is no longer possible.

		}
		// Now call the superclass to apply the result of this move to the unit map,
		// so that further pathfinding takes it into account.
		mapbuilder_visitor::visit_move(move);
	}
	else //path invalid
	{
		// Don't apply the move's results to the unit map

		// Mark the move as invalid
		move->set_valid(false);
	}

}

}//end namespace wb
