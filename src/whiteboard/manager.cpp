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

manager::manager(): planning_mode_(false)
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

void manager::insert_move(unit& subject, const map_location& target_hex, arrow& arrow, size_t index)
{
	action_ptr action(new move(subject, target_hex, arrow));
	assert(index < end());
	planned_actions_.insert(planned_actions_.begin() + index, action);

}

void manager::queue_move(unit& subject, const map_location& target_hex, arrow& arrow)
{
	insert_move(subject, target_hex, arrow, end());
}

void manager::move_earlier(size_t index, size_t increment)
{
	move_in_queue(index, - (int) increment);
}

void manager::move_later(size_t index, size_t increment)
{
	move_in_queue(index, (int) increment);
}

void manager::remove_action(size_t index)
{
	assert(!planned_actions_.empty());
	assert(index < end());

	action_set::iterator position = planned_actions_.begin()+index;
	if (position < planned_actions_.end())
	{
		planned_actions_.erase(position);
	}
}

/*
 * Utility function to move actions around the queue.
 * Positive increment = move toward back of the queue and later execution.
 * Negative increment = move toward front of the queue and earlier execution.
 */
void manager::move_in_queue(size_t index, int increment)
{
	assert(!planned_actions_.empty());
	assert(index < end());
	if (planned_actions_.empty() || index >= end())
	{
		return;
	}

	action_set::iterator position;
	position = planned_actions_.begin() + index;

	assert(index + increment < end());
	if (index + increment >= end())
	{
		increment = int(end()) - index;
	}

	assert(int(index) + increment >= 0);
	if (int(index) + increment < 0)
	{
		increment = -index;
	}

	action_ptr action = *position;
	action_set::iterator after = planned_actions_.erase(position);
	//be careful, previous iterators have just been invalidated by erase()
	action_set::iterator destination = after + increment;
	planned_actions_.insert(destination, action);
}

} // end namespace wb
