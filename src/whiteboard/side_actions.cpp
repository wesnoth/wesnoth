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
#include "attack.hpp"
#include "manager.hpp"
#include "move.hpp"
#include "validate_visitor.hpp"

#include "foreach.hpp"
#include "game_display.hpp"
#include "resources.hpp"

#include <set>
#include <sstream>

namespace wb
{

side_actions::side_actions()
	: actions_()
{
}

side_actions::~side_actions()
{
}

void side_actions::draw_hex(const map_location& hex)
{
	const_iterator it;
	for(it = begin(); it != end(); ++it)
	{
		//call the action's own draw_hex method
		(*it)->draw_hex(hex);

		if((*it)->is_numbering_hex(hex))
		{
			//draw number corresponding to iterator's position + 1
			size_t number = (it - begin()) + 1;
			std::stringstream number_text;
			number_text << number;
			const size_t font_size = 15;
			SDL_Color color; color.r = 255; color.g = 255; color.b = 0; //yellow
			// position 0,0 in the hex is the upper left corner
			const double x_in_hex = 0.80; // 0.80 = horizontal coord., close to the right side of the hex
			const double y_in_hex = 0.5; //0.5 = halfway in the hex vertically
			resources::screen->draw_text_in_hex(hex, display::LAYER_ACTIONS_NUMBERING,
					number_text.str(), font_size, color, x_in_hex, y_in_hex);
		}
	}
}

side_actions::iterator side_actions::execute_next()
{
	if (!actions_.empty())
	{
		execute(begin());
		return begin();
	}
	return end();
}

side_actions::iterator side_actions::execute(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	if (!actions_.empty() && validate_iterator(position))
	{
		size_t distance = std::distance(begin(), position);
		action_ptr action = *position;
		bool finished = action->execute();
		if (finished)
		{
			actions_.erase(position);
			validate_actions();
			return begin() + distance;
		}
		else
		{
			actions_.erase(position);
			actions_.insert(end(), action);
			validate_actions();
			return end() - 1;
		}
	}
	else
	{
		return end();
	}
}

side_actions::iterator side_actions::insert_move(unit& subject, const map_location& source_hex, const map_location& target_hex, side_actions::iterator position,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	action_ptr action(new move(subject, source_hex, target_hex, arrow, fake_unit));
	return insert_action(position, action);
}

side_actions::iterator side_actions::queue_move(unit& subject, const map_location& source_hex,
		const map_location& target_hex,	arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	action_ptr action(new move(subject, source_hex, target_hex, arrow, fake_unit));
	return queue_action(action);
}

side_actions::iterator side_actions::queue_attack(unit& subject, const map_location& target_hex, const map_location& source_hex,
		const map_location& dest_hex, arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	action_ptr action(new attack(subject, target_hex, source_hex, dest_hex, arrow, fake_unit));
	return queue_action(action);
}

side_actions::iterator side_actions::insert_action(iterator position, action_ptr action)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}
	assert(position >= begin() && position < end());
	iterator valid_position = actions_.insert(position, action);
	validate_actions();
	return valid_position;
}

side_actions::iterator side_actions::queue_action(action_ptr action)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}
	actions_.push_back(action);
	// Contrary to insert_action, no need to validate actions here since we're adding to the end of the queue
	return end() - 1;
}

//move action toward front of queue
side_actions::iterator side_actions::bump_earlier(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	assert(validate_iterator(position));
	//Do nothing if the result position would be impossible
	if(!validate_iterator(position - 1))
		return end();

	//Verify we're not moving an action out-of-order compared to other action of the same unit
	side_actions::iterator previous = position - 1;
	if (&(*previous)->get_unit() == &(*position)->get_unit())
		return end();

	action_ptr action = *position;
	action_queue::iterator after = actions_.erase(position);
	//be careful, previous iterators have just been invalidated by erase()
	action_queue::iterator destination = after - 1;
	assert(destination >= begin() && destination <= end());
	action_queue::iterator valid_position = actions_.insert(destination, action);
	assert(validate_iterator(valid_position));
	validate_actions();
	return valid_position;
}

//move action toward back of queue
side_actions::iterator side_actions::bump_later(side_actions::iterator position)
{
	DBG_WB << "Bump requested for action from pos. " << position - begin() << "/" << actions_.size()
			<< " to pos. " << (position + 1) - begin()  << "/" << actions_.size() << ".\n";
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	assert(validate_iterator(position));
	//Do nothing if the result position would be impossible
	if(!validate_iterator(position + 1))
		return end();

	//Verify we're not moving an action out-of-order compared to other action of the same unit
	side_actions::iterator previous = position + 1;
	if (&(*previous)->get_unit() == &(*position)->get_unit())
		return end();

	action_ptr action = *position;
	action_queue::iterator after = actions_.erase(position);
	//be careful, previous iterators have just been invalidated by erase()
	DBG_WB << "Action temp. removed, position after is " << after - begin()  << "/" << actions_.size() << ".\n";
	action_queue::iterator destination = after + 1;
	assert(destination >= begin() && destination <= end());
	action_queue::iterator valid_position = actions_.insert(destination, action);
	assert(validate_iterator(valid_position));
	validate_actions();
	return valid_position;
}

side_actions::iterator side_actions::remove_action(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	assert(!actions_.empty());
	assert(validate_iterator(position));
	size_t distance = std::distance(begin(), position);
	if (!actions_.empty() && validate_iterator(position))
	{
		actions_.erase(position);
		validate_actions();
	}
	return begin() + distance;
}

side_actions::iterator side_actions::get_position_of(action_ptr action)
{
	if (!actions_.empty())
	{
		action_queue::iterator position;
		for (position = actions_.begin(); position != actions_.end(); ++position)
		{
			if (*position == action)
			{
				return position;
			}
		}
	}
	return end();
}

side_actions::iterator side_actions::find_first_action_of(const unit& unit, side_actions::iterator start_position)
{

	if (start_position == side_actions::iterator())
	{
		start_position = begin();
	}

	if (validate_iterator(start_position))
	{
		side_actions::iterator position;
		for (position = start_position; position != end(); ++position)
		{
			action_ptr action = *position;
			if (&action->get_unit() == &unit)
			{
				return position;
			}
		}
	}
	return end();
}

side_actions::iterator side_actions::find_last_action_of(const unit& unit, side_actions::iterator start_position)
{
	if (start_position == side_actions::iterator())
	{
		start_position = end() - 1;
	}

	if (validate_iterator(start_position))
	{
		side_actions::iterator position;
		for (position = start_position; position != begin() - 1; --position)
		{
			action_ptr action = *position;
			if (&action->get_unit() == &unit)
			{
				return position;
			}
		}
	}
	return end();
}

void side_actions::validate_actions()
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Validating action queue while temp modifiers are applied!!!\n";
	}

	validate_visitor validator(*resources::units, shared_from_this());
	validator.validate_actions();
}

} //end namespace wb
