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

#include "composite/aspect.hpp"
#include "composite/engine.hpp"
#include "composite/goal.hpp"

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
	return actions::execute_attack_action(get_side(),true,attacker_loc,defender_loc,attacker_weapon, get_aggression());
}


attack_result_ptr readonly_context_impl::check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon){
	return actions::execute_attack_action(get_side(),false,attacker_loc,defender_loc,attacker_weapon, get_aggression());
}


move_result_ptr readwrite_context_impl::execute_move_action(const map_location& from, const map_location& to, bool remove_movement){
	return actions::execute_move_action(get_side(),true,from,to,remove_movement);
}


move_result_ptr readonly_context_impl::check_move_action(const map_location& from, const map_location& to, bool remove_movement){
	return actions::execute_move_action(get_side(),false,from,to,remove_movement);
}


recruit_result_ptr readwrite_context_impl::execute_recruit_action(const std::string& unit_name, const map_location &where){
	return actions::execute_recruit_action(get_side(),true,unit_name,where);
}


recruit_result_ptr readonly_context_impl::check_recruit_action(const std::string& unit_name, const map_location &where){
	return actions::execute_recruit_action(get_side(),false,unit_name,where);
}


stopunit_result_ptr readwrite_context_impl::execute_stopunit_action(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	return actions::execute_stopunit_action(get_side(),true,unit_location,remove_movement,remove_attacks);
}


stopunit_result_ptr readonly_context_impl::check_stopunit_action(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	return actions::execute_stopunit_action(get_side(),false,unit_location,remove_movement,remove_attacks);
}


template<typename T>
void readonly_context_impl::add_known_aspect(const std::string &name, boost::shared_ptr< typesafe_aspect <T> > &where)
{
	boost::shared_ptr< typesafe_known_aspect <T> > ka_ptr(new typesafe_known_aspect<T>(name,where,aspects_));
	known_aspects_.insert(make_pair(name,ka_ptr));
}

readonly_context_impl::readonly_context_impl(side_context &context, const config &cfg)
		: cfg_(cfg),
		engines_(),
		known_aspects_(),
		aggression_(),
		attack_depth_(),
		aspects_(),
		avoid_(),
		caution_(),
		dstsrc_(),enemy_dstsrc_(),
		enemy_possible_moves_(),
		enemy_srcdst_(),
		grouping_(),
		goals_(),
		leader_goal_(),
		leader_value_(),
		move_maps_enemy_valid_(false),
		move_maps_valid_(false),
		number_of_possible_recruits_to_force_recruit_(),
		passive_leader_(),
		passive_leader_shares_keep_(),
		possible_moves_(),
		recruitment_ignore_bad_combat_(),
		recruitment_ignore_bad_movement_(),
		recruitment_pattern_(),
		recursion_counter_(context.get_recursion_count()),
		scout_village_targeting_(),
		simple_targeting_(),
		srcdst_(),
		support_villages_(),
		village_value_(),
		villages_per_scout_()
	{
		init_side_context_proxy(context);
		manager::add_gamestate_observer(this);

		add_known_aspect("aggression",aggression_);
		add_known_aspect("attack_depth",attack_depth_);
		add_known_aspect("avoid",avoid_);
		add_known_aspect("caution",caution_);
		add_known_aspect("grouping",grouping_);
		add_known_aspect("leader_goal",leader_goal_);
		add_known_aspect("leader_value",leader_value_);
		add_known_aspect("number_of_possible_recruits_to_force_recruit",number_of_possible_recruits_to_force_recruit_);
		add_known_aspect("passive_leader",passive_leader_);
		add_known_aspect("passive_leader_shares_keep",passive_leader_shares_keep_);
		add_known_aspect("recruitment_ignore_bad_combat",recruitment_ignore_bad_combat_);
		add_known_aspect("recruitment_ignore_bad_movement",recruitment_ignore_bad_movement_);
		add_known_aspect("recruitment_pattern",recruitment_pattern_);
		add_known_aspect("scout_village_targeting",scout_village_targeting_);
		add_known_aspect("simple_targeting",simple_targeting_);
		add_known_aspect("support_villages",support_villages_);
		add_known_aspect("village_value",village_value_);
		add_known_aspect("villages_per_scout",villages_per_scout_);

	}

void readonly_context_impl::on_readonly_context_create() {
	//init the composite ai engines
	foreach(const config &cfg_element, cfg_.child_range("engine")){
		engine::parse_engine_from_config(*this,cfg_element,std::back_inserter(engines_));
	}

	// init the composite ai aspects
	foreach(const config &cfg_element, cfg_.child_range("aspect")){
		std::vector<aspect_ptr> aspects;
		engine::parse_aspect_from_config(*this,cfg_element,cfg_element["id"],std::back_inserter(aspects));
		add_aspects(aspects);
	}

	// init the composite ai goals
	foreach(const config &cfg_element, cfg_.child_range("goal")){
		engine::parse_goal_from_config(*this,cfg_element,std::back_inserter(get_goals()));
	}
}


config readonly_context_impl::to_readonly_context_config() const
{
	config cfg;
	foreach(const engine_ptr e, engines_) {
		cfg.add_child("engine",e->to_config());
	}
	foreach(const aspect_map::value_type a, aspects_) {
		cfg.add_child("aspect",a.second->to_config());
	}
	foreach(const goal_ptr g, goals_) {
		cfg.add_child("goal",g->to_config());
	}
	cfg["default_config_applied"] = "yes";
	return cfg;
}

readonly_context_impl::~readonly_context_impl()
{
	manager::remove_gamestate_observer(this);
}

void readonly_context_impl::handle_generic_event(const std::string& /*event_name*/)
{
	invalidate_move_maps();
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


	const events::command_disabler disable_commands;
	// See if we can actually recruit (i.e. have enough room etc.)
	const std::string recruit_err = find_recruit_location(get_side(), loc);
	if(recruit_err.empty()) {
		const unit new_unit(&get_info().units, &u->second, get_side(), true);
		place_recruit(new_unit, loc, false, preferences::show_ai_moves());

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
				events::chat_handler::MESSAGE_PUBLIC, false);
	}
}


map_location readwrite_context_impl::move_unit(map_location from, map_location to,
		const moves_map &possible_moves)
{
	const map_location loc = move_unit_partial(from,to,possible_moves);
	const unit_map::iterator u = get_info().units.find(loc);
	if(u != get_info().units.end()) {
		if(u->second.movement_left()==u->second.total_movement()) {
			u->second.set_movement(0);
			u->second.set_state(unit::STATE_NOT_MOVED,true);
			raise_unit_moved();
		} else if (from == loc) {
			u->second.set_movement(0);
			raise_unit_moved();
		}
	}
	return loc;
}


map_location readwrite_context_impl::move_unit_partial(map_location from, map_location to,
		const moves_map &possible_moves)
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
		return map_location();
	}

	if(from == to) {
		LOG_AI << "moving unit at " << from << " on spot. resetting moves\n";
		return to;
	}

	const bool show_move = preferences::show_ai_moves();

	const std::map<map_location,paths>::const_iterator p_it = possible_moves.find(from);

	std::vector<map_location> steps;

	if(p_it != possible_moves.end()) {
		const paths& p = p_it->second;
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
	return to;
}


void readonly_context_impl::calculate_possible_moves(std::map<map_location,paths>& res, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement,
		const terrain_filter* remove_destinations) const
{
  calculate_moves(get_info().units,res,srcdst,dstsrc,enemy,assume_full_movement,remove_destinations);
}

void readonly_context_impl::calculate_moves(const unit_map& units, std::map<map_location,paths>& res, move_map& srcdst,
		move_map& dstsrc, bool enemy, bool assume_full_movement,
		const terrain_filter* remove_destinations,
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

			if(remove_destinations != NULL && remove_destinations->match(dst)) {
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
		recorder.add_attack(u, target, weapon, def_weapon);
	}
	try {
		rand_rng::invalidate_seed();
		rand_rng::clear_new_seed_callback();
		while (!rand_rng::has_valid_seed()) {
			manager::raise_user_interact();
			manager::raise_sync_network();
			SDL_Delay(10);
		}
		recorder.add_seed("attack", rand_rng::get_last_seed());
		attack(u, target, weapon, def_weapon, get_info().units);
	}
	catch (end_level_exception&)
	{
		dialogs::advance_unit(u, true);

		const unit_map::const_iterator defender = get_info().units.find(target);
		if(defender != get_info().units.end()) {
			const size_t defender_team = size_t(defender->second.side()) - 1;
			if(defender_team < get_info().teams.size()) {
				dialogs::advance_unit(target, !get_info().teams[defender_team].is_human());
			}
		}

		throw;
	}
	dialogs::advance_unit(u, true);

	const unit_map::const_iterator defender = get_info().units.find(target);
	if(defender != get_info().units.end()) {
		const size_t defender_team = size_t(defender->second.side()) - 1;
		if(defender_team < get_info().teams.size()) {
			dialogs::advance_unit(target, !get_info().teams[defender_team].is_human());
		}
	}

	check_victory();
	raise_enemy_attacked();

}


void readonly_context_impl::add_aspects(std::vector< aspect_ptr > &aspects )
{
	foreach (aspect_ptr a, aspects) {
		const std::string name = a->get_id();
		known_aspect_map::iterator i = known_aspects_.find(name);
		if (i != known_aspects_.end()) {
			i->second->set(a);
		} else {
			ERR_AI << "when adding aspects, unknown aspect id["<<name<<"]"<<std::endl;
		}
	}
}


const terrain_filter& readonly_context_impl::get_avoid() const
{
	if (avoid_) {
		return avoid_->get();
	}
	config cfg;
	cfg.add_child("not");
	static terrain_filter tf(vconfig(cfg),get_info().units);
	return tf;
}


double readonly_context_impl::get_aggression() const
{
	if (aggression_) {
		return aggression_->get();
	}
	return 0;
}


int readonly_context_impl::get_attack_depth() const
{
	if (attack_depth_) {
		return std::max<int>(1,attack_depth_->get()); //@todo 1.7: add minmax filter to attack_depth aspect
	}
	return 1;
}


const aspect_map& readonly_context_impl::get_aspects() const
{
	return aspects_;
}


double readonly_context_impl::get_caution() const
{
	if (caution_) {
		return caution_->get();
	}
	return 0;
}

const move_map& readonly_context_impl::get_dstsrc() const
{
	if (!move_maps_valid_) {
		recalculate_move_maps();
	}
	return dstsrc_;
}


const move_map& readonly_context_impl::get_enemy_dstsrc() const
{
	if (!move_maps_enemy_valid_) {
		recalculate_move_maps_enemy();
	}
	return enemy_dstsrc_;
}


const moves_map& readonly_context_impl::get_enemy_possible_moves() const
{
	if (!move_maps_enemy_valid_) {
		recalculate_move_maps_enemy();
	}
	return enemy_possible_moves_;
}


const move_map& readonly_context_impl::get_enemy_srcdst() const
{
	if (!move_maps_enemy_valid_) {
		recalculate_move_maps_enemy();
	}
	return enemy_srcdst_;
}


engine_ptr readonly_context_impl::get_engine(const config& cfg)
{
	const std::string& engine_name = cfg["engine"];
	std::vector<engine_ptr>::iterator en = engines_.begin();
	while (en!=engines_.end() && ((*en)->get_name()!=engine_name)) {
		en++;
	}

	if (en != engines_.end()){
		return *en;
	}

	engine_factory::factory_map::iterator eng = engine_factory::get_list().find(engine_name);
	if (eng == engine_factory::get_list().end()){
		ERR_AI << "side "<<get_side()<<" : UNABLE TO FIND engine["<<
			engine_name <<"]" << std::endl;
		DBG_AI << "config snippet contains: " << std::endl << cfg << std::endl;
		return engine_ptr();
	}

	engine_ptr new_engine = eng->second->get_new_instance(*this,engine_name);
	if (!new_engine) {
		ERR_AI << "side "<<get_side()<<" : UNABLE TO CREATE engine["<<
			engine_name <<"] " << std::endl;
		DBG_AI << "config snippet contains: " << std::endl << cfg << std::endl;
		return engine_ptr();
	}
	engines_.push_back(new_engine);
	return engines_.back();
}

std::string readonly_context_impl::get_grouping() const
{
	if (grouping_) {
		return grouping_->get();
	}
	return std::string();
}


const std::vector<goal_ptr>& readonly_context_impl::get_goals() const
{
	return goals_;
}


std::vector<goal_ptr>& readonly_context_impl::get_goals()
{
	return goals_;
}




config readonly_context_impl::get_leader_goal() const
{
	if (leader_goal_) {
		return leader_goal_->get();
	}
	return config();
}


double readonly_context_impl::get_leader_value() const
{
	if (leader_value_) {
		return leader_value_->get();
	}
	return 0;
}


double readonly_context_impl::get_number_of_possible_recruits_to_force_recruit() const
{
	if (number_of_possible_recruits_to_force_recruit_) {
		return number_of_possible_recruits_to_force_recruit_->get();
	}
	return 0;
}


bool readonly_context_impl::get_passive_leader() const
{
	if (passive_leader_) {
		return passive_leader_->get();
	}
	return false;
}


bool readonly_context_impl::get_passive_leader_shares_keep() const
{
	if (passive_leader_shares_keep_) {
		return passive_leader_shares_keep_->get();
	}
	return false;
}


const moves_map& readonly_context_impl::get_possible_moves() const
{
	if (!move_maps_valid_) {
		recalculate_move_maps();
	}
	return possible_moves_;
}


bool readonly_context_impl::get_recruitment_ignore_bad_combat() const
{
	if (recruitment_ignore_bad_combat_) {
		return recruitment_ignore_bad_combat_->get();
	}
	return false;
}


bool readonly_context_impl::get_recruitment_ignore_bad_movement() const
{
	if (recruitment_ignore_bad_movement_) {
		return recruitment_ignore_bad_movement_->get();
	}
	return false;
}


const std::vector<std::string> readonly_context_impl::get_recruitment_pattern() const
{
	if (recruitment_pattern_) {
		return recruitment_pattern_->get();
	}
	return std::vector<std::string>();
}


double readonly_context_impl::get_scout_village_targeting() const
{
	if (scout_village_targeting_) {
		return scout_village_targeting_->get();
	}
	return 1;
}


bool readonly_context_impl::get_simple_targeting() const
{
	if (simple_targeting_) {
		return simple_targeting_->get();
	}
	return false;
}


const move_map& readonly_context_impl::get_srcdst() const
{
	if (!move_maps_valid_) {
		recalculate_move_maps();
	}
	return srcdst_;
}


bool readonly_context_impl::get_support_villages() const
{
	if (support_villages_) {
		return support_villages_->get();
	}
	return false;
}


double readonly_context_impl::get_village_value() const
{
	if (village_value_) {
		return village_value_->get();
	}
	return 0;
}


int readonly_context_impl::get_villages_per_scout() const
{
	if (villages_per_scout_) {
		return villages_per_scout_->get();
	}
	return 0;
}


void readonly_context_impl::invalidate_move_maps() const
{
	move_maps_valid_ = false;
	move_maps_enemy_valid_ = false;
}


void readonly_context_impl::recalculate_move_maps() const
{
	dstsrc_ = move_map();
	possible_moves_ = moves_map();
	srcdst_ = move_map();
	calculate_possible_moves(possible_moves_,srcdst_,dstsrc_,false,false,&get_avoid());
	move_maps_valid_ = true;
}


void readonly_context_impl::recalculate_move_maps_enemy() const
{
	enemy_dstsrc_ = move_map();
	enemy_srcdst_ = move_map();
	enemy_possible_moves_ = moves_map();
	calculate_possible_moves(enemy_possible_moves_,enemy_srcdst_,enemy_dstsrc_,true);
	move_maps_enemy_valid_ = true;
}

} //of namespace ai
