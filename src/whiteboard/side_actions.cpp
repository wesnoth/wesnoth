/* $Id$ */
/*
 Copyright (C) 2010 - 2013 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "map.hpp"
#include "resources.hpp"

#include <boost/foreach.hpp>

#include <set>
#include <sstream>

namespace wb
{

/** Dumps side_actions on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, wb::side_actions const& side_actions)
{
	s << "Content of side_actions:";
	int turn = 1;
	BOOST_FOREACH(action_queue const& turn_queue, side_actions.actions())
	{
		s << "\n  Turn " << turn;
		++turn;

		int count = 1;
		BOOST_FOREACH(action_ptr const& action, turn_queue)
		{
			s << "\n    (" << count << ") " << action;
			++count;
		}
	}
	if (turn == 1) s << " (empty)";
	return s;
}

side_actions::side_actions()
	: actions_()
	, team_index_(0)
	, team_index_defined_(false)
	, gold_spent_(0)
	, hidden_(false)
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
	if(empty()) {
		return;
	}

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

				BOOST_FOREACH(weak_action_ptr action, highlighter->get_secondary_highlights())
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
	if(!empty() && !actions_.front().empty())
		return execute(begin());
	else //nothing is executable right now
		return false;
}

bool side_actions::execute(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	assert(position.turn_num_ == 0); //can't execute actions from future turns

	if(actions_.empty() || !validate_iterator(position))
		return false;

	LOG_WB << "Before execution, " << *this << "\n";

	action_ptr action = *position;

	if (!action->is_valid())
	{
		LOG_WB << "Invalid action sent to execution, deleting.\n";
		synced_erase(position);
		return true;
	}

	bool action_successful;
	// Determines whether action should be deleted. Interrupted moves return action_complete == false.
	bool action_complete;
	try	{
		 action->execute(action_successful,action_complete);
	} catch (end_turn_exception&) {
		synced_erase(position);
		LOG_WB << "End turn exception caught during execution, deleting action. " << *this << "\n";
		//validate actions at next map rebuild
		resources::whiteboard->on_gamestate_change();
		throw;
	}

	if(resources::whiteboard->should_clear_undo())
		resources::whiteboard->clear_undo();

	std::stringstream ss;
	ss << "After " << (action_successful? "successful": "failed") << " execution ";
	if(action_complete)
	{
		ss << "with deletion, ";
		synced_erase(position);
	}
	else //action may have revised itself; let's tell our allies.
	{
		ss << "without deletion, ";
		resources::whiteboard->queue_net_cmd(team_index_,make_net_cmd_replace(position,*position));

		//Idea that needs refining: move action at the end of the queue if it failed executing:
			//actions_.erase(position);
			//actions_.insert(end(), action);
	}
	ss << *this << "\n";
	LOG_WB << ss.str();

	validate_actions();
	return action_successful;
}

size_t side_actions::size() const
{
	size_t result = 0;
	BOOST_FOREACH(action_queue const& queue, actions_)
		result += queue.size();
	return result;
}

side_actions::iterator side_actions::turn_begin(size_t turn_num)
{
	if(turn_num < num_turns())
		return iterator(actions_[turn_num].begin(),turn_num,*this);
	else //turn_num out of range
		return end();
}
side_actions::iterator side_actions::turn_end(size_t turn_num)
	{return turn_begin(turn_num+1);}
side_actions::reverse_iterator side_actions::turn_rbegin(size_t turn_num)
	{return reverse_iterator(turn_end(turn_num));}
side_actions::reverse_iterator side_actions::turn_rend(size_t turn_num)
	{return reverse_iterator(turn_begin(turn_num));}

side_actions::range_t side_actions::iter_turn(size_t turn_num)
	{return range_t(turn_begin(turn_num),turn_end(turn_num));}
side_actions::rrange_t side_actions::riter_turn(size_t turn_num)
	{return rrange_t(turn_rbegin(turn_num),turn_rend(turn_num));}

void side_actions::hide()
{
	if(hidden_)
		return;

	hidden_ = true;

	if(empty()) {
		return;
	}

	BOOST_FOREACH(action_ptr act, *this)
		act->hide();
}
void side_actions::show()
{
	if(!hidden_)
		return;

	hidden_ = false;

	BOOST_FOREACH(action_ptr act, *this)
		act->show();
}

side_actions::iterator side_actions::queue_move(size_t turn, unit& mover, const pathfind::marked_route& route, arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	move_ptr new_move(new move(team_index(), hidden_, mover, route, arrow, fake_unit));
	return queue_action(turn,new_move);
}

side_actions::iterator side_actions::queue_attack(size_t turn, unit& mover, const map_location& target_hex, int weapon_choice,
		const pathfind::marked_route& route,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	attack_ptr new_attack(new attack(team_index(), hidden_, mover, target_hex, weapon_choice, route, arrow, fake_unit));
	return queue_action(turn,new_attack);
}

side_actions::iterator side_actions::queue_recruit(size_t turn, const std::string& unit_name, const map_location& recruit_hex)
{
	recruit_ptr new_recruit(new recruit(team_index(), hidden_, unit_name, recruit_hex));
	return queue_action(turn,new_recruit);
}

side_actions::iterator side_actions::queue_recall(size_t turn, const unit& unit, const map_location& recall_hex)
{
	recall_ptr new_recall(new recall(team_index(), hidden_, unit, recall_hex));
	return queue_action(turn,new_recall);
}

side_actions::iterator side_actions::queue_suppose_dead(size_t turn, unit& curr_unit, map_location const& loc)
{
	suppose_dead_ptr new_suppose_dead(new suppose_dead(team_index(),hidden_,curr_unit,loc));
	return queue_action(turn,new_suppose_dead);
}

side_actions::iterator side_actions::insert_action(iterator position, action_ptr action)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}
	iterator valid_position = synced_insert(position, action);
	LOG_WB << "Inserted into turn #" << (valid_position.turn_num_+1) << " at position #"
			<< (valid_position.base_-actions_[valid_position.turn_num_].begin()+1) << " : " << action <<"\n";
	validate_actions();
	return valid_position;
}

side_actions::iterator side_actions::queue_action(size_t turn_num, action_ptr action)
{
	if(resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}
	iterator result = synced_enqueue(turn_num, action);
	LOG_WB << "Inserted into turn #" << (turn_num+1) << " at position #" << actions_[turn_num].size()
			<< " : " << action <<"\n";
	validate_actions();
	return result;
}

//move action toward front of queue
side_actions::iterator side_actions::bump_earlier(side_actions::iterator position)
{
	if (resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!\n";
	}

	assert(validate_iterator(position));

	//Don't allow bumping the very first action any earlier, of course.
	//Also, don't allow bumping an action into a previous turn queue
	if(position.base_ == actions_[position.turn_num_].begin())
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
						if (!(it == end() || position < it))
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

	int action_number = std::distance(actions_[position.turn_num_].begin(), position.base_) + 1;
	int last_position = actions_[position.turn_num_].size();
	LOG_WB << "In turn-queue #" << (position.turn_num_+1)
			<< ", bumping action #" << action_number << "/" << last_position
			<< " to position #" << action_number - 1  << "/" << last_position << ".\n";

	action_ptr action = *position;
	resources::whiteboard->queue_net_cmd(team_index_,make_net_cmd_bump_later(position-1));
	iterator after = raw_erase(position);
	//be careful, previous iterators have just been invalidated by raw_erase()
	iterator destination = after - 1;
	iterator valid_position = raw_insert(destination, action);
	assert(validate_iterator(valid_position));
	validate_actions();
	LOG_WB << "After bumping earlier, " << *this << "\n";
	return valid_position;
}

//move action toward back of queue
side_actions::iterator side_actions::bump_later(side_actions::iterator position)
{
	iterator end_itor = end();
	assert(position != end_itor);

	++position;
	if(position == end_itor)
		return end_itor;
	position = bump_earlier(position);
	if(position == end())
		return end_itor;
	return position+1;
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

		synced_erase(position);

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
		iterator position = begin();
		iterator end_itor = end();
		for(; position != end_itor; ++position)
			if (*position == action)
				return position;
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
				iterator found_position(position);
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
	BOOST_FOREACH(action_ptr action, *this)
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

static side_actions::const_iterator find_last_valid_of(unit const& u, side_actions const& sa_const)
{
	side_actions& sa = const_cast<side_actions&>(sa_const);
	side_actions::iterator end = sa.end();

	if(sa.empty())
		return sa.end();

	side_actions::iterator begin = sa.begin();
	side_actions::iterator itor  = end;
	do {
		if(itor==begin && !(*itor)->is_valid())
			return end; //this unit has no valid actions!
		itor = sa.find_last_action_of(&u,itor - 1);
	} while(itor!=end && !(*itor)->is_valid());

	return itor;
}
size_t side_actions::get_turn_num_of(unit const& u) const
{
	const_iterator itor = find_last_valid_of(u,*this); //helper fcn -- above
	if(itor == end())
		return 0;
	return itor.base_.turn_num_;
}

void side_actions::validate_actions()
{
	assert(!resources::whiteboard->has_planned_unit_map());

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

void side_actions::change_gold_spent_by(int difference)
{
	DBG_WB << "Changing gold spent for side " << (team_index() + 1)	<< "; old value: "
			<< gold_spent_ << "; new value: " << (gold_spent_ + difference) << "\n";
	gold_spent_ += difference; assert(gold_spent_ >= 0);
}

void side_actions::reset_gold_spent()
{
	DBG_WB << "Resetting gold spent for side " << (team_index() + 1) << " to 0.\n";
	gold_spent_ = 0;
}

/* private */
void side_actions::update_size()
{
	while(!actions_.empty() && actions_.back().empty())
		actions_.pop_back();
}

side_actions::iterator side_actions::raw_erase(iterator itor)
{
	//precondition
	assert(itor!=end());

	//prepare
	iterator next = itor+1;
	bool deleting_last_element = (next == end());
	size_t turn_num = itor.turn_num_;
	bool next_is_still_valid = (turn_num != next.turn_num_);

	//erase!
	action_queue::iterator after_erase = actions_[turn_num].erase(itor.base_);

	//post-processing (determine return value, possibly resize container)
	if(deleting_last_element)
	{
		update_size(); //might need to shrink the container
		return end();
	}
	else if(next_is_still_valid)
		return next;
	else
		return iterator(after_erase, turn_num, *this);
}

side_actions::iterator side_actions::raw_insert(iterator itor, action_ptr act)
{
	size_t turn_num = itor.turn_num_;
	action_queue::iterator new_itor = actions_[itor.turn_num_].insert(itor.base_,act);
	return iterator(new_itor,turn_num,*this);
}

side_actions::iterator side_actions::raw_enqueue(size_t turn_num, action_ptr act)
{
	//for a little extra safety, since we should never resize by much at a time
	assert(turn_num <= actions_.size() || turn_num <= 2);

	if(turn_num >= actions_.size())
		actions_.resize(turn_num+1);
	actions_[turn_num].push_back(act);
	return iterator(actions_[turn_num].end()-1,turn_num,*this);
}

side_actions::iterator side_actions::safe_insert(size_t turn, size_t pos, action_ptr act)
{
	assert(act);

	iterator result;

	size_t queue_count = actions_.size();
	if((turn == queue_count   &&   pos == 0)
			|| (turn < queue_count   &&   pos == actions_[turn].size()))
		result = raw_enqueue(turn,act);
	else if(turn < queue_count   &&   pos < actions_[turn].size())
	{
		result = iterator(actions_[turn].begin()+pos,turn,*this);
		result = raw_insert(result,act);
	}
	else //bad position
		result = end();

	return result;
}

side_actions::iterator side_actions::synced_erase(iterator itor)
{
	resources::whiteboard->queue_net_cmd(team_index_,make_net_cmd_remove(itor));
	return safe_erase(itor);
}

side_actions::iterator side_actions::synced_insert(iterator itor, action_ptr act)
{
	resources::whiteboard->queue_net_cmd(team_index_,make_net_cmd_insert(itor,act));
	return raw_insert(itor,act);
}

side_actions::iterator side_actions::synced_enqueue(size_t turn_num, action_ptr act)
{
	//raw_enqueue() creates actions_[turn_num] if it doesn't exist already, so we
	//have to do it first -- before subsequently calling actions_[turn_num].size().
	iterator result = raw_enqueue(turn_num,act);
	resources::whiteboard->queue_net_cmd(team_index_,make_net_cmd_insert(turn_num,actions_[turn_num].size()-1,act));
	return result;
}

side_actions::iterator side_actions::safe_erase(iterator const& itor)
{
	action_ptr action = *itor;
	resources::whiteboard->pre_delete_action(action); //misc cleanup
	iterator return_itor = raw_erase(itor);
	resources::whiteboard->post_delete_action(action);
	return return_itor;
}

void side_actions::execute_net_cmd(net_cmd const& cmd)
{
	std::string type = cmd["type"];

	if(type=="insert")
	{
		size_t turn = cmd["turn"].to_int();
		size_t pos = cmd["pos"].to_int();
		action_ptr act = action::from_config(cmd.child("action"),hidden_);
		if(!act)
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid action data!\n";
			return;
		}

		iterator itor = safe_insert(turn,pos,act);
		if(itor == end())
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid insertion position!\n";
			return;
		}

		//update numbering hexes as necessary
		++itor;
		iterator end_itor = end();
		for( ; itor!=end_itor; ++itor)
			resources::screen->invalidate((*itor)->get_numbering_hex());
	}
	else if(type=="replace")
	{
		size_t turn = cmd["turn"].to_int();
		size_t pos = cmd["pos"].to_int();
		action_ptr act = action::from_config(cmd.child("action"),hidden_);
		if(!act)
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid action data!\n";
			return;
		}

		if(turn >= actions_.size()
				|| pos >= actions_[turn].size())
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!\n";
			return;
		}

		iterator itor = turn_begin(turn)+pos;
		itor = raw_insert(itor,act);
		safe_erase(++itor);
	}
	else if(type=="remove")
	{
		size_t turn = cmd["turn"].to_int();
		size_t pos = cmd["pos"].to_int();
		if(turn >= actions_.size()
				|| pos >= actions_[turn].size())
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!\n";
			return;
		}

		iterator itor = turn_begin(turn)+pos;
		itor = safe_erase(itor);

		//update numbering hexes as necessary
		iterator end_itor = end();
		for( ; itor!=end_itor; ++itor)
			resources::screen->invalidate((*itor)->get_numbering_hex());
	}
	else if(type=="bump_later")
	{
		size_t turn = cmd["turn"].to_int();
		size_t pos = cmd["pos"].to_int();
		if(turn >= actions_.size()
				|| pos+1 >= actions_[turn].size())
		{
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!\n";
			return;
		}

		action_queue::iterator itor = actions_[turn].begin()+pos;
		action_ptr first_action = *itor;
		itor = actions_[turn].erase(itor);
		action_ptr second_action = *itor;
		actions_[turn].insert(++itor,first_action);

		//update numbering hexes as necessary
		resources::screen->invalidate(first_action->get_numbering_hex());
		resources::screen->invalidate(second_action->get_numbering_hex());
	}
	else if(type=="clear")
	{
		safe_clear();
	}
	else if(type=="refresh")
	{
		safe_clear();
		BOOST_FOREACH(net_cmd const& sub_cmd, cmd.child_range("net_cmd"))
			execute_net_cmd(sub_cmd);
	}
	else
	{
		ERR_WB << "side_actions::execute_network_command(): received invalid type!\n";
		return;
	}

	validate_actions();
}

side_actions::net_cmd side_actions::make_net_cmd_insert(size_t turn_num, size_t pos, action_const_ptr act) const
{
	net_cmd result;
	result["type"] = "insert";
	result["turn"] = static_cast<int>(turn_num);
	result["pos"] = static_cast<int>(pos);
	result.add_child("action",act->to_config());
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_insert(const_iterator const& pos, action_const_ptr act) const
{
	return make_net_cmd_insert(pos.base_.turn_num_,std::distance<action_queue::const_iterator>(actions_[pos.base_.turn_num_].begin(),pos.base_.base_),act);
}
side_actions::net_cmd side_actions::make_net_cmd_replace(const_iterator const& pos, action_const_ptr act) const
{
	net_cmd result;
	result["type"] = "replace";
	result["turn"] = static_cast<int>(pos.base_.turn_num_);
	result["pos"] = static_cast<int>(std::distance<action_queue::const_iterator>(actions_[pos.base_.turn_num_].begin(),pos.base_.base_));
	result.add_child("action",act->to_config());
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_remove(const_iterator const& pos) const
{
	net_cmd result;
	result["type"] = "remove";
	result["turn"] = static_cast<int>(pos.base_.turn_num_);
	result["pos"] = static_cast<int>(std::distance<action_queue::const_iterator>(actions_[pos.base_.turn_num_].begin(),pos.base_.base_));
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_bump_later(const_iterator const& pos) const
{
	net_cmd result;
	result["type"] = "bump_later";
	result["turn"] = static_cast<int>(pos.base_.turn_num_);
	result["pos"] = static_cast<int>(std::distance<action_queue::const_iterator>(actions_[pos.base_.turn_num_].begin(),pos.base_.base_));
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_clear() const
{
	net_cmd result;
	result["type"] = "clear";
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_refresh() const
{
	net_cmd result;
	result["type"] = "refresh";

	const_iterator end = this->end();
	for(const_iterator itor=begin(); itor!=end; ++itor)
		result.add_child("net_cmd",make_net_cmd_insert(itor.base_.turn_num_,std::distance<action_queue::const_iterator>(actions_[itor.base_.turn_num_].begin(),itor.base_.base_),*itor));

	return result;
}

//initialize static member
side_actions::iterator side_actions::null(action_queue::iterator(),0,NULL);

bool side_actions::validate_iterator(iterator position) { return position != end(); }

side_actions::iterator side_actions::begin()
{
	if(actions_.empty())
		return null;
	return iterator(actions_.front().begin(),0,*this);
}
side_actions::reverse_iterator side_actions::rbegin()
	{ return reverse_iterator(end()); }
side_actions::const_iterator side_actions::begin() const
	{ return const_iterator(const_cast<side_actions*>(this)->begin()); }
side_actions::const_reverse_iterator side_actions::rbegin() const
	{ return const_reverse_iterator(const_cast<side_actions*>(this)->rbegin()); }

side_actions::iterator side_actions::end()
{
	if(actions_.empty())
		return null;
	return iterator(actions_.back().end(),actions_.size()-1,*this);
}
side_actions::reverse_iterator side_actions::rend()
	{ return reverse_iterator(begin()); }
side_actions::const_iterator side_actions::end() const
	{ return const_iterator(const_cast<side_actions*>(this)->end()); }
side_actions::const_reverse_iterator side_actions::rend() const
	{ return const_reverse_iterator(const_cast<side_actions*>(this)->rend()); }

void side_actions::raw_turn_shift()
{
	//optimization
	if(actions_.size() < 2)
		return;

	//find units who still have plans for turn 0 (i.e. were too lazy to finish their jobs)
	std::set<unit const*> lazy_units;
	BOOST_FOREACH(action_ptr const& act, iter_turn(0))
	{
		unit const* u = act->get_unit();
		if(u)
			lazy_units.insert(u);
	}

	//push their plans back one turn
	std::set<unit const*>::iterator lazy_end = lazy_units.end();
	iterator itor = end();
	while(itor != begin())
	{
		--itor;
		action_ptr act = *itor;

		if(lazy_units.find(act->get_unit()) != lazy_end)
		{
			safe_insert(itor.turn_num_+1,0,act);
			itor = raw_erase(itor);
		}
	}

	//push any remaining first-turn plans into the second turn
	BOOST_FOREACH(action_ptr act, actions_.front())
		actions_[1].push_front(act);
	actions_.front().clear();

	//shift everything forward one turn
	actions_.pop_front();
}

void side_actions::synced_turn_shift()
{
	raw_turn_shift();
	resources::whiteboard->queue_net_cmd(team_index(),make_net_cmd_refresh());
}

} //end namespace wb
