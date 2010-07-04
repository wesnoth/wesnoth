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

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

class arrow;
class unit;

namespace pathfind {
	struct marked_route;
}

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
	bool is_active() const { return active_; }
	void set_active(bool active){ active_ = active; }

	/**
	 * Temporarily apply the effects of the current team's
	 * planned moves to the unit map.
	 */
	void set_planned_unit_map();
	void set_real_unit_map();
	bool has_planned_unit_map() const { return planned_unit_map_active_; }

	/**
	 * Highlights the action for this unit,
	 * for instance highlights the arrow if it's a move.
	 */
	void highlight_action(const unit& unit);

	void on_mouseover_change(const map_location& hex);
	void highlight_hex(const map_location& hex);
	void remove_highlight();

	/** Choose the target unit for action creation */
	void on_unit_select(unit& unit);
	void on_unit_deselect();

	/** Creates a temporary visual arrow, that follows the cursor, for move creation purposes */
	void create_temp_move(const pathfind::marked_route &route);
	/** Informs whether an arrow is being displayed for move creation purposes */
	bool has_temp_move() const { return route_; }

	void erase_temp_move();

	/**
	 * Creates a move action for the current side,
	 * and erases the temp move. The move action is inserted
	 * at the end of the queue, to be executed last.
     */
	void save_temp_move();

	/** Executes first action in the queue for current side */
	void contextual_execute();

	/** Deletes last action in the queue for current side */
	void contextual_delete();

	/** Checks whether the specified unit has at least one planned action */
	bool unit_has_actions(const unit& unit) const;

	struct fake_unit_deleter {
	    void operator() (unit*& ptr);
	};

private:
	/**
	 * Tracks whether the whiteboard is active.
	 */
	bool active_;

	boost::scoped_ptr<mapbuilder_visitor> mapbuilder_;

	boost::scoped_ptr<pathfind::marked_route> route_;
	std::vector<map_location> steps_;

	typedef boost::shared_ptr<arrow> arrow_ptr;
	arrow_ptr move_arrow_;
	typedef boost::shared_ptr<unit> fake_unit_ptr;
	fake_unit_ptr fake_unit_;

	unit* selected_unit_;
	unit* highlighted_unit_;

	typedef boost::interprocess::interprocess_mutex wb_mutex;
	typedef boost::interprocess::scoped_lock<wb_mutex> wb_scoped_lock;
	wb_mutex move_saving_mutex_;
	//TODO: this mutex might find a better home within the side_actions class.
	wb_mutex actions_modification_mutex_;

	bool planned_unit_map_active_;
};

/** Applies the planned unit map for the duration of the struct's life.
 *  Reverts to real unit map on exit, no matter what was the status when the struct was created. */
struct scoped_planned_unit_map
{
	scoped_planned_unit_map();
	~scoped_planned_unit_map();
};

/** Ensures that the real unit map is active for the duration of the struct's life,
 * and reverts to planned unit map if it was active when the struct was created. */
struct scoped_real_unit_map
{
	scoped_real_unit_map();
	~scoped_real_unit_map();
	bool has_planned_unit_map_;
};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
