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
 * @file action.hpp
 */

#ifndef WB_ACTION_HPP_
#define WB_ACTION_HPP_

#include "typedefs.hpp"

namespace wb {

class visitor;

/**
 * Superclass for all the whiteboard planned actions.
 */
class action
{
	friend class validate_visitor;
	friend class highlight_visitor;

public:
	action();
	virtual ~action();

	virtual void accept(visitor& v) = 0;

	/** Returns true if the action has been completely executed and can be deleted */
	virtual bool execute() = 0;

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map) = 0;
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map) = 0;

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(const map_location& hex) = 0;

	/**
	 * Indicates whether this hex is the preferred hex to draw the numbering for this action.
	 */
	virtual bool is_numbering_hex(const map_location& hex) const = 0;

	/** Return the unit targeted by this action. */
	virtual unit& get_unit() = 0;
	virtual unit const& get_unit() const = 0;

	/**
	 * Indicates to an action whether its status is invalid, and whether it should change its
	 * display (and avoid any change to the game state) accordingly
	 */
	virtual void set_valid(bool valid) = 0;
	virtual bool is_valid() = 0;
};

} // end namespace wb

#endif /* WB_ACTION_HPP_ */
