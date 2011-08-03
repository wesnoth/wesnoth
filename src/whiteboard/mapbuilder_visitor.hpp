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

#ifndef WB_MAPBUILDER_VISITOR_HPP_
#define WB_MAPBUILDER_VISITOR_HPP_

#include <boost/ptr_container/ptr_vector.hpp>

#include "action.hpp"
#include "utility.hpp"
#include "visitor.hpp"

struct unit_movement_resetter;

namespace wb
{

/**
 * Visitor that collects and applies unit_map modifications from the actions it visits
 * and reverts all changes on destruction.
 */
class mapbuilder_visitor
	: private visitor
	, private enable_visit_all<mapbuilder_visitor>
{
	friend class enable_visit_all<mapbuilder_visitor>;

	friend class validate_visitor;

public:
	mapbuilder_visitor(unit_map& unit_map);
	virtual ~mapbuilder_visitor();

	/**
	 * Calls the appropriate visit_* method on each of the actions contained in the
	 * side_actions objects of every team whose turn comes earlier in the turn order,
	 * including (and stopping at) the viewer's team.
	 */
	void build_map();

private:
	virtual void visit_move(move_ptr move);
	virtual void visit_attack(attack_ptr attack);
	virtual void visit_recruit(recruit_ptr recruit);
	virtual void visit_recall(recall_ptr recall);
	virtual void visit_suppose_dead(suppose_dead_ptr sup_d);

	//"Inherited" from enable_visit_all
	bool visit(size_t team_index, team&, side_actions&, side_actions::iterator);
	bool pre_visit_team(size_t team_index, team& t, side_actions& sa)
		{return enable_visit_all<mapbuilder_visitor>::pre_visit_team(team_index,t,sa);}
	bool post_visit_team(size_t team_index, team&, side_actions&)
		{return team_index != viewer_team();} //< Stop after visiting viewer_team

	//Helper fcn: Temporarily resets all units' moves to max EXCEPT for
	//the ones controlled by the player whose turn it is currently.
	void reset_moves();

	void restore_normal_map();

	unit_map& unit_map_;

	action_queue applied_actions_;

	enum mapbuilder_mode {
		BUILD_PLANNED_MAP,
		RESTORE_NORMAL_MAP
	};

	mapbuilder_mode mode_;

	//Used by reset_moves()
	boost::ptr_vector<unit_movement_resetter> resetters_;
};

}

#endif /* WB_MAPBUILDER_VISITOR_HPP_ */
