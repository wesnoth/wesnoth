/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "visitor.hpp"

#include "action.hpp"

namespace wb
{

/**
 * Visitor that collects and applies unit_map modifications from the actions it visits
 * and reverts all changes on destruction.
 */
class mapbuilder_visitor: public visitor
{

public:
	mapbuilder_visitor(unit_map& unit_map, side_actions_ptr side_actions, bool include_recruits = false);
	virtual ~mapbuilder_visitor();

	/**
	 * Visits all the actions contained in the side_actions object passed to the constructor,
	 * and calls the appropriate visit_* method on each of them.
	 */
	virtual void build_map();

	/// Visitor pattern method, no need to call this directly
	virtual void visit_move(move_ptr move);
	virtual void visit_attack(attack_ptr attack);
	virtual void visit_recruit(recruit_ptr recruit);
	virtual void visit_recall(recall_ptr recall);

protected:

	virtual void restore_normal_map();

	unit_map& unit_map_;

	bool for_pathfinding_;

	action_queue applied_actions_;

	enum mapbuilder_mode {
		BUILD_PLANNED_MAP,
		RESTORE_NORMAL_MAP
	};

	mapbuilder_mode mode_;
};

}

#endif /* WB_MAPBUILDER_VISITOR_HPP_ */
