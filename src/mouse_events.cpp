/* $Id$ */
/*
   Copyright (C) 2006 - 2011 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "mouse_events.hpp"

#include "attack_prediction_display.hpp"
#include "dialogs.hpp"
#include "foreach.hpp"
#include "game_end_exceptions.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "play_controller.hpp"
#include "sound.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "rng.hpp"
#include "tod_manager.hpp"
#include "wml_separators.hpp"

#include <boost/bind.hpp>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace events{


mouse_handler::mouse_handler(game_display* gui, std::vector<team>& teams,
		unit_map& units, gamemap& map, tod_manager& tod_mng,
		undo_list& undo_stack, undo_list& redo_stack) :
	mouse_handler_base(),
	map_(map),
	gui_(gui),
	teams_(teams),
	units_(units),
	tod_manager_(tod_mng),
	undo_stack_(undo_stack),
	redo_stack_(redo_stack),
	previous_hex_(),
	previous_free_hex_(),
	selected_hex_(),
	next_unit_(),
	current_route_(),
	waypoints_(),
	current_paths_(),
	enemy_paths_(false),
	path_turns_(0),
	side_num_(1),
	undo_(false),
	over_route_(false),
	attackmove_(false),
	reachmap_invalid_(false),
	show_partial_move_(false)
{
	singleton_ = this;
}

mouse_handler::~mouse_handler()
{
	rand_rng::clear_new_seed_callback();
	singleton_ = NULL;
}

void mouse_handler::set_side(int side_number)
{
	side_num_ = side_number;
}

int mouse_handler::drag_threshold() const
{
	return 14;
}

void mouse_handler::mouse_motion(int x, int y, const bool browse, bool update)
{
	if (attackmove_) return;

	// we ignore the position coming from event handler
	// because it's always a little obsolete and we don't need
	// to hightlight all the hexes where the mouse passed.
	// Also, sometimes it seems to have one *very* obsolete
	// and isolated mouse motion event when using drag&drop
	SDL_GetMouseState(&x,&y);  // <-- modify x and y

	if (mouse_handler_base::mouse_motion_default(x, y, update)) return;

	const map_location new_hex = gui().hex_clicked_on(x,y);

	if(new_hex != last_hex_) {
		update = true;
		if (last_hex_.valid()) {
			// we store the previous hexes used to propose attack direction
			previous_hex_ = last_hex_;
			// the hex of the selected unit is also "free"
			if (last_hex_ == selected_hex_ || find_unit(last_hex_) == units_.end()) {
				previous_free_hex_ = last_hex_;
			}
		}
		last_hex_ = new_hex;
	}


	if (reachmap_invalid_) update = true;

	if (update) {
		if (reachmap_invalid_) {
			reachmap_invalid_ = false;
			if (!current_paths_.destinations.empty() && !show_partial_move_) {
				unit_map::iterator u = find_unit(selected_hex_);
				if(selected_hex_.valid() && u != units_.end() ) {
					// reselect the unit without firing events (updates current_paths_)
					select_hex(selected_hex_, true);
				}
				// we do never deselect here, mainly because of canceled attack-move
			}
		}

		// reset current_route_ and current_paths if not valid anymore
		// we do it before cursor selection, because it uses current_paths_
		if(new_hex.valid() == false) {
			current_route_.steps.clear();
			gui().set_route(NULL);
		}

		if(enemy_paths_) {
			enemy_paths_ = false;
			current_paths_ = pathfind::paths();
			gui().unhighlight_reach();
		} else if(over_route_) {
			over_route_ = false;
			current_route_.steps.clear();
			gui().set_route(NULL);
		}

		gui().highlight_hex(new_hex);

		const unit_map::iterator selected_unit = find_unit(selected_hex_);
		const unit_map::iterator mouseover_unit = find_unit(new_hex);

		// we search if there is an attack possibility and where
		map_location attack_from = current_unit_attacks_from(new_hex);

		//see if we should show the normal cursor, the movement cursor, or
		//the attack cursor
		//If the cursor is on WAIT, we don't change it and let the setter
		//of this state end it
		if (cursor::get() != cursor::WAIT) {
			if (selected_unit != units_.end() &&
			    selected_unit->second.side() == side_num_ &&
			    !selected_unit->second.incapacitated() && !browse)
			{
				if (attack_from.valid()) {
					cursor::set(dragging_started_ ? cursor::ATTACK_DRAG : cursor::ATTACK);
				}
				else if (mouseover_unit==units_.end() &&
				         current_paths_.destinations.contains(new_hex))
				{
					cursor::set(dragging_started_ ? cursor::MOVE_DRAG : cursor::MOVE);
				} else {
					// selecte unit can't attack or move there
					cursor::set(cursor::NORMAL);
				}
			} else {
				// no selected unit or we can't move it
				cursor::set(cursor::NORMAL);
			}
		}

		// show (or cancel) the attack direction indicator
		if (attack_from.valid() && !browse) {
			gui().set_attack_indicator(attack_from, new_hex);
		} else {
			gui().clear_attack_indicator();
		}

		// the destination is the pointed hex or the adjacent hex
		// used to attack it
		map_location dest;
		unit_map::const_iterator dest_un;
		if (attack_from.valid()) {
			dest = attack_from;
			dest_un = find_unit(dest);
		}	else {
			dest = new_hex;
			dest_un = mouseover_unit;
		}

		if(dest == selected_hex_ || dest_un != units_.end()) {
			current_route_.steps.clear();
			gui().set_route(NULL);
		}
		else if (!current_paths_.destinations.empty() &&
		         map_.on_board(selected_hex_) && map_.on_board(new_hex))
		{
			if(selected_unit != units_.end() && !selected_unit->second.incapacitated()) {
				// Show the route from selected unit to mouseover hex
				// the movement_reset is active only if it's not the unit's turn
				unit_movement_resetter move_reset(selected_unit->second,
						selected_unit->second.side() != side_num_);
				current_route_ = get_route(selected_unit, dest, waypoints_, viewing_team());
				if(!browse) {
					gui().set_route(&current_route_);
				}
			}
		}

		unit_map::iterator un = mouseover_unit;

		if (un != units_.end() && current_paths_.destinations.empty() &&
		    !gui().fogged(un->first))
		{
			if (un->second.side() != side_num_) {
				//unit under cursor is not on our team, highlight reach
				unit_movement_resetter move_reset(un->second);

				bool teleport = un->second.get_ability_bool("teleport");
				current_paths_ = pathfind::paths(map_,units_,new_hex,teams_,
									false,teleport,viewing_team(),path_turns_);
				gui().highlight_reach(current_paths_);
				enemy_paths_ = true;
			} else {
				//unit is on our team, show path if the unit has one
				const map_location go_to = un->second.get_goto();
				if(map_.on_board(go_to)) {
					pathfind::marked_route route = get_route(un, go_to, un->second.waypoints(), current_team());
					gui().set_route(&route);
				}
				over_route_ = true;
			}
		}
	}
}

unit_map::iterator mouse_handler::selected_unit()
{
	unit_map::iterator res = find_unit(selected_hex_);
	if(res != units_.end()) {
		return res;
	} else {
		return find_unit(last_hex_);
	}
}

unit_map::iterator mouse_handler::find_unit(const map_location& hex)
{
	return find_visible_unit(units_, hex, viewing_team());
}

unit_map::const_iterator mouse_handler::find_unit(const map_location& hex) const
{
	return find_visible_unit(units_, hex, viewing_team());
}

map_location mouse_handler::current_unit_attacks_from(const map_location& loc)
{
	const unit_map::const_iterator current = find_unit(selected_hex_);
	if(current == units_.end() || current->second.side() != side_num_
		|| current->second.attacks_left()==0) {
		return map_location();
	}

	const unit_map::const_iterator enemy = find_unit(loc);
	if(enemy == units_.end() || current_team().is_enemy(enemy->second.side()) == false
		|| enemy->second.incapacitated())
	{
		return map_location();
	}

	const map_location::DIRECTION preferred = loc.get_relative_dir(previous_hex_);
	const map_location::DIRECTION second_preferred = loc.get_relative_dir(previous_free_hex_);

	int best_rating = 100;//smaller is better
	map_location res;
	map_location adj[6];
	get_adjacent_tiles(loc,adj);

	for(size_t n = 0; n != 6; ++n) {
		if(map_.on_board(adj[n]) == false) {
			continue;
		}

		if(adj[n] != selected_hex_ && find_unit(adj[n]) != units_.end()) {
			continue;
		}

		if (current_paths_.destinations.contains(adj[n]))
		{
			static const size_t NDIRECTIONS = map_location::NDIRECTIONS;
			unsigned int difference = abs(int(preferred - n));
			if(difference > NDIRECTIONS/2) {
				difference = NDIRECTIONS - difference;
			}
			unsigned int second_difference = abs(int(second_preferred - n));
			if(second_difference > NDIRECTIONS/2) {
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

void mouse_handler::add_waypoint(const map_location& loc) {
	std::vector<map_location>::iterator w = std::find(waypoints_.begin(), waypoints_.end(), loc);
	//toggle between add a new one and remove an old one
	if(w != waypoints_.end()){
		waypoints_.erase(w);
	} else {
		waypoints_.push_back(loc);
	}

	// we need to update the route, simulate a mouse move for the moment
	// (browse is supposed false here, 0,0 are dummy values)
	mouse_motion(0,0, false, true);
}

pathfind::marked_route mouse_handler::get_route(unit_map::const_iterator un, map_location go_to, const std::vector<map_location>& waypoints, team &team)
{
	// The pathfinder will check unit visibility (fogged/stealthy).
	const pathfind::shortest_path_calculator calc(un->second,team,units_,teams_,map_);

	std::set<map_location> allowed_teleports = pathfind::get_teleport_locations(
		un->second, units_, viewing_team());

	pathfind::plain_route route;

	if (waypoints.empty()) {
		// standard shortest path
		route = pathfind::a_star_search(un->first, go_to, 10000.0, &calc, map_.w(), map_.h(), &allowed_teleports);
	} else {
		// initialize the main route with the first step
		route.steps.push_back(un->first);
		route.move_cost = 0;

		//copy waypoints and add first source and last destination
		//TODO: don't copy but use vector index trick
		std::vector<map_location> waypts;
		waypts.push_back(un->first);
		waypts.insert(waypts.end(), waypoints.begin(), waypoints.end());
		waypts.push_back(go_to);

		std::vector<map_location>::iterator src = waypts.begin(),
			dst = ++waypts.begin();
		for(; dst != waypts.end(); ++src,++dst){
			if (*src == *dst) continue;
			pathfind::plain_route inter_route = pathfind::a_star_search(*src, *dst, 10000.0, &calc, map_.w(), map_.h(), &allowed_teleports);
			if(inter_route.steps.size()>=1) {
				// add to the main route but skip the head (already in)
				route.steps.insert(route.steps.end(),
					inter_route.steps.begin()+1,inter_route.steps.end());
				route.move_cost+=inter_route.move_cost;
			} else {
				// we can't reach dst, stop the route at the last src
				// as the normal case do
				break;
			}
		}
	}

	return mark_route(route, waypoints, un->second, viewing_team(), units_,teams_,map_);
}

void mouse_handler::mouse_press(const SDL_MouseButtonEvent& event, const bool browse)
{
	mouse_handler_base::mouse_press(event, browse);
}

bool mouse_handler::right_click_show_menu(int x, int y, const bool browse)
{
	// The first right-click cancel the selection if any,
	// the second open the context menu
	if (selected_hex_.valid() && find_unit(selected_hex_) != units_.end()) {
		select_hex(map_location(), browse);
		return false;
	} else {
		return point_in_rect(x, y, gui().map_area());
	}
}

bool mouse_handler::left_click(int x, int y, const bool browse)
{
	undo_ = false;
	if (mouse_handler_base::left_click(x, y, browse)) return false;

	bool check_shroud = current_team().auto_shroud_updates();

	//we use the last registered highlighted hex
	//since it's what update our global state
	map_location hex = last_hex_;

	unit_map::iterator u = find_unit(selected_hex_);

	//if the unit is selected and then itself clicked on,
	//any goto command and waypoints are cancelled
	if(u != units_.end() && !browse && selected_hex_ == hex && u->second.side() == side_num_) {
		u->second.set_goto(map_location());
		u->second.waypoints().clear();
		waypoints_.clear();
	}

	unit_map::iterator clicked_u = find_unit(hex);

	const map_location src = selected_hex_;
	pathfind::paths orig_paths = current_paths_;
	const map_location& attack_from = current_unit_attacks_from(hex);

	//see if we're trying to do a attack or move-and-attack
	if(!browse && !commands_disabled && attack_from.valid()) {
		if (attack_from == selected_hex_) { //no move needed
			int choice = show_attack_dialog(attack_from, clicked_u->first);
			if (choice >=0 ) {
				attack_enemy(u, clicked_u, choice);
			}
			return false;
		}
		else {
			// we will now temporary move next to the enemy
			pathfind::paths::dest_vect::const_iterator itor =
					current_paths_.destinations.find(attack_from);
			if(itor == current_paths_.destinations.end()) {
				// can't reach the attacking location
				// not supposed to happen, so abort
				return false;
			}
			// update movement_left as if we did the move
			int move_left_dst = itor->move_left;
			int move_left_src = u->second.movement_left();
			u->second.set_movement(move_left_dst);

			int choice = -1;
			// block where we temporary move the unit
			{
				temporary_unit_mover temp_mover(units_, src, attack_from);
				choice = show_attack_dialog(attack_from, clicked_u->first);
			}
			// restore unit as before
			u = units_.find(src);
			u->second.set_movement(move_left_src);
			u->second.set_standing();

			if (choice < 0) {
				// user hit cancel, don't start move+attack
				return false;
			}

			//register the mouse-UI waypoints into the unit's waypoints
			u->second.waypoints() = waypoints_;

			// move the unit without clearing fog (to avoid interruption)
			//TODO: clear fog and interrupt+resume move
			if(!move_unit_along_current_route(false, true)) {
				// interrupted move
				// we assume that move_unit() did the cleaning
				// (update shroud/fog, clear undo if needed)
				return false;
			}

			// a WML event could have invalidated both attacker and defender
			// so make sure they're valid before attacking
			u = find_unit(attack_from);
			unit_map::iterator enemy = find_unit(hex);
			if(u != units_.end() && u->second.side() == side_num_ &&
				enemy != units_.end() && current_team().is_enemy(enemy->second.side()) && !enemy->second.incapacitated()
				&& !commands_disabled) {

				attack_enemy(u, enemy, choice); // Fight !!
				return false;
			}

		}
	}

	//otherwise we're trying to move to a hex
	else if(!commands_disabled && !browse && selected_hex_.valid() && selected_hex_ != hex &&
		     u != units_.end() && u->second.side() == side_num_ &&
		     clicked_u == units_.end() && !current_route_.steps.empty() &&
		     current_route_.steps.front() == selected_hex_) {

		gui().unhighlight_reach();

		//register the mouse-UI waypoints into the unit's waypoints
		u->second.waypoints() = waypoints_;

		move_unit_along_current_route(check_shroud);
		// during the move, we may have selected another unit
		// (but without triggering a select event (command was disabled)
		// in that case reselect it now to fire the event (+ anim & sound)
		if (selected_hex_ != src) {
			select_hex(selected_hex_, browse);
		}
		return false;
	} else if (!attackmove_) {
		// we select a (maybe empty) hex
		// we block selection during attack+move (because motion is blocked)
		select_hex(hex, browse);
	}
	return false;
	//FIXME: clean all these "return false"
}

void mouse_handler::select_hex(const map_location& hex, const bool browse) {
	selected_hex_ = hex;
	gui().select_hex(hex);
	gui().clear_attack_indicator();
	gui().set_route(NULL);
	waypoints_.clear();
	show_partial_move_ = false;

	unit_map::iterator u = find_unit(hex);
	if(hex.valid() && u != units_.end() && !u->second.get_hidden()) {
		next_unit_ = u->first;

		{
			// if it's not the unit's turn, we reset its moves
			// and we restore them before the "select" event is raised
			unit_movement_resetter move_reset(u->second, u->second.side() != side_num_);
			bool teleport = u->second.get_ability_bool("teleport");
			current_paths_ = pathfind::paths(map_, units_, hex, teams_,
				false, teleport, viewing_team(), path_turns_);
		}
		show_attack_options(u);
		gui().highlight_reach(current_paths_);
		// the highlight now comes from selection
		// and not from the mouseover on an enemy
		enemy_paths_ = false;
		gui().set_route(NULL);

		// selection have impact only if we are not observing and it's our unit
		if (!browse && !commands_disabled && u->second.side() == gui().viewing_side()) {
			sound::play_UI_sound("select-unit.wav");
			u->second.set_selecting();
			game_events::fire("select", hex);
		}

	} else {
		gui().unhighlight_reach();
		current_paths_ = pathfind::paths();
		current_route_.steps.clear();
	}
}

void mouse_handler::deselect_hex() {
	select_hex(map_location(), true);
}

void mouse_handler::clear_undo_stack()
{
	apply_shroud_changes(undo_stack_, side_num_);
	undo_stack_.clear();
}

bool mouse_handler::move_unit_along_current_route(bool check_shroud, bool attackmove)
{
	const std::vector<map_location> steps = current_route_.steps;
	if(steps.empty()) {
		return false;
	}

	// do not show footsteps during movement
	gui().set_route(NULL);

	// do not keep the hex highlighted that we started from
	selected_hex_ = map_location();
	gui().select_hex(map_location());

	// will be invalid after the move
	current_paths_ = pathfind::paths();
	current_route_.steps.clear();

	attackmove_ = attackmove;
	size_t moves = 0;
	try{
		moves = ::move_unit(NULL, steps, &recorder, &undo_stack_, true, &next_unit_, false, check_shroud);
	} catch(end_turn_exception&) {
		attackmove_ = false;
		cursor::set(cursor::NORMAL);
		gui().invalidate_game_status();
		throw;
	}
	attackmove_ = false;

	cursor::set(cursor::NORMAL);

	gui().invalidate_game_status();

	if(moves == 0)
		return false;

	redo_stack_.clear();

	assert(moves <= steps.size());
	const map_location& dst = steps[moves-1];
	const unit_map::const_iterator u = units_.find(dst);

	//u may be equal to units_.end() in the case of e.g. a [teleport]
	if(u != units_.end()) {
		if(dst != steps.back()) {
			// the move was interrupted (or never started)
			if (u->second.movement_left() > 0) {
				// reselect the unit (for "press t to continue")
				select_hex(dst, false);
				// the new discovery is more important than the new movement range
				show_partial_move_ = true;
				gui().unhighlight_reach();
			}
		}
	}

	return moves == steps.size();
}


int mouse_handler::fill_weapon_choices(std::vector<battle_context>& bc_vector, unit_map::iterator attacker, unit_map::iterator defender)
{
	int best = 0;
	for (unsigned int i = 0; i < attacker->second.attacks().size(); i++) {
		// skip weapons with attack_weight=0
		if (attacker->second.attacks()[i].attack_weight() > 0) {
			battle_context bc(units_, attacker->first, defender->first, i);
			bc_vector.push_back(bc);
			if (bc.better_attack(bc_vector[best], 0.5)) {
				best = i;
			}
		}
	}
	return best;
}

int mouse_handler::show_attack_dialog(const map_location& attacker_loc, const map_location& defender_loc)
{
	unit_map::iterator attacker = units_.find(attacker_loc);
	unit_map::iterator defender = units_.find(defender_loc);

	std::vector<battle_context> bc_vector;
	int best = fill_weapon_choices(bc_vector, attacker, defender);

	if (bc_vector.empty())
	{
		dialogs::units_list_preview_pane attacker_preview(attacker->second, dialogs::unit_preview_pane::SHOW_BASIC, true);
		dialogs::units_list_preview_pane defender_preview(defender->second, dialogs::unit_preview_pane::SHOW_BASIC, false);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&attacker_preview);
		preview_panes.push_back(&defender_preview);

		gui::show_dialog(gui(), NULL, _("Attack Enemy"),
			_("No usable weapon"), gui::CANCEL_ONLY, NULL,
			&preview_panes, "", NULL, -1, NULL, -1, -1, NULL, NULL);
		return -1;
	}


	std::vector<std::string> items;

	for (unsigned int i = 0; i < bc_vector.size(); i++) {
		const battle_context::unit_stats& att = bc_vector[i].get_attacker_stats();
		const battle_context::unit_stats& def = bc_vector[i].get_defender_stats();
		config tmp_config;
		attack_type no_weapon(tmp_config);
		const attack_type& attw = attack_type(*att.weapon);
		const attack_type& defw = attack_type(def.weapon ? *def.weapon : no_weapon);

		attw.set_specials_context(attacker->first, defender->first, attacker->second, true);
		defw.set_specials_context(attacker->first, defender->first, attacker->second, false);

		// if missing, add dummy special, to be sure to have
		// big enough mimimum width (weapon's name can be very short)
		std::string att_weapon_special = attw.weapon_specials();
		if (att_weapon_special.empty())
			att_weapon_special += "       ";
		std::string def_weapon_special = defw.weapon_specials();
		if (def_weapon_special.empty())
			def_weapon_special += "       ";

		std::stringstream atts;
		if (static_cast<int>(i) == best) {
			atts << DEFAULT_ITEM;
		}

		std::string range = attw.range().empty() ? defw.range() : attw.range();
		if (!range.empty()) {
			range = gettext(range.c_str());
		}

		// add dummy names if missing, to keep stats aligned
		std::string attw_name = attw.name();
		if(attw_name.empty())
			attw_name = " ";
		std::string defw_name = defw.name();
		if(defw_name.empty())
			defw_name = " ";

		// color CtH in red-yellow-green
		SDL_Color att_cth_color =
				int_to_color( game_config::red_to_green(att.chance_to_hit) );
		SDL_Color def_cth_color =
				int_to_color( game_config::red_to_green(def.chance_to_hit) );

		atts << IMAGE_PREFIX << attw.icon() << COLUMN_SEPARATOR
			 << font::BOLD_TEXT << attw_name  << "\n"
			 << att.damage << "-" << att.num_blows
			 << "  " << att_weapon_special << "\n"
			 << font::color2markup(att_cth_color) << att.chance_to_hit << "%"
			 << COLUMN_SEPARATOR << font::weapon_details << "- " << range << " -" << COLUMN_SEPARATOR
			 << font::BOLD_TEXT << defw_name  << "\n"
			 << def.damage << "-" << def.num_blows
			 << "  " << def_weapon_special << "\n"
			 << font::color2markup(def_cth_color) << def.chance_to_hit << "%"
			 << COLUMN_SEPARATOR << IMAGE_PREFIX << defw.icon();

		items.push_back(atts.str());
	}

	attack_prediction_displayer ap_displayer(bc_vector, attacker_loc, defender_loc);
	std::vector<gui::dialog_button_info> buttons;
	buttons.push_back(gui::dialog_button_info(&ap_displayer, _("Damage Calculations")));

	int res = 0;
	{
		dialogs::units_list_preview_pane attacker_preview(attacker->second, dialogs::unit_preview_pane::SHOW_BASIC, true);
		dialogs::units_list_preview_pane defender_preview(defender->second, dialogs::unit_preview_pane::SHOW_BASIC, false);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&attacker_preview);
		preview_panes.push_back(&defender_preview);

		res = gui::show_dialog(gui(),NULL,_("Attack Enemy"),
				_("Choose weapon:")+std::string("\n"),
				gui::OK_CANCEL,&items,&preview_panes,"",NULL,-1,NULL,-1,-1,
				NULL,&buttons);
	}
	cursor::set(cursor::NORMAL);

	return res;
}

void mouse_handler::attack_enemy(unit_map::iterator attacker, unit_map::iterator defender, int choice)
{
	try {
		attack_enemy_(attacker, defender, choice);
	} catch(std::bad_alloc) {
		lg::wml_error << "Memory exhausted a unit has either a lot hitpoints or a negative amount.\n";
	}
}

void mouse_handler::attack_enemy_(unit_map::iterator attacker, unit_map::iterator defender, int choice)
{
	//we must get locations by value instead of by references, because the iterators
	//may become invalidated later
	const map_location attacker_loc = attacker->first;
	const map_location defender_loc = defender->first;

	commands_disabled++;

	attacker->second.set_goto(map_location());
	//This triggers a shroud update which could fire a sighted event
	//invalidating our iterators (or do more changes)
	clear_undo_stack();
	redo_stack_.clear();

	// refresh iterators
	attacker = units_.find(attacker_loc);
	defender = units_.find(defender_loc);

	if(attacker == units_.end() || attacker->second.incapacitated()
			|| attacker->second.side() != side_num_) {
		return;
	}
	if(defender == units_.end() || defender->second.incapacitated()
			|| current_team().is_enemy(defender->second.side()) == false) {
		return;
	}

	std::vector<battle_context> bc_vector;
	fill_weapon_choices(bc_vector, attacker, defender);

	if(size_t(choice) >= bc_vector.size()) {
		return;
	}

	const battle_context::unit_stats &att = bc_vector[choice].get_attacker_stats();
	const battle_context::unit_stats &def = bc_vector[choice].get_defender_stats();

	current_paths_ = pathfind::paths();
	// make the attacker's stats appear during the attack
	gui().display_unit_hex(attacker_loc);
	// remove highlighted hexes etc..
	gui().select_hex(map_location());
	gui().highlight_hex(map_location());
	gui().clear_attack_indicator();
	gui().unhighlight_reach();
	gui().draw();

	//@TODO: change ToD to be location specific for the defender
	recorder.add_attack(attacker_loc, defender_loc, att.attack_num, def.attack_num,
		attacker->second.type_id(), defender->second.type_id(), att.level,
		def.level, resources::tod_manager->turn(), resources::tod_manager->get_time_of_day());
	rand_rng::invalidate_seed();
	if (rand_rng::has_valid_seed()) { //means SRNG is disabled
		perform_attack(attacker_loc, defender_loc, att.attack_num, def.attack_num, rand_rng::get_last_seed());
	} else {
		rand_rng::set_new_seed_callback(boost::bind(&mouse_handler::perform_attack,
			this, attacker_loc, defender_loc, att.attack_num, def.attack_num, _1));
	}
}

void mouse_handler::perform_attack(
	map_location attacker_loc, map_location defender_loc,
	int attacker_weapon, int defender_weapon, rand_rng::seed_t seed)
{
	// this function gets it's arguments by value because the calling function
	// object might get deleted in the clear callback call below, invalidating
	// const ref arguments
	rand_rng::clear_new_seed_callback();
	LOG_NG << "Performing attack with seed " << seed << "\n";
	recorder.add_seed("attack", seed);
	//MP_COUNTDOWN grant time bonus for attacking
	current_team().set_action_bonus_count(1 + current_team().action_bonus_count());

	try {
		events::command_disabler disabler; // Rather than decrementing for every possible exception, use RAII
		commands_disabled--;
		attack_unit(attacker_loc, defender_loc, attacker_weapon, defender_weapon);
	} catch(end_level_exception&) {
		//if the level ends due to a unit being killed, still see if
		//either the attacker or defender should advance
		dialogs::advance_unit(attacker_loc);
		unit_map::const_iterator defu = units_.find(defender_loc);
		if (defu != units_.end()) {
			bool defender_human = teams_[defu->second.side()-1].is_human();
			dialogs::advance_unit(defender_loc, !defender_human);
		}
		throw;
	}

	dialogs::advance_unit(attacker_loc);
	unit_map::const_iterator defu = units_.find(defender_loc);
	if (defu != units_.end()) {
		bool defender_human = teams_[defu->second.side()-1].is_human();
		dialogs::advance_unit(defender_loc, !defender_human);
	}

	resources::controller->check_victory();
	gui().draw();
}

void mouse_handler::show_attack_options(const unit_map::const_iterator &u)
{
	if (u == units_.end() || u->second.attacks_left() == 0)
		return;

	map_location adj[6];
	get_adjacent_tiles(u->first, adj);
	BOOST_FOREACH (const map_location &loc, adj)
	{
		if (!map_.on_board(loc)) continue;
		unit_map::const_iterator i = units_.find(loc);
		if (i == units_.end()) continue;
		const unit &target = i->second;
		if (current_team().is_enemy(target.side()) && !target.incapacitated())
			current_paths_.destinations.insert(loc);
	}
}

bool mouse_handler::unit_in_cycle(unit_map::const_iterator it)
{
	if (it == units_.end())
		return false;

	if (it->second.side() != side_num_ || it->second.user_end_turn()
	    || gui().fogged(it->first) || !unit_can_move(it->second))
		return false;

	if (current_team().is_enemy(int(gui().viewing_team()+1)) &&
			it->second.invisible(it->first,units_,teams_))
		return false;

	if(it->second.get_hidden())
		return false;

	return true;

}

void mouse_handler::cycle_units(const bool browse, const bool reverse)
{
	if (units_.begin() == units_.end()) {
		return;
	}

	unit_map::const_iterator it = find_unit(next_unit_);
	if (it == units_.end())
		it = units_.begin();
	const unit_map::const_iterator itx = it;

	do {
		if (reverse) {
			if (it == units_.begin())
				it = units_.end();
			--it;
		} else {
			if (it == units_.end())
				it = units_.begin();
			else
				++it;
		}
	} while (it != itx && !unit_in_cycle(it));

	if (unit_in_cycle(it)) {
		gui().scroll_to_tile(it->first,game_display::WARP);
		select_hex(it->first, browse);
		mouse_update(browse);
	}
}

void mouse_handler::set_current_paths(pathfind::paths new_paths) {
	gui().unhighlight_reach();
	current_paths_ = new_paths;
	current_route_.steps.clear();
	gui().set_route(NULL);
}

mouse_handler *mouse_handler::singleton_ = NULL;
}
