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
 * @file manager.cpp
 */

#include "manager.hpp"
#include "action.hpp"
#include "move.hpp"

namespace wb {

manager* manager::instance_ = NULL;

manager::manager()
{

}

manager::~manager()
{

}

manager& manager::instance()
{
	if (instance_ == NULL)
	{
		instance_ = new manager;
	}
	return *instance_;
}

const action_set& manager::get_actions() const
{
	return planned_actions_;
}

void manager::add_move(unit& subject, const map_location& target_hex, arrow& arrow, int index)
{
	action_ptr ptr(new move(subject, target_hex, arrow));
	if (index == -1)
	{
		planned_actions_.push_back(ptr);
	}
	else
	{
		assert(index < (int) planned_actions_.size());
		planned_actions_[index] = ptr;
	}
}

void manager::push_up(int index, size_t increment)
{
	action_set::iterator position;
	if (index == -1)
	{
		position = planned_actions_.end() - 1;
	}
	else
	{
		assert(index < (int) planned_actions_.size());
		position = planned_actions_.begin() + index;
	}

	action_set::iterator destination;
	destination = position - increment;

	assert(destination >= planned_actions_.begin() &&
			destination < planned_actions_.end());

	action_ptr action = *position;
	planned_actions_.erase(position);
	planned_actions_.insert(destination, action);
}

void manager::push_down(int index, size_t increment)
{
	push_up(index,-increment);
}

void manager::remove_action(int index)
{
	if (index == -1)
	{
		planned_actions_.pop_back();
	}
	else
	{
		assert(index < (int) planned_actions_.size());
		planned_actions_.erase(planned_actions_.begin()+index);
	}
}

} // end namespace wb
