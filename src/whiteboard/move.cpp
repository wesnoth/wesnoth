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
#include "unit_map.hpp"

namespace wb {

std::ostream& operator<<(std::ostream &s, wb::move const& move)
{
	return move.print(s);
}

std::ostream& move::print(std::ostream &s) const
{
	s << "Move for unit " << get_unit()->name() << " [" << get_unit()->id() << "] "
			<< "from (" << get_source_hex() << ") to (" << get_dest_hex() << ")";
	return s;
}

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

move::move(const map_location& source_hex, const map_location& target_hex,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
: underlying_unit_id_(-1),
  source_hex_(source_hex),
  dest_hex_(target_hex),
  movement_cost_(0),
  arrow_(arrow),
  fake_unit_(fake_unit),
  valid_(true)
{
	if (source_hex_.valid() && dest_hex_.valid() && source_hex_ != dest_hex_ && get_unit())
	{

		// Calculate move cost
		pathfind::shortest_path_calculator path_calc(*get_unit(),
				(*resources::teams)[get_unit()->side() - 1],
				*resources::units,
				*resources::teams,
				*resources::game_map);

		pathfind::plain_route route = pathfind::a_star_search(source_hex_,
				dest_hex_, 10000, &path_calc, resources::game_map->w(), resources::game_map->h());

		// TODO: find a better treatment of movement points when defining moves out-of-turn
		if(get_unit()->movement_left() - route.move_cost < 0
				&& resources::controller->current_side() == resources::screen->viewing_side()) {
			WRN_WB << "Move defined with insufficient movement left.\n";
		}

		//TODO: if unit finishes move in a village, set the move cost to unit_.movement_left()

		movement_cost_ = route.move_cost;
	}
}

move::~move()
{
	//reminder: here we rely on the ~arrow destructor to invalidate
	//its whole path.
}

void move::accept(visitor& v)
{
	v.visit_move(shared_from_this());
}

bool move::execute()
{
	if (!valid_)
		return false;

	if (source_hex_ == dest_hex_)
		return true; //zero-hex move, probably used by attack subclass

	LOG_WB << "Executing: " << *this << "\n";

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

unit* move::get_unit() const
{
	unit_map::iterator it = resources::units->find(source_hex_);
	if (it != resources::units->end())
		return &*it;
	else
		return NULL;
}

void move::apply_temp_modifier(unit_map& unit_map)
{
	if (source_hex_ == dest_hex_)
		return; //zero-hex move, probably used by attack subclass

	// Move the unit
	unit* unit = get_unit();
	assert(unit);
	DBG_WB << "Temporarily moving unit " << unit->name() << " [" << unit->underlying_id()
			<< "] from (" << source_hex_ << ") to (" << dest_hex_ <<")\n";
	unit_map.move(source_hex_, dest_hex_);

	//Modify movement points accordingly
	DBG_WB <<"Changing movement points for unit " << unit->name() << " [" << unit->underlying_id()
			<< "] from " << unit->movement_left() <<" to "
			<< unit->movement_left() - movement_cost_ << ".\n";
	unit->set_movement(unit->movement_left() - movement_cost_);

}

void move::remove_temp_modifier(unit_map& unit_map)
{
	if (source_hex_ == dest_hex_)
		return; //zero-hex move, probably used by attack subclass

	unit_map::iterator unit_it = resources::units->find(dest_hex_);
	assert(unit_it != resources::units->end());
	unit& unit = *unit_it;

	// Restore the unit to its original position
	unit_map.move(dest_hex_, source_hex_);

	// Restore movement points
	unit.set_movement(unit.movement_left() + movement_cost_);
}

void move::draw_hex(const map_location& hex)
{
	(void) hex; //temporary to avoid unused param warning
}

bool move::is_numbering_hex(const map_location& hex) const
{
	return hex == dest_hex_;
}

void move::set_valid(bool valid)
{
	valid_ = valid;

	//TODO: restore this once we have artwork for invalid arrows,
	// and if we decide not to delete them after all.
//	if (valid_)
//	{
//		arrow_->set_style(ARROW_STYLE_VALID);
//	}
//	else
//	{
//		arrow_->set_style(ARROW_STYLE_INVALID);
//	}
}

} // end namespace wb
