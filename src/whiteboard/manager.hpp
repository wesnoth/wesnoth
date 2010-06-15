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

#include <memory>
#include <vector>

class arrow;
class unit;

namespace wb {

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

	void select_unit(unit& unit);
	void deselect_unit();

	/** Set the route for move creation purposes */
	void create_temp_move(const std::vector<map_location> &steps);

	void erase_temp_move();

	/**
	 * Creates a move action for the current side,
	 * and erases the stored route. The move is inserted
	 * at the end of the queue, to be executed last.
     */
	void save_temp_move();

private:
	/**
	 * Tracks whether the whiteboard is active.
	 */
	bool active_;

	std::auto_ptr<mapbuilder_visitor> mapbuilder_;

	std::vector<map_location> route_;

	boost::shared_ptr<arrow> move_arrow_;
	boost::shared_ptr<unit> fake_unit_;

	unit* selected_unit_;
};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
