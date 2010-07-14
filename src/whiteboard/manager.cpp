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
 * @file manager.cpp
 */

#include "manager.hpp"

#include "action.hpp"
#include "highlight_visitor.hpp"
#include "mapbuilder_visitor.hpp"
#include "move.hpp"
#include "side_actions.hpp"

#include "arrow.hpp"
#include "foreach.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit_display.hpp"

namespace wb {


static team& current_team()
{
	int current_side = resources::controller->current_side();
	team& current_team = (*resources::teams)[current_side - 1];
	return current_team;
}

static side_actions_ptr current_actions()
{
	side_actions_ptr side_actions = current_team().get_side_actions();
	return side_actions;
}

manager::manager():
		active_(false),
		mapbuilder_(),
		highlighter_(),
		route_(),
		steps_(),
		move_arrow_(),
		fake_unit_(),
		selected_unit_(NULL),
		planned_unit_map_active_(false),
		modifying_actions_(false)
{
	highlighter_.reset(new highlight_visitor(*resources::units, current_actions()));
}

manager::~manager()
{
}

void manager::set_planned_unit_map()
{
	if (active_ && !modifying_actions_)
	{
		modifying_actions_ = true;
		//TODO: enable back this assert, after modifying the mouse code that triggers it constantly
		//assert(!modifying_actions_);
		if (!planned_unit_map_active_)
		{
			DBG_WB << "Building planned unit map.\n";
			mapbuilder_.reset(new mapbuilder_visitor(*resources::units, current_actions()));
			mapbuilder_->build_map();
			planned_unit_map_active_ = true;
		}
		else
		{
			WRN_WB << "Attempt to set planned unit map when it was already set.\n";
		}
		modifying_actions_ = false;
	}
}

void manager::set_real_unit_map()
{
	if (active_ && !modifying_actions_)
	{
		modifying_actions_ = true;
		if (planned_unit_map_active_)
		{
			DBG_WB << "Restoring regular unit map.\n";
			mapbuilder_.reset();
			planned_unit_map_active_ = false;
		}
		else
		{
			WRN_WB << "Attempt to disable the planned unit map, when it was already disabled.\n";
		}
		modifying_actions_ = false;
	}
}

void manager::draw_hex(const map_location& hex)
{
	current_actions()->draw_hex(hex);
}

void manager::on_mouseover_change(const map_location& hex)
{
	if (!active_)
		return;

	//FIXME: Detect if a WML event is executing, and if so, avoid modifying the unit map during that time.
	// Acting otherwise causes a crash.
	if (active_ && !selected_unit_)
	{
		highlighter_->set_mouseover_hex(hex);
		highlighter_->highlight();
	}
}

void manager::on_unit_select(unit& unit)
{
	if (!active_)
		return;

	erase_temp_move();
	selected_unit_ = NULL;
	if (unit.side() == resources::controller->current_side())
	{
		selected_unit_ = &unit;
	}
	LOG_WB << "Selected unit " << selected_unit_->name() << " [" << selected_unit_->id() << "]\n";
}

void manager::on_unit_deselect()
{
	if (!active_)
		return;

	if (selected_unit_)
	{
		LOG_WB << "Deselecting unit " << selected_unit_->name() << " [" << selected_unit_->id() << "]\n";
		erase_temp_move();
		selected_unit_ = NULL;
	}
}

void manager::create_temp_move(const pathfind::marked_route &route)
{
	if (!active_)
		return;

	//TODO: properly handle turn end

	if (selected_unit_ == NULL)
	{
		erase_temp_move();
		return;
	}

	//Temporary: Don't draw move arrow if move goes beyond range.
	bool cancel = false;
	foreach (const map_location& hex, route.steps)
	{
		if (cancel)
		{
			erase_temp_move();
			return;
		}
		pathfind::marked_route::mark_map::const_iterator w = route.marks.find(hex);
		if(w != route.marks.end() && w->second.turns == 1)
		{
			cancel = true;
		}
	}

	if (route.steps.empty() || route.steps.size() < 2)
	{
		route_.reset(new pathfind::marked_route()); //empty route
	}
	else
	{
		assert(selected_unit_->side() == resources::controller->current_side());
		route_.reset(new pathfind::marked_route(route));
		//NOTE: route_.steps.back() = dst, and route_.steps.front() = src

		if (!move_arrow_)
		{
			// Create temp arrow
			move_arrow_.reset(new arrow());
			int current_side = resources::controller->current_side();
			move_arrow_->set_color(team::get_side_color_index(current_side));
			move_arrow_->set_alpha(move::ALPHA_HIGHLIGHT);
			resources::screen->add_arrow(*move_arrow_);

		}
		if (!fake_unit_)
		{
			// Create temp ghost unit
			fake_unit_.reset(new unit(*selected_unit_), wb::manager::fake_unit_deleter());
			resources::screen->place_temporary_unit(fake_unit_.get());
			fake_unit_->set_ghosted(false);
		}


		move_arrow_->set_path(route_->steps);

		unit_display::move_unit(route_->steps, *fake_unit_, *resources::teams, false); //get facing right
		fake_unit_->set_location(route_->steps.back());
		fake_unit_->set_ghosted(false);
	}
}

void manager::erase_temp_move()
{
	if (move_arrow_)
	{
		move_arrow_.reset(); //auto-removes itself from display
	}
	if (fake_unit_)
	{
		fake_unit_.reset(); //auto-removes itself from display thanks to custom deleter in the shared_ptr
	}
	if (route_)
	{
		route_.reset();
	}
}

void manager::save_temp_move()
{
	if (active_ && !modifying_actions_)
	{
		assert(!has_planned_unit_map());

		modifying_actions_ = true;
		std::vector<map_location> steps;
		arrow_ptr move_arrow;
		fake_unit_ptr fake_unit;
		unit* subject_unit;

		steps = route_->steps;
		move_arrow = arrow_ptr(move_arrow_);
		fake_unit = fake_unit_ptr(fake_unit_);
		subject_unit = selected_unit_;

		on_unit_deselect();

		LOG_WB << "Creating move for unit " << subject_unit->name() << " [" << subject_unit->id() << "]"
				<< " from " << steps.front()
				<< " to " << steps.back() << "\n";

		fake_unit->set_disabled_ghosted(false);
		current_actions()->queue_move(*subject_unit, steps.front(), steps.back(), move_arrow, fake_unit);
		modifying_actions_ = false;
	}
}

void manager::save_temp_attack(const map_location& attack_from, const map_location& target_hex)
{
	if (active_ && !modifying_actions_)
	{
		assert(!has_planned_unit_map());

		modifying_actions_ = true;
		std::vector<map_location> steps;
		arrow_ptr move_arrow;
		fake_unit_ptr fake_unit;
		unit* subject_unit;

		subject_unit = selected_unit_;

		map_location source_hex;
		map_location dest_hex;
		if (route_)
		{
			move_arrow = arrow_ptr(move_arrow_);
			fake_unit = fake_unit_ptr(fake_unit_);

			steps = route_->steps;
			source_hex = steps.front();
			dest_hex = steps.back();

			fake_unit->set_disabled_ghosted(false);
		}
		else
		{
			move_arrow.reset(new arrow);
			dest_hex = source_hex = attack_from;
		}

		on_unit_deselect();

		LOG_WB << "Creating attack for unit " << subject_unit->name() << " [" << subject_unit->id()
				<< "]: moving from " << source_hex << " to " << dest_hex
				<< " and attacking " << target_hex << "\n";

		current_actions()->queue_attack(*subject_unit, target_hex, source_hex, dest_hex, move_arrow, fake_unit);
		modifying_actions_ = false;

		resources::screen->invalidate(target_hex);
	}
}

void manager::contextual_execute()
{
	if (!active_)
		return;

	if (!(modifying_actions_ || current_actions()->empty()))
	{
		modifying_actions_ = true;
		erase_temp_move();

		//TODO: catch end_turn_exception somewhere here?
		action_ptr action;
		if (selected_unit_)
		{
				current_actions()->execute(current_actions()->find_first_action_of(*selected_unit_));
		}
		else if (highlighter_
				&& (action = highlighter_->get_execute_target())) //intentional assignment of action with '='
		{
			current_actions()->execute(current_actions()->get_position_of(action));
		}
		else
		{
			current_actions()->execute_next();
		}
		modifying_actions_ = false;
	}
}

void manager::contextual_delete()
{
	if (!active_)
		return;

	if (!(modifying_actions_ || current_actions()->empty()))
	{
		modifying_actions_ = true;
		erase_temp_move();

		action_ptr action;
		if (selected_unit_)
		{
			current_actions()->remove_action(current_actions()->find_first_action_of(*selected_unit_));
		}
		else if (highlighter_ && (action = highlighter_->get_delete_target()))
		{
			current_actions()->remove_action(current_actions()->get_position_of(action));
		}
		else
		{
			current_actions()->remove_action(current_actions()->end() - 1);
		}
		modifying_actions_ = false;
	}
}

void manager::contextual_bump_up_action()
{
	if (!active_)
		return;

	if (!(modifying_actions_ || current_actions()->empty()) && highlighter_)
	{
		modifying_actions_ = true;
		action_ptr action = highlighter_->get_bump_target();
		if (action)
		{
			current_actions()->bump_earlier(current_actions()->get_position_of(action));
		}
		modifying_actions_ = false;
	}
}

void manager::contextual_bump_down_action()
{
	if (!active_)
		return;

	if (!(modifying_actions_ || current_actions()->empty()) && highlighter_)
	{
		modifying_actions_ = true;
		action_ptr action = highlighter_->get_bump_target();
		if (action)
		{
			current_actions()->bump_later(current_actions()->get_position_of(action));
		}
		modifying_actions_ = false;
	}
}

bool manager::unit_has_actions(const unit& unit) const
{
	return current_actions()->find_first_action_of(unit)
			!= current_actions()->end();
}

void manager::fake_unit_deleter::operator() (unit*& fake_unit)
{
    if (fake_unit)
    {
        if(resources::screen)
        {
        	resources::screen->remove_temporary_unit(fake_unit);
        }
        DBG_WB << "Erasing temporary unit " << fake_unit->name() << " [ " << fake_unit->underlying_id() << "]\n";
        delete fake_unit;
    }
}

scoped_planned_unit_map::scoped_planned_unit_map()
{
	if (!resources::whiteboard->has_planned_unit_map())
		resources::whiteboard->set_planned_unit_map();
}

scoped_planned_unit_map::~scoped_planned_unit_map()
{
	if (resources::whiteboard->has_planned_unit_map())
		resources::whiteboard->set_real_unit_map();
}

scoped_real_unit_map::scoped_real_unit_map()
:has_planned_unit_map_(resources::whiteboard->has_planned_unit_map())
{
	if (has_planned_unit_map_)
		resources::whiteboard->set_real_unit_map();
}

scoped_real_unit_map::~scoped_real_unit_map()
{
	if (has_planned_unit_map_ && !resources::whiteboard->has_planned_unit_map())
		resources::whiteboard->set_planned_unit_map();
}

} // end namespace wb
