/* $Id$ */
/*
 Copyright (C) 2010 - 2012 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "validate_visitor.hpp"
#include "attack.hpp"
#include "manager.hpp"
#include "move.hpp"
#include "recall.hpp"
#include "recruit.hpp"
#include "side_actions.hpp"
#include "suppose_dead.hpp"
#include "utility.hpp"

#include "arrow.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"

#include <boost/foreach.hpp>

namespace wb
{

validate_visitor::validate_visitor(unit_map& unit_map)
	: builder_(unit_map,*this)
	, viewer_actions_(*viewer_actions())
	, actions_to_erase_()
	, arg_itor_()
{
	assert(!resources::whiteboard->has_planned_unit_map());
}

validate_visitor::~validate_visitor()
{
}

bool validate_visitor::validate_actions()
{
	builder_.build_map();

	//FIXME: by reverse iterating this can be done in a more efficiant way
	// by using the iterator returned by remove_action it could even be done in visit_all above
	if (!actions_to_erase_.empty()) {
		int side_actions_size_before = viewer_actions_.size();
		LOG_WB << "Erasing " << actions_to_erase_.size() << " invalid actions.\n";
		BOOST_FOREACH(action_ptr action, actions_to_erase_) {
			viewer_actions_.remove_action(viewer_actions_.get_position_of(action), false);
		}
		assert(side_actions_size_before - viewer_actions_.size() == actions_to_erase_.size());
		actions_to_erase_.clear();
		return false;
	} else {
		return true;
	}
}


void validate_visitor::helper::validate(side_actions::iterator const& itor)
{
	if((*itor)->validate()) {
		parent_.actions_to_erase_.insert(*itor);
	}
}

}//end namespace wb
