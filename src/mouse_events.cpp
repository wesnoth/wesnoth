/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mouse_events.hpp"

#include "attack_prediction_display.hpp"
#include "dialogs.hpp"
#include "game_end_exceptions.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "sound.hpp"
#include "replay.hpp"
#include "wml_separators.hpp"


namespace events{


namespace{
	//minimum dragging distance to fire the drag&drop
	const double drag_threshold = 14.0;
}

mouse_handler::mouse_handler(game_display* gui, std::vector<team>& teams,
		unit_map& units, gamemap& map, gamestatus& status,
		undo_list& undo_stack, undo_list& redo_stack) :
	mouse_handler_base(),
	map_(map),
	gui_(gui),
	teams_(teams),
	units_(units),
	status_(status),
	undo_stack_(undo_stack),
	redo_stack_(redo_stack),
	previous_hex_(),
	previous_free_hex_(),
	selected_hex_(),
	next_unit_(),
	current_route_(),
	current_paths_(),
	enemy_paths_(false),
	path_turns_(0),
	team_num_(1),
	enemies_visible_(false),
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
	singleton_ = NULL;
}

void mouse_handler::set_team(const int team_number)
{
	team_num_ = team_number;
}

void mouse_handler::mouse_motion(int x, int y, const bool browse, bool update)
{
	if (attackmove_) return;

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
			if (!current_paths_.routes.empty() && !show_partial_move_) {
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
			current_paths_ = paths();
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
			if(selected_unit != units_.end() && selected_unit->second.side() == team_num_
			   && !selected_unit->second.incapacitated() && !browse) {
				if (attack_from.valid()) {
					cursor::set(dragging_started_ ? cursor::ATTACK_DRAG : cursor::ATTACK);
				} else if (mouseover_unit==units_.end() && current_paths_.routes.count(new_hex)) {
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
		} else if(!current_paths_.routes.empty() && map_.on_board(selected_hex_) &&
		   map_.on_board(new_hex)) {

			if(selected_unit != units_.end() && !selected_unit->second.incapacitated()) {
				// the movement_reset is active only if it's not the unit's turn
				unit_movement_resetter move_reset(selected_unit->second,
						selected_unit->second.side() != team_num_);
				current_route_ = get_route(selected_unit, dest, viewing_team());
				if(!browse) {
					gui().set_route(&current_route_);
				}
			}
		}

		unit_map::iterator un = mouseover_unit;

		if(un != units_.end() && current_paths_.routes.empty() && !gui().fogged(un->first)) {
			if (un->second.side() != team_num_) {
				//unit under cursor is not on our team, highlight reach
				unit_movement_resetter move_reset(un->second);

				const bool teleport = un->second.get_ability_bool("teleport",un->first);
				current_paths_ = paths(map_,units_,new_hex,teams_,
									false,teleport,viewing_team(),path_turns_);
				gui().highlight_reach(current_paths_);
				enemy_paths_ = true;
			} else {
				//unit is on our team, show path if the unit has one
				const map_location go_to = un->second.get_goto();
				if(map_.on_board(go_to)) {
					paths::route route = get_route(un, go_to, current_team());
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
	return find_visible_unit(units_,hex,map_,teams_,viewing_team());
}

unit_map::const_iterator mouse_handler::find_unit(const map_location& hex) const
{
	return find_visible_unit(units_,hex,map_,teams_,viewing_team());
}

map_location mouse_handler::current_unit_attacks_from(const map_location& loc)
{
	const unit_map::const_iterator current = find_unit(selected_hex_);
	if(current == units_.end() || current->second.side() != team_num_
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

		if(current_paths_.routes.count(adj[n])) {
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

paths::route mouse_handler::get_route(unit_map::const_iterator un, map_location go_to, team &team)
{
	// The pathfinder will check unit visibility (fogged/stealthy).
	const shortest_path_calculator calc(un->second,team,units_,teams_,map_);

	std::set<map_location> allowed_teleports;

	if(un->second.get_ability_bool("teleport",un->first)) {
		// search all known empty friendly villages
		for(std::set<map_location>::const_iterator i = team.villages().begin();
				i != team.villages().end(); ++i) {
			if (viewing_team().is_enemy(un->second.side()) && viewing_team().fogged(*i))
				continue;

			unit_map::const_iterator occupant = find_unit(*i);
			if (occupant != units_.end() && occupant != un)
				continue;

			allowed_teleports.insert(*i);
		}
	}
	paths::route route = a_star_search(un->first, go_to, 10000.0, &calc, map_.w(), map_.h(), &allowed_teleports);
	route_turns_to_complete(un->second, route, viewing_team(), units_,teams_,map_);
	return route;
}

void mouse_handler::mouse_press(const SDL_MouseButtonEvent& event, const bool browse)
{
	mouse_handler_base::mouse_press(event, browse);
}

bool mouse_handler::right_click_show_menu(int /*x*/, int /*y*/, const bool browse)
{
	// The first right-click cancel the selection if any,
	// the second open the context menu
	if (selected_hex_.valid() && find_unit(selected_hex_) != units_.end()) {
		select_hex(map_location(), browse);
		return false;
	} else {
		return true;
	}
}

bool mouse_handler::left_click(int x, int y, const bool browse)
{
	undo_ = false;
	if (mouse_handler_base::left_click(x, y, browse)) return false;

	bool check_shroud = teams_[team_num_ - 1].auto_shroud_updates();

	//we use the last registered highlighted hex
	//since it's what update our global state
	map_location hex = last_hex_;

	unit_map::iterator u = find_unit(selected_hex_);

	//if the unit is selected and then itself clicked on,
	//any goto command is cancelled
	if(u != units_.end() && !browse && selected_hex_ == hex && u->second.side() == team_num_) {
		u->second.set_goto(map_location());
	}

	unit_map::iterator clicked_u = find_unit(hex);

	const map_location src = selected_hex_;
	paths orig_paths = current_paths_;
	const map_location& attack_from = current_unit_attacks_from(hex);

	//see if we're trying to do a attack or move-and-attack
	if(!browse && !commands_disabled && attack_from.valid()) {
		if (attack_from == selected_hex_) { //no move needed
			attack_enemy(u, clicked_u);
			return false;
		}
		else if (move_unit_along_current_route(false, true)) {//move the unit without updating shroud
			// a WML event could have invalidated both attacker and defender
			// so make sure they're valid before attacking
			u = find_unit(attack_from);
			unit_map::iterator enemy = find_unit(hex);
			if(u != units_.end() && u->second.side() == team_num_ &&
				enemy != units_.end() && current_team().is_enemy(enemy->second.side()) && !enemy->second.incapacitated()
				&& !commands_disabled) {

				// reselect the unit to make the attacker's stats appear during the attack dialog
				gui().select_hex(attack_from);

				if(attack_enemy(u,enemy)) { // Fight !!
					return false;
				} else { //canceled attack, undo the move
					undo_ = true;
					selected_hex_ = src;
					gui().select_hex(src);
					current_paths_ = orig_paths;
					gui().highlight_reach(current_paths_);
					return false;
				}
			}
			else {  // the attack is not valid anymore, abort
				return false;
			}
			
		}
		else { // interrupted move
			// we assume that move_unit() did the cleaning
			// (update shroud/fog, clear undo if needed)
			return false;
		}
	}

	//otherwise we're trying to move to a hex
	else if(!commands_disabled && !browse && selected_hex_.valid() && selected_hex_ != hex &&
		     u != units_.end() && u->second.side() == team_num_ &&
		     clicked_u == units_.end() && !current_route_.steps.empty() &&
		     current_route_.steps.front() == selected_hex_) {

		gui().unhighlight_reach();
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
	show_partial_move_ = false;

	unit_map::iterator u = find_unit(hex);
	if(hex.valid() && u != units_.end() ) {
		next_unit_ = u->first;

		// if it's not the unit's turn, we reset its moves
		unit_movement_resetter move_reset(u->second, u->second.side() != team_num_);
		const bool teleport = u->second.get_ability_bool("teleport",u->first);
		current_paths_ = paths(map_,units_,hex,teams_,
						   false,teleport,viewing_team(),path_turns_);
		show_attack_options(u);
		gui().highlight_reach(current_paths_);
		// the highlight now comes from selection
		// and not from the mouseover on an enemy
		enemy_paths_ = false;
		gui().set_route(NULL);

		// selection have impact only if we are not observing and it's our unit
		if (!browse && !commands_disabled && u->second.side() == gui().viewing_team()+1) {
			sound::play_UI_sound("select-unit.wav");
			u->second.set_selecting(gui(), u->first);
			game_events::fire("select", hex);
		}

	} else {
		gui().unhighlight_reach();
		current_paths_ = paths();
		current_route_.steps.clear();
	}
}

void mouse_handler::deselect_hex() {
	select_hex(map_location(), true);
}

void mouse_handler::clear_undo_stack()
{
	apply_shroud_changes(undo_stack_,&gui(),map_,units_,teams_,team_num_-1);
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
	current_paths_ = paths();
	current_route_.steps.clear();

	attackmove_ = attackmove;
	const size_t moves = ::move_unit(&gui(),map_,units_,teams_,
	                   steps,&recorder,&undo_stack_,&next_unit_,false,check_shroud);
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

bool mouse_handler::attack_enemy(unit_map::iterator attacker, unit_map::iterator defender)
{
	try {
		return attack_enemy_(attacker, defender);
	} catch(std::bad_alloc) {
		lg::wml_error << "Memory exhausted a unit has either a lot hitpoints or a negative amount.\n";
		return false;
	}

}

bool mouse_handler::attack_enemy_(unit_map::iterator attacker, unit_map::iterator defender)
{
	//we must get locations by value instead of by references, because the iterators
	//may become invalidated later
	const map_location attacker_loc = attacker->first;
	const map_location defender_loc = defender->first;

	std::vector<std::string> items;

	std::vector<battle_context> bc_vector;
	unsigned int i, best = 0;
	for (i = 0; i < attacker->second.attacks().size(); i++) {
		// skip weapons with attack_weight=0
		if (attacker->second.attacks()[i].attack_weight() > 0) {
			battle_context bc(map_, teams_, units_, status_, attacker->first, defender->first, i);
			bc_vector.push_back(bc);
			if (bc.better_attack(bc_vector[best], 0.5)) {
				best = i;
			}
		}
	}

	for (i = 0; i < bc_vector.size(); i++) {
		const battle_context::unit_stats& att = bc_vector[i].get_attacker_stats();
		const battle_context::unit_stats& def = bc_vector[i].get_defender_stats();
		config tmp_config;
		attack_type no_weapon(tmp_config);
		const attack_type& attw = attack_type(*att.weapon);
		const attack_type& defw = attack_type(def.weapon ? *def.weapon : no_weapon);

		attw.set_specials_context(attacker->first, defender->first, attacker->second, true);
		defw.set_specials_context(attacker->first, defender->first, attacker->second, false);

		//if there is an attack special or defend special, we output a single space for the other unit, to make sure
		//that the attacks line up nicely.
		std::string special_pad = "";
		if (!attw.weapon_specials().empty() || !defw.weapon_specials().empty())
			special_pad = " ";

		std::stringstream atts;
		if (i == best) {
			atts << DEFAULT_ITEM;
		}

		std::string range = attw.range().empty() ? defw.range() : attw.range();
		if (!range.empty()) {
			range = gettext(range.c_str());
		}
		atts << IMAGE_PREFIX << attw.icon() << COLUMN_SEPARATOR
			 << font::BOLD_TEXT << attw.name() << "\n" << att.damage << "-"
			 << att.num_blows << " "  << " (" << att.chance_to_hit << "%)\n"
			 << attw.weapon_specials() << special_pad
			 << COLUMN_SEPARATOR << "<245,230,193>" << "- " << range << " -" << COLUMN_SEPARATOR
			 << font::BOLD_TEXT << defw.name() << "\n" << def.damage << "-"
			 << def.num_blows << " "  << " (" << def.chance_to_hit << "%)\n"
			 << defw.weapon_specials() << special_pad << COLUMN_SEPARATOR
			 << IMAGE_PREFIX << defw.icon();

		items.push_back(atts.str());
	}

	//make it so that when we attack an enemy, the attacking unit
	//is again shown in the status bar, so that we can easily
	//compare between the attacking and defending unit
	gui().highlight_hex(map_location());
	gui().draw(true,true);

	attack_prediction_displayer ap_displayer(gui(), bc_vector, map_, teams_, units_, status_, attacker_loc, defender_loc);
	std::vector<gui::dialog_button_info> buttons;
	buttons.push_back(gui::dialog_button_info(&ap_displayer, _("Damage Calculations")));

	int res = 0;

	{
		dialogs::units_list_preview_pane attacker_preview(gui(),&map_,attacker->second,dialogs::unit_preview_pane::SHOW_BASIC,true);
		dialogs::units_list_preview_pane defender_preview(gui(),&map_,defender->second,dialogs::unit_preview_pane::SHOW_BASIC,false);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&attacker_preview);
		preview_panes.push_back(&defender_preview);

		res = gui::show_dialog(gui(),NULL,_("Attack Enemy"),
				_("Choose weapon:")+std::string("\n"),
				gui::OK_CANCEL,&items,&preview_panes,"",NULL,-1,NULL,-1,-1,
				NULL,&buttons);
	}

	cursor::set(cursor::NORMAL);
	if(size_t(res) < bc_vector.size()) {
		const battle_context::unit_stats &att = bc_vector[res].get_attacker_stats();
		const battle_context::unit_stats &def = bc_vector[res].get_defender_stats();

		attacker->second.set_goto(map_location());
		clear_undo_stack();
		redo_stack_.clear();

		current_paths_ = paths();
		gui().clear_attack_indicator();
		gui().unhighlight_reach();

		gui().draw();

		const bool defender_human = teams_[defender->second.side()-1].is_human();

		recorder.add_attack(attacker_loc,defender_loc,att.attack_num,def.attack_num);

		//MP_COUNTDOWN grant time bonus for attacking
		current_team().set_action_bonus_count(1 + current_team().action_bonus_count());

		try {
			attack(gui(),map_,teams_,attacker_loc,defender_loc,att.attack_num,def.attack_num,units_,status_);
		} catch(end_level_exception&) {
			//if the level ends due to a unit being killed, still see if
			//either the attacker or defender should advance
			dialogs::advance_unit(map_,units_,attacker_loc,gui());
			dialogs::advance_unit(map_,units_,defender_loc,gui(),!defender_human);
			throw;
		}

		dialogs::advance_unit(map_,units_,attacker_loc,gui());
		dialogs::advance_unit(map_,units_,defender_loc,gui(),!defender_human);

		check_victory(units_, teams_, gui());

		gui().draw();

		return true;
	} else {
		return false;
	}
}

void mouse_handler::show_attack_options(unit_map::const_iterator u)
{
	team& current_team = teams_[team_num_-1];

	if(u == units_.end() || u->second.attacks_left() == 0)
		return;

	for(unit_map::const_iterator target = units_.begin(); target != units_.end(); ++target) {
		if(current_team.is_enemy(target->second.side()) &&
			distance_between(target->first,u->first) == 1 && !target->second.incapacitated()) {
			current_paths_.routes[target->first] = paths::route();
		}
	}
}

bool mouse_handler::unit_in_cycle(unit_map::const_iterator it)
{
	if (it == units_.end())
		return false;

	if(it->second.side() != team_num_ || it->second.user_end_turn()
			|| gui().fogged(it->first) || !unit_can_move(it->first,it->second,units_,map_,teams_))
		return false;

	if (current_team().is_enemy(int(gui().viewing_team()+1)) &&
			it->second.invisible(it->first,units_,teams_))
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

void mouse_handler::set_current_paths(paths new_paths) {
	gui().unhighlight_reach();
	current_paths_ = new_paths;
	current_route_.steps.clear();
	gui().set_route(NULL);
}

mouse_handler *mouse_handler::singleton_ = NULL;
}
