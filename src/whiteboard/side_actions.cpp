/* $Id$ */
/*
 Copyright (C) 2010 - 2011 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
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
#include "suppose_dead.hpp"
#include "highlight_visitor.hpp"
#include "utility.hpp"
#include "validate_visitor.hpp"

#include "actions.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "map.hpp"
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

void side_actions::get_numbers(const map_location& hex, numbers_t& result)
{
	std::vector<int>& numbers_to_draw = result.numbers_to_draw;
	std::vector<size_t>& team_numbers = result.team_numbers;
	int& main_number = result.main_number;
	std::set<size_t>& secondary_numbers = result.secondary_numbers;
	boost::shared_ptr<highlight_visitor> highlighter =
						resources::whiteboard->get_highlighter().lock();

	const_iterator it;
	for(it = begin(); it != end(); ++it)
	{
		if((*it)->is_numbering_hex(hex))
		{
			//store number corresponding to iterator's position + 1
			size_t number = (it - begin()) + 1;
			size_t index = numbers_to_draw.size();
			numbers_to_draw.push_back(number);
			team_numbers.push_back(team_index());

			if (highlighter)
			{
				if (highlighter->get_main_highlight().lock() == *it) {
					main_number = index;
				}

				foreach(weak_action_ptr action, highlighter->get_secondary_highlights())
				{
					if (action.lock() == *it)
					{
						secondary_numbers.insert(index);
					}
				}
			}
		}
	}
}

bool side_actions::execute_next()
{
	if (!actions_.empty())
	{
		return execute(begin());
	}
	else
	{
		return false;
	}
}

void side_actions::execute_all()
{
	if (actions_.empty())
	{
		WRN_WB << "\"Execute All\" attempt with empty queue.\n";
		return;
	}

	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	LOG_WB << "Before executing all actions, " << *this << "\n";

	bool keep_executing = true;
	while (keep_executing)
	{
		iterator position = begin();
		bool finished = execute(position);
		keep_executing = finished && !empty();
	}
}

bool side_actions::execute(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	if (actions_.empty() || !validate_iterator(position))
		return false;

	LOG_WB << "Before execution, " << *this << "\n";
	action_ptr action = *position;
	action::EXEC_RESULT exec_result;
	try	{
		 exec_result = action->execute();
	} catch (end_turn_exception&) {
		resources::whiteboard->queue_net_cmd(make_net_cmd_remove(position));
		actions_.erase(position);
		LOG_WB << "End turn exception caught during execution, deleting action. " << *this << "\n";
		validate_actions();
		throw;
	}

	if(resources::whiteboard->should_clear_undo())
		resources::whiteboard->clear_undo();

	if(exec_result!=action::FAIL)
	{
		resources::whiteboard->queue_net_cmd(make_net_cmd_remove(position));
		actions_.erase(position);
	}
	else //action may have revised itself; let's tell our allies.
		resources::whiteboard->queue_net_cmd(make_net_cmd_replace(position,*position));

	switch(exec_result)
	{
	case action::SUCCESS:
		LOG_WB << "After execution and deletion, " << *this << "\n";
		break;
	case action::PARTIAL:
		LOG_WB << "After failed execution *with* deletion, " << *this << "\n";
		break;
	case action::FAIL:
		//Idea that needs refining: move action at the end of the queue if it failed executing:
			//actions_.erase(position);
			//actions_.insert(end(), action);

		LOG_WB << "After failed execution *without* deletion, " << *this << "\n";
		break;
	}

	validate_actions();
	return exec_result == action::SUCCESS;
}

side_actions::iterator side_actions::queue_move(const pathfind::marked_route& route, arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	move_ptr new_move;
	new_move.reset(new move(team_index(), false, route, arrow, fake_unit));
	return queue_action(new_move);
}

side_actions::iterator side_actions::queue_attack(const map_location& target_hex, int weapon_choice,
		const pathfind::marked_route& route,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	attack_ptr new_attack;
	new_attack.reset(new attack(team_index(), false, target_hex, weapon_choice, route, arrow, fake_unit));
	return queue_action(new_attack);
}

side_actions::iterator side_actions::queue_recruit(const std::string& unit_name, const map_location& recruit_hex)
{
	recruit_ptr new_recruit;
	new_recruit.reset(new recruit(team_index(), false, unit_name, recruit_hex));
	return queue_action(new_recruit);
}

side_actions::iterator side_actions::queue_recall(const unit& unit, const map_location& recall_hex)
{
	recall_ptr new_recall;
	new_recall.reset(new recall(team_index(), false, unit, recall_hex));
	return queue_action(new_recall);
}

side_actions::iterator side_actions::queue_suppose_dead(unit& curr_unit, map_location const& loc)
{
	suppose_dead_ptr new_suppose_dead;
	new_suppose_dead.reset(new suppose_dead(team_index(),false,curr_unit,loc));
	return queue_action(new_suppose_dead);
}

side_actions::iterator side_actions::insert_action(iterator position, action_ptr action)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}
	assert(position >= begin() && position <= end());
	resources::whiteboard->queue_net_cmd(make_net_cmd_insert(position, action));
	iterator valid_position = actions_.insert(position, action);
	LOG_WB << "Inserted at position #" << std::distance(begin(), valid_position) + 1
		   << " : " << action <<"\n";
	validate_actions();
	return valid_position;
}

side_actions::iterator side_actions::queue_action(action_ptr action)
{
	//This method can no longer be any more efficient than the above insert_action(), because
	//inserting at the end of this queue may invalidate something in a different queue.
	return insert_action(end(),action);
}

//move action toward front of queue
side_actions::iterator side_actions::bump_earlier(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	assert(validate_iterator(position));

	//Do nothing if ...
	if(position == begin()) //... because (position-1) would cause deque assertion failure.
		return end();

	side_actions::iterator previous = position - 1;
	//Verify we're not moving an action out-of-order compared to other action of the same unit
	{
		unit const* previous_ptr = (*previous)->get_unit();
		unit const* current_ptr = (*position)->get_unit();
		if (previous_ptr && current_ptr && previous_ptr == current_ptr)
			return end();
	}

	{
		using boost::dynamic_pointer_cast;
		//If this is a move, verify that it doesn't depend on a previous move for freeing its destination
		if (move_ptr bump_earlier = dynamic_pointer_cast<move>(*position))
		{
			if (move_ptr previous_move = dynamic_pointer_cast<move>(*previous))
			{
				if (bump_earlier->get_dest_hex() == previous_move->get_source_hex())
				{
					return end();
				}
			}
			//Also check the case of reordering a leader's move with respect to a recruit that depend on him
			map_location recruit_recall_loc;
			if (recruit_ptr previous_recruit = dynamic_pointer_cast<recruit>(*previous))
			{
				recruit_recall_loc = previous_recruit->get_recruit_hex();
			} else if (recall_ptr previous_recall = dynamic_pointer_cast<recall>(*previous))
			{
				recruit_recall_loc = previous_recall->get_recall_hex();
			}
			if (recruit_recall_loc.valid())
			{
				unit const* leader = bump_earlier->get_unit();
				if(leader->can_recruit() &&
						resources::game_map->is_keep(leader->get_location()) &&
						can_recruit_on(*resources::game_map, leader->get_location(), recruit_recall_loc))
				{
					if(unit const* backup_leader = find_backup_leader(*leader))
					{
						side_actions::iterator it = find_first_action_of(backup_leader);
						if (!(it == end() || it > position))
							return end(); //backup leader but he moves before us, refuse bump
					}
					else
					{
						return end(); //no backup leader, refuse bump
					}
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
	resources::whiteboard->queue_net_cmd(make_net_cmd_bump_later(position-1));
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
	if (resources::whiteboard->has_planned_unit_map()) {
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

	//If the action whose place we're gonna take is a move, verify that it doesn't
	//depend on us for freeing its destination
	{
		using boost::dynamic_pointer_cast;
		if (move_ptr next_move = dynamic_pointer_cast<move>(*next))
		{
			if (move_ptr bump_later = dynamic_pointer_cast<move>(*position))
			{
				if (next_move->get_dest_hex() == bump_later->get_source_hex())
				{
					return end();
				}
			} else {
				//Check that we're not bumping this planned recruit after a move of the leader it depends on
				map_location recruit_recall_loc;
				if (recruit_ptr bump_later = dynamic_pointer_cast<recruit>(*position))
				{
					recruit_recall_loc = bump_later->get_recruit_hex();
				}
				else if (recall_ptr bump_later = dynamic_pointer_cast<recall>(*position))
				{
					recruit_recall_loc = bump_later->get_recall_hex();
				}

				if (recruit_recall_loc.valid())
				{
					unit const* leader = next_move->get_unit();
					if(leader->can_recruit() &&
							resources::game_map->is_keep(leader->get_location()) &&
							can_recruit_on(*resources::game_map, leader->get_location(), recruit_recall_loc))
					{
						if (unit const* backup_leader = find_backup_leader(*leader))
						{
							side_actions::iterator it = find_first_action_of(backup_leader);
							if (!(it == end() || it > position))
								return end(); //backup leader but already programmed to act before us, refuse bump
						}
						else
						{
							return end(); //no backup leader, refuse bump
						}
					}
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
	resources::whiteboard->queue_net_cmd(make_net_cmd_bump_later(position));
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

		resources::whiteboard->queue_net_cmd(make_net_cmd_remove(position));
		safe_erase(position);

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

side_actions::iterator side_actions::find_first_action_of(unit const* unit, side_actions::iterator start_position)
{
	if (!empty() && validate_iterator(start_position))
	{
		side_actions::iterator position;
		for (position = start_position; position != end(); ++position)
		{
			action_ptr action = *position;
			if (action->get_unit() == unit)
			{
				return position;
			}
		}
	}
	return end();
}

side_actions::iterator side_actions::find_first_action_of(unit const* unit)
{
	if (empty())
		return end();
	else
		return find_first_action_of(unit, begin());
}

side_actions::iterator side_actions::find_last_action_of(unit const* unit, side_actions::iterator start_position)
{
	if (!empty() && validate_iterator(start_position))
	{
		reverse_iterator position(start_position);
		for (--position ; position != rend(); ++position)
		{
			if ((*position)->get_unit() == unit)
			{
				iterator found_position = position.base();
				//need to decrement after changing from reverse to regular iterator
				return --found_position;
			}
		}
	}
	return end();
}

side_actions::iterator side_actions::find_last_action_of(unit const* unit)
{
	if (empty())
		return end();
	else
		return find_last_action_of(unit, end() - 1);
}

bool side_actions::unit_has_actions(unit const* unit)
{
	if (empty())
		return false;
	else
		return find_first_action_of(unit) != end();
}

size_t side_actions::count_actions_of(unit const* unit)
{
	size_t count = 0;
	foreach(action_ptr action, *this)
	{
		if (action->get_unit() == unit)
		{
			++count;
		}
	}
	return count;
}

void side_actions::remove_invalid_of(unit const* u)
{
	iterator i = begin();
	while(i != end())
	{
		action& act = **i;
		if(!act.is_valid() && u == act.get_unit())
			i = remove_action(i,false);
		else ++i;
	}
}

void side_actions::validate_actions()
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Validating action queue while temp modifiers are applied!!!\n";
	}

	bool validation_finished = false;
	int passes = 1;
	while(!validation_finished){
		log_scope2("whiteboard", "Validating actions for side "
				+ lexical_cast<std::string>(team_index() + 1) + ", pass "
				+ lexical_cast<std::string>(passes));
		validate_visitor validator(*resources::units);
		validation_finished = validator.validate_actions();
		++passes;
	}
}

void side_actions::execute_net_cmd(net_cmd const& cmd)
{
	std::string type = cmd["type"];

	if(type=="insert")
	{
		int pos = cmd["pos"];
		if((pos < 0) || (pos > static_cast<int>(actions_.size())))
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!\n";
			return;
		}

		action_queue::iterator itor = actions_.begin()+pos;

		try {
			itor = actions_.insert(itor,action::from_config(cmd.child("action"),false));
		} catch(action::ctor_err const&) {
			ERR_WB << "side_actions::execute_network_command(): received invalid data!\n";
			return;
		}

		//update numbering hexes as necessary
		++itor;
		action_queue::iterator end = actions_.end();
		for( ; itor!=end; ++itor)
			resources::screen->invalidate((*itor)->get_numbering_hex());
	}
	else if(type=="replace")
	{
		int pos = cmd["pos"];
		if(pos<0 || pos>=static_cast<int>(actions_.size()))
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!\n";
			return;
		}

		action_queue::iterator itor = actions_.begin()+pos;
		itor = safe_erase(itor);

		try {
			itor = actions_.insert(itor,action::from_config(cmd.child("action"),false));
		} catch(action::ctor_err const&) {
			ERR_WB << "side_actions::execute_network_command(): received invalid data!\n";
			return;
		}
	}
	else if(type=="remove")
	{
		int pos = cmd["pos"];
		if(pos<0 || pos>=static_cast<int>(actions_.size()))
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!\n";
			return;
		}
		action_queue::iterator itor = actions_.begin()+pos;
		itor = safe_erase(itor);

		//update numbering hexes as necessary
		action_queue::iterator end = actions_.end();
		for( ; itor!=end; ++itor)
			resources::screen->invalidate((*itor)->get_numbering_hex());
	}
	else if(type=="bump_later")
	{
		int pos = cmd["pos"];
		if(pos<0 || pos>=static_cast<int>(actions_.size())-1)
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!\n";
			return;
		}

		action_queue::iterator itor = actions_.begin()+pos;
		action_ptr first_action = *itor;
		itor = actions_.erase(itor);
		action_ptr second_action = *itor;
		actions_.insert(++itor,first_action);

		//update numbering hexes as necessary
		resources::screen->invalidate(first_action->get_numbering_hex());
		resources::screen->invalidate(second_action->get_numbering_hex());
	}
	else if(type=="clear")
	{
		safe_clear();
	}
	else
	{
		ERR_WB << "side_actions::execute_network_command(): received invalid type!\n";
		return;
	}

	validate_actions();
}

side_actions::net_cmd side_actions::make_net_cmd_insert(const_iterator const& pos, action_ptr act) const
{
	net_cmd result;
	result["type"] = "insert";
	result["pos"] = static_cast<int>(std::distance(begin(),pos));
	result.add_child("action",act->to_config());
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_replace(const_iterator const& pos, action_ptr act) const
{
	net_cmd result;
	result["type"] = "replace";
	result["pos"] = static_cast<int>(std::distance(begin(),pos));
	result.add_child("action",act->to_config());
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_remove(const_iterator const& pos) const
{
	net_cmd result;
	result["type"] = "remove";
	result["pos"] = static_cast<int>(std::distance(begin(),pos));
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_bump_later(const_iterator const& pos) const
{
	net_cmd result;
	result["type"] = "bump_later";
	result["pos"] = static_cast<int>(std::distance(begin(),pos));
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_clear() const
{
	net_cmd result;
	result["type"] = "clear";
	return result;
}

} //end namespace wb
