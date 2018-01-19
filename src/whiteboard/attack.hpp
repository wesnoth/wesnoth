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

#include "move.hpp"

namespace wb
{

class attack: public move
{
public:
	attack(size_t team_index, bool hidden, unit& mover, const map_location& target_hex, int weapon_choice, const pathfind::marked_route& route,
			arrow_ptr arrow, fake_unit_ptr fake_unit);
	attack(config const&, bool hidden); // For deserialization
	virtual ~attack();

	virtual std::ostream& print(std::ostream& s) const;

	virtual void accept(visitor& v);

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

	map_location const& get_target_hex() const {return target_hex_; }

	virtual config to_config() const;

protected:

	std::shared_ptr<attack> shared_from_this() {
		return std::static_pointer_cast<attack>(move::shared_from_this());
	}

private:

	void init();

	virtual void do_hide() {invalidate();}
	virtual void do_show() {invalidate();}

	///invalidates the move-destination and attack-target hexes
	void invalidate();

	///the target of the attack
	map_location target_hex_;

	int weapon_choice_;
	int attack_movement_cost_;
	int temp_movement_subtracted_;
};

/** Dumps an attack on a stream, for debug purposes. */
std::ostream& operator<<(std::ostream &s, attack_ptr attack);
std::ostream& operator<<(std::ostream &s, attack_const_ptr attack);

} // end namespace wb
