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
#include "find_visitor.hpp"
#include "highlight_visitor.hpp"
#include "mapbuilder_visitor.hpp"
#include "move.hpp"

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
		planned_unit_map_(false)
{
}

manager::~manager()
{
	if (resources::screen && fake_unit_)
	{
		resources::screen->remove_temporary_unit(fake_unit_.get());
	}
}

static side_actions_ptr get_current_side_actions()
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
		assert (!planned_unit_map_);
		if (!planned_unit_map_)
		{
			mapbuilder_.reset(new mapbuilder_visitor(*resources::units));
			const action_set& actions = get_current_side_actions()->actions();
			DBG_WB << "Building planned unit map.\n";
			foreach (const action_ptr &action, actions)
			{
				assert(action);
				action->accept(*mapbuilder_);
			}
			planned_unit_map_ = true;
		}
	}
}

void manager::set_real_unit_map()
{
	if (active_)
	{
		assert (planned_unit_map_);
		if (planned_unit_map_)
		{
			DBG_WB << "Restoring regular unit map.\n";
			mapbuilder_.reset();
			planned_unit_map_ = false;
		}
	}
}

void manager::on_mouseover_change(const map_location& hex)
{
	if (active_ && !selected_unit_)
	{
		remove_highlight();
		highlight_hex(hex);
	}
}

void manager::highlight_hex(const map_location& hex)
{
	scoped_planned_unit_map wb_modifiers;

	unit_map::iterator highlighted_unit = resources::units->find(hex);

	if (highlighted_unit.valid())
	{
		highlighted_unit_ = &*highlighted_unit;
	}
	else
	{
		action_set actions = get_current_side_actions()->actions();
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

		action_set actions = get_current_side_actions()->actions();
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

	action_set actions = get_current_side_actions()->actions();
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
	get_current_side_actions()->set_future_view(true);
	action_set actions = get_current_side_actions()->actions();
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
		if (try_lock && selected_unit_ && !get_first_action_of(*selected_unit_))
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
	wb_scoped_lock try_lock(move_saving_mutex_, boost::interprocess::try_to_lock);
	if (try_lock)
	{
		std::vector<map_location> route = route_;
		arrow_ptr move_arrow;
		move_arrow.reset(new arrow(*move_arrow_.get()));
		resources::screen->add_arrow(*move_arrow);
		fake_unit_ptr fake_unit;
		fake_unit.reset(new unit(*fake_unit_.get()));
		resources::screen->place_temporary_unit(fake_unit.get());
		unit* target_unit = selected_unit_;

		erase_temp_move();
		selected_unit_ = NULL;
		//TODO: properly handle movement points

		LOG_WB << "Creating move for unit " << target_unit->name() << " [" << target_unit->id() << "]"
				<< " from " << route.front()
				<< " to " << route.back() << "\n";

		assert(!has_planned_unit_map());

		target_unit->set_ghosted(false); //FIXME: doesn't take effect until after the move animation, boucman: help!
		unit_display::move_unit(route, *fake_unit, *resources::teams, true);

		get_current_side_actions()->queue_move(*target_unit, route.front(), route.back(), move_arrow, fake_unit);

	}

}

void manager::contextual_execute()
{
	//TODO: catch end_turn_exception somewhere here?
	//TODO: properly handle movement points
	get_current_side_actions()->set_future_view(false);

	if (selected_unit_)
	{
		get_current_side_actions()->execute(get_first_action_of(*selected_unit_));
	}
	else if (highlighted_unit_)
	{
		get_current_side_actions()->execute(get_first_action_of(*highlighted_unit_));
	}
	else
	{
		get_current_side_actions()->execute_next();
	}


	get_current_side_actions()->set_future_view(true);
}

//TODO: transfer most of this function into side_actions
void manager::delete_last()
{
		get_current_side_actions()->remove_action(get_current_side_actions()->end() - 1);
}

//TODO: better to move this function into side_actions
action_ptr manager::get_first_action_of(const unit& unit) const
{
	find_visitor finder;
	action_ptr action = finder.find_first_action_of(unit, get_current_side_actions()->actions());
	return action;
}

scoped_planned_unit_map::scoped_planned_unit_map()
{
	resources::whiteboard->set_planned_unit_map();
}

scoped_planned_unit_map::~scoped_planned_unit_map()
{
	resources::whiteboard->set_real_unit_map();
}

scoped_real_unit_map::scoped_real_unit_map()
{
	resources::whiteboard->set_real_unit_map();
}

scoped_real_unit_map::~scoped_real_unit_map()
{
	resources::whiteboard->set_planned_unit_map();
}

} // end namespace wb
