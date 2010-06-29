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
#include <set>

namespace wb
{

side_actions::side_actions()
	: actions_()
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
		bool finished = actions_.front()->execute();
		if (finished)
		{
			actions_.pop_front();
			update_last_action_display();
		}

		//TODO: Validate remaining actions here
	}
}

void side_actions::execute(action_ptr action)
{
	assert(!actions_.empty());
	bool finished = action->execute();
	if (finished)
	{
		remove_action(action);
	}
	//TODO: Validate remaining actions here
}

void side_actions::insert_move(unit& subject, const map_location& source_hex, const map_location& target_hex, size_t index, boost::shared_ptr<arrow> arrow,
		boost::shared_ptr<unit> fake_unit)
{
	action_ptr action(new move(subject, source_hex, target_hex, arrow, fake_unit));
	assert(index < end());
	actions_.insert(actions_.begin() + index, action);
	update_last_action_display();
}

void side_actions::queue_move(unit& subject, const map_location& source_hex, const map_location& target_hex,
		boost::shared_ptr<arrow> arrow,	boost::shared_ptr<unit> fake_unit)
{
	action_ptr action(new move(subject, source_hex, target_hex, arrow, fake_unit));
	actions_.push_back(action);
	update_last_action_display();
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
	if(!actions_.empty())
	{
		assert(index < end());

		action_set::iterator position = actions_.begin()+index;
		if (position < actions_.end())
		{
			actions_.erase(position);
			update_last_action_display();
		}
	}
}

void side_actions::remove_action(action_ptr action)
{
	if (!actions_.empty())
	{
		action_set::iterator position;
		for (position = actions_.begin(); position != actions_.end(); ++position)
		{
			if (*position == action)
			{
				actions_.erase(position);
				update_last_action_display();
				break;
			}
		}
	}
}

void side_actions::set_future_view(bool future_view)
{
	foreach (action_ptr action, actions_)
	{
		action->set_future_display(future_view);
	}
}

void side_actions::update_last_action_display()
{
	std::set<unit const*> seen_unit_list;
	action_set::reverse_iterator it;
	for (it = actions_.rbegin(); it != actions_.rend(); ++it)
	{
		action_ptr action = *it;
		unit const* target_unit = &action->get_unit();
		if (seen_unit_list.find(target_unit) == seen_unit_list.end()) // last action of this unit
		{
			seen_unit_list.insert(target_unit);
			action->set_last_action(true);
		}
		else // not the last action of this unit
		{
			action->set_last_action(false);
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
