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
	void set_active(bool active);

	void set_invert_behavior(bool invert);
	bool can_execute_hotkey() const;

	void on_init_side();
	void on_finish_side_turn();

	void validate_viewer_actions();

	/**
	 * Temporarily apply the effects of the current team's
	 * planned moves to the unit map.
	 */
	void set_planned_unit_map();
	void set_real_unit_map();
	bool has_planned_unit_map() const { return planned_unit_map_active_; }
	///Applies the future unit map and returns a pointer to the unit at hex, NULL if none
	unit* find_future_unit(map_location hex);

	/**
	 * Callback from the display when drawing hexes, to allow the whiteboard to
	 * add visual elements. Some visual elements such as arrows and fake units
	 * are not handled through this function, but separately registered with the display.
	 */
	void draw_hex(const map_location& hex);

	void on_mouseover_change(const map_location& hex);
	/** Choose the target unit for action creation */
	void on_select_hex(const map_location& hex);
	void on_deselect_hex();

	/** Creates a temporary visual arrow, that follows the cursor, for move creation purposes */
	void create_temp_move();
	/** Informs whether an arrow is being displayed for move creation purposes */
	bool has_temp_move() const { return route_ && fake_unit_ && move_arrow_; }
	/** Informs whether the whiteboard tracks a selected unit */
	bool has_selected_unit() const { return selected_unit_; }

	void erase_temp_move();

	/**
	 * Creates a move action for the current side,
	 * and erases the temp move. The move action is inserted
	 * at the end of the queue, to be executed last.
     */
	void save_temp_move();

	void save_temp_attack(const map_location& attack_from, const map_location& target_hex);

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
	///Tracks whether the whiteboard is active.
	bool active_;
	bool inverted_behavior_;
	bool wait_for_side_init_;

	boost::scoped_ptr<mapbuilder_visitor> mapbuilder_;
	boost::scoped_ptr<highlight_visitor> highlighter_;

	boost::scoped_ptr<pathfind::marked_route> route_;
	std::vector<map_location> steps_;

	arrow_ptr move_arrow_;
	fake_unit_ptr fake_unit_;

	unit* selected_unit_;

	bool planned_unit_map_active_;
	/** Track whenever we're modifying actions, to avoid dual execution etc. */
	bool executing_actions_;
};

/** Applies the planned unit map for the duration of the struct's life.
 *  Reverts to real unit map on exit, no matter what was the status when the struct was created. */
struct scoped_planned_unit_map
{
	scoped_planned_unit_map();
	~scoped_planned_unit_map();
	bool has_planned_unit_map_;
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
