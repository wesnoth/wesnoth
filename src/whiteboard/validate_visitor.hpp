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

#ifndef WB_VALIDATE_VISITOR_HPP_
#define WB_VALIDATE_VISITOR_HPP_

#include "mapbuilder_visitor.hpp"

#include <set>

namespace wb
{

class validate_visitor: public mapbuilder_visitor
{
public:
	validate_visitor(unit_map& unit_map, side_actions_ptr side_actions);
	virtual ~validate_visitor();

	/// @return false some actions had to be deleted during validation,
	/// which may warrant a second validation
	bool validate_actions();

	virtual void visit_move(move_ptr move);
	virtual void visit_attack(attack_ptr attack);
	virtual void visit_recruit(recruit_ptr recruit);
	virtual void visit_recall(recall_ptr recall);

private:
	std::set<action_ptr> actions_to_erase_;
};

}

#endif /* WB_VALIDATE_VISITOR_HPP_ */
