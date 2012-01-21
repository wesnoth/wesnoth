/* $Id$ */
/*
 Copyright (C) 2010 - 2012 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#ifndef WB_SIDE_ACTIONS_HPP_
#define WB_SIDE_ACTIONS_HPP_

#include "typedefs.hpp"

#include "map_location.hpp"

namespace wb
{

class move;

/**
 * This internal whiteboard class holds the planned action queues for a team, and offers many
 * utility methods to create and manipulate them. It maintains an internal data structure
 * but mostly hides it by providing its own iterators, begin() and end() methods, etc.
 */
class side_actions: public boost::enable_shared_from_this<side_actions>
{
	/**
	 * Class invariant:
	 *   actions_.empty() || !actions_.back().empty();
	 */

	typedef std::deque<action_queue> contents_t;

public:
	class iterator;
	class const_iterator;
	friend class iterator;
	friend class const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	side_actions();
	virtual ~side_actions();

	///Must be called only once, right after the team that owns this side_actions is added to the teams vector
	void set_team_index(size_t team_index);

	///Returns the team index this action queue belongs to
	size_t team_index() { assert(team_index_defined_); return team_index_; }

	/// Get the underlying action container
	contents_t const& actions() const { return actions_; }

	struct numbers_t;
	/** Gets called when display is drawing a hex to determine which numbers to draw on it */
	void get_numbers(const map_location& hex, numbers_t& result);

	/**
	 * Executes the first action in the queue, and then deletes it.
	 * @return true - if the action was completed successfully
	 */
	bool execute_next();

	/**
	 * Executes the specified action, if it exists in the queue.
	 * If the action is not finished, it's moved at the end of the queue.
	 * @return true - if the action was completed successfully
	 */
	bool execute(iterator position);

	/**
	 * Returns the iterator for the first (executed earlier) action within the actions queue.
	 */
	iterator begin();
	/// reverse version of the above
	reverse_iterator rbegin();
	/// const versions of the above
	const_iterator begin() const;
	const_reverse_iterator rbegin() const;

	/**
	 * Returns the iterator for the position *after* the last executed action within the actions queue.
	 */
	iterator end();
	/// reverse version of the above
	reverse_iterator rend();
	/// const versions of the above
	const_iterator end() const;
	const_reverse_iterator rend() const;

	/**
	 * Indicates whether the action queue is empty.
	 * Since it's a queue of queues (one per turn), it can be empty of actions,
	 * while still containing empty queues.
	 * @todo devise a cleanup system to prevent the possibility of empty queues.
	 */
	bool empty() const { return actions_.empty(); }

	/**
	 * Returns the number of actions in the action queue.
	 */
	size_t size() const;

	///Returns the number of turns that have plans.
	size_t num_turns() const {return actions_.size();}

	///Returns an iterator to a specific turn queue.
	iterator turn_begin(size_t turn_num);
	iterator turn_end(size_t turn_num);
	reverse_iterator turn_rbegin(size_t turn_num);
	reverse_iterator turn_rend(size_t turn_num);

	typedef std::pair<iterator,iterator> range_t;
	typedef std::pair<reverse_iterator,reverse_iterator> rrange_t;
	///Returns an iterator range corresponding to the requested turn.
	range_t iter_turn(size_t turn_num);
	rrange_t riter_turn(size_t turn_num);

	/** @return the number of actions planned for turn turn_num */
	size_t turn_size(size_t turn_num) const {
		if (num_turns() >= turn_num)
			return actions_[turn_num].size();
		else
			return 0;
	}

	/**
	 * Empties the action queue.
	 */
	void clear() { safe_clear(); }

	/** Sets whether or not the contents should be drawn on the screen. */
	void hide();
	void show();
	bool hidden() const {return hidden_;}

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
	 * Moves an action earlier in the execution order (i.e. at the front of the queue),
	 * by one position.
	 * @return The action's new position.
	 */
	iterator bump_earlier(iterator position);

	/**
	 * Moves an action later in the execution order (i.e. at the back of the queue),
	 * by one position.
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
	iterator get_position_of(action_ptr action);

	/**
	 * Finds the first action that belongs to this unit, starting the search at the specified position.
	 * @return The position, or end() if not found.
	 */
	iterator find_first_action_of(unit const* unit, iterator start_position);
	///Variant of this method that always start searching at the beginning of the queue
	iterator find_first_action_of(unit const* unit);

	/**
	 * Finds the last action that belongs to this unit, starting the search backwards from the specified position.
	 * @return The position, or end() if not found.
	 */
	iterator find_last_action_of(unit const* unit, iterator start_position);
	///Variant of the previous method that always start searching at the end of the queue
	iterator find_last_action_of(unit const* unit);

	bool unit_has_actions(unit const* unit);
	size_t count_actions_of(unit const* unit);

	///Removes all invalid actions "attached" to the unit
	void remove_invalid_of(unit const*);

	///Determines the appropriate turn number for the next action planned for this unit
	size_t get_turn_num_of(unit const&) const;

	///Validates all planned actions in the queue
	void validate_actions();

	///Used to track gold spending by recruits/recalls when building the future unit map
	int get_gold_spent() const { return gold_spent_; }
	///Used to track gold spending by recruits/recalls when building the future unit map
	void change_gold_spent_by(int difference);
	///Set gold spent back to zero
	void reset_gold_spent();

	void raw_turn_shift();
	void synced_turn_shift();

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
	net_cmd make_net_cmd_replace(const_iterator const& pos, action_const_ptr) const; //< an optimized remove+insert
	net_cmd make_net_cmd_remove(const_iterator const& pos) const;
	net_cmd make_net_cmd_bump_later(const_iterator const& pos) const;
	net_cmd make_net_cmd_clear() const;
	net_cmd make_net_cmd_refresh() const;

private:
	bool validate_iterator(iterator position);
	void update_size();
	iterator raw_erase(iterator itor);
	iterator raw_insert(iterator itor, action_ptr to_insert);
	iterator raw_enqueue(size_t turn_num, action_ptr to_insert);
	iterator safe_insert(size_t turn_num, size_t pos, action_ptr to_insert);
	iterator synced_erase(iterator itor);
	iterator synced_insert(iterator itor, action_ptr to_insert);
	iterator synced_enqueue(size_t turn_num, action_ptr to_insert);
	iterator safe_erase(iterator const& itor);
	void safe_clear() { contents_t temp = actions_; return actions_.clear(); }

	static iterator null;

	contents_t actions_;
	size_t team_index_;
	bool team_index_defined_;

	/// Used to store gold "spent" in planned recruits/recalls when the future unit map is applied
	int gold_spent_;

	bool hidden_;
};

/** Dumps side_actions on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, wb::side_actions const& side_actions);

class side_actions::iterator
	: public std::iterator<std::bidirectional_iterator_tag, action_ptr>
{
	friend class side_actions;
	typedef action_queue::iterator base_t;
	typedef iterator this_t;

public:
	iterator()
			: base_()
			, turn_num_()
			, contents_()
		{}
	explicit iterator(side_actions::reverse_iterator const& that)
			: base_(that.base().base_)
			, turn_num_(that.base().turn_num_)
			, contents_(that.base().contents_)
		{}

	/**
	 * Copy constructor.
	 *
	 * If the contents_ is NULL the iterator is singular. Copying singular
	 * iterators is UB, so avoid it.
	 */
	iterator(const iterator& that)
			: base_(that.contents_ ? that.base_ : base_t())
			, turn_num_(that.contents_ ? that.turn_num_ : 0)
			, contents_(that.contents_)
	{
	}

	action_ptr& operator*() const {return *base_;}
	action_ptr* operator->() const {return base_.operator->();}
	this_t& operator++()
	{
		++base_;
		init();
		return *this;
	}
	this_t& operator--()
	{
		assert(contents_);
		while(base_ == (*contents_)[turn_num_].begin())
			base_ = (*contents_)[--turn_num_].end();
		--base_;
		return *this;
	}
	this_t operator+(int x) const
	{
		this_t result = *this;
		if(x >= 0)
			for(int i=0; i<x; ++i)
				++result;
		else
			for(int i=0; i>x; --i)
				--result;
		return result;
	}
	this_t operator-(int x) const {return operator+(-x);}
	size_t operator-(this_t x) const
	{
		int result = 0;
		for(; x!=*this; ++x)
			++result;
		return result;
	}
	bool operator<(this_t const& that) const
	{
		assert(contents_ == that.contents_);
		if(contents_==NULL)
			return false;
		return turn_num_<that.turn_num_ || (turn_num_==that.turn_num_ && base_<that.base_);
	}
	bool operator==(this_t const& that) const
	{
		assert(contents_ == that.contents_);
		if(contents_==NULL)
			return true;
		return turn_num_==that.turn_num_ && base_==that.base_;
	}
	bool operator!=(this_t const& that) const {return !(*this==that);}

private:
	iterator(base_t const& base, size_t turn_num, side_actions& sa)
			: base_(base), turn_num_(turn_num), contents_(&sa.actions_)
		{init();}

	iterator(base_t const& base, size_t turn_num, side_actions::contents_t* contents)
			// If contents == NULL base is a singualar iterator
			: base_(contents ? base : base_t())
			, turn_num_(turn_num)
			, contents_(contents)
		{}

	void init()
	{
		assert(contents_);
		while(base_ == (*contents_)[turn_num_].end() //terminates thanks to invariant
				&& turn_num_+1 < contents_->size())
			base_ = (*contents_)[++turn_num_].begin();
	}

	base_t base_;
	size_t turn_num_;
	contents_t* contents_;
};
class side_actions::const_iterator
	: public std::iterator<std::bidirectional_iterator_tag, action_ptr const>
{
	friend class side_actions;
	typedef side_actions::iterator base_t;
	typedef side_actions::const_iterator this_t;

public:
	const_iterator()
			: std::iterator<std::bidirectional_iterator_tag, action_ptr const>()
			, base_()
		{}
	//conversion from non-const to const
	/*implicit*/ const_iterator(base_t const& base)
			: std::iterator<std::bidirectional_iterator_tag, action_ptr const>()
			, base_(base)
		{}

	action_ptr const& operator*() const {return *base_;}
	action_ptr const* operator->() const {return base_.operator->();}
	this_t& operator++() {++base_;   return *this;}
	this_t& operator--() {--base_;   return *this;}
	this_t operator+(int x) const {return base_ + x;}
	this_t operator-(int x) const {return base_ - x;}
	size_t operator-(this_t const& that) const {return base_-that.base_;}
	bool operator==(this_t const& that) const {return base_ == that.base_;}
	bool operator!=(this_t const& that) const {return base_ != that.base_;}
	bool operator<(this_t const& that) const {return base_ < that.base_;}

private:
	base_t base_;
};

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

} //end namespace wb

#endif /* WB_SIDE_ACTIONS_HPP_ */
