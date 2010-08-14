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
 * @file
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
#include "key.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit_display.hpp"

#include <sstream>

namespace wb {

manager::manager():
		active_(false),
		inverted_behavior_(false),
		print_help_once_(true),
		wait_for_side_init_(true),
		planned_unit_map_active_(false),
		is_map_for_pathfinding_(false),
		executing_actions_(false),
		gamestate_mutated_(false),
		mapbuilder_(),
		highlighter_(new highlight_visitor(*resources::units, viewer_actions())),
		route_(),
		move_arrow_(),
		fake_unit_(),
		key_poller_(new CKey),
		hidden_unit_hex_()
{
	LOG_WB << "Manager initialized.\n";
}

manager::~manager()
{
	LOG_WB << "Manager destroyed.\n";
}

static void print_to_chat(const std::string& title, const std::string& message)
{
	resources::screen->add_chat_message(time(NULL), title, 0, message,
			events::chat_handler::MESSAGE_PRIVATE, false);
}

void manager::print_help_once()
{
	if (!print_help_once_)
		return;
	else
		print_help_once_ = false;

	print_to_chat("whiteboard", std::string("Type :wb to activate/deactivate planning mode.")
		+ "  Hold TAB to temporarily deactivate/activate it.");
	std::stringstream hotkeys;
	const hotkey::hotkey_item& hk_execute = hotkey::get_hotkey(hotkey::HOTKEY_WB_EXECUTE_ACTION);
	if(!hk_execute.null()) {
		//print_to_chat("[execute action]", "'" + hk_execute.get_name() + "'");
		hotkeys << "Execute: " << hk_execute.get_name() << ", ";
	}
	const hotkey::hotkey_item& hk_delete = hotkey::get_hotkey(hotkey::HOTKEY_WB_DELETE_ACTION);
	if(!hk_delete.null()) {
		//print_to_chat("[delete action]", "'" + hk_delete.get_name() + "'");
		hotkeys << "Delete: " << hk_delete.get_name() << ", ";
	}
	const hotkey::hotkey_item& hk_bump_up = hotkey::get_hotkey(hotkey::HOTKEY_WB_BUMP_UP_ACTION);
	if(!hk_bump_up.null()) {
		//print_to_chat("[move action earlier in queue]", "'" + hk_bump_up.get_name() + "'");
		hotkeys << "Move earlier: " << hk_bump_up.get_name() << ", ";
	}
	const hotkey::hotkey_item& hk_bump_down = hotkey::get_hotkey(hotkey::HOTKEY_WB_BUMP_DOWN_ACTION);
	if(!hk_bump_down.null()) {
		//print_to_chat("[move action later in queue]", "'" + hk_bump_down.get_name() + "'");
		hotkeys << "Move later: " << hk_bump_down.get_name() << ", ";
	}
	print_to_chat("HOTKEYS:", hotkeys.str() + "\n");
}

void manager::set_active(bool active)
{
	if(wait_for_side_init_
				|| executing_actions_
				|| is_observer()
				|| resources::controller->is_linger_mode())
	{
		active_ = false;
		LOG_WB << "Whiteboard can't be activated now.\n";
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
	bool block_whiteboard_activation = false;
	if(wait_for_side_init_
			|| executing_actions_
			|| is_observer()
			|| resources::controller->is_linger_mode())
	{
		 block_whiteboard_activation = true;
	}

	if (invert)
	{
		if (!inverted_behavior_)
		{
			if (active_)
			{
				DBG_WB << "Whiteboard deactivated temporarily.\n";
				inverted_behavior_ = true;
				set_active(false);
			}
			else if (!block_whiteboard_activation)
			{
				DBG_WB << "Whiteboard activated temporarily.\n";
				inverted_behavior_ = true;
				set_active(true);
			}
		}
	}
	else
	{
		if (inverted_behavior_)
		{
			if (active_)
			{
				DBG_WB << "Whiteboard set back to deactivated status.\n";
				inverted_behavior_ = false;
				set_active(false);
			}
			else if (!block_whiteboard_activation)
			{
				DBG_WB << "Whiteboard set back to activated status.\n";
				inverted_behavior_ = false;
				set_active(true);
			}
		}
	}
}

bool manager::can_execute_hotkey() const
{
	return !resources::controller->is_linger_mode() && !viewer_actions()->empty();
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

side_actions_ptr manager::viewer_actions()
{
	side_actions_ptr side_actions =
			(*resources::teams)[resources::screen->viewing_team()].get_side_actions();
	return side_actions;
}

side_actions_ptr manager::current_side_actions()
{
	side_actions_ptr side_actions =
			(*resources::teams)[resources::controller->current_side() - 1].get_side_actions();
	return side_actions;
}

bool manager::current_side_has_actions()
{
	return !current_side_actions()->empty();
}

void manager::validate_viewer_actions()
{
	assert(!executing_actions_);
	if (viewer_actions()->empty()) return;
	viewer_actions()->validate_actions();
}

void manager::set_planned_unit_map(bool for_pathfinding)
{
	if (!executing_actions_ && !wait_for_side_init_)
	{
		if (!planned_unit_map_active_)
		{
			if (!viewer_actions()->empty())
			{
				validate_actions_if_needed();
				std::string message;
				for_pathfinding ? message = "Building planned unit map for pathfinding."
						: message = "Building planned unit map";
				log_scope2("whiteboard", message);
				is_map_for_pathfinding_ = for_pathfinding;
				mapbuilder_.reset(new mapbuilder_visitor(*resources::units, viewer_actions(), for_pathfinding));
				mapbuilder_->build_map();
			}
			planned_unit_map_active_ = true;
		}
		else
		{
			LOG_WB << "Attempt to set planned unit map when it was already set.\n";
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
			if (mapbuilder_)
			{
				log_scope2("whiteboard", "Restoring regular unit map.");
				mapbuilder_.reset();
			}
			planned_unit_map_active_ = false;
			is_map_for_pathfinding_ = false;
		}
		else if (!wait_for_side_init_)
		{
			LOG_WB << "Attempt to disable the planned unit map, when it was already disabled.\n";
		}
	}
	else //executing_actions_
	{
		LOG_WB << "Attempt to set planned_unit_map during action execution.\n";
	}
}

unit* manager::future_visible_unit(map_location hex, int viewer_side)
{
	scoped_planned_unit_map planned_unit_map;
	assert(resources::whiteboard->has_planned_unit_map());
	//use global method get_visible_unit
	return get_visible_unit(hex, resources::teams->at(viewer_side - 1), false);
}

unit* manager::future_visible_unit(int on_side, map_location hex, int viewer_side)
{
	unit* unit = future_visible_unit(hex, viewer_side);
	if (unit && unit->side() == on_side)
		return unit;
	else
		return NULL;
}

void manager::draw_hex(const map_location& hex)
{
	if (!wait_for_side_init_)
	{
		viewer_actions()->draw_hex(hex);
	}

	//Little hack to make the TAB key work properly: check at every draw if it's pressed,
	//to compensate for faulty detection of the "up" key event
	if(!(*key_poller_)[SDLK_TAB])
	{
		set_invert_behavior(false);
	}

}

void manager::on_mouseover_change(const map_location& hex)
{
	if (hidden_unit_hex_.valid())
	{
		resources::screen->remove_exclusive_draw(hidden_unit_hex_);
		hidden_unit_hex_ = map_location();
	}

	if (!resources::screen->selected_hex().valid() && highlighter_)
	{
		highlighter_->set_mouseover_hex(hex);
		highlighter_->highlight();
	}
}

void manager::on_gamestate_change()
{
	DBG_WB << "Manager received gamestate change notification.\n";
	gamestate_mutated_ = true;
}

void manager::create_temp_move()
{
	route_.reset();

	if (!active_ || resources::controller->is_linger_mode()) return;

	/*
	 * CHECK PRE-CONDITIONS
	 * (This section has multiple return paths.)
	 */

	pathfind::marked_route const& route =
			resources::controller->get_mouse_handler_base().get_current_route();

	if (route.steps.empty() || route.steps.size() < 2) return;

	unit const* selected_unit = future_visible_unit(resources::screen->selected_hex(), viewer_side());
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

	//@todo: May be appropriate to replace these separate components by a temporary
	//      wb::move object

	route_.reset(new pathfind::marked_route(route));
	//NOTE: route_.steps.back() = dst, and route_.steps.front() = src

	if (!move_arrow_)
	{
		// Create temp arrow
		move_arrow_.reset(new arrow());
		move_arrow_->set_color(team::get_side_color_index(
				viewer_side()));
		move_arrow_->set_style(arrow::STYLE_HIGHLIGHTED);
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

	//if destination is over another unit, temporarily hide it
	resources::screen->add_exclusive_draw(fake_unit_->get_location(), *fake_unit_);
	hidden_unit_hex_ = fake_unit_->get_location();
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
	if (has_temp_move() && !executing_actions_ && !resources::controller->is_linger_mode())
	{
		arrow_ptr move_arrow;
		fake_unit_ptr fake_unit;

		move_arrow = arrow_ptr(move_arrow_);
		fake_unit = fake_unit_ptr(fake_unit_);

		fake_unit->set_disabled_ghosted(false);
		viewer_actions()->queue_move(*route_, move_arrow, fake_unit);
		erase_temp_move();
		LOG_WB << *viewer_actions() << "\n";

		print_help_once();
	}
}

void manager::save_temp_attack(const map_location& attack_from, const map_location& target_hex)
{
	if (active_ && !executing_actions_ && !resources::controller->is_linger_mode())
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

		unit const* attacking_unit = future_visible_unit(source_hex);
		assert(attacking_unit);

		int weapon_choice = resources::controller->get_mouse_handler_base().show_attack_dialog(
					attacking_unit->get_location(), target_hex);

		if (weapon_choice >= 0)
		{
			viewer_actions()->queue_attack(target_hex, weapon_choice, *route_, move_arrow, fake_unit);

			print_help_once();
		}

		resources::screen->invalidate(target_hex);
		erase_temp_move();
		LOG_WB << *viewer_actions() << "\n";
	}
}

bool manager::save_recruit(const std::string& name, int side_num, const map_location& recruit_hex)
{
	bool created_planned_recruit = false;

	if (active_ && !executing_actions_ && !resources::controller->is_linger_mode()) {
		if (side_num != resources::screen->viewing_side())
		{
			LOG_WB <<"manager::save_recruit called for a different side than viewing side.\n";
			created_planned_recruit = false;
		}
		else
		{
			viewer_actions()->queue_recruit(name, recruit_hex);
			created_planned_recruit = true;

			print_help_once();
		}
	}
	return created_planned_recruit;
}

bool manager::save_recall(const unit& unit, int side_num, const map_location& recall_hex)
{
	bool created_planned_recall = false;

	if (active_ && !executing_actions_ && !resources::controller->is_linger_mode())
	{
		if (side_num != resources::screen->viewing_side())
		{
			LOG_WB <<"manager::save_recall called for a different side than viewing side.\n";
			created_planned_recall = false;
		}
		else
		{
			viewer_actions()->queue_recall(unit, recall_hex);
			created_planned_recall = true;
			print_help_once();
		}
	}
	return created_planned_recall;
}

void manager::contextual_execute()
{
	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode())
			&& resources::controller->current_side() == resources::screen->viewing_side())
	{
		erase_temp_move();
		validate_viewer_actions();

		action_ptr action;
		side_actions::iterator it;
		unit const* selected_unit = future_visible_unit(resources::screen->selected_hex(), viewer_side());
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
	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode()))
	{
		erase_temp_move();
		validate_viewer_actions();

		action_ptr action;
		side_actions::iterator it;
		unit const* selected_unit = future_visible_unit(resources::screen->selected_hex(), viewer_side());
		if (selected_unit &&
				(it = viewer_actions()->find_first_action_of(*selected_unit)) != viewer_actions()->end())
		{
			viewer_actions()->remove_action(it);
		}
		else if (highlighter_ && (action = highlighter_->get_delete_target()) &&
				(it = viewer_actions()->get_position_of(action)) != viewer_actions()->end())
		{
			viewer_actions()->remove_action(it);
			highlighter_->set_mouseover_hex(highlighter_->get_mouseover_hex());
			highlighter_->highlight();
		}
		else //we already check above for viewer_actions()->empty()
		{
			viewer_actions()->remove_action(viewer_actions()->end() - 1);
		}
	}
}

void manager::contextual_bump_up_action()
{
	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode())
			&& highlighter_)
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
	if (!(executing_actions_ || viewer_actions()->empty() || resources::controller->is_linger_mode())
			&& highlighter_)
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
	return viewer_actions()->unit_has_actions(unit);
}

int manager::get_spent_gold_for(int side)
{
	return resources::teams->at(side - 1).get_side_actions()->get_gold_spent();
}

void manager::fake_unit_deleter::operator() (unit*& fake_unit)
{
    if (fake_unit)
    {
        if(resources::screen)
        {
        	resources::screen->remove_temporary_unit(fake_unit);
        }
        DBG_WB << "Erasing temporary unit " << fake_unit->name() << " [" << fake_unit->id() << "]\n";
        delete fake_unit;
    }
}

void manager::validate_actions_if_needed()
{
	if (gamestate_mutated_)
	{
		LOG_WB << "'gamestate_mutated_' flag dirty, validating actions.\n";
		validate_viewer_actions();
	}
	gamestate_mutated_ = false;
}


size_t manager::viewer_team()
{
	return resources::screen->viewing_team();
}
int manager::viewer_side()
{
	return resources::screen->viewing_side();
}


scoped_planned_unit_map::scoped_planned_unit_map():
		has_planned_unit_map_(resources::whiteboard->has_planned_unit_map()),
		is_map_for_pathfinding_(resources::whiteboard->is_map_for_pathfinding())
{
	if (!has_planned_unit_map_)
	{
		resources::whiteboard->set_planned_unit_map();
	}
	else if (is_map_for_pathfinding_)
	{
		resources::whiteboard->set_real_unit_map();
		resources::whiteboard->set_planned_unit_map();
	}
}

scoped_planned_unit_map::~scoped_planned_unit_map()
{
	if (!has_planned_unit_map_)
	{
		resources::whiteboard->set_real_unit_map();
	}
	else if (is_map_for_pathfinding_ != resources::whiteboard->is_map_for_pathfinding())
	{
		resources::whiteboard->set_real_unit_map();
		resources::whiteboard->set_planned_unit_map(is_map_for_pathfinding_);
	}
}

scoped_real_unit_map::scoped_real_unit_map():
		has_planned_unit_map_(resources::whiteboard->has_planned_unit_map()),
		is_map_for_pathfinding_(resources::whiteboard->is_map_for_pathfinding())
{
	if (has_planned_unit_map_)
		resources::whiteboard->set_real_unit_map();
}

scoped_real_unit_map::~scoped_real_unit_map()
{
	if (has_planned_unit_map_ &&
			(!resources::whiteboard->has_planned_unit_map() ||
			is_map_for_pathfinding_ != resources::whiteboard->is_map_for_pathfinding()))
		resources::whiteboard->set_planned_unit_map(is_map_for_pathfinding_);
}

scoped_planned_pathfind_map::scoped_planned_pathfind_map():
		has_planned_unit_map_(resources::whiteboard->has_planned_unit_map()),
		is_map_for_pathfinding_(resources::whiteboard->is_map_for_pathfinding())
{
	if (!has_planned_unit_map_)
	{
		resources::whiteboard->set_planned_unit_map(true);
	}
	else if (!is_map_for_pathfinding_)
	{
		resources::whiteboard->set_real_unit_map();
		resources::whiteboard->set_planned_unit_map(true);
	}
}

scoped_planned_pathfind_map::~scoped_planned_pathfind_map()
{
	if (has_planned_unit_map_ && is_map_for_pathfinding_)
	{
	}
	else
	{
		resources::whiteboard->set_real_unit_map();
		if (has_planned_unit_map_)
		{
			resources::whiteboard->set_planned_unit_map(false);
		}
	}
}

bool unit_comparator_predicate::operator()(unit const& unit)
{
	return unit_.id() == unit.id();
}

} // end namespace wb
