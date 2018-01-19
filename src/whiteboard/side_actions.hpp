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

#pragma once

#include <deque>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

#include "action.hpp"
#include "typedefs.hpp"

namespace wb
{

class move;

/**
 * Datastructure holding the actions of a side on multiple turns.
 *
 * @invariant forall(t>0) if turn_size(t)==0 then turn_size(t+1)==0
 */
class side_actions_container
{
public:

	side_actions_container();

	//! Tag for action_set's random_access index.
	struct chronological{};

	//! Tag for action_set's hashed_non_unique index.
	struct by_unit{};

	//! Tag for action_set's hashed_non_unique index.
	struct by_hex{};

	//! Underlying container
	typedef boost::multi_index::multi_index_container <
		action_ptr,
		boost::multi_index::indexed_by<
			boost::multi_index::random_access<
				boost::multi_index::tag< chronological > >,
			boost::multi_index::hashed_non_unique<
				boost::multi_index::tag< by_unit >,
				boost::multi_index::const_mem_fun< action, size_t, &action::get_unit_id > >,
			boost::multi_index::hashed_non_unique<
				boost::multi_index::tag< by_hex >,
				boost::multi_index::const_mem_fun< action, map_location, &action::get_numbering_hex > >
			>
		> action_set;

	typedef action_set::index<chronological>::type::iterator iterator;
	typedef action_set::index<chronological>::type::const_iterator const_iterator;
	typedef action_set::index<chronological>::type::reverse_iterator reverse_iterator;
	typedef action_set::index<chronological>::type::const_reverse_iterator const_reverse_iterator;

	typedef std::pair<iterator,iterator> range_t;
	typedef std::pair<reverse_iterator,reverse_iterator> rrange_t;
	typedef std::pair<const_iterator,const_iterator> crange_t;
	typedef std::pair<const_reverse_iterator,const_reverse_iterator> crrange_t;

	typedef std::deque<iterator> action_limits;


	/**
	 * Inserts an action at the specified position.
	 *
	 * The planned turn of the inserted action is the same as the planned turn of position-1 before the insertion.
	 * If position == begin(), the new action will became the first action of the current turn.
	 *
	 * @param position   The iterator before which action will be inserted.
	 * @param action     The action to insert.
	 *
	 * @return           The inserted action's position.
	 * @retval end()     When the action can't be inserted.
	 */
	iterator insert(iterator position, action_ptr action);

	/**
	 * Queues an action to be executed last
	 * @return The queued action's position
	 * @retval end() when the action can't be inserted
	 */
	iterator queue(size_t turn_num, action_ptr action);

	/**
	 * Pushes an action in front of a given turn.
	 * @return The inserted action's position
	 */
	iterator push_front(size_t turn, action_ptr action);

	/**
	 * Moves an action earlier in the execution order.
	 * i.e. at the front of the queue by one position.
	 * @return The action's new position.
	 */
	iterator bump_earlier(iterator position);

	/**
	 * Moves an action later in the execution order.
	 * i.e. at the back of the queue by one position.
	 * @return The action's new position.
	 */
	iterator bump_later(iterator position);

	/**
	 * Deletes the action at the specified position.
	 * @return The position of the element after the one deleted, or end() if the queue is empty.
	 */
	iterator erase(iterator position);

	/**
	 * Deletes the action at the specified position.
	 * @return last
	 */
	iterator erase(iterator first, iterator last);

	/**
	 * Empties the action queue.
	 */
	void clear() { actions_.clear(); turn_beginnings_.clear(); }

	/**
	 * Shift turn.
	 *
	 * The turn 0 is deleted, the actions of turn n are moved to turn n-1.
	 * @pre turn_size(0)==0
	 */
	void turn_shift() { assert(turn_size(0)==0); turn_beginnings_.pop_front(); }

	/**
	 * Replaces the action at a given position with another action.
	 */
	bool replace(iterator it, action_ptr act){ return actions_.replace(it, act); }


	/**
	 * Returns a given index.
	 */
	template <typename T>
	typename action_set::index<T>::type& get(){ return actions_.get<T>(); }
	template <typename T>
	typename action_set::index<T>::type const& get() const { return actions_.get<T>(); }

	/**
	 * Projects an iterator on a given index.
	 */
	template <typename T, typename U>
	typename action_set::index<T>::type::iterator project(U it){ return actions_.project<T>(it); }
	template <typename T, typename U>
	typename action_set::index<T>::type::const_iterator project(U it) const { return actions_.project<T>(it); }

	/**
	 * Returns the iterator for the first (executed earlier) action within the actions queue.
	 */
	iterator begin(){ return actions_.get<chronological>().begin(); }
	/** reverse version of the above */
	reverse_iterator rbegin(){ return actions_.get<chronological>().rbegin(); }
	/** const versions of the above */
	const_iterator begin() const { return actions_.get<chronological>().cbegin(); }
	/** const reverse versions of the above */
	const_reverse_iterator rbegin() const { return actions_.get<chronological>().crbegin(); }

	/**
	 * Returns the iterator for the position *after* the last executed action within the actions queue.
	 */
	iterator end(){ return actions_.get<chronological>().end(); }
	/** reverse version of the above */
	reverse_iterator rend(){ return actions_.get<chronological>().rend(); }
	/** const versions of the above */
	const_iterator end() const { return actions_.get<chronological>().cend(); }
	/** const reverse versions of the above */
	const_reverse_iterator rend() const { return actions_.get<chronological>().crend(); }

	/**
	 * Indicates whether the action queue is empty.
	 */
	bool empty() const { return actions_.empty(); }

	/**
	 * Returns the number of actions in the action queue.
	 */
	size_t size() const { return actions_.size(); }

	/**
	 * Returns the number of turns that have plans.
	 * If the container holds only one action on turn 1 (that is turn 0 is empty),
	 * this function will still returns 2. Indeed, turn 0 has an "empty" plan.
	 *
	 * @note The current turn is counted. That is if num_turns()==0 then empty()==true.
	 */
	size_t num_turns() const { return turn_beginnings_.size(); }

	/**
	 * Returns the turn of a given iterator planned execution.
	 *
	 * The value returned is the difference between the planned turn and the current turn.
	 *
	 * @retval 0 If the action is planned for the current turn.
	 */
	size_t get_turn(const_iterator it) const;

	/**
	 * Returns the position of a given iterator in its turn.
	 */
	size_t position_in_turn(const_iterator it) const;

	/**
	 * Returns the iterator for the first (executed earlier) action of a given turn within the actions queue.
	 */
	iterator turn_begin(size_t turn_num);
	const_iterator turn_begin(size_t turn_num) const;
	reverse_iterator turn_rbegin(size_t turn_num){ return reverse_iterator(turn_end(turn_num)); }
	const_reverse_iterator turn_rbegin(size_t turn_num) const { return reverse_iterator(turn_end(turn_num)); }

	/*
	 * Returns the iterator for the position *after* the last executed action of a given turn within the actions queue.
	 */
	iterator turn_end(size_t turn_num){ return turn_begin(turn_num+1); }
	const_iterator turn_end(size_t turn_num) const { return turn_begin(turn_num+1); }
	reverse_iterator turn_rend(size_t turn_num){ return reverse_iterator(turn_begin(turn_num)); }
	const_reverse_iterator turn_rend(size_t turn_num) const { return reverse_iterator(turn_begin(turn_num)); }

	/**
	 * Returns an iterator range corresponding to the requested turn.
	 */
	range_t iter_turn(size_t turn_num){ return range_t(turn_begin(turn_num),turn_end(turn_num)); }
	rrange_t riter_turn(size_t turn_num){ return rrange_t(turn_rbegin(turn_num),turn_rend(turn_num)); }
	crange_t iter_turn(size_t turn_num) const { return crange_t(turn_begin(turn_num),turn_end(turn_num)); }
	crrange_t riter_turn(size_t turn_num) const { return crrange_t(turn_rbegin(turn_num),turn_rend(turn_num)); }

	/** Returns the number of actions planned for turn turn_num */
	size_t turn_size(size_t turn_num) const { return turn_end(turn_num) - turn_begin(turn_num); }

	/** Get the underlying action container */
	action_set const& actions() const { return actions_; }


private:
	/**
	 * Binary search to find the occuring turn of the action pointed by an iterator.
	 */
	size_t get_turn_impl(size_t begin, size_t end, const_iterator it) const;

	action_set actions_;

	/**
	 * Contains a list of iterator to the beginning of each turn.
	 *
	 * @invariant turn_beginnings_.front()==actions_.begin() || actions_.empty()
	 */
	action_limits turn_beginnings_;
};


/**
 * This internal whiteboard class holds the planned action queues for a team, and offers many
 * utility methods to create and manipulate them.
 * It also provides an interface to the underlying side_actions_container.
 */
class side_actions: public std::enable_shared_from_this<side_actions>
{
public:
	typedef side_actions_container container;

	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;
	typedef container::reverse_iterator reverse_iterator;
	typedef container::const_reverse_iterator const_reverse_iterator;

	typedef std::pair<iterator,iterator> range_t;
	typedef std::pair<reverse_iterator,reverse_iterator> rrange_t;
	typedef std::pair<const_iterator,const_iterator> crange_t;
	typedef std::pair<const_reverse_iterator,const_reverse_iterator> crrange_t;


	side_actions();

	/** Must be called only once, right after the team that owns this side_actions is added to the teams vector */
	void set_team_index(size_t team_index);

	/** Returns the team index this action queue belongs to */
	size_t team_index() { assert(team_index_defined_); return team_index_; }

	struct numbers_t;
	/** Gets called when display is drawing a hex to determine which numbers to draw on it */
	void get_numbers(const map_location& hex, numbers_t& result);

	/**
	 * Executes the first action in the queue, and then deletes it.
	 * @return true if the action was completed successfully
	 */
	bool execute_next();

	/**
	 * Executes the specified action, if it exists in the queue.
	 * If the action is not finished, it's moved at the end of the queue.
	 * @return true if the action was completed successfully
	 */
	bool execute(iterator position);


	/**
	 * Indicates whether the action queue is empty.
	 */
	bool empty() const { return actions_.empty(); }

	/**
	 * Returns the number of actions in the action queue.
	 */
	size_t size() const { return actions_.size(); }

	/**
	 * Returns the number of turns that have plans.
	 * If the container holds only one action on turn 1 (that is turn 0 is empty),
	 * this function will still returns 2. Indeed, turn 0 has an "empty" plan.
	 *
	 * @note The current turn is counted. That is if num_turns()==0 then empty()==true.
	 */
	size_t num_turns() const { return actions_.num_turns(); }

	/** Returns the number of actions planned for turn turn_num */
	size_t turn_size(size_t turn_num) const { return actions_.turn_size(turn_num); }

	/**
	 * Returns the turn of a given iterator planned execution.
	 *
	 * The value returned is the difference between the planned turn and the current turn.
	 *
	 * @retval 0 If the action is planned for the current turn.
	 */
	size_t get_turn(const_iterator it) const { return actions_.get_turn(it); }

	/**
	 * Empties the action queue.
	 */
	void clear() { actions_.clear(); }

	/** Sets whether or not the contents should be drawn on the screen. */
	void hide();
	void show();
	bool hidden() const {return hidden_;}

	/**
	 * Inserts an action at the specified position. The begin() and end() functions might prove useful here.
	 * @return The inserted action's position.
	 */
	iterator insert_action(iterator position, action_ptr action);

	/**
	 * Queues an action to be executed last
	 * @return The queued action's position
	 */
	iterator queue_action(size_t turn_num, action_ptr action);

	/**
	 * Moves an action earlier in the execution order.
	 * i.e. at the front of the queue by one position.
	 * @return The action's new position.
	 */
	iterator bump_earlier(iterator position);

	/**
	 * Moves an action later in the execution order.
	 * i.e. at the back of the queue by one position.
	 * @return The action's new position.
	 */
	iterator bump_later(iterator position);

	/**
	 * Deletes the action at the specified position.
	 * @return The position of the element after the one deleted, or end() if the queue is empty.
	 */
	iterator remove_action(iterator position, bool validate_after_delete = true);

	/**
	 * @param action The action whose position you're looking for
	 * @return The action's position within the queue, or end() if action wasn't found.
	 */
	iterator get_position_of(action_ptr action){ return std::find(begin(), end(), action); }

	/**
	 * Returns the iterator for the first (executed earlier) action within the actions queue.
	 */
	iterator begin(){ return actions_.begin(); }
	/** reverse version of the above */
	reverse_iterator rbegin(){ return actions_.rbegin(); }
	/** const versions of the above */
	const_iterator begin() const { return actions_.begin(); }
	const_reverse_iterator rbegin() const { return actions_.rbegin(); }

	/**
	 * Returns the iterator for the position *after* the last executed action within the actions queue.
	 */
	iterator end(){ return actions_.end(); }
	/** reverse version of the above */
	reverse_iterator rend(){ return actions_.rend(); }
	/** const versions of the above */
	const_iterator end() const { return actions_.end(); }
	const_reverse_iterator rend() const { return actions_.rend(); }

	iterator turn_begin(size_t turn_num){ return actions_.turn_begin(turn_num); }
	iterator turn_end(size_t turn_num){ return actions_.turn_end(turn_num); }
	reverse_iterator turn_rbegin(size_t turn_num){ return actions_.turn_rbegin(turn_num); }
	reverse_iterator turn_rend(size_t turn_num){ return actions_.turn_rend(turn_num); }
	const_iterator turn_begin(size_t turn_num) const { return actions_.turn_begin(turn_num); }
	const_iterator turn_end(size_t turn_num) const { return actions_.turn_end(turn_num); }
	const_reverse_iterator turn_rbegin(size_t turn_num) const { return actions_.turn_rbegin(turn_num); }
	const_reverse_iterator turn_rend(size_t turn_num) const { return actions_.turn_rend(turn_num); }

	/** Returns an iterator range corresponding to the requested turn. */
	range_t iter_turn(size_t turn_num){ return actions_.iter_turn(turn_num); }
	rrange_t riter_turn(size_t turn_num){ return actions_.riter_turn(turn_num); }
	crange_t iter_turn(size_t turn_num) const { return actions_.iter_turn(turn_num); }
	crrange_t riter_turn(size_t turn_num) const { return actions_.riter_turn(turn_num); }


	/**
	 * Find the (chronologically) first action between the iterators between.first and between.second but after or equals to limit with respect to the predicate comp.
	 *
	 * This function makes sense when T is a non-chronological iterator.
	 * If T is @ref iterator and Compare is std::less<iterator>,
	 * this function returns limit if limit is in [between.first, between.second)
	 * or between.first if between.first>limit or end() otherwise.
	 *
	 * @param between the two iterators between which the action will be searched.
	 * @param limit the lower bound to search from, that is the return value `it' will verify !comp(limit, it).
	 * @param comp the predicate to compare with.
	 * @return `it' so that for all values `x' in [between.first, between.second), chronologically, !comp(x, it) and !comp(it, limit).
	 * @retval end() if no such action exist.
	 */
	template <typename T, typename Compare>
	iterator find_first_action_of(std::pair<T,T> between, iterator limit, Compare comp);
	template <typename T, typename Compare>
	const_iterator find_first_action_of(std::pair<T,T> between, const_iterator limit, Compare comp) const;

	/**
	 * Find the first action occurring at a given hex.
	 *
	 * @retval end() if no action occurs at the given location.
	 */
	iterator find_first_action_at(map_location hex);

	/**
	 * Finds the first action that belongs to this unit, starting the search at the specified position.
	 * @return The position, or end() if not found.
	 */
	iterator find_first_action_of(unit const& unit, iterator start_position);
	/** Variant of this method that always start searching at the beginning of the queue */
	iterator find_first_action_of(unit const& unit){ return find_first_action_of(unit, begin()); }

	/**
	 * Finds the last action that belongs to this unit, starting the search backwards from the specified position.
	 * @return The position, or end() if not found.
	 */
	iterator find_last_action_of(unit const& unit, iterator start_position);
	/** const variant of the previous function */
	const_iterator find_last_action_of(unit const& unit, const_iterator start_position) const;
	/** Variant of the previous method that always start searching at the end of the queue */
	iterator find_last_action_of(unit const& unit);
	/** const variant of the previous function */
	const_iterator find_last_action_of(unit const& unit) const;

	bool unit_has_actions(unit const& unit);
	size_t count_actions_of(unit const& unit);
	std::deque<action_ptr> actions_of(unit const& unit);

	/**
	 * Determines the appropriate turn number for the next action planned for this unit
	 *
	 * @warning A return value of 0 can mean that the unit has one action planned on turn 0 or that the unit doesn't have any action planned on any turn.
	 *
	 * @retval 0 if the unit doesn't have any planned action
	 */
	size_t get_turn_num_of(unit const&) const;

	/** Used to track gold spending by recruits/recalls when building the future unit map */
	int get_gold_spent() const { return gold_spent_; }
	/** Used to track gold spending by recruits/recalls when building the future unit map */
	void change_gold_spent_by(int difference);
	/** Set gold spent back to zero */
	void reset_gold_spent();

	void raw_turn_shift();
	void synced_turn_shift();

	/**
	 * Queues a move to be executed last
	 * @return The queued move's position
	 */
	iterator queue_move(size_t turn_num, unit& mover, const pathfind::marked_route& route,
			arrow_ptr arrow, fake_unit_ptr fake_unit);

	/**
	 * Queues an attack or attack-move to be executed last
	 * @return The queued attack's position
	 */
	iterator queue_attack(size_t turn_num, unit& mover, const map_location& target_hex, int weapon_choice, const pathfind::marked_route& route,
			arrow_ptr arrow, fake_unit_ptr fake_unit);

	/**
	 * Queues a recruit to be executed last
	 * @return The queued recruit's position
	 */
	iterator queue_recruit(size_t turn_num, const std::string& unit_name, const map_location& recruit_hex);

	/**
	 * Queues a recall to be executed last
	 * @return The queued recall's position
	 */
	iterator queue_recall(size_t turn_num, const unit& unit, const map_location& recall_hex);

	/**
	 * Queues a suppose_dead to be executed last
	 * @return The queued suppose_dead's position (an iterator to it)
	 */
	iterator queue_suppose_dead(size_t turn_num, unit& curr_unit, map_location const& loc);

	/**
	 * Network code. A net_cmd object (a config in disguise) represents a modification
	 * to a side_actions object. execute_net_cmd() translates one of these into
	 * a real modification of *this. The make_net_cmd_***() family of functions is
	 * convenient for building specific types of net_cmds.
	 */
	typedef config net_cmd;
	void execute_net_cmd(net_cmd const&);
	net_cmd make_net_cmd_insert(size_t turn_num, size_t pos, action_const_ptr) const;
	net_cmd make_net_cmd_insert(const_iterator const& pos, action_const_ptr) const;
	net_cmd make_net_cmd_replace(const_iterator const& pos, action_const_ptr) const;
	net_cmd make_net_cmd_remove(const_iterator const& pos) const;
	net_cmd make_net_cmd_bump_later(const_iterator const& pos) const;
	net_cmd make_net_cmd_clear() const;
	net_cmd make_net_cmd_refresh() const;

private:
	iterator safe_insert(size_t turn_num, size_t pos, action_ptr to_insert);
	iterator synced_erase(iterator itor);
	iterator synced_insert(iterator itor, action_ptr to_insert);
	iterator synced_enqueue(size_t turn_num, action_ptr to_insert);
	iterator safe_erase(iterator const& itor);

	container actions_;

	size_t team_index_;
	bool team_index_defined_;

	/** Used to store gold "spent" in planned recruits/recalls when the future unit map is applied */
	int gold_spent_;

	bool hidden_;
};

/** Dumps side_actions on a stream, for debug purposes. */
std::ostream& operator<<(std::ostream &out, wb::side_actions const &side_actions);

struct side_actions::numbers_t
{
	std::vector<int> numbers_to_draw;
	std::vector<size_t> team_numbers;
	int main_number;
	std::set<size_t> secondary_numbers;

	numbers_t()
			: numbers_to_draw()
			, team_numbers()
			, main_number(-1)
			, secondary_numbers()
		{}
};

template <typename T, typename Compare>
side_actions::iterator side_actions::find_first_action_of(std::pair<T,T> between, iterator limit, Compare comp)
{
	iterator first = actions_.end();
	for(T it = between.first; it != between.second; ++it) {
		iterator chrono_it = actions_.project<container::chronological>(it);
		if((comp(chrono_it, first) || first==actions_.end()) && !comp(chrono_it, limit)) {
			first = chrono_it;
		}
	}
	return first;
}

template <typename T, typename Compare>
side_actions::const_iterator side_actions::find_first_action_of(std::pair<T,T> between, const_iterator limit, Compare comp) const
{
	const_iterator first = actions_.end();
	for(T it = between.first; it != between.second; ++it) {
		const_iterator chrono_it = actions_.project<container::chronological>(it);
		if((comp(chrono_it, first) || first==actions_.end()) && !comp(chrono_it, limit)) {
			first = chrono_it;
		}
	}
	return first;
}

} //end namespace wb
