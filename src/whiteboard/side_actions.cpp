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

#include "side_actions.hpp"
#include "action.hpp"
#include "attack.hpp"
#include "manager.hpp"
#include "move.hpp"
#include "recall.hpp"
#include "recruit.hpp"
#include "validate_visitor.hpp"

#include "foreach.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "resources.hpp"

#include <set>
#include <sstream>

namespace wb
{

/** Dumps side_actions on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, wb::side_actions const& side_actions)
{
	s << "Content of side_actions:";
	int count = 1;
	foreach(action_ptr action, side_actions.actions())
	{
		s << "\n" << "(" << count << ") " << action;
		count++;
	}
	if (count == 1) s << " (empty)";
	return s;
}

side_actions::side_actions()
	: actions_()
	, team_index_(0)
	, team_index_defined_(false)
	, gold_spent_(0)
{
}

side_actions::~side_actions()
{
}

void side_actions::set_team_index(size_t team_index)
{
	assert(!team_index_defined_);
	team_index_ = team_index;
	team_index_defined_ = true;
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
		return execute(begin());
	}
	else
	{
		return end();
	}
}

side_actions::iterator side_actions::execute(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	if (!actions_.empty() && validate_iterator(position))
	{
		LOG_WB << "Before execution, " << *this << "\n";
		size_t distance = std::distance(begin(), position);
		action_ptr action = *position;
		bool finished;
		try	{
			 finished = action->execute();
		} catch (end_turn_exception e) {
			actions_.erase(position);
			LOG_WB << "End turn exception caught during execution, deleting action. " << *this << "\n";
			validate_actions();
			throw;
		}

		if (finished)
		{
			actions_.erase(position);
			LOG_WB << "After execution and deletion, " << *this << "\n";
			validate_actions();
			return begin() + distance;
		}
		else
		{
			actions_.erase(position);
			actions_.insert(end(), action);
			LOG_WB << "After execution *without* deletion, " << *this << "\n";
			validate_actions();
			return end() - 1;
		}
	}
	else
	{
		return end();
	}
}

//side_actions::iterator side_actions::insert_move(const map_location& source_hex, const map_location& target_hex, side_actions::iterator position,
//		arrow_ptr arrow, fake_unit_ptr fake_unit)
//{
//	action_ptr action(new move(source_hex, target_hex, arrow, fake_unit));
//	return insert_action(position, action);
//}

side_actions::iterator side_actions::queue_move(const pathfind::marked_route& route, arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	move_ptr new_move;
	new_move.reset(new move(team_index(), route, arrow, fake_unit));
	return queue_action(new_move);
}

side_actions::iterator side_actions::queue_attack(const map_location& target_hex, int weapon_choice,
		const pathfind::marked_route& route,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	attack_ptr new_attack;
	new_attack.reset(new attack(team_index(), target_hex, weapon_choice, route, arrow, fake_unit));
	return queue_action(new_attack);
}

side_actions::iterator side_actions::queue_recruit(const std::string& unit_name, const map_location& recruit_hex)
{
	recruit_ptr new_recruit;
	new_recruit.reset(new recruit(team_index(), unit_name, recruit_hex));
	return queue_action(new_recruit);
}

side_actions::iterator side_actions::queue_recall(const unit& unit, const map_location& recall_hex)
{
	recall_ptr new_recall;
	new_recall.reset(new recall(team_index(), unit, recall_hex));
	return queue_action(new_recall);
}

side_actions::iterator side_actions::insert_action(iterator position, action_ptr action)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}
	assert(position >= begin() && position < end());
	iterator valid_position = actions_.insert(position, action);
	LOG_WB << "Inserted at position #" << std::distance(begin(), valid_position) + 1
		   << " : " << action <<"\n";
	validate_actions();
	return valid_position;
}

side_actions::iterator side_actions::queue_action(action_ptr action)
{
	actions_.push_back(action);
	// Contrary to insert_action, no need to validate actions here since we're adding to the end of the queue
	LOG_WB << "Queued: " << action <<"\n";
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

	side_actions::iterator previous = position - 1;
	//Verify we're not moving an action out-of-order compared to other action of the same unit
	{
		unit const* previous_ptr = (*previous)->get_unit();
		unit const* current_ptr = (*position)->get_unit();
		if (previous_ptr && current_ptr && previous_ptr == current_ptr)
			return end();
	}

	//If this is a move, verify that it doesn't depend on a previous move for freeing its destination
	{
		using boost::dynamic_pointer_cast;
		if (move_ptr bump_earlier = dynamic_pointer_cast<move>(*position)) {
			if (move_ptr previous_move = dynamic_pointer_cast<move>(*previous))	{
				if (bump_earlier->get_dest_hex() == previous_move->get_source_hex()) {
					return end();
				}
			}
		}
	}

	LOG_WB << "Before bumping earlier, " << *this << "\n";

	int action_number = std::distance(begin(), position) + 1;
	int last_position = actions_.size();
	LOG_WB << "Bumping action #" << action_number << "/" << last_position
			<< " to position #" << action_number - 1  << "/" << last_position << ".\n";

	action_ptr action = *position;
	action_queue::iterator after = actions_.erase(position);
	//be careful, previous iterators have just been invalidated by erase()
	action_queue::iterator destination = after - 1;
	assert(destination >= begin() && destination <= end());
	action_queue::iterator valid_position = actions_.insert(destination, action);
	assert(validate_iterator(valid_position));
	validate_actions();
	LOG_WB << "After bumping earlier, " << *this << "\n";
	return valid_position;
}

//move action toward back of queue
side_actions::iterator side_actions::bump_later(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	assert(validate_iterator(position));
	//Do nothing if the result position would be impossible
	if(!validate_iterator(position + 1))
		return end();

	side_actions::iterator next = position + 1;
	{
		unit const* next_ptr = (*next)->get_unit();
		unit const* current_ptr = (*position)->get_unit();
		//Verify we're not moving an action out-of-order compared to other action of the same unit
		if (next_ptr && current_ptr && next_ptr == current_ptr)
			return end();
	}

	//If this is a move, verify that an earlier move doesn't depend on it for freeing its destination
	{
		using boost::dynamic_pointer_cast;
		if (move_ptr bump_later = dynamic_pointer_cast<move>(*position)) {
			if (move_ptr next_move = dynamic_pointer_cast<move>(*next))	{
				if (next_move->get_dest_hex() == bump_later->get_source_hex()) {
					return end();
				}
			}
		}
	}

	LOG_WB << "Before bumping later, " << *this << "\n";

	int action_number = std::distance(begin(), position) + 1;
	int last_position = actions_.size();
	LOG_WB << "Bumping action #" << action_number << "/" << last_position
			<< " to position #" << action_number + 1  << "/" << last_position << ".\n";

	action_ptr action = *position;
	action_queue::iterator after = actions_.erase(position);
	//be careful, previous iterators have just been invalidated by erase()
	DBG_WB << "Action temp. removed, position after is #" << after - begin() + 1  << "/" << actions_.size() << ".\n";
	action_queue::iterator destination = after + 1;
	assert(destination >= begin() && destination <= end());
	action_queue::iterator valid_position = actions_.insert(destination, action);
	assert(validate_iterator(valid_position));
	validate_actions();
	LOG_WB << "After bumping later, " << *this << "\n";
	return valid_position;
}

side_actions::iterator side_actions::remove_action(side_actions::iterator position, bool validate_after_delete)
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
		LOG_WB << "Erasing action at position #" << distance + 1 << "\n";
		actions_.erase(position);
		if (validate_after_delete)
		{
			validate_actions();
		}
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
	if (!empty() && validate_iterator(start_position))
	{
		side_actions::iterator position;
		for (position = start_position; position != end(); ++position)
		{
			action_ptr action = *position;
			if (action->get_unit() == &unit)
			{
				return position;
			}
		}
	}
	return end();
}

side_actions::iterator side_actions::find_first_action_of(const unit& unit)
{
	return find_first_action_of(unit, begin());
}

side_actions::iterator side_actions::find_last_action_of(const unit& unit, side_actions::iterator start_position)
{
	if (!empty() && validate_iterator(start_position))
	{
		side_actions::iterator position;
		for (position = start_position; position != begin() - 1; --position)
		{
			action_ptr action = *position;
			if (action->get_unit() == &unit)
			{
				return position;
			}
		}
	}
	return end();
}

side_actions::iterator side_actions::find_last_action_of(const unit& unit)
{
	return find_last_action_of(unit, end() - 1);
}

bool side_actions::unit_has_actions(const unit& unit)
{
	if (empty())
		return false;
	else
		return find_first_action_of(unit) != end();
}

size_t side_actions::count_actions_of(const unit& unit)
{
	size_t count = 0;
	foreach(action_ptr action, *this)
	{
		if (action->get_unit() == &unit)
		{
			++count;
		}
	}
	return count;
}


void side_actions::validate_actions()
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Validating action queue while temp modifiers are applied!!!\n";
	}

	if (empty()) return;

	bool validation_finished = false;
	while(!validation_finished){
		validate_visitor validator(*resources::units, shared_from_this());
		validation_finished = validator.validate_actions();
	}
}

} //end namespace wb
