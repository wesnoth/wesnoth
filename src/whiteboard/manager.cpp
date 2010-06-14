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
#include "mapbuilder_visitor.hpp"

#include "arrow.hpp"
#include "foreach.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"

namespace wb {

manager::manager():
		active_(false),
		mapbuilder_(NULL),
		route_(),
		move_arrow_(NULL),
		fake_unit_(NULL)
{
}

manager::~manager()
{
	if (resources::screen != NULL)
	{
		if (fake_unit_.get() != NULL)
		{
			resources::screen->remove_temporary_unit(fake_unit_.get());
		}
		if (move_arrow_.get() != NULL)
		{
			resources::screen->remove_arrow(*move_arrow_);
		}
	}
}

void manager::apply_temp_modifiers()
{
	mapbuilder_.reset(new mapbuilder_visitor(*resources::units));
	int current_side = resources::controller->current_side();
	team& current_team = (*resources::teams)[current_side - 1];
	side_actions& side_actions = current_team.get_side_actions();
	const action_set& actions = side_actions.actions();
	foreach (const action_ptr &action, actions)
	{
		assert(action);
		action->accept(*mapbuilder_);
	}
}
void manager::remove_temp_modifiers()
{
	DBG_WB << "Removing temporary modifiers.\n";
	mapbuilder_.reset();
	DBG_WB << "Removed temporary modifiers.\n";
}

void manager::create_temp_move(const std::vector<map_location> &steps)
{
	route_ = steps;
	if (route_.size() > 1)
	{
		bool show_ghosted_unit_bars = false;

		if (move_arrow_.get() == NULL)
		{
			// Create temp arrow
			game_display *screen = resources::screen;
			move_arrow_.reset(new arrow((display*) screen));
			int current_side = resources::controller->current_side();
			move_arrow_->set_color(team::get_side_color_index(current_side));
			move_arrow_->set_alpha(2.0);
			screen->add_arrow(*move_arrow_);

			// Create temp ghost unit
			fake_unit_.reset(new unit(*resources::units->find(route_.front())));
			fake_unit_->set_location(route_.back());
			fake_unit_->set_ghosted(show_ghosted_unit_bars);
			screen->place_temporary_unit(fake_unit_.get());
		}

		move_arrow_->set_path(route_);
		fake_unit_->set_location(route_.back());
		fake_unit_->set_ghosted(show_ghosted_unit_bars);
	}
}

void manager::erase_temp_move()
{
	if (move_arrow_.get() != NULL)
	{
		resources::screen->remove_arrow(*move_arrow_);
		move_arrow_.reset();
		resources::screen->remove_temporary_unit(fake_unit_.get());
		fake_unit_.reset();
	}
}

void manager::save_temp_move(unit& subject)
{
	int current_side = resources::controller->current_side();
	team& current_team = (*resources::teams)[current_side - 1];

	LOG_WB << "Creating move for unit " << subject.name() << " [" << subject.id() << "]"
			<< " from " << subject.get_location()
			<< " to " << route_.back() << "\n";

	move_arrow_->set_alpha(0.6);

	current_team.get_side_actions().queue_move(subject, route_.back(),
			*(move_arrow_.release()) /* ownership of the arrow transferred to the new move action */,
			*(fake_unit_.release())  /* ownership of the fake unit transferred to the new move action */);

}

} // end namespace wb
