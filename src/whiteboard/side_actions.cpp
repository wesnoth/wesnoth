/*
 Copyright (C) 2010 - 2018 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include <set>
#include <sstream>
#include <iterator>

#include "whiteboard/side_actions.hpp"

#include "whiteboard/action.hpp"
#include "whiteboard/attack.hpp"
#include "whiteboard/manager.hpp"
#include "whiteboard/mapbuilder.hpp"
#include "whiteboard/move.hpp"
#include "whiteboard/recall.hpp"
#include "whiteboard/recruit.hpp"
#include "whiteboard/suppose_dead.hpp"
#include "whiteboard/highlighter.hpp"
#include "whiteboard/utility.hpp"

#include "actions/create.hpp"
#include "actions/undo.hpp"
#include "display.hpp"
#include "game_end_exceptions.hpp"
#include "game_state.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "units/unit.hpp"
#include "utils/iterable_pair.hpp"

namespace wb
{

/** Dumps side_actions on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &out, const wb::side_actions& side_actions)
{
	out << "Content of side_actions:";
	for(size_t turn = 0; turn < side_actions.num_turns(); ++turn) {
		out << "\n  Turn " << turn;

		int count = 1;
		for(wb::side_actions::const_iterator it = side_actions.turn_begin(turn); it != side_actions.turn_end(turn); ++it) {
			out << "\n    (" << count++ << ") " << *it;
		}

		if(side_actions.turn_size(turn) == 0) {
			out << "\n    (empty)";
		}
	}

	if(side_actions.empty()) {
		out << " (empty)";
	}

	return out;
}

side_actions_container::side_actions_container()
	: actions_()
	, turn_beginnings_()
{
}

size_t side_actions_container::get_turn_impl(size_t begin, size_t end, const_iterator it) const
{
	if(begin+1 >= end) {
		return begin;
	}
	size_t mid = (begin+end) / 2;
	if(it < turn_beginnings_[mid]) {
		return get_turn_impl(begin, mid, it);
	} else if(it > turn_beginnings_[mid]) {
		return get_turn_impl(mid, end, it);
	} else {
		return mid;
	}
}

size_t side_actions_container::get_turn(const_iterator it) const
{
	return get_turn_impl(0, num_turns(), it);
}

size_t side_actions_container::position_in_turn(const_iterator it) const
{
	return it - turn_begin( get_turn(it) );
}

side_actions_container::iterator side_actions_container::turn_begin(size_t turn_num){
	if(turn_num >= num_turns()) {
		return end();
	} else {
		return turn_beginnings_[turn_num];
	}
}

side_actions_container::const_iterator side_actions_container::turn_begin(size_t turn_num) const
{
	if(turn_num >= num_turns()) {
		return end();
	} else {
		return turn_beginnings_[turn_num];
	}
}

side_actions_container::iterator side_actions_container::push_front(size_t turn, action_ptr action){
	if(turn_size(turn) == 0) {
		return queue(turn, action);
	}

	iterator res = insert(turn_begin(turn), action);
	if(res != end()) {
		bool current_turn_unplanned = turn_size(0) == 0;
		turn_beginnings_[turn] = res;

		if(current_turn_unplanned && turn == 1) {
			turn_beginnings_.front() = res;
		}
	}
	return res;
}

side_actions_container::iterator side_actions_container::insert(iterator position, action_ptr action)
{
	assert(position <= end());

	bool first = position == begin();

	std::pair<iterator,bool> res = actions_.insert(position, action);
	if(!res.second) {
		return end();
	}
	if(first) {
		// If we are inserting before the first action, then the inserted action should became the first of turn 0.
		turn_beginnings_.front() = begin();
	}
	return res.first;
}

side_actions_container::iterator side_actions_container::queue(size_t turn_num, action_ptr action)
{
	// Are we inserting an action in the future while the current turn is empty?
	// That is, are we in the sole case where an empty turn can be followed by a non-empty one.
	bool future_only = turn_num == 1 && num_turns() == 0;

	bool current_turn_unplanned = turn_size(0) == 0;

	//for a little extra safety, since we should never resize by much at a time
	assert(turn_num <= num_turns() || future_only);

	std::pair<iterator,bool> res = actions_.insert(turn_end(turn_num), action);
	if(!res.second) {
		return end();
	}

	if(future_only) {
		// No action are planned for the current turn but we are planning an action for turn 1 (the next turn).
		turn_beginnings_.push_back(res.first);
	}
	if(turn_num >= num_turns()) {
		turn_beginnings_.push_back(res.first);
	} else if(current_turn_unplanned && turn_num == 0) {
		// We are planning the first action of the current turn while others actions are planned in the future.
		turn_beginnings_.front() = res.first;
	}

	return res.first;
}

side_actions_container::iterator side_actions_container::bump_earlier(iterator position)
{
	assert(position > begin());
	assert(position < end());

	action_ptr rhs = *position;
	action_ptr lhs = *(position - 1);

	actions_.replace(position, lhs);
	actions_.replace(position - 1, rhs);
	return position - 1;
}

side_actions_container::iterator side_actions_container::bump_later(iterator position)
{
	return bump_earlier(position + 1);
}

side_actions_container::iterator side_actions_container::erase(iterator position)
{
	//precondition
	assert(position < end());

	//prepare
	iterator next = position + 1;
	bool deleting_last_element = next == end();

	// pre-processing (check if position is at the beginning of a turn)
	action_limits::iterator beginning = std::find(turn_beginnings_.begin(), turn_beginnings_.end(), position);
	if(beginning != turn_beginnings_.end()) {
		if(deleting_last_element) {
			if(size() == 1) {
				// If we are deleting our sole action, we can clear turn_beginnings_ (and we have to if this last action is in turn 1)
				turn_beginnings_.clear();
			} else {
				// Otherwise, we just delete the last turn
				turn_beginnings_.pop_back();
			}
		} else {
			size_t turn_of_position = std::distance(turn_beginnings_.begin(), beginning);
			// If we still have action this turn
			if(get_turn(next) == turn_of_position) {
				*beginning = next; // We modify the beginning of the turn
			} else {
				assert(turn_of_position == 0);
				*beginning = turn_end(0); // Otherwise, we are emptying the current turn.
			}
		}
	}

	//erase!
	return actions_.erase(position);
}

side_actions_container::iterator side_actions_container::erase(iterator first, iterator last){
	// @todo rewrite using boost::multi_index::erase(iterator,iterator) for efficiency.
	if(first>=last) {
		return last;
	}
	for(iterator it = last-1; it>first; --it) {
		it = erase(it);
	}
	return erase(first);
}


side_actions::side_actions()
	: actions_()
	, team_index_(0)
	, team_index_defined_(false)
	, gold_spent_(0)
	, hidden_(false)
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
	std::shared_ptr<highlighter> hlighter = resources::whiteboard->get_highlighter().lock();

	for(const_iterator it = begin(); it != end(); ++it) {
		if((*it)->is_numbering_hex(hex)) {
			//store number corresponding to iterator's position + 1
			size_t number = (it - begin()) + 1;
			size_t index = numbers_to_draw.size();
			numbers_to_draw.push_back(number);
			team_numbers.push_back(team_index());

			if(hlighter) {
				if(hlighter->get_main_highlight().lock() == *it) {
					main_number = index;
				}

				for(weak_action_ptr action : hlighter->get_secondary_highlights()) {
					if(action.lock() == *it) {
						secondary_numbers.insert(index);
					}
				}
			}
		}
	}
}

bool side_actions::execute_next()
{
	if(!empty()) {
		return execute(begin());
	} else { //nothing is executable right now
		return false;
	}
}

bool side_actions::execute(side_actions::iterator position)
{
	if(resources::whiteboard->has_planned_unit_map()) {
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!" << std::endl;
	}

	if(actions_.empty() || position == actions_.end()) {
		return false;
	}

	assert(position < turn_end(0)); //can't execute actions from future turns

	LOG_WB << "Before execution, " << *this << "\n";

	action_ptr action = *position;

	if(!action->valid()) {
		LOG_WB << "Invalid action sent to execution, deleting.\n";
		synced_erase(position);
		return true;
	}

	bool action_successful;
	// Determines whether action should be deleted. Interrupted moves return action_complete == false.
	bool action_complete;
	try	{
		 action->execute(action_successful, action_complete);
	} catch (return_to_play_side_exception&) {
		synced_erase(position);
		LOG_WB << "End turn exception caught during execution, deleting action. " << *this << "\n";
		//validate actions at next map rebuild
		resources::whiteboard->on_gamestate_change();
		throw;
	}

	if(resources::whiteboard->should_clear_undo()) {
		resources::undo_stack->clear();
	}

	std::stringstream ss;
	ss << "After " << (action_successful? "successful": "failed") << " execution ";
	if(action_complete) {
		ss << "with deletion, ";
		synced_erase(position);
	}
	else { //action may have revised itself; let's tell our allies.
		ss << "without deletion, ";
		resources::whiteboard->queue_net_cmd(team_index_,make_net_cmd_replace(position,*position));

		//Idea that needs refining: move action at the end of the queue if it failed executing:
			//actions_.erase(position);
			//actions_.insert(end(), action);
	}
	ss << *this << "\n";
	LOG_WB << ss.str();

	resources::whiteboard->validate_viewer_actions();
	return action_successful;
}

void side_actions::hide()
{
	if(hidden_) {
		return;
	}

	hidden_ = true;

	for(action_ptr act : *this) {
		act->hide();
	}
}
void side_actions::show()
{
	if(!hidden_) {
		return;
	}

	hidden_ = false;

	for(action_ptr act : *this) {
		act->show();
	}
}

side_actions::iterator side_actions::insert_action(iterator position, action_ptr action)
{
	if(resources::whiteboard->has_planned_unit_map()) {
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!" << std::endl;
	}
	iterator valid_position = synced_insert(position, action);
	LOG_WB << "Inserted into turn #" << get_turn(valid_position) << " at position #"
			<< actions_.position_in_turn(valid_position) << " : " << action <<"\n";
	resources::whiteboard->validate_viewer_actions();
	return valid_position;
}

side_actions::iterator side_actions::queue_action(size_t turn_num, action_ptr action)
{
	if(resources::whiteboard->has_planned_unit_map()) {
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!" << std::endl;
	}
	iterator result = synced_enqueue(turn_num, action);
	LOG_WB << "Queue into turn #" << turn_num << " : " << action <<"\n";
	resources::whiteboard->validate_viewer_actions();
	return result;
}

namespace
{
	/**
	 * Check whether a move is swapable with a given action.
	 */
	struct swapable_with_move: public visitor
	{
	public:
		swapable_with_move(side_actions &sa, side_actions::iterator position, move_ptr second): sa_(sa), valid_(false), position_(position), second_(second) {}
		bool valid() const { return valid_; }

		void visit(move_ptr first) {
			valid_ = second_->get_dest_hex() != first->get_source_hex();
		}

		void visit(attack_ptr first) {
			visit(std::static_pointer_cast<move>(first));
		}

		void visit(recruit_ptr first) {
			check_recruit_recall(first->get_recruit_hex());
		}

		void visit(recall_ptr first) {
			check_recruit_recall(first->get_recall_hex());
		}

		void visit(suppose_dead_ptr) {
			valid_ = true;
		}

	private:
		side_actions &sa_;
		bool valid_;
		side_actions::iterator position_;
		move_ptr second_;

		void check_recruit_recall(const map_location &loc) {
			const unit_const_ptr leader = second_->get_unit();
			if(leader->can_recruit() && dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(*leader, loc)) {
				if(const unit_const_ptr backup_leader = find_backup_leader(*leader)) {
					side_actions::iterator it = sa_.find_first_action_of(*backup_leader);
					if(!(it == sa_.end() || position_ < it)) {
						return; //backup leader but he moves before us, refuse bump
					}
				} else {
					return; //no backup leader, refuse bump
				}
			}
			valid_ = true;
		}
	};
}

//move action toward front of queue
side_actions::iterator side_actions::bump_earlier(side_actions::iterator position)
{
	if(resources::whiteboard->has_planned_unit_map()) {
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!" << std::endl;
	}

	assert(position <= end());

	//Don't allow bumping the very first action any earlier, of course.
	//Also, don't allow bumping an action into a previous turn queue
	if(actions_.position_in_turn(position) == 0) {
		return end();
	}

	side_actions::iterator previous = position - 1;

	//Verify we're not moving an action out-of-order compared to other action of the same unit
	const unit_const_ptr previous_ptr = (*previous)->get_unit();
	const unit_const_ptr current_ptr = (*position)->get_unit();
	if(previous_ptr && current_ptr && previous_ptr.get() == current_ptr.get()) {
		return end();
	}

	if(move_ptr second = std::dynamic_pointer_cast<move>(*position)) {
		swapable_with_move check(*this, position, second);
		(*previous)->accept(check);
		if(!check.valid()) {
			return end();
		}
	}

	LOG_WB << "Before bumping earlier, " << *this << "\n";

	int turn_number = get_turn(position);
	int action_number = actions_.position_in_turn(position);
	int last_position = turn_size(turn_number) - 1;
	LOG_WB << "In turn #" << turn_number
			<< ", bumping action #" << action_number << "/" << last_position
			<< " to position #" << action_number - 1  << "/" << last_position << ".\n";

	resources::whiteboard->queue_net_cmd(team_index_, make_net_cmd_bump_later(position - 1));

	actions_.bump_earlier(position);

	LOG_WB << "After bumping earlier, " << *this << "\n";
	return position - 1;
}

//move action toward back of queue
side_actions::iterator side_actions::bump_later(side_actions::iterator position)
{
	assert(position < end());

	++position;
	if(position == end()) {
		return end();
	}
	position = bump_earlier(position);
	if(position == end()) {
		return end();
	}
	return position + 1;
}

side_actions::iterator side_actions::remove_action(side_actions::iterator position, bool validate_after_delete)
{
	if(resources::whiteboard->has_planned_unit_map()) {
		ERR_WB << "Modifying action queue while temp modifiers are applied!!!" << std::endl;
	}

	assert(position < end());

	LOG_WB << "Erasing action at turn #" << get_turn(position) << " position #" << actions_.position_in_turn(position) << "\n";

	position = synced_erase(position);

	if(validate_after_delete) {
		resources::whiteboard->validate_viewer_actions();
	}

	return position;
}

side_actions::iterator side_actions::find_first_action_at(map_location hex)
{
	return find_first_action_of(actions_.get<container::by_hex>().equal_range(hex), begin(), std::less<iterator>());
}

side_actions::iterator side_actions::find_first_action_of(const unit& unit, side_actions::iterator start_position)
{
	return find_first_action_of(actions_.get<container::by_unit>().equal_range(unit.underlying_id()), start_position, std::less<iterator>());
}

side_actions::const_iterator side_actions::find_last_action_of(const unit& unit, side_actions::const_iterator start_position) const {
	return find_first_action_of(actions_.get<container::by_unit>().equal_range(unit.underlying_id()), start_position, std::greater<iterator>());
}

side_actions::iterator side_actions::find_last_action_of(const unit& unit, side_actions::iterator start_position)
{
	return find_first_action_of(actions_.get<container::by_unit>().equal_range(unit.underlying_id()), start_position, std::greater<iterator>());
}

side_actions::const_iterator side_actions::find_last_action_of(const unit& unit) const
{
	if(end() == begin()) {
		return end();
	}
	return find_last_action_of(unit, end() - 1);
}

side_actions::iterator side_actions::find_last_action_of(const unit& unit)
{
	if(end() == begin()) {
		return end();
	}
	return find_last_action_of(unit, end() - 1);
}

bool side_actions::unit_has_actions(const unit& unit)
{
	return actions_.get<container::by_unit>().find(unit.underlying_id()) != actions_.get<container::by_unit>().end();
}

size_t side_actions::count_actions_of(const unit& unit)
{
	return actions_.get<container::by_unit>().count(unit.underlying_id());
}

std::deque<action_ptr> side_actions::actions_of(const unit& target)
{
	typedef container::action_set::index<container::by_unit>::type::iterator unit_iterator;
	std::pair<unit_iterator, unit_iterator> action_its = actions_.get<container::by_unit>().equal_range(target.underlying_id());

	std::deque<action_ptr> actions (action_its.first, action_its.second);
	return actions;
}

size_t side_actions::get_turn_num_of(const unit& u) const
{
	const_iterator itor = find_last_action_of(u);
	if(itor == end()) {
		return 0;
	}
	return get_turn(itor);
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

side_actions::iterator side_actions::safe_insert(size_t turn, size_t pos, action_ptr act)
{
	assert(act);
	if(pos == 0) {
		return actions_.push_front(turn, act);
	} else {
		return actions_.insert(turn_begin(turn) + pos, act);
	}
}

side_actions::iterator side_actions::synced_erase(iterator itor)
{
	resources::whiteboard->queue_net_cmd(team_index_, make_net_cmd_remove(itor));
	return safe_erase(itor);
}

side_actions::iterator side_actions::synced_insert(iterator itor, action_ptr act)
{
	resources::whiteboard->queue_net_cmd(team_index_, make_net_cmd_insert(itor, act));
	return actions_.insert(itor, act);
}

side_actions::iterator side_actions::synced_enqueue(size_t turn_num, action_ptr act)
{
	//raw_enqueue() creates actions_[turn_num] if it doesn't exist already, so we
	//have to do it first -- before subsequently calling actions_[turn_num].size().
	iterator result = actions_.queue(turn_num, act);
	if(result != end()) {
		resources::whiteboard->queue_net_cmd(team_index_, make_net_cmd_insert(turn_num, turn_size(turn_num) - 1, act));
		// The insert position is turn_size(turn_num)-1 since we already inserted the action.
	}
	return result;
}

side_actions::iterator side_actions::safe_erase(const iterator& itor)
{
	action_ptr action = *itor;
	resources::whiteboard->pre_delete_action(action); //misc cleanup
	iterator return_itor = actions_.erase(itor);
	resources::whiteboard->post_delete_action(action);
	return return_itor;
}
side_actions::iterator side_actions::queue_move(size_t turn, unit& mover, const pathfind::marked_route& route, arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	move_ptr new_move(std::make_shared<move>(team_index(), hidden_, std::ref(mover), route, arrow, fake_unit));
	return queue_action(turn, new_move);
}

side_actions::iterator side_actions::queue_attack(size_t turn, unit& mover, const map_location& target_hex, int weapon_choice, const pathfind::marked_route& route, arrow_ptr arrow, fake_unit_ptr fake_unit)
{
	attack_ptr new_attack(std::make_shared<attack>(team_index(), hidden_, std::ref(mover), target_hex, weapon_choice, route, arrow, fake_unit));
	return queue_action(turn, new_attack);
}

side_actions::iterator side_actions::queue_recruit(size_t turn, const std::string& unit_name, const map_location& recruit_hex)
{
	recruit_ptr new_recruit(std::make_shared<recruit>(team_index(), hidden_, unit_name, recruit_hex));
	return queue_action(turn, new_recruit);
}

side_actions::iterator side_actions::queue_recall(size_t turn, const unit& unit, const map_location& recall_hex)
{
	recall_ptr new_recall(std::make_shared<recall>(team_index(), hidden_, unit, recall_hex));
	return queue_action(turn, new_recall);
}

side_actions::iterator side_actions::queue_suppose_dead(size_t turn, unit& curr_unit, const map_location& loc)
{
	suppose_dead_ptr new_suppose_dead(std::make_shared<suppose_dead>(team_index(), hidden_, std::ref(curr_unit), loc));
	return queue_action(turn, new_suppose_dead);
}

void side_actions::execute_net_cmd(const net_cmd& cmd)
{
	std::string type = cmd["type"];

	if(type=="insert") {
		size_t turn = cmd["turn"].to_int();
		size_t pos = cmd["pos"].to_int();
		action_ptr act = action::from_config(cmd.child("action"), hidden_);
		if(!act) {
			ERR_WB << "side_actions::execute_network_command(): received invalid action data!" << std::endl;
			return;
		}

		iterator itor = safe_insert(turn, pos, act);
		if(itor >= end()) {
			ERR_WB << "side_actions::execute_network_command(): received invalid insertion position!" << std::endl;
			return;
		}

		LOG_WB << "Command received: action inserted on turn #" << turn << ", position #" << pos << ": " << act << "\n";

		//update numbering hexes as necessary
		++itor;
		for(iterator end_itor = end(); itor != end_itor; ++itor) {
			display::get_singleton()->invalidate((*itor)->get_numbering_hex());
		}
	} else if(type=="replace") {
		size_t turn = cmd["turn"].to_int();
		size_t pos = cmd["pos"].to_int();
		action_ptr act = action::from_config(cmd.child("action"), hidden_);
		if(!act) {
			ERR_WB << "side_actions::execute_network_command(): received invalid action data!" << std::endl;
			return;
		}

		iterator itor = turn_begin(turn) + pos;
		if(itor >= end() || get_turn(itor) != turn) {
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!" << std::endl;
			return;
		}

		if(!actions_.replace(itor, act)){
			ERR_WB << "side_actions::execute_network_command(): replace failed!" << std::endl;
			return;
		}

		LOG_WB << "Command received: action replaced on turn #" << turn << ", position #" << pos << ": " << act << "\n";
	} else if(type=="remove") {
		size_t turn = cmd["turn"].to_int();
		size_t pos = cmd["pos"].to_int();

		iterator itor = turn_begin(turn) + pos;
		if(itor >= end() || get_turn(itor) != turn) {
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!" << std::endl;
			return;
		}

		itor = safe_erase(itor);

		LOG_WB << "Command received: action removed on turn #" << turn << ", position #" << pos << "\n";

		//update numbering hexes as necessary
		for(iterator end_itor = end(); itor != end_itor; ++itor) {
			display::get_singleton()->invalidate((*itor)->get_numbering_hex());
		}
	} else if(type=="bump_later") {
		size_t turn = cmd["turn"].to_int();
		size_t pos = cmd["pos"].to_int();

		iterator itor = turn_begin(turn) + pos;
		if(itor+1 >= end() || get_turn(itor) != turn) {
			ERR_WB << "side_actions::execute_network_command(): received invalid pos!" << std::endl;
			return;
		}

		action_ptr first_action = *itor;
		action_ptr second_action = itor[1];
		bump_later(itor);

		LOG_WB << "Command received: action bumped later from turn #" << turn << ", position #" << pos << "\n";

		//update numbering hexes as necessary
		display::get_singleton()->invalidate(first_action->get_numbering_hex());
		display::get_singleton()->invalidate(second_action->get_numbering_hex());
	} else if(type=="clear") {
		LOG_WB << "Command received: clear\n";
		clear();
	} else if(type=="refresh") {
		LOG_WB << "Command received: refresh\n";
		clear();
		for(const net_cmd& sub_cmd : cmd.child_range("net_cmd"))
			execute_net_cmd(sub_cmd);
	} else {
		ERR_WB << "side_actions::execute_network_command(): received invalid type!" << std::endl;
		return;
	}

	resources::whiteboard->validate_viewer_actions();
}

side_actions::net_cmd side_actions::make_net_cmd_insert(size_t turn_num, size_t pos, action_const_ptr act) const
{
	net_cmd result;
	result["type"] = "insert";
	result["turn"] = static_cast<int>(turn_num);
	result["pos"] = static_cast<int>(pos);
	result.add_child("action", act->to_config());
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_insert(const const_iterator& pos, action_const_ptr act) const
{
	if(pos == begin()) {
		return make_net_cmd_insert(0,0,act);
	} else {
		const_iterator prec = pos - 1;
		return make_net_cmd_insert(get_turn(prec), actions_.position_in_turn(prec)+1, act);
	}
}
side_actions::net_cmd side_actions::make_net_cmd_replace(const const_iterator& pos, action_const_ptr act) const
{
	net_cmd result;
	result["type"] = "replace";
	result["turn"] = static_cast<int>(get_turn(pos));
	result["pos"] = static_cast<int>(actions_.position_in_turn(pos));
	result.add_child("action", act->to_config());
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_remove(const const_iterator& pos) const
{
	net_cmd result;
	result["type"] = "remove";
	result["turn"] = static_cast<int>(get_turn(pos));
	result["pos"] = static_cast<int>(actions_.position_in_turn(pos));
	return result;
}
side_actions::net_cmd side_actions::make_net_cmd_bump_later(const const_iterator& pos) const
{
	net_cmd result;
	result["type"] = "bump_later";
	result["turn"] = static_cast<int>(get_turn(pos));
	result["pos"] = static_cast<int>(actions_.position_in_turn(pos));
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

	for(const_iterator itor = begin(), end_itor = end(); itor != end_itor; ++itor) {
		result.add_child("net_cmd", make_net_cmd_insert(get_turn(itor), actions_.position_in_turn(itor), *itor));
	}

	return result;
}

void side_actions::raw_turn_shift()
{
	//find units who still have plans for turn 0 (i.e. were too lazy to finish their jobs)
	std::set<unit_const_ptr> lazy_units;
	for(const action_ptr& act : iter_turn(0)) {
		unit_const_ptr u = act->get_unit();
		if(u) {
			lazy_units.insert(u);
		}
	}

	//push their plans back one turn
	std::set<unit_const_ptr>::iterator lazy_end = lazy_units.end();
	iterator itor = end();
	while(itor != begin()) {
		--itor;
		action_ptr act = *itor;

		if(lazy_units.find(act->get_unit()) != lazy_end) {
			safe_insert(get_turn(itor)+1, 0, act);
			itor = actions_.erase(itor);
		}
	}

	//push any remaining first-turn plans into the second turn
	for(iterator act=turn_begin(0), end=turn_end(0); act!=end; ++act) {
		safe_insert(1, 0, *act);
	}

	//shift everything forward one turn
	actions_.erase(turn_begin(0), turn_end(0));
	actions_.turn_shift();
}

void side_actions::synced_turn_shift()
{
	raw_turn_shift();
	resources::whiteboard->queue_net_cmd(team_index(), make_net_cmd_refresh());
}

} //end namespace wb
