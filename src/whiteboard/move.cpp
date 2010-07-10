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
 * @file move.cpp
 */

#include "move.hpp"

#include "visitor.hpp"

#include "actions.hpp"
#include "arrow.hpp"
#include "config.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "play_controller.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_display.hpp"

namespace wb {

const double move::ALPHA_HIGHLIGHT = 1.0;
const double move::ALPHA_NORMAL = 0.6;
const std::string move::ARROW_STYLE_VALID = "";
const std::string move::ARROW_STYLE_INVALID = "invalid";

//FIXME: move this out of here if it ends up being used only once
static team& get_current_team()
{
	int current_side = resources::controller->current_side();
	team& current_team = (*resources::teams)[current_side - 1];
	return current_team;
}

move::move(unit& subject, const map_location& source_hex, const map_location& target_hex,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
: unit_(subject),
  source_hex_(source_hex),
  dest_hex_(target_hex),
  movement_cost_(0),
  arrow_(arrow),
  fake_unit_(fake_unit),
  valid_(true)
{
	// Calculate move cost
	pathfind::shortest_path_calculator path_calc(unit_, get_current_team(), *resources::units,
			*resources::teams, *resources::game_map);

	pathfind::plain_route route = pathfind::a_star_search(source_hex_,
			dest_hex_, 10000, &path_calc, resources::game_map->w(), resources::game_map->h());

	assert(unit_.movement_left() - route.move_cost >= 0);

	//TODO: if unit finishes move in a village, set the move cost to unit_.movement_left()

	movement_cost_ = route.move_cost;
}

move::~move()
{
}

void move::accept(visitor& v)
{
	v.visit_move(shared_from_this());
}

bool move::execute()
{
	bool move_finished_completely = false;

	arrow_->set_alpha(ALPHA_HIGHLIGHT);

	const arrow_path_t& arrow_path = arrow_->get_path();
	static const bool show_move = true;
	map_location final_location;
	int steps_done = ::move_unit(NULL, arrow_path, &recorder, resources::undo_stack, show_move, &final_location,
			get_current_team().auto_shroud_updates());
	// final_location now contains the final unit location
	// if that isn't needed, pass NULL rather than &final_location

	if (arrow_path.back() == final_location)
	{
		move_finished_completely = true;
	}
	else if (steps_done == 0)
	{
		DBG_WB << "Move execution resulted in zero movement.\n";
	}
	else if (final_location.valid())
	{
		LOG_WB << "Move finished at (" << final_location << ") instead of at (" << dest_hex_ << "), analysing\n";
		arrow_path_t::const_iterator start_new_path;
		bool found = false;
		for (start_new_path = arrow_path.begin(); ((start_new_path != arrow_path.end()) && !found); ++start_new_path)
		{
			if (*start_new_path == final_location)
			{
				found = true;
			}
		}
		if (found)
		{
			source_hex_ = final_location;
			--start_new_path; //since the for loop incremented the iterator once after we found the right one.
			arrow_path_t new_path(start_new_path, arrow_path.end());
			LOG_WB << "Setting new path for this move from (" << new_path.front()
					<< ") to (" << new_path.back() << ").\n";
			arrow_->set_path(new_path);
		}
		else //Unit ended up in location outside path, , likely due to a WML event
		{
			//TODO: handle unit ending up in unexpected location
			WRN_WB << "Unit ended up in location outside path during move execution; Case unhandled as yet.\n";
		}
	}
	else //Unit disappeared from the map, likely due to a WML event
	{
		//TODO: handle unit disappearing from map
		WRN_WB << "Unit disappeared from map during move execution; Case unhandled as yet.\n";
	}

	arrow_->set_alpha(ALPHA_NORMAL);
	return move_finished_completely;
}

void move::apply_temp_modifier(unit_map& unit_map)
{
	// Move the unit
	assert(unit_.get_location() == source_hex_);
	DBG_WB << "Temporarily moving unit " << unit_.name() << " [" << unit_.underlying_id() << "] "
			<< " from (" << source_hex_ << ") to (" << dest_hex_ <<")\n";
	unit_map.move(source_hex_, dest_hex_);
	assert(unit_.get_location() == dest_hex_);

	//Modify movement points accordingly
	DBG_WB <<"Changing movement points for unit " << unit_.name() << " [" << unit_.underlying_id()
			<< "] from " << unit_.movement_left() <<" to "
			<< unit_.movement_left() - movement_cost_ << ".\n";
	unit_.set_movement(unit_.movement_left() - movement_cost_);

}

void move::remove_temp_modifier(unit_map& unit_map)
{
	// Restore the unit to its original position
	assert(unit_.get_location() == dest_hex_);
	unit_map.move(dest_hex_, source_hex_);
	assert(unit_.get_location() == source_hex_);

	// Restore movement points
	unit_.set_movement(unit_.movement_left() + movement_cost_);
}

void move::draw_hex(const map_location& hex)
{
	(void) hex; //temporary to avoid unused param warning
}

bool move::is_target_hex(const map_location& hex) const
{
	bool is_related = hex == dest_hex_;
	return is_related;
}

bool move::is_related_to(const unit& unit) const
{
	bool is_related = &unit_ == &unit;
	return is_related;
}

void move::set_valid(bool valid)
{
	valid_ = valid;
	if (valid_)
	{
		arrow_->set_style(ARROW_STYLE_VALID);
	}
	else
	{
		arrow_->set_style(ARROW_STYLE_INVALID);
	}
}

} // end namespace wb
