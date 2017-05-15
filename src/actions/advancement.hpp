/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
 * Various functions that implement advancements of units.
 */

#pragma once

struct map_location;
class  team;
class  unit;
class  config;
#include "units/types.hpp"
#include "ai/lua/aspect_advancements.hpp"

#include <string>
#include <vector>

/**
	advances the unit at loc if it has enough experience, maximum 20 times.
	if the unit is on the currently active side, and that side is controlled by a human, a dialog pops up.
	if we are in a non mp game, and the side is controlled by a human then a dialog is shown too.
	if the side is controlled by an ai, and if ai_advancement is passed, then ai_advancement will be used.
	otherwise a random decision will be taken.

	this method is currently not used by unstore_unit, if we want to do that we'd need to allow more arguments (animate, fire_events).
*/
struct advance_unit_params
{
	advance_unit_params(const map_location& loc) : loc_(loc), ai_advancements_(nullptr), force_dialog_(false), fire_events_(true), animate_(true) {}
	advance_unit_params& ai_advancements(const ai::unit_advancements_aspect& value) {ai_advancements_ = &value; return *this;}
	advance_unit_params& force_dialog(bool value) {force_dialog_ = value; return *this;}
	advance_unit_params& fire_events(bool value) {fire_events_ = value; return *this;}
	advance_unit_params& animate(bool value) {animate_ = value; return *this;}
	friend void advance_unit_at(const advance_unit_params&);
private:
	map_location loc_;
	const ai::unit_advancements_aspect* ai_advancements_;
	bool force_dialog_;
	bool fire_events_;
	bool animate_;
};
void advance_unit_at(const advance_unit_params& params);
/**
 * Returns the advanced version of a unit (with traits and items retained).
 */
unit_ptr get_advanced_unit(const unit &u, const std::string &advance_to);

/**
 * Returns the AMLA-advanced version of a unit (with traits and items retained).
 */
unit_ptr get_amla_unit(const unit &u, const config &mod_option);

using advancement_option = boost::variant<std::string /*change type*/, const config* /*apply amla*/>;

/**
 * Function which will advance the unit at @a loc to 'advance_to'.
 * which is eigher a type to advance to or a config containing the
 * [advancement] to perform an amla.
 * Note that 'loc' is not a reference, because if it were a reference,
 * we couldn't safely pass in a reference to the item in the map
 * that we're going to delete, since deletion would invalidate the reference.
 */
void advance_unit(map_location loc, const advancement_option &advance_to, bool fire_event = true);
