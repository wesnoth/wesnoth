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
 * @file find_visitor.cpp
 */

#include "find_visitor.hpp"
#include "manager.hpp"
#include "move.hpp"

#include "foreach.hpp"

namespace wb
{

find_visitor::find_visitor()
	: found_(false)
	, search_target_(NULL)
	, search_result_()
	  // found == !search_result_.empty();
{
}

find_visitor::~find_visitor()
{
}

void find_visitor::visit_move(boost::shared_ptr<move> move)
{
	if( &(move->get_unit()) == search_target_ )
	{
		search_result_.push_back(move);
		found_ = true;
	}
}

action_set find_visitor::find_actions_of(const unit& unit, action_set actions)
{
	search_target_ = &unit;
	found_ = false;
	search_result_.clear();
	foreach (action_ptr a, actions)
	{
		a->accept(*this);
	}
	return search_result_;
}

action_ptr find_visitor::find_first_action_of(const unit& unit, action_set actions)
{
	search_target_ = &unit;
	found_ = false;
	search_result_.clear();
	foreach (action_ptr a, actions)
	{
		a->accept(*this);
		if (found_)
		{
			return search_result_[0];
		}
	}
	return action_ptr();
}

}//end namespace wb
