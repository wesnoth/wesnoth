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
 * @file find_visitor.hpp
 */

#ifndef WB_FIND_VISITOR_HPP_
#define WB_FIND_VISITOR_HPP_

#include "visitor.hpp"
#include "manager.hpp"

class unit;

namespace wb
{

/**
 * Visitor to find the action(s) associated with a unit.
 */
class find_visitor: public visitor
{
public:
	find_visitor();
	virtual ~find_visitor();

	virtual void visit_move(move& p_move);

	virtual action_set find_action_of(const unit& target);
	virtual action_ptr find_first_action_of(const unit& target);

private:
	bool found_;
	const unit* search_target_;
	action_set search_result_;
};

}

#endif /* WB_FIND_VISITOR_HPP_ */
