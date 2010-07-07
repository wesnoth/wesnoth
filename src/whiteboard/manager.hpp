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

#include "typedefs.hpp"

#include "map_location.hpp"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

namespace pathfind {
	struct marked_route;
}

namespace wb {

class mapbuilder_visitor;
class highlight_visitor;

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
	 * Callback from the display when drawing hexes, to allow the whiteboard to
	 * add visual elements. Some visual elements such as arrows and fake units
	 * are not handled through this function, but separately registered with the display.
	 */
	void draw_hex(const map_location& hex);

	void on_mouseover_change(const map_location& hex);
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
	/** Moves the action determined by the UI toward the beginning of the queue  */
	void contextual_bump_up_action();
	/** Moves the action determined by the UI toward the beginning of the queue  */
	void contextual_bump_down_action();

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
	boost::scoped_ptr<highlight_visitor> highlighter_;

	boost::scoped_ptr<pathfind::marked_route> route_;
	std::vector<map_location> steps_;

	arrow_ptr move_arrow_;
	fake_unit_ptr fake_unit_;

	unit* selected_unit_;

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
