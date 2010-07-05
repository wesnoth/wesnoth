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

manager::manager():
		active_(false),
		mapbuilder_(),
		route_(),
		steps_(),
		move_arrow_(),
		fake_unit_(),
		selected_unit_(NULL),
		highlighted_unit_(NULL),
		move_saving_mutex_(),
		planned_unit_map_active_(false)
{
}

manager::~manager()
{
}

static side_actions_ptr current_actions()
{
	int current_side = resources::controller->current_side();
	team& current_team = (*resources::teams)[current_side - 1];
	side_actions_ptr side_actions = current_team.get_side_actions();
	return side_actions;
}

void manager::set_planned_unit_map()
{
	if (active_)
	{
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
	}
}

void manager::set_real_unit_map()
{
	if (active_)
	{
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
	}
}

void manager::on_mouseover_change(const map_location& hex)
{
	//FIXME: Detect if a WML event is executing, and if so, avoid modifying the unit map during that time.
	// Acting otherwise causes a crash.
	if (active_ && !selected_unit_)
	{
		remove_highlight();
		highlight_hex(hex);
	}
}

void manager::highlight_hex(const map_location& hex)
{
	unit_map::iterator highlighted_unit;
	bool unit_exists;
	{
		scoped_planned_unit_map wb_modifiers;
		highlighted_unit = resources::units->find(hex);
		unit_exists = highlighted_unit.valid();
	}

	if (unit_exists)
	{
		highlighted_unit_ = &*highlighted_unit;
	}
	else
	{
		action_set actions = current_actions()->actions();
		foreach(action_ptr action, actions)
		{
			if (action->is_related_to(hex))
			{
				highlighted_unit_ = &action->get_unit();
			}
		}
	}

	if (highlighted_unit_)
	{
		highlight_visitor highlighter(true);

		action_set actions = current_actions()->actions();
		foreach(action_ptr action, actions)
		{
			if (action->is_related_to(*highlighted_unit_))
			{
				action->accept(highlighter);
			}
		}
	}
}

void manager::remove_highlight()
{
	highlight_visitor unhighlighter(false);

	action_set actions = current_actions()->actions();
	foreach(action_ptr action, actions)
	{
		action->accept(unhighlighter);
	}
	highlighted_unit_ = NULL;
}

void manager::on_unit_select(unit& unit)
{
	erase_temp_move();
	remove_highlight();
	action_set actions = current_actions()->actions();
	highlight_visitor highlighter(true);
	foreach(action_ptr action, actions)
	{
		if (action->is_related_to(unit))
		{
			action->accept(highlighter);
		}
	}
	selected_unit_ = &unit;
	DBG_WB << "Selected unit " << selected_unit_->name() << " [" << selected_unit_->id() << "]\n";
}

void manager::on_unit_deselect()
{
	if (selected_unit_)
	{
		DBG_WB << "Deselecting unit " << selected_unit_->name() << " [" << selected_unit_->id() << "]\n";
		erase_temp_move();
		selected_unit_ = NULL;
	}
}

void manager::create_temp_move(const pathfind::marked_route &route)
{
	//TODO: properly handle turn end

	if (selected_unit_ == NULL || route.steps.empty() || route.steps.size() < 2)
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
	std::vector<map_location> steps;
	arrow_ptr move_arrow;
	fake_unit_ptr fake_unit;
	unit* target_unit;

	{
		// Wait until the block is finished and the variables have finished copying,
		// before granting another thread access.
		wb_scoped_lock lock(move_saving_mutex_); //waits for lock

		//Temporary: Only keep as path the steps can be done this turn
		steps = route_->steps;
		move_arrow = arrow_ptr(move_arrow_);
		fake_unit = fake_unit_ptr(fake_unit_);
		target_unit = selected_unit_;

		erase_temp_move();
		selected_unit_ = NULL;

		LOG_WB << "Creating move for unit " << target_unit->name() << " [" << target_unit->id() << "]"
				<< " from " << steps.front()
				<< " to " << steps.back() << "\n";
	}

	assert(!has_planned_unit_map());

	//unit_display::move_unit(steps, *fake_unit, *resources::teams, false);
	fake_unit->set_disabled_ghosted(false);
	current_actions()->queue_move(*target_unit, steps.front(), steps.back(), move_arrow, fake_unit);
}

void manager::contextual_execute()
{
	wb_scoped_lock try_lock(actions_modification_mutex_, boost::interprocess::try_to_lock);
	if (!try_lock)
		return;

	if (!current_actions()->empty())
	{
		//TODO: catch end_turn_exception somewhere here?
		//TODO: properly handle movement points, probably through the mapbuilder_visitor

		if (selected_unit_ && unit_has_actions(*selected_unit_))
		{
			current_actions()->execute(current_actions()->find_first_action_of(*selected_unit_));
		}
		else if (highlighted_unit_ && unit_has_actions(*highlighted_unit_))
		{
			current_actions()->execute(current_actions()->find_first_action_of(*highlighted_unit_));
		}
		else
		{
			current_actions()->execute_next();
		}
	}
}

void manager::contextual_delete()
{
	wb_scoped_lock try_lock(actions_modification_mutex_, boost::interprocess::try_to_lock);
	if (!try_lock)
		return;

	if (!current_actions()->empty())
	{
		if (selected_unit_ && unit_has_actions(*selected_unit_))
		{
			remove_highlight();
			erase_temp_move();
			current_actions()->remove_action(current_actions()->find_last_action_of(*selected_unit_));
		}
		else if (highlighted_unit_ && unit_has_actions(*highlighted_unit_))
		{
			current_actions()->remove_action(current_actions()->find_last_action_of(*highlighted_unit_));
		}
		else
		{
			current_actions()->remove_action(current_actions()->end() - 1);
		}
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
