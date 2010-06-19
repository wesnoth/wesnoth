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
 * @file side_actions.cpp
 */

#include "side_actions.hpp"
#include "action.hpp"
#include "move.hpp"

#include "foreach.hpp"

namespace wb
{

side_actions::side_actions(): actions_()
{
}

side_actions::~side_actions()
{
}

const action_set& side_actions::actions() const
{
	return actions_;
}

void side_actions::execute_next()
{
	if (!actions_.empty())
	{
		actions_.front()->execute();
		actions_.pop_front();
		//TODO: Validate remaining actions here
	}
}

void side_actions::execute(action_ptr action)
{
	assert(!actions_.empty());
	action->execute();
	remove_action(action);
	//TODO: Validate remaining actions here
}

void side_actions::insert_move(unit& subject, const map_location& target_hex, size_t index, boost::shared_ptr<arrow> arrow,
		boost::shared_ptr<unit> fake_unit)
{
	action_ptr action(new move(subject, target_hex, arrow, fake_unit));
	assert(index < end());
	actions_.insert(actions_.begin() + index, action);
}

void side_actions::queue_move(unit& subject, const map_location& target_hex, boost::shared_ptr<arrow> arrow,
		boost::shared_ptr<unit> fake_unit)
{
	action_ptr action(new move(subject, target_hex, arrow, fake_unit));
	actions_.push_back(action);
}

void side_actions::move_earlier(size_t index, size_t increment)
{
	move_in_queue(index, - (int) increment);
}

void side_actions::move_later(size_t index, size_t increment)
{
	move_in_queue(index, (int) increment);
}

void side_actions::remove_action(size_t index)
{
	assert(!actions_.empty());
	assert(index < end());

	action_set::iterator position = actions_.begin()+index;
	if (position < actions_.end())
	{
		actions_.erase(position);
	}
}

void side_actions::remove_action(action_ptr action)
{
	assert(!actions_.empty());

	action_set::iterator position;
	for ( position = actions_.begin(); position != actions_.end(); ++position)
	{
		if (*position == action)
		{
			actions_.erase(position);
			break;
		}
	}
}

/*
 * Utility function to move actions around the queue.
 * Positive increment = move toward back of the queue and later execution.
 * Negative increment = move toward front of the queue and earlier execution.
 */
void side_actions::move_in_queue(size_t index, int increment)
{
	assert(!actions_.empty());
	assert(index < end());
	if (actions_.empty() || index >= end())
	{
		return;
	}

	action_set::iterator position;
	position = actions_.begin() + index;

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
	action_set::iterator after = actions_.erase(position);
	//be careful, previous iterators have just been invalidated by erase()
	action_set::iterator destination = after + increment;
	actions_.insert(destination, action);
}


}
