/*
 Copyright (C) 2011 - 2017 by Tommy Schmitz
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

#include "action.hpp"

namespace wb {

/**
 * A planned action that temporarily removes a unit from the map
 * for planning purposes
 */
class suppose_dead: public action
{
public:
	suppose_dead(size_t team_index, bool hidden, unit& curr_unit, map_location const& loc);
	suppose_dead(config const&, bool hidden); // For deserialization
	virtual ~suppose_dead();

	/** Return the unit targeted by this action. Null if unit doesn't exist. */
	virtual unit_ptr get_unit() const;
	/** @return null pointer */
	virtual fake_unit_ptr get_fake_unit() { return fake_unit_ptr(); }
	/** Return the location at which this action was planned. */
	virtual map_location get_source_hex() const { return loc_; }

	virtual std::ostream& print(std::ostream& s) const;

	virtual void accept(visitor &v);

	virtual void execute(bool& success, bool& complete);

	/**
	 * Check the validity of the action.
	 *
	 * @return the error preventing the action from being executed.
	 * @retval OK if there isn't any error (the action can be executed.)
	 */
	virtual error check_validity() const;

	/** Applies temporarily the result of this action to the specified unit map. */
	virtual void apply_temp_modifier(unit_map& unit_map);
	/** Removes the result of this action from the specified unit map. */
	virtual void remove_temp_modifier(unit_map& unit_map);

	/** Gets called by display when drawing a hex, to allow actions to draw to the screen. */
	virtual void draw_hex(const map_location& hex);
	/** Redrawing function, called each time the action situation might have changed. */
	virtual void redraw();

	virtual map_location get_numbering_hex() const { return loc_; }

	virtual config to_config() const;

protected:

	std::shared_ptr<suppose_dead> shared_from_this() {
		return std::static_pointer_cast<suppose_dead>(action::shared_from_this());
	}

	size_t unit_underlying_id_;
	std::string unit_id_;
	map_location loc_;
};

/** Dumps a suppose_dead on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, suppose_dead_ptr sup_d);
std::ostream &operator<<(std::ostream &s, suppose_dead_const_ptr sup_d);
} // end namespace wb
