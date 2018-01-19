/*
 Copyright (C) 2010 - 2018 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#pragma once

#include "side_actions.hpp"

#include <boost/ptr_container/ptr_vector.hpp>
#include <list>

#include "utility.hpp"

struct unit_movement_resetter;

struct temporary_unit_remover;

namespace wb
{

/**
 * Class that collects and applies unit_map modifications from the actions it visits
 * and reverts all changes on destruction.
 */
class mapbuilder
{

public:
	mapbuilder(unit_map& unit_map);
	virtual ~mapbuilder();

	/**
	 * Builds every team's actions as far into the future as possible, in the correct order.
	 */
	void build_map();

private:
	/** Function called on each action. */
	void process(side_actions &sa, side_actions::iterator action_it);

	/** Function called after visiting a team. */
	void post_visit_team(size_t turn);

	/** Does various preliminary actions on the unit map such as resetting moves for some units. */
	void pre_build();

	void restore_normal_map();

	unit_map& unit_map_;

	action_queue applied_actions_;
	action_queue applied_actions_this_turn_;

	//Used by pre_build()
	boost::ptr_vector<unit_movement_resetter> resetters_;
	boost::ptr_vector<temporary_unit_remover> removers_;

	//Used by process()
	std::set<unit const*> acted_this_turn_;
	std::set<unit const*> has_invalid_actions_;
	std::list<side_actions::iterator> invalid_actions_; ///< Conserved invalid actions.
};

}
