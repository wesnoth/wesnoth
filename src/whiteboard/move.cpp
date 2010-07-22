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
#include "manager.hpp"

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

std::ostream& operator<<(std::ostream &s, move_ptr move)
{
	assert(move);
	return move->print(s);
}

std::ostream& operator<<(std::ostream &s, move_const_ptr move)
{
	assert(move);
	return move->print(s);
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

move::move(const pathfind::marked_route& route,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
: unit_(NULL),
  unit_id_(),
  route_(new pathfind::marked_route(route)),
  movement_cost_(0),
  arrow_(arrow),
  fake_unit_(fake_unit),
  valid_(true)
{
	assert(!route_->steps.empty());

	unit_ = resources::whiteboard->find_unit_future(get_source_hex());
	assert(unit_);
	unit_id_ = unit_->id();

	this->calculate_move_cost();
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

	if (get_source_hex() == get_dest_hex())
		return true; //zero-hex move, probably used by attack subclass

	LOG_WB << "Executing: " << shared_from_this() << "\n";

	bool move_finished_completely = false;

	arrow_->set_alpha(ALPHA_HIGHLIGHT);

	const arrow_path_t& arrow_path = arrow_->get_path();
	static const bool show_move = true;
	map_location final_location;
	int steps_done = ::move_unit(NULL, arrow_path, &recorder, resources::undo_stack, show_move, &final_location,
			false, get_current_team().auto_shroud_updates());
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
		LOG_WB << "Move finished at (" << final_location << ") instead of at (" << get_dest_hex() << "), analysing\n";
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
			get_source_hex() = final_location;
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

map_location move::get_source_hex() const
{
	assert(route_ && !route_->steps.empty());
	return route_->steps.front();
}

map_location move::get_dest_hex() const
{
	assert(route_ && !route_->steps.empty());
	return route_->steps.back();
}

void move::set_route(const pathfind::marked_route& route)
{
	route_.reset(new pathfind::marked_route(route));
	this->calculate_move_cost();
	arrow_->set_path(route_->steps);
}

bool move::calculate_new_route(const map_location& source_hex, const map_location& dest_hex)
{
	pathfind::plain_route new_plain_route;
	pathfind::shortest_path_calculator path_calc(*get_unit(), get_current_team(), *resources::units,
						*resources::teams, *resources::game_map);
	new_plain_route = pathfind::a_star_search(source_hex,
						dest_hex, 10000, &path_calc, resources::game_map->w(), resources::game_map->h());
	if (new_plain_route.move_cost >= path_calc.getNoPathValue()) return false;
	route_.reset(new pathfind::marked_route(pathfind::mark_route(new_plain_route, std::vector<map_location>())));
	calculate_move_cost();
	return true;
}

void move::apply_temp_modifier(unit_map& unit_map)
{
	if (get_source_hex() == get_dest_hex())
		return; //zero-hex move, probably used by attack subclass

	//TODO: deal with multi-turn moves, which may for instance end their first turn
	// by capturing a village

	//TODO: we may need to change unit status here and change it back in remove_temp_modifier

	unit_map::iterator unit_it = resources::units->find(get_source_hex());
	assert(unit_it != resources::units->end());

	unit& unit = *unit_it;
	//Modify movement points
	DBG_WB <<"Changing movement points for unit " << unit.name() << " [" << unit.underlying_id()
			<< "] from " << unit.movement_left() << " to "
			<< unit.movement_left() - movement_cost_ << ".\n";
	unit.set_movement(unit.movement_left() - movement_cost_);

	// Move the unit
	DBG_WB << "Temporarily moving unit " << unit.name() << " [" << unit.underlying_id()
			<< "] from (" << get_source_hex() << ") to (" << get_dest_hex() <<")\n";
	unit_map.move(get_source_hex(), get_dest_hex());

}

void move::remove_temp_modifier(unit_map& unit_map)
{
	if (get_source_hex() == get_dest_hex())
		return; //zero-hex move, probably used by attack subclass

	unit_map::iterator unit_it = resources::units->find(get_dest_hex());
	assert(unit_it != resources::units->end());

	// Restore movement points
	unit_it->set_movement(unit_it->movement_left() + movement_cost_);

	// Restore the unit to its original position
	unit_map.move(get_dest_hex(), get_source_hex());

}

void move::draw_hex(const map_location& hex)
{
	(void) hex; //temporary to avoid unused param warning
}

bool move::is_numbering_hex(const map_location& hex) const
{
	return hex == get_dest_hex();
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

void move::calculate_move_cost()
{
	assert(unit_);
	assert(route_);
	if (get_source_hex().valid() && get_dest_hex().valid() && get_source_hex() != get_dest_hex())
	{

		// TODO: find a better treatment of movement points when defining moves out-of-turn
		if(get_unit()->movement_left() - route_->move_cost < 0
				&& resources::controller->current_side() == resources::screen->viewing_side()) {
			WRN_WB << "Move defined with insufficient movement left.\n";
		}

		// If unit finishes move in a village it captures, set the move cost to unit_.movement_left()
		 if (route_->marks[get_dest_hex()].capture)
		 {
			 movement_cost_ = get_unit()->movement_left();
		 }
		 else
		 {
			 movement_cost_ = route_->move_cost;
		 }
	}
}

} // end namespace wb
