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
 * @file manager.hpp
 */

#ifndef WB_MANAGER_HPP_
#define WB_MANAGER_HPP_

#include "map_location.hpp"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

class arrow;
class unit;

namespace wb {

class action;
class mapbuilder_visitor;

/**
 * This class holds and manages all of the whiteboard's planned actions.
 */
class manager : private boost::noncopyable
{
public:

	manager();
	~manager();

	/**
	 * Determine whether the whiteboard is activated.
	 */
	bool active() const { return active_; }
	void set_active(bool active){ active_ = active; }

	/**
	 * Temporarily apply the effects of the current team's
	 * planned moves to the unit map.
	 */
	void apply_temp_modifiers();
	void remove_temp_modifiers();
	void toggle_temp_modifiers();
	bool temp_modifiers_applied() { return temp_modifiers_applied_; }

	/**
	 * Highlights the action for this unit,
	 * for instance highlights the arrow if it's a move.
	 */
	void highlight_action(const unit& unit);

	void highlight_hex(const map_location& hex);

	void remove_highlight();

	/** Choose the target unit for action creation */
	void select_unit(unit& unit);
	void deselect_unit();

	/** Creates a temporary visual arrow, that follows the cursor, for move creation purposes */
	void create_temp_move(const std::vector<map_location> &steps);
	/** Informs whether an arrow is being displayed for move creation purposes */
	bool during_move_creation() const;

	void erase_temp_move();

	/**
	 * Creates a move action for the current side,
	 * and erases the temp move. The move action is inserted
	 * at the end of the queue, to be executed last.
     */
	void save_temp_move();

	/** Executes first action in the queue for current side */
	void execute_next();

	/** Deletes last action in the queue for current side */
	void delete_last();

	/** Checks whether the specified unit has at least one planned action,
	 *  and returns the first action found. */
	boost::shared_ptr<action> has_action(const unit& unit) const;

private:
	/**
	 * Tracks whether the whiteboard is active.
	 */
	bool active_;

	boost::scoped_ptr<mapbuilder_visitor> mapbuilder_;

	std::vector<map_location> route_;

	boost::shared_ptr<arrow> move_arrow_;
	boost::shared_ptr<unit> fake_unit_;

	unit* selected_unit_;
	map_location highlighted_hex_;

	bool temp_modifiers_applied_;
};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
