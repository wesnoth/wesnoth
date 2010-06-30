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

/**
 * Visitor that collects and applies unit_map modifications from the actions it visits
 * and reverts all changes on destruction.
 */
class mapbuilder_visitor: public visitor
{

public:
	mapbuilder_visitor(unit_map& unit_map);
	virtual ~mapbuilder_visitor();

	virtual void visit_move(boost::shared_ptr<move> move);

	// Any actions associated with this unit will be ignored when modifying the unit map
	virtual void exclude(const unit& unit) { excluded_units_.insert(&unit); }

protected:
	unit_map& unit_map_;

	std::set<unit const*> excluded_units_;

private:
	std::stack<modifier_ptr> modifiers_;
};

}

#endif /* WB_MAPBUILDER_VISITOR_HPP_ */
