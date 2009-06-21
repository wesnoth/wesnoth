/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Helper functions for the object which operates in the context of AI for specific side
 * This is part of AI interface
 * @file ai/contexts.cpp
 */

#include "contexts.hpp"
#include "manager.hpp"
#include "../callable_objects.hpp"
#include "../dialogs.hpp"
#include "../formula.hpp"
#include "../formula_callable.hpp"
#include "../formula_function.hpp"
#include "../formula_fwd.hpp"
#include "../game_end_exceptions.hpp"
#include "../game_events.hpp"
#include "../game_preferences.hpp"
#include "../log.hpp"
#include "../mouse_handler_base.hpp"
#include "../replay.hpp"
#include "../statistics.hpp"
#include "../unit_display.hpp"

static lg::log_domain log_ai("ai/general");
#define DBG_AI LOG_STREAM(debug, log_ai)
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

// =======================================================================
//
// =======================================================================
namespace ai {

int side_context_impl::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


int readonly_context_impl::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


int readwrite_context_impl::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


void readonly_context_impl::raise_user_interact() const
{
	manager::raise_user_interact();
}


void readwrite_context_impl::raise_unit_recruited() const
{
	manager::raise_unit_recruited();
}


void readwrite_context_impl::raise_unit_moved() const
{
	manager::raise_unit_moved();
}


void readwrite_context_impl::raise_enemy_attacked() const
{
	manager::raise_enemy_attacked();
}


attack_result_ptr readwrite_context_impl::execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon){
	attack_result_ptr r = actions::execute_attack_action(get_side(),true,attacker_loc,defender_loc,attacker_weapon);
	recalculate_move_maps();//@todo 1.7 replace with event system
	return r;

}


attack_result_ptr readonly_context_impl::check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon){
	return actions::execute_attack_action(get_side(),false,attacker_loc,defender_loc,attacker_weapon);
}


move_result_ptr readwrite_context_impl::execute_move_action(const map_location& from, const map_location& to, bool remove_movement){
	move_result_ptr r = actions::execute_move_action(get_side(),true,from,to,remove_movement);
	recalculate_move_maps();//@todo 1.7 replace with event system
	return r;
}


move_result_ptr readonly_context_impl::check_move_action(const map_location& from, const map_location& to, bool remove_movement){
	return actions::execute_move_action(get_side(),false,from,to,remove_movement);
}


recruit_result_ptr readwrite_context_impl::execute_recruit_action(const std::string& unit_name, const map_location &where){
	recruit_result_ptr r = actions::execute_recruit_action(get_side(),true,unit_name,where);
	recalculate_move_maps();//@todo 1.7 replace with event system
	return r;
}


recruit_result_ptr readonly_context_impl::check_recruit_action(const std::string& unit_name, const map_location &where){
	return actions::execute_recruit_action(get_side(),false,unit_name,where);
}


stopunit_result_ptr readwrite_context_impl::execute_stopunit_action(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	stopunit_result_ptr r = actions::execute_stopunit_action(get_side(),true,unit_location,remove_movement,remove_attacks);
	recalculate_move_maps();//@todo 1.7 replace with event system
	return r;
}


stopunit_result_ptr readonly_context_impl::check_stopunit_action(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	return actions::execute_stopunit_action(get_side(),false,unit_location,remove_movement,remove_attacks);
}


bool readwrite_context_impl::recruit(const std::string& unit_name, map_location loc)
{
	const std::set<std::string>& recruits = current_team().recruits();

	const std::set<std::string>::const_iterator i = recruits.find(unit_name);
	if(i == recruits.end()) {
		return false;
	}

	const int num = std::distance(recruits.begin(),i);

	// We have to add the recruit command now, because when the unit
	// is created it has to have the recruit command in the recorder
	// to be able to put random numbers into to generate unit traits.
	// However, we're not sure if the transaction will be successful,
	// so use a replay_undo object to cancel it if we don't get
	// a confirmation for the transaction.
	recorder.add_recruit(num,loc);
	replay_undo replay_guard(recorder);

	unit_type_data::unit_type_map::const_iterator u = unit_type_data::types().find_unit_type(unit_name);
	if(u == unit_type_data::types().end() || u->first == "dummy_unit") {
		return false;
	}

	// Check if we have enough money
	if(current_team().gold() < u->second.cost()) {
		return false;
	}
	LOG_AI << "trying recruit: team=" << (get_side()) <<
	    " type=" << unit_name <<
	    " cost=" << u->second.cost() <<
	    " loc=(" << loc << ')' <<
	    " gold=" << (current_team().gold()) <<
	    " (-> " << (current_team().gold()-u->second.cost()) << ")\n";

	unit new_unit(&get_info().units,&get_info().map,&get_info().state,&get_info().tod_manager_,&get_info().teams,&u->second,get_side(),true);

	// See if we can actually recruit (i.e. have enough room etc.)
	std::string recruit_err = recruit_unit(get_info().map,get_side(),get_info().units,new_unit,loc,false,preferences::show_ai_moves());
	if(recruit_err.empty()) {

		statistics::recruit_unit(new_unit);
		current_team_w().spend_gold(u->second.cost());

		// Confirm the transaction - i.e. don't undo recruitment
		replay_guard.confirm_transaction();

		raise_unit_recruited();
		const team_data data = calculate_team_data(current_team(),get_side(),get_info().units);
		LOG_AI <<
		"recruit confirmed: team=" << get_side() <<
		" units=" << data.units <<
		" gold=" << data.gold <<
		((data.net_income < 0) ? "" : "+") <<
		data.net_income << "\n";
		recorder.add_checksum_check(loc);
		recalculate_move_maps();
		return true;
	} else {
		const team_data data = calculate_team_data(current_team(),get_side(),get_info().units);
		LOG_AI << recruit_err << "\n";
		LOG_AI <<
		"recruit UNconfirmed: team=" << (get_side()) <<
		" units=" << data.units <<
		" gold=" << data.gold <<
		((data.net_income < 0) ? "" : "+") <<
		data.net_income << "\n";
		recalculate_move_maps();
		return false;
	}
}


const game_info& readonly_context_impl::get_info() const{
	return manager::get_active_ai_info_for_side(get_side());
}


game_info& readwrite_context_impl::get_info_w(){
	return manager::get_active_ai_info_for_side(get_side());
}

void readonly_context_impl::diagnostic(const std::string& msg)
{
	if(game_config::debug) {
		get_info().disp.set_diagnostic(msg);
	}
}


void readonly_context_impl::log_message(const std::string& msg)
{
	if(game_config::debug) {
		get_info().disp.add_chat_message(time(NULL), "ai", get_side(), msg,
				game_display::MESSAGE_PUBLIC, false);
	}
}


map_location readwrite_context_impl::move_unit(map_location from, map_location to,
		std::map<map_location,paths>& possible_moves)
{
	const map_location loc = move_unit_partial(from,to,possible_moves);
	const unit_map::iterator u = get_info().units.find(loc);
	if(u != get_info().units.end()) {
		if(u->second.movement_left()==u->second.total_movement()) {
			u->second.set_movement(0);
			u->second.set_state(unit::STATE_NOT_MOVED,true);
		} else if (from == loc) {
			u->second.set_movement(0);
		}
	}
	recalculate_move_maps();
	return loc;
}


map_location readwrite_context_impl::move_unit_partial(map_location from, map_location to,
		std::map<map_location,paths>& possible_moves)
{
	LOG_AI << "readwrite_context_impl::move_unit " << from << " -> " << to << '\n';
	assert(to.valid() && to.x <= MAX_MAP_AREA && to.y <= MAX_MAP_AREA);
	// Stop the user from issuing any commands while the unit is moving.
	const events::command_disabler disable_commands;

	log_scope2(log_ai, "move_unit");
	unit_map::iterator u_it = get_info().units.find(from);
	if(u_it == get_info().units.end()) {
		ERR_AI << "Could not find unit at " << from << '\n';
		assert(false);
		recalculate_move_maps();
		return map_location();
	}

	if(from == to) {
		LOG_AI << "moving unit at " << from << " on spot. resetting moves\n";
		recalculate_move_maps();
		return to;
	}

	const bool show_move = preferences::show_ai_moves();

	const std::map<map_location,paths>::iterator p_it = possible_moves.find(from);

	std::vector<map_location> steps;

	if(p_it != possible_moves.end()) {
		paths& p = p_it->second;
		paths::dest_vect::const_iterator rt = p.destinations.find(to);
		if (rt != p.destinations.end())
		{
			u_it->second.set_movement(rt->move_left);

			while (rt != p.destinations.end() &&
			       get_info().units.find(to) != get_info().units.end() && from != to)
			{
				LOG_AI << "AI attempting illegal move. Attempting to move onto existing unit\n";
				LOG_AI << "\t" << get_info().units.find(to)->second.underlying_id() <<" already on " << to << "\n";
				LOG_AI <<"\tremoving last step\n";
				to = rt->prev;
				rt = p.destinations.find(to);
				LOG_AI << "\tresetting to " << from << " -> " << to << '\n';
			}

			if (rt != p.destinations.end()) // First step is starting hex
			{
				steps = p.destinations.get_path(rt);
				unit_map::const_iterator utest=get_info().units.find(*(steps.begin()));
				if(utest != get_info().units.end() && current_team().is_enemy(utest->second.side())){
					ERR_AI << "AI tried to move onto existing enemy unit at" << *steps.begin() << '\n';
					//			    return(from);
				}

				// Check if there are any invisible units that we uncover
				for(std::vector<map_location>::iterator i = steps.begin()+1; i != steps.end(); ++i) {
					map_location adj[6];
					get_adjacent_tiles(*i,adj);

					size_t n;
					for(n = 0; n != 6; ++n) {

						// See if there is an enemy unit next to this tile.
						// If it's invisible, we need to stop: we're ambushed.
						// If it's not, we must be a skirmisher, otherwise AI wouldn't try.

						// Or would it?  If it doesn't cheat, it might...
						const unit_map::const_iterator u = get_info().units.find(adj[n]);
						// If level 0 is invisible it ambush us too
						if (u != get_info().units.end() && (u->second.emits_zoc() || u->second.invisible(adj[n], get_info().units, get_info().teams))
								&& current_team().is_enemy(u->second.side())) {
							if (u->second.invisible(adj[n], get_info().units, get_info().teams)) {
								to = *i;
								u->second.ambush();
								steps.erase(i,steps.end());
								break;
							} else {
								if (!u_it->second.get_ability_bool("skirmisher",*i)){
									ERR_AI << "AI tried to skirmish with non-skirmisher\n";
									LOG_AI << "\tresetting destination from " <<to;
									to = *i;
									LOG_AI << " to " << to;
									steps.erase(i,steps.end());
									while(steps.empty() == false && (!(get_info().units.find(to) == get_info().units.end() || from == to))){
										to = *(steps.end()-1);
										steps.pop_back();
										LOG_AI << "\tresetting to " << from << " -> " << to << '\n';
									}

									break;
								}
							}
						}
					}

					if(n != 6) {
						u_it->second.set_movement(0); // Enter enemy ZoC, no movement left
						break;
					}
				}
			}

			if(steps.empty() || steps.back() != to) {
				//Add the destination to the end of the steps if it's not
				//already there.
				steps.push_back(to);
			}

			if(show_move && unit_display::unit_visible_on_path(steps,
						u_it->second, get_info().units,get_info().teams)) {
				get_info().disp.display_unit_hex(from);

				unit_map::iterator up = get_info().units.find(u_it->first);
				unit_display::move_unit(steps,up->second,get_info().teams);
			} else if(steps.size()>1) {
				unit_map::iterator up = get_info().units.find(u_it->first);
				std::vector<map_location>::const_reverse_iterator last_step = steps.rbegin();
				std::vector<map_location>::const_reverse_iterator before_last = last_step +1;
				up->second.set_facing(before_last->get_relative_dir(*last_step));
			}
		}
	}
	//FIXME: probably missing some "else" here
	// It looks like if the AI doesn't find a route in possible_move,
	// she will just teleport her unit between 'from' and 'to'
	// I suppose this never happen, but in the meantime, add code for replay
	if (steps.empty()) {
		steps.push_back(from);
		steps.push_back(to);
	}

	std::pair<map_location,unit> *p = get_info().units.extract(u_it->first);

	p->first = to;
	get_info().units.insert(p);
	p->second.set_standing();
	if(get_info().map.is_village(to)) {
		// If a new village is captured, disallow any future movement.
		if (!current_team().owns_village(to))
			get_info().units.find(to)->second.set_movement(-1);
		get_village(to,get_info().disp,get_info().teams,get_side()-1,get_info().units);
	}

	if(show_move) {
		get_info().disp.invalidate(to);
		get_info().disp.draw();
	}

	recorder.add_movement(steps);

	game_events::fire("moveto",to,from);

	if((get_info().teams.front().uses_fog() || get_info().teams.front().uses_shroud()) &&
			!get_info().teams.front().fogged(to)) {
		game_events::fire("sighted",to);
	}

	// would have to go via mousehandler to make this work:
	//get_info().disp.unhighlight_reach();
	raise_unit_moved();
	recalculate_move_maps();
	return to;
}


void readonly_context_impl::calculate_possible_moves(std::map<map_location,paths>& res, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement,
		const std::set<map_location>* remove_destinations) const
{
  calculate_moves(get_info().units,res,srcdst,dstsrc,enemy,assume_full_movement,remove_destinations);
}

void readonly_context_impl::calculate_moves(const unit_map& units, std::map<map_location,paths>& res, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement,
	     const std::set<map_location>* remove_destinations,
		bool see_all
          ) const
{

	for(unit_map::const_iterator un_it = units.begin(); un_it != units.end(); ++un_it) {
		// If we are looking for the movement of enemies, then this unit must be an enemy unit.
		// If we are looking for movement of our own units, it must be on our side.
		// If we are assuming full movement, then it may be a unit on our side, or allied.
		if((enemy && current_team().is_enemy(un_it->second.side()) == false) ||
		   (!enemy && !assume_full_movement && un_it->second.side() != get_side()) ||
		   (!enemy && assume_full_movement && current_team().is_enemy(un_it->second.side()))) {
			continue;
		}
		// Discount incapacitated units
		if(un_it->second.incapacitated()
			|| (!assume_full_movement && un_it->second.movement_left() == 0)) {
			continue;
		}

		// We can't see where invisible enemy units might move.
		if(enemy && un_it->second.invisible(un_it->first,units,get_info().teams) && !see_all) {
			continue;
		}
		// If it's an enemy unit, reset its moves while we do the calculations.
		unit* held_unit = const_cast<unit*>(&(un_it->second));
		const unit_movement_resetter move_resetter(*held_unit,enemy || assume_full_movement);

		// Insert the trivial moves of staying on the same map location.
		if(un_it->second.movement_left() > 0 ) {
			std::pair<map_location,map_location> trivial_mv(un_it->first,un_it->first);
			srcdst.insert(trivial_mv);
			dstsrc.insert(trivial_mv);
		}
		bool teleports = un_it->second.get_ability_bool("teleport");
		res.insert(std::pair<map_location,paths>(
		                un_it->first,paths(get_info().map,units,
					 un_it->first,get_info().teams,false,teleports,
									current_team(),0,see_all)));
	}

	for(std::map<map_location,paths>::iterator m = res.begin(); m != res.end(); ++m) {
		foreach (const paths::step &dest, m->second.destinations)
		{
			const map_location& src = m->first;
			const map_location& dst = dest.curr;

			if(remove_destinations != NULL && remove_destinations->count(dst) != 0) {
				continue;
			}

			bool friend_owns = false;

			// Don't take friendly villages
			if(!enemy && get_info().map.is_village(dst)) {
				for(size_t n = 0; n != get_info().teams.size(); ++n) {
					if(get_info().teams[n].owns_village(dst)) {
						int side = n + 1;
						if (get_side() != side && !current_team().is_enemy(side)) {
							friend_owns = true;
						}

						break;
					}
				}
			}

			if(friend_owns) {
				continue;
			}

			if(src != dst && units.find(dst) == units.end()) {
				srcdst.insert(std::pair<map_location,map_location>(src,dst));
				dstsrc.insert(std::pair<map_location,map_location>(dst,src));
			}
		}
	}
}


void readwrite_context_impl::attack_enemy(const map_location u,
		const map_location target, int weapon, int def_weapon)
{
	// Stop the user from issuing any commands while the unit is attacking
	const events::command_disabler disable_commands;

	if(!get_info().units.count(u))
	{
		ERR_AI << "attempt to attack without attacker\n";
		return;
	}
	if (!get_info().units.count(target))
	{
		ERR_AI << "attempt to attack without defender\n";
		return;
	}

	if(get_info().units.find(target)->second.incapacitated()) {
		ERR_AI << "attempt to attack unit that is petrified\n";
		return;
	}
	if(!get_info().units.find(u)->second.attacks_left()) {
		ERR_AI << "attempt to attack twice with the same unit\n";
		return;
	}

	if(weapon >= 0) {
		recorder.add_attack(u,target,weapon,def_weapon);
	}
	try {
		attack(get_info().disp, get_info().map, get_info().teams, u, target, weapon, def_weapon,
				get_info().units, get_info().state, get_info().tod_manager_);
	}
	catch (end_level_exception&)
	{
		dialogs::advance_unit(get_info().map,get_info().units,u,get_info().disp,true);

		const unit_map::const_iterator defender = get_info().units.find(target);
		if(defender != get_info().units.end()) {
			const size_t defender_team = size_t(defender->second.side()) - 1;
			if(defender_team < get_info().teams.size()) {
				dialogs::advance_unit(get_info().map, get_info().units,
						target, get_info().disp, !get_info().teams[defender_team].is_human());
			}
		}

		throw;
	}
	dialogs::advance_unit(get_info().map,get_info().units,u,get_info().disp,true);

	const unit_map::const_iterator defender = get_info().units.find(target);
	if(defender != get_info().units.end()) {
		const size_t defender_team = size_t(defender->second.side()) - 1;
		if(defender_team < get_info().teams.size()) {
			dialogs::advance_unit(get_info().map, get_info().units,
					target, get_info().disp, !get_info().teams[defender_team].is_human());
		}
	}

	check_victory(get_info().state,get_info().units,get_info().teams, get_info().disp);
	raise_enemy_attacked();
	recalculate_move_maps();

}

const std::set<map_location>& readonly_context_impl::avoided_locations()
{
	if(avoided_locations_.empty()) {
		foreach (const config &av, current_team().ai_parameters().child_range("avoid"))
		{
			foreach (const map_location &loc, parse_location_range(av["x"], av["y"])) {
				avoided_locations_.insert(loc);
			}
		}

		if(avoided_locations_.empty()) {
			avoided_locations_.insert(map_location());
		}
	}

	return avoided_locations_;
}

const move_map& readonly_context_impl::get_dstsrc() const
{
	return dstsrc_;
}


const move_map& readonly_context_impl::get_enemy_dstsrc() const
{
	return enemy_dstsrc_;
}


const moves_map& readonly_context_impl::get_enemy_possible_moves() const
{
	return enemy_possible_moves_;
}


const move_map& readonly_context_impl::get_enemy_srcdst() const
{
	return enemy_srcdst_;
}


const moves_map& readonly_context_impl::get_possible_moves() const
{
	return possible_moves_;
}


const move_map& readonly_context_impl::get_srcdst() const
{
	return srcdst_;
}


void readonly_context_impl::invalidate_avoided_locations_cache(){
	avoided_locations_.clear();
}


void readonly_context_impl::recalculate_move_maps()
{
	dstsrc_ = move_map();
	enemy_dstsrc_ = move_map();
	enemy_srcdst_ = move_map();
	enemy_possible_moves_ = moves_map();
	possible_moves_ = moves_map();
	srcdst_ = move_map();

	calculate_possible_moves(possible_moves_,srcdst_,dstsrc_,false,false,&avoided_locations());
	calculate_possible_moves(enemy_possible_moves_,enemy_srcdst_,enemy_dstsrc_,true);

}

} //of namespace ai
