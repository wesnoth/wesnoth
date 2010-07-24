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
#include "chat_events.hpp"
#include "foreach.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit_display.hpp"

namespace wb {

manager::manager():
		active_(false),
		inverted_behavior_(false),
		wait_for_side_init_(true),
		mapbuilder_(),
		highlighter_(),
		route_(),
		move_arrow_(),
		fake_unit_(),
		planned_unit_map_active_(false),
		executing_actions_(false)
{
	LOG_WB << "Manager initialized.\n";
	highlighter_.reset(new highlight_visitor(*resources::units, viewer_actions()));
}

manager::~manager()
{
}

static void print_to_chat(const std::string& title, const std::string& message)
{
	resources::screen->add_chat_message(time(NULL), title, 0, message,
			events::chat_handler::MESSAGE_PRIVATE, false);
}

void manager::print_help()
{
	print_to_chat("-------------",  "List of whiteboard hotkeys:");
	const hotkey::hotkey_item& hk_execute = hotkey::get_hotkey(hotkey::HOTKEY_WB_EXECUTE_ACTION);
	if(!hk_execute.null()) {
		print_to_chat("[execute action]", "'" + hk_execute.get_name() + "'");
	}
	const hotkey::hotkey_item& hk_delete = hotkey::get_hotkey(hotkey::HOTKEY_WB_DELETE_ACTION);
	if(!hk_delete.null()) {
		print_to_chat("[delete action]", "'" + hk_delete.get_name() + "'");
	}
	const hotkey::hotkey_item& hk_bump_up = hotkey::get_hotkey(hotkey::HOTKEY_WB_BUMP_UP_ACTION);
	if(!hk_bump_up.null()) {
		print_to_chat("[move action earlier in queue]", "'" + hk_bump_up.get_name() + "'");
	}
	const hotkey::hotkey_item& hk_bump_down = hotkey::get_hotkey(hotkey::HOTKEY_WB_BUMP_DOWN_ACTION);
	if(!hk_bump_down.null()) {
		print_to_chat("[move action later in queue]", "'" + hk_bump_down.get_name() + "'");
	}
}

void manager::set_active(bool active)
{
	if(is_observer())
	{
		active_ = false;
		LOG_WB << "Whiteboard can't be activated by observers.\n";
	}
	else if (active != active_)
	{
		active_ = active;
		erase_temp_move();

		if (active_)
		{
			validate_viewer_actions();
			LOG_WB << "Whiteboard activated! " << *viewer_actions() << "\n";
			create_temp_move();
		} else {
			LOG_WB << "Whiteboard deactivated!\n";
		}
	}
}

void manager::set_invert_behavior(bool invert)
{
	if(wait_for_side_init_
			|| executing_actions_
			|| is_observer())
		return;

	static bool print_once = true;
	if (print_once)
	{
		print_once = false;
		print_to_chat("whiteboard", std::string("Hold the ctrl key to temporarily toggle the whiteboard.")
			+ " (Activate and then) Deactivate the whiteboard with the :wb command to erase all actions.");
		print_help();
	}

	log_scope("set_invert_behavior");
	if (invert)
	{
		if (!inverted_behavior_)
		{
			inverted_behavior_ = true;
			if (active_)
			{
				DBG_WB << "Whiteboard deactivated temporarily.\n";
				set_active(false);
			}
			else // active_ == false
			{
				DBG_WB << "Whiteboard activated temporarily.\n";
				set_active(true);
			}
		}
	}
	else
	{
		if (inverted_behavior_)
		{
			inverted_behavior_ = false;
			if (active_)
			{
				DBG_WB << "Whiteboard set back to deactivated status.\n";
				set_active(false);
			}
			else // active_ == false
			{
				DBG_WB << "Whiteboard set back to activated status.\n";
				set_active(true);
			}
		}
	}
}

bool manager::can_execute_hotkey() const
{
	return !viewer_actions()->empty();
}

void manager::on_init_side()
{
	validate_viewer_actions();
	highlighter_.reset(new highlight_visitor(*resources::units, viewer_actions()));
	wait_for_side_init_ = false;
	LOG_WB << "on_init_side()\n";
}

void manager::on_finish_side_turn()
{
	wait_for_side_init_ = true;

	highlighter_.reset();
	erase_temp_move();
	LOG_WB << "on_finish_side_turn()\n";
}

side_actions_ptr manager::viewer_actions() const
{
	side_actions_ptr side_actions =
			(*resources::teams)[resources::screen->viewing_team()].get_side_actions();
	return side_actions;
}

side_actions_ptr manager::current_side_actions() const
{
	side_actions_ptr side_actions =
			(*resources::teams)[resources::controller->current_side() - 1].get_side_actions();
	return side_actions;
}

bool manager::current_side_has_actions() const
{
	return !current_side_actions()->empty();
}

void manager::validate_viewer_actions()
{
	assert(!executing_actions_);
	viewer_actions()->validate_actions();
}

void manager::set_planned_unit_map()
{
	if (!executing_actions_ && !wait_for_side_init_)
	{
		if (!planned_unit_map_active_)
		{
			DBG_WB << "Building planned unit map.\n";
			mapbuilder_.reset(new mapbuilder_visitor(*resources::units, viewer_actions()));
			mapbuilder_->build_map();
			planned_unit_map_active_ = true;
		}
		else
		{
			WRN_WB << "Attempt to set planned unit map when it was already set.\n";
		}
	}
	else if (executing_actions_)
	{
		LOG_WB << "Attempt to set planned_unit_map during action execution.\n";
	}
}

void manager::set_real_unit_map()
{
	if (!executing_actions_)
	{
		if (planned_unit_map_active_)
		{
			DBG_WB << "Restoring regular unit map.\n";
			mapbuilder_.reset();
			planned_unit_map_active_ = false;
		}
		else
		{
			DBG_WB << "Attempt to disable the planned unit map, when it was already disabled.\n";
		}
	}
	else //executing_actions_
	{
		LOG_WB << "Attempt to set planned_unit_map during action execution.\n";
	}

}

unit* manager::find_unit_future(map_location hex)
{
	scoped_planned_unit_map planned_unit_map;
	assert(has_planned_unit_map());
	unit_map::iterator it;
		if ((it = resources::units->find(hex)) != resources::units->end())
			return &*it;
		else
			return NULL;
}

unit* manager::find_selected_future()
{
	return find_unit_future(resources::screen->selected_hex());
}

unit* manager::find_selected_actor_future()
{
	unit* unit = find_selected_future();
	if (unit && unit->side() == resources::screen->viewing_side())
		return unit;
	else
		return NULL;
}

void manager::draw_hex(const map_location& hex)
{
	if (!wait_for_side_init_)
		viewer_actions()->draw_hex(hex);
}

void manager::on_mouseover_change(const map_location& hex)
{
	//FIXME: Detect if a WML event is executing, and if so, avoid modifying the unit map during that time.
	// Acting otherwise causes a crash.
	if (!resources::screen->selected_hex().valid() && highlighter_)

	{
		highlighter_->set_mouseover_hex(hex);
		highlighter_->highlight();
	}
}

void manager::create_temp_move()
{
	route_.reset();

	if (!active_) return;

	/*
	 * CHECK PRE-CONDITIONS
	 * (This section has multiple return paths.)
	 */

	pathfind::marked_route const& route =
			resources::controller->get_mouse_handler_base().get_current_route();

	if (route.steps.empty() || route.steps.size() < 2) return;

	unit const* selected_unit = this->find_selected_actor_future();
	if (!selected_unit) return;
	if (selected_unit->side() != resources::screen->viewing_side()) return;

	//FIXME: Temporary: Don't draw move arrow if move goes beyond range.
	bool cancel = false;
	foreach (const map_location& hex, route.steps)
	{
		if (cancel)
		{
			erase_temp_move();
			return;
		}
		pathfind::marked_route::mark_map::const_iterator w =
				route.marks.find(hex);
		//We only accept an end-of-first-turn or a capture mark if this is the move's last hex.
		if (w != route.marks.end() && (w->second.turns == 1
				|| w->second.capture))
		{
			cancel = true;
		}
	}

	/*
	 * DONE CHECKING PRE-CONDITIONS, CREATE THE TEMP MOVE
	 * (This section has only one return path.)
	 */

	//TODO: May be appropriate to replace these separate components by a temporary
	//      wb::move object

	route_.reset(new pathfind::marked_route(route));
	//NOTE: route_.steps.back() = dst, and route_.steps.front() = src

	if (!move_arrow_)
	{
		// Create temp arrow
		move_arrow_.reset(new arrow());
		move_arrow_->set_color(team::get_side_color_index(
				resources::screen->viewing_side()));
		move_arrow_->set_alpha(move::ALPHA_HIGHLIGHT);
		resources::screen->add_arrow(*move_arrow_);

	}
	if (!fake_unit_)
	{
		// Create temp ghost unit
		fake_unit_.reset(new unit(*selected_unit),
				wb::manager::fake_unit_deleter());
		resources::screen->place_temporary_unit(fake_unit_.get());
		fake_unit_->set_ghosted(false);
	}

	move_arrow_->set_path(route_->steps);

	unit_display::move_unit(route_->steps, *fake_unit_, *resources::teams,
			false); //get facing right
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
	if (has_temp_move() && !executing_actions_)
	{
		scoped_planned_unit_map planned_unit_map;

		arrow_ptr move_arrow;
		fake_unit_ptr fake_unit;

		move_arrow = arrow_ptr(move_arrow_);
		fake_unit = fake_unit_ptr(fake_unit_);

		fake_unit->set_disabled_ghosted(false);
		viewer_actions()->queue_move(*route_, move_arrow, fake_unit);
		erase_temp_move();
		LOG_WB << *viewer_actions() << "\n";
	}
}

void manager::save_temp_attack(const map_location& attack_from, const map_location& target_hex)
{
	if (active_ && !executing_actions_)
	{
		arrow_ptr move_arrow;
		fake_unit_ptr fake_unit;

		map_location source_hex;
		if (route_ && !route_->steps.empty())
		{
			move_arrow = arrow_ptr(move_arrow_);
			fake_unit = fake_unit_ptr(fake_unit_);

			assert(route_->steps.back() == attack_from);
			source_hex = route_->steps.front();

			fake_unit->set_disabled_ghosted(false);
		}
		else
		{
			move_arrow.reset(new arrow);
			source_hex = attack_from;
			route_.reset(new pathfind::marked_route);
			// We'll pass as parameter a one-hex route with no marks.
			route_->steps.push_back(attack_from);
		}

		unit* attacking_unit = find_unit_future(source_hex);
		assert(attacking_unit);

		int weapon_choice = resources::controller->get_mouse_handler_base().show_attack_dialog(
					attacking_unit->get_location(), target_hex);

		if (weapon_choice >= 0)
		{
			viewer_actions()->queue_attack(target_hex, weapon_choice, *route_, move_arrow, fake_unit);
		}

		resources::screen->invalidate(target_hex);
		erase_temp_move();
		LOG_WB << *viewer_actions() << "\n";
	}
}

void manager::contextual_execute()
{
	if (!(executing_actions_ || viewer_actions()->empty())
			&& resources::controller->current_side() == resources::screen->viewing_side())
	{
		erase_temp_move();
		validate_viewer_actions();

		action_ptr action;
		side_actions::iterator it;
		unit* selected_unit = this->find_selected_actor_future();
		if (selected_unit &&
				(it = viewer_actions()->find_first_action_of(*selected_unit)) != viewer_actions()->end())
		{
			executing_actions_ = true;
			viewer_actions()->execute(it);
			executing_actions_ = false;
		}
		else if (highlighter_ && (action = highlighter_->get_execute_target()) &&
				 (it = viewer_actions()->get_position_of(action)) != viewer_actions()->end())
		{
			executing_actions_ = true;
			viewer_actions()->execute(it);
			executing_actions_ = false;
		}
		else //we already check above for viewer_actions()->empty()
		{
			executing_actions_ = true;
			viewer_actions()->execute_next();
			executing_actions_ = false;
		}
	}
}

void manager::contextual_delete()
{
	if (!(executing_actions_ || viewer_actions()->empty()))
	{
		erase_temp_move();
		validate_viewer_actions();

		action_ptr action;
		side_actions::iterator it;
		unit* selected_unit = this->find_selected_actor_future();
		if (selected_unit &&
				(it = viewer_actions()->find_first_action_of(*selected_unit)) != viewer_actions()->end())
		{
			viewer_actions()->remove_action(it);
		}
		else if (highlighter_ && (action = highlighter_->get_delete_target()) &&
				(it = viewer_actions()->get_position_of(action)) != viewer_actions()->end())
		{
			viewer_actions()->remove_action(it);
		}
		else //we already check above for viewer_actions()->empty()
		{
			viewer_actions()->remove_action(viewer_actions()->end() - 1);
		}
	}
}

void manager::contextual_bump_up_action()
{
	if (!(executing_actions_ || viewer_actions()->empty()) && highlighter_)
	{

		validate_viewer_actions();
		action_ptr action = highlighter_->get_bump_target();
		if (action)
		{
			viewer_actions()->bump_earlier(viewer_actions()->get_position_of(action));
		}
	}
}

void manager::contextual_bump_down_action()
{
	if (!(executing_actions_ || viewer_actions()->empty()) && highlighter_)
	{

		validate_viewer_actions();
		action_ptr action = highlighter_->get_bump_target();
		if (action)
		{
			viewer_actions()->bump_later(viewer_actions()->get_position_of(action));
		}
	}
}

void manager::erase_all_actions()
{
	on_deselect_hex();
	set_real_unit_map();
	foreach(team& team, *resources::teams)
	{
		team.get_side_actions()->clear();
	}
}

bool manager::unit_has_actions(const unit& unit) const
{
	if (viewer_actions()->empty())
		return false;
	else
		return viewer_actions()->find_first_action_of(unit)
				!= viewer_actions()->end();
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
:has_planned_unit_map_(resources::whiteboard->has_planned_unit_map())
{
	if (!has_planned_unit_map_)
		resources::whiteboard->set_planned_unit_map();
}

scoped_planned_unit_map::~scoped_planned_unit_map()
{
	if (!has_planned_unit_map_)
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
