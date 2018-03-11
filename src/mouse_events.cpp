/*
   Copyright (C) 2006 - 2018 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mouse_events.hpp"

#include "actions/attack.hpp"                // for battle_context, etc
#include "actions/move.hpp"                  // for move_and_record
#include "actions/undo.hpp"                  // for undo_list
#include "config.hpp"                        // for config
#include "cursor.hpp"                        // for set, CURSOR_TYPE::NORMAL, etc
#include "game_board.hpp"                    // for game_board, etc
#include "game_config.hpp"                   // for red_to_green
#include "game_events/pump.hpp"              // for fire
#include "gettext.hpp"                       // for _
#include "gui/dialogs/transient_message.hpp" // for show_transient_message
#include "gui/dialogs/unit_attack.hpp"       // for unit_attack
#include "gui/widgets/settings.hpp"          // for new_widgets
#include "gui/widgets/window.hpp"            // for enum
#include "language.hpp"                      // for string_table, symbol_table
#include "log.hpp"                           // for LOG_STREAM, logger, etc
#include "map/map.hpp"                       // for gamemap
#include "pathfind/teleport.hpp"             // for get_teleport_locations, etc
#include "play_controller.hpp"               // for playing_side, set_button_state
#include "replay_helper.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "sound.hpp"
#include "synced_context.hpp"
#include "team.hpp" // for team
#include "tod_manager.hpp"
#include "units/animation_component.hpp"
#include "units/ptr.hpp"           // for unit_const_ptr
#include "units/unit.hpp"          // for unit
#include "whiteboard/manager.hpp"  // for manager, etc
#include "whiteboard/typedefs.hpp" // for whiteboard_lock

#include <SDL_mouse.h> // for SDL_GetMouseState
#include <cassert>     // for assert
#include <new>         // for bad_alloc
#include <ostream>     // for operator<<, basic_ostream, etc
#include <string>      // for string, operator<<, etc

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace events
{
mouse_handler::mouse_handler(game_display* gui, play_controller& pc)
	: mouse_handler_base()
	, gui_(gui)
	, pc_(pc)
	, previous_hex_()
	, previous_free_hex_()
	, selected_hex_()
	, next_unit_()
	, current_route_()
	, current_paths_()
	, unselected_paths_(false)
	, unselected_reach_(false)
	, path_turns_(0)
	, side_num_(1)
	, over_route_(false)
	, reachmap_invalid_(false)
	, show_partial_move_(false)
	, preventing_units_highlight_(false)
{
	singleton_ = this;
}

mouse_handler::~mouse_handler()
{
	singleton_ = nullptr;
}

void mouse_handler::set_side(int side_number)
{
	side_num_ = side_number;
}

int mouse_handler::drag_threshold() const
{
	return 14;
}

void mouse_handler::mouse_motion(int x, int y, const bool browse, bool update, map_location new_hex)
{
	// we ignore the position coming from event handler
	// because it's always a little obsolete and we don't need
	// to highlight all the hexes where the mouse passed.
	// Also, sometimes it seems to have one *very* obsolete
	// and isolated mouse motion event when using drag&drop
	SDL_GetMouseState(&x, &y); // <-- modify x and y

	if(mouse_handler_base::mouse_motion_default(x, y, update)) {
		return;
	}

	game_board& board = pc_.gamestate().board_;

	if(new_hex == map_location::null_location()) {
		new_hex = gui().hex_clicked_on(x, y);
	}

	if(new_hex != last_hex_) {
		if(game_lua_kernel* lk = pc_.gamestate().lua_kernel_.get()) {
			lk->mouse_over_hex_callback(new_hex);
		}

		update = true;

		if(pc_.get_map_const().on_board(last_hex_)) {
			// we store the previous hexes used to propose attack direction
			previous_hex_ = last_hex_;

			// the hex of the selected unit is also "free"
			{ // start planned unit map scope
				wb::future_map_if_active raii;
				if(last_hex_ == selected_hex_ || !find_unit(last_hex_)) {
					previous_free_hex_ = last_hex_;
				}
			} // end planned unit map scope
		}

		last_hex_ = new_hex;
	}

	if(reachmap_invalid_) {
		update = true;
	}

	if(!update) {
		return;
	}

	if(reachmap_invalid_) {
		reachmap_invalid_ = false;

		if(!current_paths_.destinations.empty() && !show_partial_move_) {
			bool selected_hex_has_unit;
			{ // start planned unit map scope
				wb::future_map_if_active planned_unit_map;
				selected_hex_has_unit = find_unit(selected_hex_).valid();
			} // end planned unit map scope

			if(selected_hex_.valid() && selected_hex_has_unit) {
				// reselect the unit without firing events (updates current_paths_)
				select_hex(selected_hex_, true);
			}

			// we do never deselect here, mainly because of canceled attack-move
		}
	}

	// reset current_route_ and current_paths if not valid anymore
	// we do it before cursor selection, because it uses current_paths_
	if(!pc_.get_map_const().on_board(new_hex)) {
		current_route_.steps.clear();
		gui().set_route(nullptr);
		pc_.get_whiteboard()->erase_temp_move();
	}

	if(unselected_paths_) {
		unselected_paths_ = false;
		current_paths_ = pathfind::paths();
		gui().unhighlight_reach();
	} else if(over_route_) {
		over_route_ = false;
		current_route_.steps.clear();
		gui().set_route(nullptr);
		pc_.get_whiteboard()->erase_temp_move();
	}

	gui().highlight_hex(new_hex);
	pc_.get_whiteboard()->on_mouseover_change(new_hex);

	unit_map::iterator selected_unit;
	unit_map::iterator mouseover_unit;
	map_location attack_from;

	{ // start planned unit map scope
		wb::future_map_if_active planned_unit_map;
		selected_unit = find_unit(selected_hex_);
		mouseover_unit = find_unit(new_hex);

		// we search if there is an attack possibility and where
		attack_from = current_unit_attacks_from(new_hex);

		// see if we should show the normal cursor, the movement cursor, or
		// the attack cursor
		// If the cursor is on WAIT, we don't change it and let the setter
		// of this state end it
		if(cursor::get() != cursor::WAIT) {
			if(selected_unit && selected_unit->side() == side_num_ && !selected_unit->incapacitated() && !browse) {
				if(attack_from.valid()) {
					cursor::set(dragging_started_ ? cursor::ATTACK_DRAG : cursor::ATTACK);
				} else if(!mouseover_unit && current_paths_.destinations.contains(new_hex)) {
					cursor::set(dragging_started_ ? cursor::MOVE_DRAG : cursor::MOVE);
				} else {
					// selected unit can't attack or move there
					cursor::set(cursor::NORMAL);
				}
			} else {
				// no selected unit or we can't move it

				if(selected_hex_.valid() && mouseover_unit && mouseover_unit->side() == side_num_) {
					// empty hex field selected and unit on our site under the cursor
					cursor::set(dragging_started_ ? cursor::MOVE_DRAG : cursor::MOVE);
				} else {
					cursor::set(cursor::NORMAL);
				}
			}
		}
	} // end planned unit map scope

	// show (or cancel) the attack direction indicator
	if(attack_from.valid() && (!browse || pc_.get_whiteboard()->is_active())) {
		gui().set_attack_indicator(attack_from, new_hex);
	} else {
		gui().clear_attack_indicator();
	}

	unit_ptr un; // will later point to unit at mouseover_hex_

	// the destination is the pointed hex or the adjacent hex
	// used to attack it
	map_location dest;
	unit_map::const_iterator dest_un;

	{ // start planned unit map scope
		wb::future_map_if_active raii;
		if(attack_from.valid()) {
			dest = attack_from;
			dest_un = find_unit(dest);
		} else {
			dest = new_hex;
			dest_un = find_unit(new_hex);
		}

		if(dest == selected_hex_ || dest_un) {
			current_route_.steps.clear();
			gui().set_route(nullptr);
			pc_.get_whiteboard()->erase_temp_move();
		} else if(!current_paths_.destinations.empty() && board.map().on_board(selected_hex_)
				  && board.map().on_board(new_hex)) {
			if(selected_unit && !selected_unit->incapacitated()) {
				// Show the route from selected unit to mouseover hex
				current_route_ = get_route(&*selected_unit, dest, viewing_team());

				pc_.get_whiteboard()->create_temp_move();

				if(!browse) {
					gui().set_route(&current_route_);
				}
			}
		}

		if(board.map().on_board(selected_hex_) && !selected_unit && mouseover_unit.valid() && mouseover_unit) {
			// Show the route from selected hex to mouseover unit
			current_route_ = get_route(&*mouseover_unit, selected_hex_, viewing_team());

			pc_.get_whiteboard()->create_temp_move();

			if(!browse) {
				gui().set_route(&current_route_);
			}
		} else if(!selected_unit) {
			current_route_.steps.clear();
			gui().set_route(nullptr);
			pc_.get_whiteboard()->erase_temp_move();
		}

		unit_map::iterator iter = mouseover_unit;
		if(iter) {
			un = iter.get_shared_ptr();
		} else {
			un.reset();
		}
	} // end planned unit map scope

	if((!selected_hex_.valid()) && un && current_paths_.destinations.empty() && !gui().fogged(un->get_location())) {
		/*
		 * Only process unit if toggler not preventing normal unit
		 * processing. This can happen e.g. if, after activating 'show
		 * [best possible] enemy movements' through the UI menu, the
		 * mouse cursor lands on a hex with unit in it.
		 */
		if(!preventing_units_highlight_) {
			if(un->side() == side_num_) {
				// unit is on our team, show path if the unit has one
				const map_location go_to = un->get_goto();
				if(board.map().on_board(go_to)) {
					pathfind::marked_route route;
					{ // start planned unit map scope
						wb::future_map_if_active raii;
						route = get_route(un.get(), go_to, current_team());
					} // end planned unit map scope
					gui().set_route(&route);
				}
				over_route_ = true;

				wb::future_map_if_active raii;
				current_paths_ = pathfind::paths(*un, false, true, viewing_team(), path_turns_);
			} else {
				// unit under cursor is not on our team
				// Note: planned unit map must be activated after this is done,
				// since the future state includes changes to units' movement.
				unit_movement_resetter move_reset(*un);

				wb::future_map_if_active raii;
				current_paths_ = pathfind::paths(*un, false, true, viewing_team(), path_turns_);
			}

			unselected_paths_ = true;
			gui().highlight_reach(current_paths_);

		}
	}

	if(!un && preventing_units_highlight_) {
		// Cursor on empty hex, turn unit highlighting back on.
		enable_units_highlight();
	}
}

unit_map::iterator mouse_handler::selected_unit()
{
	unit_map::iterator res = find_unit(selected_hex_);
	if(res) {
		return res;
	}

	return find_unit(last_hex_);
}

unit_map::iterator mouse_handler::find_unit(const map_location& hex)
{
	unit_map::iterator it = pc_.gamestate().board_.find_visible_unit(hex, viewing_team());
	if(it.valid()) {
		return it;
	}

	return pc_.gamestate().board_.units_.end();
}

unit_map::const_iterator mouse_handler::find_unit(const map_location& hex) const
{
	return pc_.gamestate().board_.find_visible_unit(hex, viewing_team());
}

const map_location mouse_handler::hovered_hex() const
{
	int x = -1;
	int y = -1;
	SDL_GetMouseState(&x, &y);
	return gui_->hex_clicked_on(x, y);
}

bool mouse_handler::hex_hosts_unit(const map_location& hex) const
{
	return find_unit(hex).valid();
}

map_location mouse_handler::current_unit_attacks_from(const map_location& loc) const
{
	if(loc == selected_hex_) {
		return map_location();
	}

	bool wb_active = pc_.get_whiteboard()->is_active();

	{
		// Check the unit SOURCE of the attack

		// Check that there's a selected unit
		const unit_map::const_iterator source_unit = find_unit(selected_hex_);

		bool source_eligible = source_unit.valid();
		if(!source_eligible) {
			return map_location();
		}

		// The selected unit must at least belong to the player currently controlling this client.
		source_eligible &= source_unit->side() == gui_->viewing_side();
		if(!source_eligible) {
			return map_location();
		}

		// In addition:
		// - If whiteboard is enabled, we allow planning attacks outside of player's turn
		// - If whiteboard is disabled, it must be the turn of the player controlling this client
		if(!wb_active) {
			source_eligible &= gui_->viewing_side() == pc_.current_side();
			if(!source_eligible) {
				return map_location();
			}
		}

		// Unit must have attacks left
		source_eligible &= source_unit->attacks_left() != 0;
		if(!source_eligible) {
			return map_location();
		}

		// Check the unit TARGET of the attack

		const team& viewer = viewing_team();

		// Check that there's a unit at the target location
		const unit_map::const_iterator target_unit = find_unit(loc);

		bool target_eligible = target_unit.valid();
		if(!target_eligible) {
			return map_location();
		}

		// The player controlling this client must be an enemy of the target unit's side
		target_eligible &= viewer.is_enemy(target_unit->side());
		if(!target_eligible) {
			return map_location();
		}

		// Sanity check: source and target of the attack shouldn't be on the same team
		assert(source_unit->side() != target_unit->side());

		target_eligible &= !target_unit->incapacitated();
		if(!target_eligible) {
			return map_location();
		}
	}

	const map_location::DIRECTION preferred = loc.get_relative_dir(previous_hex_);
	const map_location::DIRECTION second_preferred = loc.get_relative_dir(previous_free_hex_);

	int best_rating = 100; // smaller is better

	map_location res;
	adjacent_loc_array_t adj;
	get_adjacent_tiles(loc, adj.data());

	for(size_t n = 0; n < adj.size(); ++n) {
		if(pc_.gamestate().board_.map().on_board(adj[n]) == false) {
			continue;
		}

		if(adj[n] != selected_hex_ && find_unit(adj[n])) {
			continue;
		}

		if(current_paths_.destinations.contains(adj[n])) {
			static const size_t NDIRECTIONS = map_location::NDIRECTIONS;

			unsigned int difference = std::abs(int(preferred - n));
			if(difference > NDIRECTIONS / 2) {
				difference = NDIRECTIONS - difference;
			}

			unsigned int second_difference = std::abs(int(second_preferred - n));
			if(second_difference > NDIRECTIONS / 2) {
				second_difference = NDIRECTIONS - second_difference;
			}

			const int rating = difference * 2 + (second_difference > difference);
			if(rating < best_rating || res.valid() == false) {
				best_rating = rating;
				res = adj[n];
			}
		}
	}

	return res;
}

pathfind::marked_route mouse_handler::get_route(const unit* un, map_location go_to, team& team) const
{
	game_board& board = pc_.gamestate().board_;

	// The pathfinder will check unit visibility (fogged/stealthy).
	const pathfind::shortest_path_calculator calc(*un, team, board.teams(), board.map());

	pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*un, viewing_team());

	pathfind::plain_route route;

	route = pathfind::a_star_search(
		un->get_location(), go_to, 10000.0, calc, board.map().w(), board.map().h(), &allowed_teleports);

	return mark_route(route);
}

void mouse_handler::mouse_press(const SDL_MouseButtonEvent& event, const bool browse)
{
	mouse_handler_base::mouse_press(event, browse);
}

bool mouse_handler::right_click_show_menu(int x, int y, const bool /*browse*/)
{
	if(selected_hex_.valid() || unselected_reach_) {
		unselected_reach_ = false;
		return false;
	}

	return sdl::point_in_rect(x, y, gui().map_area());
}

void mouse_handler::select_or_action(bool browse)
{
	if(!pc_.get_map_const().on_board(last_hex_)) {
		tooltips::click(drag_from_x_, drag_from_y_);
		return;
	}

	// Load whiteboard partial moves
	wb::future_map_if_active planned_unit_map;

	if(game_lua_kernel* lk = pc_.gamestate().lua_kernel_.get()) {
		lk->select_hex_callback(last_hex_);
	}

	unit_map::iterator clicked_u = find_unit(last_hex_);
	unit_map::iterator selected_u = find_unit(selected_hex_);

	if(clicked_u && (!selected_u || selected_u->side() != side_num_ ||
	  (clicked_u->side() == side_num_ && clicked_u->id() != selected_u->id()))
	) {
		select_hex(last_hex_, false);
	} else {
		move_action(browse);
	}
}

void mouse_handler::move_action(bool browse)
{
	// Lock whiteboard activation state to avoid problems due to
	// its changing while an animation takes place.
	wb::whiteboard_lock wb_lock = pc_.get_whiteboard()->get_activation_state_lock();

	// we use the last registered highlighted hex
	// since it's what update our global state
	map_location hex = last_hex_;

	// TODO
	//	// Clicks on border hexes mean to deselect.
	//	// (Check this before doing processing that might not be needed.)
	//	if ( !pc_.get_map_const().on_board(hex) ) {
	//		deselect_hex();
	//		return false;
	//	}

	unit_map::iterator u;
	unit_map::iterator clicked_u;

	map_location src;
	pathfind::paths orig_paths;
	map_location attack_from;

	{ // start planned unit map scope
		wb::future_map_if_active planned_unit_map;
		u = find_unit(selected_hex_);

		// if the unit is selected and then itself clicked on,
		// any goto command is canceled
		if(u && !browse && selected_hex_ == hex && u->side() == side_num_) {
			u->set_goto(map_location());
		}

		clicked_u = find_unit(hex);

		src = selected_hex_;
		orig_paths = current_paths_;
		attack_from = current_unit_attacks_from(hex);
	} // end planned unit map scope

	// see if we're trying to do a attack or move-and-attack
	if((!browse || pc_.get_whiteboard()->is_active()) && attack_from.valid()) {
		// Ignore this command if commands are disabled.
		if(commands_disabled) {
			return;
		}

		if(((u.valid() && u->side() == side_num_) || pc_.get_whiteboard()->is_active()) && clicked_u.valid()) {
			if(attack_from == selected_hex_) { // no move needed
				int choice = -1;
				{
					wb::future_map_if_active planned_unit_map; // start planned unit map scope
					choice = show_attack_dialog(attack_from, clicked_u->get_location());
				} // end planned unit map scope

				if(choice >= 0) {
					if(pc_.get_whiteboard()->is_active()) {
						save_whiteboard_attack(attack_from, clicked_u->get_location(), choice);
					} else {
						// clear current unit selection so that any other unit selected
						// triggers a new selection
						selected_hex_ = map_location();

						attack_enemy(u->get_location(), clicked_u->get_location(), choice);
					}
				}

				return;
			} else {
				int choice = -1; // for the attack dialog

				{
					wb::future_map_if_active planned_unit_map; // start planned unit map scope
					// we will now temporary move next to the enemy
					pathfind::paths::dest_vect::const_iterator itor = current_paths_.destinations.find(attack_from);
					if(itor == current_paths_.destinations.end()) {
						// can't reach the attacking location
						// not supposed to happen, so abort
						return;
					}

					// block where we temporary move the unit
					{
						temporary_unit_mover temp_mover(
								pc_.gamestate().board_.units_, src, attack_from, itor->move_left);
						choice = show_attack_dialog(attack_from, clicked_u->get_location());
					}

					if(choice < 0) {
						// user hit cancel, don't start move+attack
						return;
					}
				} // end planned unit map scope

				if(pc_.get_whiteboard()->is_active()) {
					save_whiteboard_attack(attack_from, hex, choice);
				} else {
					bool not_interrupted = move_unit_along_current_route();
					bool alt_unit_selected = (selected_hex_ != src);
					src = selected_hex_;
					// clear current unit selection so that any other unit selected
					// triggers a new selection
					selected_hex_ = map_location();

					if(not_interrupted)
						attack_enemy(attack_from, hex, choice); // Fight !!

					// TODO: Maybe store the attack choice so "press t to continue"
					//      can also continue the attack?

					if(alt_unit_selected && !selected_hex_.valid()) {
						// reselect other unit if selected during movement animation
						select_hex(src, browse);
					}
				}

				return;
			}
		}
	}
	// otherwise we're trying to move to a hex
	else if(
		// The old use case: move selected unit to mouse hex field.
		(
			(!browse || pc_.get_whiteboard()->is_active())
			&& selected_hex_.valid()
			&& selected_hex_ != hex
			&& u.valid()
			&& (u->side() == side_num_ || pc_.get_whiteboard()->is_active())
			&& !clicked_u
			&& !current_route_.steps.empty()
			&& current_route_.steps.front() == selected_hex_
		)
		|| // The new use case: move mouse unit to selected hex field.
		(
			(!browse || pc_.get_whiteboard()->is_active())
			&& selected_hex_.valid()
			&& selected_hex_ != hex
			&& clicked_u
			&& !current_route_.steps.empty()
			&& current_route_.steps.back() == selected_hex_
			&& !u
			&& clicked_u->side() == side_num_
		)
	) {
		// Ignore this command if commands are disabled.
		if(commands_disabled) {
			return;
		}

		// If the whiteboard is active, it intercepts any unit movement.
		if(pc_.get_whiteboard()->is_active()) {
			// Deselect the current hex, and create planned move for whiteboard.
			selected_hex_ = map_location();

			gui().select_hex(map_location());
			gui().clear_attack_indicator();
			gui().set_route(nullptr);

			show_partial_move_ = false;

			gui().unhighlight_reach();

			current_paths_ = pathfind::paths();
			current_route_.steps.clear();

			pc_.get_whiteboard()->save_temp_move();

			// Otherwise proceed to normal unit movement
		} else {
			// Don't move if the unit already has actions
			// from the whiteboard.
			if(pc_.get_whiteboard()->unit_has_actions(u ? &*u : &*clicked_u)) {
				return;
			}

			move_unit_along_current_route();

			// During the move, we may have selected another unit
			// (but without triggering a select event (command was disabled)
			// in that case reselect it now to fire the event (+ anim & sound)
			if(selected_hex_ != src) {
				select_hex(selected_hex_, browse);
			}
		}

		return;
	}
}

void mouse_handler::select_hex(const map_location& hex, const bool browse, const bool highlight, const bool fire_event)
{
	selected_hex_ = hex;

	gui().select_hex(selected_hex_);
	gui().clear_attack_indicator();
	gui().set_route(nullptr);

	show_partial_move_ = false;

	wb::future_map_if_active planned_unit_map; // lasts for whole method

	unit_map::iterator unit = find_unit(selected_hex_);

	if(selected_hex_.valid() && unit.valid() && !unit->get_hidden()) {
		next_unit_ = unit->get_location();

		{
			current_paths_ = pathfind::paths(*unit, false, true, viewing_team(), path_turns_);
		}

		if(highlight) {
			show_attack_options(unit);
			gui().highlight_reach(current_paths_);
		}

		// The highlight now comes from selection
		// and not from the mouseover on an enemy
		unselected_paths_ = false;
		gui().set_route(nullptr);

		// Selection have impact only if we are not observing and it's our unit
		if((!commands_disabled || pc_.get_whiteboard()->is_active()) && unit->side() == gui().viewing_side()) {
			if(!(browse || pc_.get_whiteboard()->unit_has_actions(&*unit))) {
				sound::play_UI_sound("select-unit.wav");

				unit->anim_comp().set_selecting();

				if(fire_event) {
					// Ensure unit map is back to normal while event is fired
					wb::real_map srum;
					pc_.pump().fire("select", hex);
					// end forced real unit map
				}
			}
		}

		return;
	}

	if(selected_hex_.valid() && !unit) {
		// Compute unit in range of the empty selected_hex field

		gui_->unhighlight_reach();

		pathfind::paths reaching_unit_locations;

		pathfind::paths clicked_location;
		clicked_location.destinations.insert(hex);

		for(unit_map::iterator u = pc_.gamestate().board_.units_.begin(); u != pc_.gamestate().board_.units_.end();
				++u) {
			bool invisible = u->invisible(u->get_location(), gui_->get_disp_context());

			if(!gui_->fogged(u->get_location()) && !u->incapacitated() && !invisible) {
				const pathfind::paths& path =
					pathfind::paths(*u, false, true, viewing_team(), path_turns_, false, false);

				if(path.destinations.find(hex) != path.destinations.end()) {
					reaching_unit_locations.destinations.insert(u->get_location());
					gui_->highlight_another_reach(clicked_location);
				}
			}
		}

		gui_->highlight_another_reach(reaching_unit_locations);
	} else {
		if(!pc_.gamestate().board_.units_.find(last_hex_)) {
			unselected_reach_ = gui_->unhighlight_reach();
		}

		current_paths_ = pathfind::paths();
		current_route_.steps.clear();

		pc_.get_whiteboard()->on_deselect_hex();
	}
}

void mouse_handler::deselect_hex()
{
	select_hex(map_location(), true);
}

/**
 * Moves a unit along the currently cached route.
 *
 * @returns  true if the end of the route was reached and no information was
 *           uncovered that would warrant interrupting a chain of actions;
 *           false otherwise.
 */
bool mouse_handler::move_unit_along_current_route()
{
	// Copy the current route to ensure it remains valid throughout the animation.
	const std::vector<map_location> steps = current_route_.steps;

	// do not show footsteps during movement
	gui().set_route(nullptr);
	gui().unhighlight_reach();

	// do not keep the hex highlighted that we started from
	selected_hex_ = map_location();
	gui().select_hex(map_location());

	bool interrupted = false;
	if(steps.size() > 1) {
		size_t num_moves = move_unit_along_route(steps, interrupted);

		interrupted = interrupted || num_moves + 1 < steps.size();
		next_unit_ = steps[num_moves];
	}

	// invalid after the move
	current_paths_ = pathfind::paths();
	current_route_.steps.clear();

	return !interrupted;
}

/**
 * Moves a unit across the board for a player.
 * This is specifically for movement at the time it is initiated by a player,
 * whether via a mouse click or executing whiteboard actions. Continued moves
 * (including goto execution) can bypass this and call actions::move_unit() directly.
 * This function call may include time for an animation, so make sure the
 * provided route will remain unchanged (the caller should probably make a local
 * copy).
 *
 * @param[in]   steps           The route to be traveled. The unit to be moved is at the beginning of this route.
 * @param[out]  interrupted     This is set to true if information was uncovered that warrants interrupting a chain of
 * actions (and set to false otherwise).
 *
 * @returns The number of hexes entered. This can safely be used as an index
 *          into steps to get the location where movement ended, provided
 *          steps is not empty (the return value is guaranteed to be less
 *          than steps.size() ).
 */
size_t mouse_handler::move_unit_along_route(const std::vector<map_location>& steps, bool& interrupted)
{
	if(steps.empty()) {
		interrupted = false;
		return 0;
	}

	// Default return value.
	interrupted = true;

	// If this is a leader on a keep, ask permission to the whiteboard to move it
	// since otherwise it may cause planned recruits to be erased.
	if(pc_.get_map_const().is_keep(steps.front())) {
		unit_map::const_iterator const u = pc_.gamestate().board_.units().find(steps.front());

		if(u && u->can_recruit() && u->side() == gui().viewing_side()
				&& !pc_.get_whiteboard()->allow_leader_to_move(*u)) {
			gui2::show_transient_message("",
					_("You cannot move your leader away from the keep with some planned recruits or recalls left."));
			return 0;
		}
	}

	LOG_NG << "move unit along route  from " << steps.front() << " to " << steps.back() << "\n";
	size_t moves = actions::move_unit_and_record(steps, &pc_.get_undo_stack(), false, true, &interrupted);

	cursor::set(cursor::NORMAL);
	gui().invalidate_game_status();

	if(moves == 0)
		return 0;

	if(interrupted && moves + 1 < steps.size()) {
		// reselect the unit (for "press t to continue")
		select_hex(steps[moves], false, false, false);
		// the new discovery is more important than the new movement range
		show_partial_move_ = true;
	}

	return moves;
}

void mouse_handler::save_whiteboard_attack(
		const map_location& attacker_loc, const map_location& defender_loc, int weapon_choice)
{
	{
		// @todo Fix flickering/reach highlight anomaly after the weapon choice dialog is closed
		// This method should do the cleanup of highlights and selection but it doesn't work properly

		// gui().highlight_hex(map_location());

		gui().unhighlight_reach();
		gui().clear_attack_indicator();

		// remove footsteps if any - useless for whiteboard as of now
		gui().set_route(nullptr);

		// do not keep the hex that we started from highlighted
		selected_hex_ = map_location();
		gui().select_hex(map_location());
		show_partial_move_ = false;

		// invalid after saving the move
		current_paths_ = pathfind::paths();
		current_route_.steps.clear();
	}

	// create planned attack for whiteboard
	pc_.get_whiteboard()->save_temp_attack(attacker_loc, defender_loc, weapon_choice);
}

int mouse_handler::fill_weapon_choices(
		std::vector<battle_context>& bc_vector, unit_map::iterator attacker, unit_map::iterator defender)
{
	int best = 0;
	for(unsigned int i = 0; i < attacker->attacks().size(); i++) {
		// skip weapons with attack_weight=0
		if(attacker->attacks()[i].attack_weight() > 0) {
			battle_context bc(pc_.gamestate().board_.units_, attacker->get_location(), defender->get_location(), i);

			if(!bc_vector.empty() && bc.better_attack(bc_vector[best], 0.5)) {
				// as some weapons can be hidden, i is not a valid index into the resulting vector
				best = bc_vector.size();
			}

			bc_vector.push_back(bc);
		}
	}

	return best;
}

int mouse_handler::show_attack_dialog(const map_location& attacker_loc, const map_location& defender_loc)
{
	game_board& board = pc_.gamestate().board_;

	unit_map::iterator attacker = board.units_.find(attacker_loc);
	unit_map::iterator defender = board.units_.find(defender_loc);

	if(!attacker || !defender) {
		ERR_NG << "One fighter is missing, can't attack";
		return -1; // abort, click will do nothing
	}

	std::vector<battle_context> bc_vector;
	const int best = fill_weapon_choices(bc_vector, attacker, defender);

	const bool all_disabled = std::all_of(bc_vector.begin(), bc_vector.end(),
		[](battle_context& context) { return (*context.get_attacker_stats().weapon).get_special_bool("disable"); }
	);

	if((*attacker).attacks().empty() || all_disabled) {
		gui2::show_transient_message("No Attacks", _("This unit has no usable weapons."));

		return -1;
	}

	gui2::dialogs::unit_attack dlg(attacker, defender, bc_vector, best);

	dlg.show();

	if(dlg.get_retval() == gui2::window::OK) {
		return dlg.get_selected_weapon();
	}

	return -1;
}

void mouse_handler::attack_enemy(const map_location& attacker_loc, const map_location& defender_loc, int choice)
{
	try {
		attack_enemy_(attacker_loc, defender_loc, choice);
	} catch(std::bad_alloc&) {
		lg::wml_error() << "Memory exhausted a unit has either a lot hitpoints or a negative amount.\n";
	}
}

void mouse_handler::attack_enemy_(const map_location& att_loc, const map_location& def_loc, int choice)
{
	// NOTE: copy the values because the const reference may change!
	// (WML events and mouse inputs during animations may modify
	// the data of the caller)
	const map_location attacker_loc = att_loc;
	const map_location defender_loc = def_loc;

	unit_map::iterator attacker = find_unit(attacker_loc);
	if(!attacker || attacker->side() != side_num_ || attacker->incapacitated()) {
		return;
	}

	unit_map::iterator defender = find_unit(defender_loc);
	if(!defender || current_team().is_enemy(defender->side()) == false || defender->incapacitated()) {
		return;
	}

	std::vector<battle_context> bc_vector;
	fill_weapon_choices(bc_vector, attacker, defender);

	if(size_t(choice) >= bc_vector.size()) {
		return;
	}

	events::command_disabler disabler;
	const battle_context_unit_stats& att = bc_vector[choice].get_attacker_stats();
	const battle_context_unit_stats& def = bc_vector[choice].get_defender_stats();

	attacker->set_goto(map_location());

	current_paths_ = pathfind::paths();

	// make the attacker's stats appear during the attack
	gui().display_unit_hex(attacker_loc);

	// remove highlighted hexes etc..
	gui().select_hex(map_location());
	gui().highlight_hex(map_location());
	gui().clear_attack_indicator();
	gui().unhighlight_reach();

	current_team().set_action_bonus_count(1 + current_team().action_bonus_count());
	///@todo change ToD to be location specific for the defender

	const tod_manager& tod_man = pc_.get_tod_manager_const();

	synced_context::run_and_throw("attack",
		replay_helper::get_attack(
			attacker_loc,
			defender_loc,
			att.attack_num,
			def.attack_num,
			attacker->type_id(),
			defender->type_id(),
			att.level,
			def.level,
			tod_man.turn(),
			tod_man.get_time_of_day()
		)
	);
}

std::set<map_location> mouse_handler::get_adj_enemies(const map_location& loc, int side) const
{
	std::set<map_location> res;

	const team& uteam = pc_.gamestate().board_.teams_[side - 1];

	adjacent_loc_array_t adj;
	get_adjacent_tiles(loc, adj.data());

	for(const map_location& aloc : adj) {
		unit_map::const_iterator i = find_unit(aloc);

		if(i && uteam.is_enemy(i->side())) {
			res.insert(aloc);
		}
	}

	return res;
}

/**
 * Causes attackable hexes to be highlighted.
 *
 * This checks the hexes that the provided unit can attack. If there is a valid
 * target there, that location is inserted into current_paths_.destinations.
 */
void mouse_handler::show_attack_options(const unit_map::const_iterator& u)
{
	// Cannot attack if no attacks are left.
	if(u->attacks_left() == 0) {
		return;
	}

	// Get the teams involved.
	const team& cur_team = current_team();
	const team& u_team = pc_.gamestate().board_.teams_[u->side() - 1];

	// Check each adjacent hex.
	adjacent_loc_array_t adj;
	get_adjacent_tiles(u->get_location(), adj.data());

	for(const map_location& loc : adj) {
		// No attack option shown if no visible unit present.
		// (Visible to current team, not necessarily the unit's team.)
		if(!pc_.get_map_const().on_board(loc)) {
			continue;
		}

		unit_map::const_iterator i = pc_.gamestate().board_.units().find(loc);
		if(!i || !i->is_visible_to_team(cur_team, gui_->get_disp_context(), false)) {
			continue;
		}

		const unit& target = *i;

		// Can only attack non-petrified enemies.
		if(u_team.is_enemy(target.side()) && !target.incapacitated()) {
			current_paths_.destinations.insert(loc);
		}
	}
}

bool mouse_handler::unit_in_cycle(unit_map::const_iterator it)
{
	game_board& board = pc_.gamestate().board_;

	if(!it) {
		return false;
	}

	if(it->side() != side_num_ || it->user_end_turn() || gui().fogged(it->get_location()) || !board.unit_can_move(*it)) {
		return false;
	}

	if(current_team().is_enemy(int(gui().viewing_team() + 1)) && it->invisible(it->get_location(), gui().get_disp_context())) {
		return false;
	}

	if(it->get_hidden()) {
		return false;
	}

	return true;
}

void mouse_handler::cycle_units(const bool browse, const bool reverse)
{
	game_board& board = pc_.gamestate().board_;

	if(board.units().begin() == board.units().end()) {
		return;
	}

	unit_map::const_iterator it = find_unit(next_unit_);
	if(!it) {
		it = board.units().begin();
	}

	const unit_map::const_iterator itx = it;

	do {
		if(reverse) {
			if(it == board.units().begin()) {
				it = board.units().end();
			}

			--it;
		} else {
			if(it == board.units().end()) {
				it = board.units().begin();
			} else {
				++it;
			}
		}
	} while(it != itx && !unit_in_cycle(it));

	if(unit_in_cycle(it)) {
		gui().scroll_to_tile(it->get_location(), game_display::WARP);

		select_hex(it->get_location(), browse);
		//		mouse_update(browse);
	}
}

void mouse_handler::set_current_paths(const pathfind::paths& new_paths)
{
	gui().unhighlight_reach();

	current_paths_ = new_paths;
	current_route_.steps.clear();

	gui().set_route(nullptr);

	pc_.get_whiteboard()->erase_temp_move();
}

team& mouse_handler::viewing_team()
{
	return pc_.gamestate().board_.teams_[gui().viewing_team()];
}

const team& mouse_handler::viewing_team() const
{
	return pc_.gamestate().board_.teams()[gui().viewing_team()];
}

team& mouse_handler::current_team()
{
	return pc_.gamestate().board_.teams_[side_num_ - 1];
}

mouse_handler* mouse_handler::singleton_ = nullptr;

void mouse_handler::disable_units_highlight()
{
	preventing_units_highlight_ = true;
}

void mouse_handler::enable_units_highlight()
{
	preventing_units_highlight_ = false;
}

} // end namespace events
