/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * @file
 * Various functions related to the creation of units (recruits, recalls,
 * and placed units).
 */

#pragma once

class config;
class team;
class vconfig;
class game_board;
class unit;

#include "map/location.hpp"

class unit_creator
{
public:
	unit_creator(team &tm, const map_location &start_pos, game_board* board = nullptr);
	unit_creator& allow_show(bool b);
	unit_creator& allow_get_village(bool b);
	unit_creator& allow_rename_side(bool b);
	unit_creator& allow_invalidate(bool b);
	unit_creator& allow_discover(bool b);
	unit_creator& allow_add_to_recall(bool b);

	/**
	 * finds a suitable location for unit
	 * @retval map_location::null_location() if unit is to be put into recall list
	 * @retval valid on-board map location otherwise
	 */
	map_location find_location(const config &cfg, const unit* pass_check=nullptr);


	/**
	 * adds a unit on map without firing any events (so, usable during team construction in gamestatus)
	 */
	void add_unit(const config &cfg, const vconfig* vcfg = nullptr);

private:
	void post_create(const map_location &loc, const unit &new_unit, bool anim, bool fire_event);

	bool add_to_recall_;
	bool discover_;
	bool get_village_;
	bool invalidate_;
	bool rename_side_;
	bool show_;
	const map_location start_pos_;
	team &team_;
	game_board* board_;

};
