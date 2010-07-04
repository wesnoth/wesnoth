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
 * @file mapbuilder_visitor.h
 */

#ifndef WB_MAPBUILDER_VISITOR_HPP_
#define WB_MAPBUILDER_VISITOR_HPP_

#include "visitor.hpp"
#include "action.hpp"

#include <set>
#include <stack>

#include <boost/shared_ptr.hpp>

class unit;
class unit_map;

namespace wb
{

class side_actions;

typedef boost::shared_ptr<side_actions> side_actions_ptr;

/**
 * Visitor that collects and applies unit_map modifications from the actions it visits
 * and reverts all changes on destruction.
 */
class mapbuilder_visitor: public visitor
{

public:
	mapbuilder_visitor(unit_map& unit_map, side_actions_ptr side_actions);
	virtual ~mapbuilder_visitor();

	enum mapbuilder_mode {
		BUILD_PLANNED_MAP,
		RESTORE_NORMAL_MAP
	};

	/**
	 * Visits all the actions contained in the side_actions object passed to the constructor,
	 * and calls the appropriate visit_* method on each of them.
	 */
	virtual void build_map();

	/// Any actions associated with this unit will be ignored when modifying the unit map
	virtual void exclude(const unit& unit) { excluded_units_.insert(&unit); }

	/// Visitor pattern method, no need to call this directly
	virtual void visit_move(boost::shared_ptr<move> move);



protected:
	unit_map& unit_map_;

	std::set<unit const*> excluded_units_;

	side_actions_ptr side_actions_;

	mapbuilder_mode mode_;

};

}

#endif /* WB_MAPBUILDER_VISITOR_HPP_ */
