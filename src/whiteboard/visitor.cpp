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

#include "visitor.hpp"

#include "action.hpp"
#include "foreach.hpp"
#include "side_actions.hpp"

#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"

namespace wb
{

void visitor::visit_all_actions()
{
	size_t current_team = resources::controller->current_side() - 1;
	size_t num_teams = resources::teams->size();
	for(size_t iteration = 0; iteration < num_teams; ++iteration)
	{
		size_t team_index = (current_team+iteration) % num_teams;
		foreach(action_ptr act, *resources::teams->at(team_index).get_side_actions())
			act->accept(*this);
	}
}

}//end namespace wb
