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
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit_display.hpp"

namespace wb {

typedef boost::shared_ptr<side_actions> side_actions_ptr;

manager::manager():
		active_(false),
		mapbuilder_(),
		route_(),
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
	if (resources::screen && fake_unit_)
	{
		resources::screen->remove_temporary_unit(fake_unit_.get());
	}
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
			//wb_scoped_lock lock(actions_modification_mutex_);
			mapbuilder_.reset(new mapbuilder_visitor(*resources::units));
			const action_set& actions = current_actions()->actions();
			DBG_WB << "Building planned unit map.\n";
			foreach (const action_ptr &action, actions)
			{
				assert(action);
				action->accept(*mapbuilder_);
			}
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
			//wb_scoped_lock lock(actions_modification_mutex_);
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
	current_actions()->set_future_view(true);
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

void manager::create_temp_move(const std::vector<map_location> &steps)
{
	//TODO: properly handle movement points

	//NOTE: route.back() = dst, and route.front() = src
	route_ = steps;
	if (route_.size() > 1 && selected_unit_ != NULL)
	{
		if (!move_arrow_) //TODO: create new arrow if turn ended
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
			fake_unit_.reset(new unit(*selected_unit_));
			resources::screen->place_temporary_unit(fake_unit_.get());
		}

		move_arrow_->set_path(route_);

		unit_display::move_unit(route_, *fake_unit_, *resources::teams, false); //get facing right
		fake_unit_->set_location(route_.back());
		fake_unit_->set_ghosted(true);
	}
}

void manager::erase_temp_move()
{
	//TODO: properly handle movement points

	if (move_arrow_)
	{
		move_arrow_.reset(); //auto-removes itself from display

		//reset src unit back to normal, if it lacks any planned action,
		//and we're not in the process of saving a move
		wb_scoped_lock try_lock(move_saving_mutex_, boost::interprocess::try_to_lock);
		if (try_lock && selected_unit_
				&& current_actions()->find_first_action_of(*selected_unit_) == current_actions()->end())
		{
				selected_unit_->set_standing(true);
		}
	}
	if (fake_unit_)
	{
		resources::screen->remove_temporary_unit(fake_unit_.get());
		fake_unit_.reset();
	}
}

void manager::save_temp_move()
{
	std::vector<map_location> route;
	arrow_ptr move_arrow;
	fake_unit_ptr fake_unit;
	unit* target_unit;

	{
		// Wait until the block is finished and the variables have finished copying,
		// before granting another thread access.
		wb_scoped_lock lock(move_saving_mutex_); //waits for lock

		route = route_;
		move_arrow.reset(new arrow(*move_arrow_.get()));
		resources::screen->add_arrow(*move_arrow);
		fake_unit.reset(new unit(*fake_unit_.get()));
		resources::screen->place_temporary_unit(fake_unit.get());
		target_unit = selected_unit_;

		erase_temp_move();
		selected_unit_ = NULL;

		LOG_WB << "Creating move for unit " << target_unit->name() << " [" << target_unit->id() << "]"
				<< " from " << route.front()
				<< " to " << route.back() << "\n";
	}

	assert(!has_planned_unit_map());

	target_unit->set_ghosted(false);
	unit_display::move_unit(route, *fake_unit, *resources::teams, true);

	current_actions()->queue_move(*target_unit, route.front(), route.back(), move_arrow, fake_unit);
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
		current_actions()->set_future_view(false);

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


		current_actions()->set_future_view(true);
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
