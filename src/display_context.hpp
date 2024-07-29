/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "units/orb_status.hpp"
#include "units/ptr.hpp"
#include <string>
#include <vector>

class team;
class gamemap;
class unit_map;

class unit;
struct map_location;

/**
 * Abstract class for exposing game data that doesn't depend on the GUI, however which for historical
 * reasons is generally accessed via the GUI method display::get_singleton().
 */
class display_context
{
public:
	virtual const std::vector<team> & teams() const = 0;
	virtual const gamemap & map() const = 0;
	virtual const unit_map & units() const = 0;
	virtual const std::vector<std::string> & hidden_label_categories() const = 0;
	virtual std::vector<std::string> & hidden_label_categories() = 0;

	/** This getter takes a 1-based side number, not a 0-based team number. */
	const team& get_team(int side) const;

	// this one is only a template function to prevent compilation erros when class team is an incomplete type.
	template<typename T = void>
	bool has_team(int side) const
	{
		return side > 0 && side <= static_cast<int>(teams().size());
	}

	// Helper for is_visible_to_team

	/**
	 * Given a location and a side number, indicates whether an invisible unit of that side at that
	 * location would be revealed (perhaps ambushed), based on what team side_num can see.
	 * If see_all is true then the calculation ignores fog, and enemy ambushers.
	 */
	bool would_be_discovered(const map_location & loc, int side_num, bool see_all = true);

	// Needed for reports

	const unit * get_visible_unit(const map_location &loc, const team &current_team, bool see_all = false) const;
	unit_const_ptr get_visible_unit_shared_ptr(const map_location &loc, const team &current_team, bool see_all = false) const;

	struct can_move_result
	{
		/**
		 * The unit can move to another hex, taking account of enemies' locations, ZoC and
		 * terrain costs vs current movement points.
		 */
		bool move = false;

		/**
		 * The unit can make an attack from the hex that it's currently on, this
		 * requires attack points and a non-petrified enemy in an adjacent hex.
		 */
		bool attack_here = false;

		operator bool() const
		{
			return move || attack_here;
		}
	};

	/**
	 * Work out what @a u can do - this does not check which player's turn is currently active, the
	 * result is calculated assuming that the unit's owner is currently active.
	 */
	can_move_result unit_can_move(const unit& u) const;

	/**
	 * Returns an enumurated summary of whether this unit can move and/or attack.
	 *
	 * This does not check which player's turn is currently active, the result is calculated
	 * assuming that the unit's owner is currently active. For this reason this never returns
	 * orb_status::enemy nor orb_status::allied.
	 */
	orb_status unit_orb_status(const unit& u) const;

	/**
	 * Given the location of a village, will return the 1-based number
	 * of the team that currently owns it, and 0 if it is unowned.
	 */
	int village_owner(const map_location & loc) const;

	// Accessors from unit.cpp

	/** Returns the number of units of the side @a side_num. */
	int side_units(int side_num) const;

	/** Returns the total cost of units of side @a side_num. */
	int side_units_cost(int side_num) const ;

	int side_upkeep(int side_num) const ;

	// Accessor from team.cpp

	/** Check if we are an observer in this game */
	bool is_observer() const;

	// Dtor
	virtual ~display_context() {}
};

struct team_data
{
	team_data(const display_context& dc, const team& tm);

	int side = 0, units = 0, upkeep = 0, expenses = 0, net_income = 0;
};
