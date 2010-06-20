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
		highlighted_hex_(map_location::null_location),
		temp_modifiers_applied_(false)
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

void manager::apply_temp_modifiers()
{
	if (!temp_modifiers_applied_)
	{
		mapbuilder_.reset(new mapbuilder_visitor(*resources::units));
		const action_set& actions = get_current_side_actions()->actions();
		foreach (const action_ptr &action, actions)
		{
			assert(action);
			action->accept(*mapbuilder_);
		}
		temp_modifiers_applied_ = true;
	}
}
void manager::remove_temp_modifiers()
{
	DBG_WB << "Removing temporary modifiers.\n";
	mapbuilder_.reset();
	temp_modifiers_applied_ = false;
}

void manager::toggle_temp_modifiers()
{
	if (temp_modifiers_applied_)
		remove_temp_modifiers();
	else
		apply_temp_modifiers();
}

void manager::highlight_hex(const map_location& hex)
{
	if (selected_unit_ != NULL) return;

	highlighted_hex_ = hex;

	unit_map::iterator highlighted_unit = resources::units->find(hex);

	highlight_visitor highlighter(true);

	action_set actions = get_current_side_actions()->actions();
	foreach(action_ptr action, actions)
	{
		if (action->is_related_to(hex)
			|| (highlighted_unit.valid() && action->is_related_to(*highlighted_unit)))
		{
			action->accept(highlighter);
		}
	}
}

void manager::remove_highlight()
{
	if (selected_unit_ != NULL) return;

	highlight_visitor unhighlighter(false);

	action_set actions = get_current_side_actions()->actions();
	foreach(action_ptr action, actions)
	{
		action->accept(unhighlighter);
	}
}

void manager::select_unit(unit& unit)
{
	erase_temp_move();
	action_set actions = get_current_side_actions()->actions();
	highlight_visitor unhighlighter(false);
	foreach(action_ptr action, actions)
	{
			action->accept(unhighlighter);
	}
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

void manager::deselect_unit()
{
	if (selected_unit_)
	{
		DBG_WB << "Deselecting unit " << selected_unit_->name() << " [" << selected_unit_->id() << "]\n";
		selected_unit_ = NULL;
	}
}

void manager::create_temp_move(const std::vector<map_location> &steps)
{
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

			// Create temp ghost unit
			fake_unit_.reset(new unit(*selected_unit_));
			unit_display::move_unit(route_, *fake_unit_, *resources::teams, false); //get facing right
			fake_unit_->set_ghosted(true);
			resources::screen->place_temporary_unit(fake_unit_.get());
		}

		move_arrow_->set_path(route_);
		fake_unit_->set_location(route_.back());
		//unit_display::move_unit(route_, *fake_unit_, *resources::teams, false); //get facing right
		fake_unit_->set_ghosted(true);
	}
}

bool manager::during_move_creation() const
{
	bool during_move_creation = selected_unit_ != NULL;
	return during_move_creation;
}

void manager::erase_temp_move()
{
	if (move_arrow_)
	{
		move_arrow_.reset(); //auto-removes itself from display

		//reset src unit back to normal, if it lacks any planned action
		if (selected_unit_ && !has_action(*selected_unit_))
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
	LOG_WB << "Creating move for unit " << selected_unit_->name() << " [" << selected_unit_->id() << "]"
			<< " from " << selected_unit_->get_location()
			<< " to " << route_.back() << "\n";

	fake_unit_->set_location(route_.front());
	fake_unit_->set_ghosted(false);
	unit_display::move_unit(route_, *selected_unit_, *resources::teams, true);
	selected_unit_->set_standing(true);
	fake_unit_->set_ghosted(false);

	move_arrow_->set_alpha(move::ALPHA_HIGHLIGHT);
	highlighted_hex_ = selected_unit_->get_location();

	resources::screen->add_arrow(*move_arrow_);

	get_current_side_actions()->queue_move(*selected_unit_, route_.back(), move_arrow_, fake_unit_);
	move_arrow_.reset();
	fake_unit_.reset();
	remove_temp_modifiers();
	apply_temp_modifiers();
	selected_unit_->set_standing(true);
	selected_unit_ = NULL;
}

void manager::execute_next()
{
	remove_temp_modifiers();
	get_current_side_actions()->execute_next();
	apply_temp_modifiers();
}

void manager::delete_last()
{
	remove_temp_modifiers();
	get_current_side_actions()->remove_action(get_current_side_actions()->end() - 1);
	apply_temp_modifiers();
}

action_ptr manager::has_action(const unit& unit) const
{
	find_visitor finder;
	action_ptr action = finder.find_first_action_of(unit, get_current_side_actions()->actions());
	return action;
}

} // end namespace wb
