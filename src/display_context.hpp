/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *
 * This class is an abstract base class designed to simplify the use
 * of the display object.
 *
 **/

#pragma once

#include <string>
#include <vector>

class team;
class gamemap;
class unit_map;

class unit;
struct map_location;

struct team_data
{
	team_data() :
		units(0),
		upkeep(0),
		villages(0),
		expenses(0),
		net_income(0),
		gold(0),
		teamname()
	{
	}

	int units, upkeep, villages, expenses, net_income, gold;
	std::string teamname;
};

class display_context {
public:
	virtual const std::vector<team> & teams() const = 0;
	virtual const gamemap & map() const = 0;
	virtual const unit_map & units() const = 0;
	virtual const std::vector<std::string> & hidden_label_categories() const = 0;
	std::vector<std::string> & hidden_label_categories_ref();
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

	// From actions:: namespace

	bool unit_can_move(const unit & u) const;

	// From class team

	/**
	 * Given the location of a village, will return the 0-based index
	 * of the team that currently owns it, and -1 if it is unowned.
	 */
	int village_owner(const map_location & loc) const;

	// Accessors from unit.cpp

	/** Returns the number of units of the side @a side_num. */
	int side_units(int side_num) const;

	/** Returns the total cost of units of side @a side_num. */
	int side_units_cost(int side_num) const ;

	int side_upkeep(int side_num) const ;

	team_data calculate_team_data(const class team& tm) const;

	// Accessor from team.cpp

	/// Check if we are an observer in this game
	bool is_observer() const;

	// Dtor

	virtual ~display_context() {}
};
